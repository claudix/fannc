/**
 * fannc
 * Copyright (c) 2014, Claudi Martínez <claudix.kernel@gmail.com>, All rights reserved.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * 
 */

#include <argtable2.h>
#include <assert.h>
#include <stdlib.h>
#include <fann.h>

#define CMD_HEADER(pname,...)\
    const char *PROGNAME = pname; int NERRORS, EXITCODE = 0;\
    static const char *DESCRLINES[] = {__VA_ARGS__, NULL}

#define CMD_ABORT  do{EXITCODE = 1; goto EXIT;} while(0)
#define CMD_ERR(lbl) do{EXITCODE = 1; goto lbl;} while(0)

#define CMD_FOOTER EXIT:\
    arg_freetable(ARGTABLE,sizeof(ARGTABLE)/sizeof(ARGTABLE[0]));\
    return EXITCODE

#define CMD_PARSE(...)\
    struct arg_lit *aHelp = arg_lit0(NULL, "help", "print this help and exit");\
    struct arg_end *aEnd  = arg_end(20);\
    void* ARGTABLE[] = { __VA_ARGS__ , aHelp, aEnd};\
    do {\
        if (arg_nullcheck(ARGTABLE) != 0) {\
            fprintf(stderr, "%s: insufficient memory\n", PROGNAME);\
            EXITCODE = 1;\
            goto EXIT;\
        }\
        NERRORS = arg_parse(argc, argv, ARGTABLE);\
        if (aHelp->count > 0) {\
            printf("Usage: %s", PROGNAME);\
            arg_print_syntax(stdout, ARGTABLE, "\n");\
            const char **descrLine = DESCRLINES;\
            while(*descrLine != NULL) {\
                printf("%s\n", *descrLine);\
                descrLine++;\
            }\
            arg_print_glossary(stdout, ARGTABLE, "  %-25s %s\n");\
            EXITCODE = 0;\
            goto EXIT;\
        }\
        if (NERRORS > 0) {\
            arg_print_errors(stdout, aEnd, PROGNAME);\
            printf("Try '%s --help' for more information.\n", PROGNAME);\
            EXITCODE = 1;\
            goto EXIT;\
        }\
    } while(0)

#define DECL_WEIGHTS_ARGS\
    struct arg_dbl *aMinRandomW = arg_dbl0(NULL, "min-random-weight", "float",  "minimum random value for initializing weights. If omitted, -0.1 is taken.");\
    struct arg_dbl *aMaxRandomW = arg_dbl0(NULL, "max-random-weight", "float",  "maximum random value for initializing weights. If omitted, 0.1 is taken.");\
    struct arg_lit *aInitW      = arg_lit0(NULL, "init-weights",                "if specified, initialize weights using Widrow + Nguyen’s algorithm from training data passed from STDIN")

#define WEIGHTS_ARGS aMinRandomW, aMaxRandomW, aInitW

#define WEIGHTS_INIT\
    if (aInitW->count > 0) {\
        struct fann_train_data *training_data = fann_read_train_from_fd(stdin, "STDIN");\
        assert(training_data != NULL);\
        fann_init_weights(ann, training_data);\
        fann_destroy_train(training_data);\
    } else {\
        fann_type minw = (fann_type) -0.1, maxw = (fann_type) 0.1;\
        if (aMinRandomW->count > 0) minw = (fann_type) aMinRandomW->dval[0];\
        if (aMaxRandomW->count > 0) maxw = (fann_type) aMaxRandomW->dval[0];\
        fann_randomize_weights(ann, minw, maxw);\
    }

/**
 * Allocate and zero memory, as well as control if memory could be allocated.
 * @param sz Memory size
 * @return Pointer to memory
 */
static void *xmalloc(size_t sz)
{
    void *ptr = calloc(1, sz);
    if (ptr == NULL) {
        fprintf(stderr, "Out of memory!\n");
    }
    return ptr;
}

/** Free memory */
#define xfree free

/**
 * Dump ANN to stdout
 * @param ann ANN
 */
static void dump_ann(struct fann *ann) {
    fann_save_internal_fd(ann, stdout, "STDOUT", 0);
}

/**
 * Decode activation function from name
 * @param name Name
 * @return Activation function or -1 if undefined
 */
static enum fann_activationfunc_enum decode_activation_func(const char *name)
{    
    int i;
    for (i = 0; i < sizeof(FANN_ACTIVATIONFUNC_NAMES) / sizeof(FANN_ACTIVATIONFUNC_NAMES[0]); i++) {
        if (strcmp(FANN_ACTIVATIONFUNC_NAMES[i], name) == 0) {            
            return (enum fann_activationfunc_enum) i;
        }
    }
    return (enum fann_activationfunc_enum) -1;
}

/**
 * Decode training algorithm from name
 * @param name Name
 * @return Training algorithm or -1 if undefined
 */
static enum fann_train_enum decode_training_algorithm(const char *name)
{    
    int i;
    for (i = 0; i < sizeof(FANN_TRAIN_NAMES) / sizeof(FANN_TRAIN_NAMES[0]); i++) {
        if (strcmp(FANN_TRAIN_NAMES[i], name) == 0) {            
            return (enum fann_train_enum) i;
        }
    }
    return (enum fann_train_enum) -1;
}

/**
 * Decode error function from name
 * @param name Name
 * @return Error function or -1 if undefined
 */
static enum fann_errorfunc_enum decode_error_function(const char *name)
{
    int i;
    for (i = 0; i < sizeof(FANN_ERRORFUNC_NAMES) / sizeof(FANN_ERRORFUNC_NAMES[0]); i++) {
        if (strcmp(FANN_ERRORFUNC_NAMES[i], name) == 0) {            
            return (enum fann_errorfunc_enum) i;
        }
    }
    return (enum fann_errorfunc_enum) -1;
}

/**
 * Decode stop function from name
 * @param name Name
 * @return Stop function or -1 if undefined
 */
static enum fann_stopfunc_enum decode_stop_function(const char *name)
{
    int i;
    for (i = 0; i < sizeof(FANN_STOPFUNC_NAMES) / sizeof(FANN_STOPFUNC_NAMES[0]); i++) {
        if (strcmp(FANN_STOPFUNC_NAMES[i], name) == 0) {            
            return (enum fann_stopfunc_enum) i;
        }
    }
    return (enum fann_stopfunc_enum) -1;
}

/** Create standard network */
static int cmd_create_std(int argc, char **argv)
{
    CMD_HEADER(
            "create_std",            
            "Create a standard fully connected backpropagation neural network.",
            "There will be a bias neuron in each layer (except the output layer),",
            "and this bias neuron will be connected to all neurons in the next layer.",
            "When running the network, the bias nodes always emits 1."            
            );
    
    DECL_WEIGHTS_ARGS;
    struct arg_int *aLayers     = arg_intn(NULL, NULL, "n", 2, argc+3,          "integer values determining the number of neurons in each layer starting with the input layer and ending with the output layer.");
    
    
    CMD_PARSE(WEIGHTS_ARGS, aLayers);    
    
    unsigned int i;
    unsigned int nLayers = aLayers->count;
    unsigned int *layers = (unsigned int *) xmalloc(sizeof(unsigned int) * nLayers);
    
    for (i = 0; i < nLayers; i++) {
        layers[i] = (unsigned int) aLayers->ival[i];
    }
    
    struct fann *ann = fann_create_standard_array(nLayers, layers);
    assert(ann != NULL);
            
    WEIGHTS_INIT;
    
    dump_ann(ann);
    
    xfree(layers);
    
    fann_destroy(ann);
    
    CMD_FOOTER;
}

/** Create sparse network */
static int cmd_create_sparse(int argc, char **argv)
{
    CMD_HEADER(
            "create_sparse",            
            "Creates a standard backpropagation neural network, which is not fully connected.",
            "There will be a bias neuron in each layer (except the output layer),",
            "and this bias neuron will be connected to all neurons in the next layer.",
            "When running the network, the bias nodes always emits 1."            
            );
    
    DECL_WEIGHTS_ARGS;
    struct arg_int *aRate   = arg_int1(NULL, "rate", "int",        "connection rate expressed as a percent. It controls how many connections there will be in the network. If the connection rate is set to 100, the network will be fully connected, but if it is set to 50 only half of the connections will be set. A connection rate of 100 will yield the same result as create_std");
    struct arg_int *aLayers = arg_intn(NULL, NULL, "n", 2, argc+3, "integer values determining the number of neurons in each layer starting with the input layer and ending with the output layer.");
    
    
    CMD_PARSE(WEIGHTS_ARGS, aRate, aLayers);    
    
    unsigned int i;
    unsigned int nLayers = aLayers->count;
    unsigned int *layers = (unsigned int *) xmalloc(sizeof(unsigned int) * nLayers);
    
    for (i = 0; i < nLayers; i++) {
        layers[i] = (unsigned int) aLayers->ival[i];
    }
    
    struct fann *ann = fann_create_sparse_array((float)aRate->ival[0] / 100.0, nLayers, layers);
    assert(ann != NULL);
    
    WEIGHTS_INIT;
    
    dump_ann(ann);
    
    xfree(layers);
    
    fann_destroy(ann);
    
    CMD_FOOTER;
}

/** Create shortcut network */
static int cmd_create_shortcut(int argc, char **argv)
{
    CMD_HEADER(
            "create_shortcut",            
            "Creates a standard backpropagation neural network, which is not fully connected and which also has shortcut connections.",
            "Shortcut connections are connections that skip layers. A fully connected network with shortcut connections is a "
            "network where all neurons are connected to all neurons in later layers, including direct connections from the input layer "
            "to the output layer.",
            "There will be a bias neuron in each layer (except the output layer),",
            "and this bias neuron will be connected to all neurons in the next layer.",
            "When running the network, the bias nodes always emits 1."            
            );
    
    DECL_WEIGHTS_ARGS;
    struct arg_int *aLayers = arg_intn(NULL, NULL, "n", 2, argc+3, "integer values determining the number of neurons in each layer starting with the input layer and ending with the output layer.");
    
    
    CMD_PARSE(WEIGHTS_ARGS, aLayers);    
    
    unsigned int i;
    unsigned int nLayers = aLayers->count;
    unsigned int *layers = (unsigned int *) xmalloc(sizeof(unsigned int) * nLayers);
    
    for (i = 0; i < nLayers; i++) {
        layers[i] = (unsigned int) aLayers->ival[i];
    }
    
    struct fann *ann = fann_create_shortcut_array(nLayers, layers);
    assert(ann != NULL);
    
    WEIGHTS_INIT;
    
    dump_ann(ann);
    
    xfree(layers);
    
    fann_destroy(ann);
    
    CMD_FOOTER;
}

/** Set network weights */
static int cmd_set_weights(int argc, char **argv)
{
    CMD_HEADER(
            "set_weights",            
            "Set weights of the connections in a neural network"
            );
    
    int i;
    struct fann_connection *conns, *conn;
    struct arg_file *aFile  = arg_file0(NULL, "ann", "filepath", "path to the ANN file. If unspecified, read from STDIN");
    struct arg_str  *aConns = arg_strn(NULL, NULL, "conn", 1, argc+2, "connection string. Syntax: SRC:DST:WEIGHT, where SRC is the index of the source neuron, DST the destination neuron and WEIGHT the connection's weight. Bad formatted or unexisting connections will be ignored.");
    
    CMD_PARSE(aFile, aConns);    
    
    struct fann *ann;
    if (aFile->count > 0) {
        ann = fann_create_from_file(aFile->filename[0]);
    } else {
        ann = fann_create_from_fd(stdin, "STDIN");
    }
    
    assert(ann != NULL);
    
    //Set weights    
    conns = (struct fann_connection *) xmalloc(sizeof(struct fann_connection) * aConns->count);    
    for (i = 0, conn = conns; i < aConns->count; i++, conn++) {        
        unsigned long int nfrom, nto;
        double weight;     
        char *next;        
        const char *sconn = aConns->sval[i];
        
        nfrom = strtoul(sconn, &next, 10);
        if (*next == ':') {
            next++;
            nto = strtoul(next, &next, 10);
            if (*next == ':') {
                next++;
                if (*next != '\0') {
                    weight = strtod(next, &next);

                    if (*next == '\0') {
                        conn->from_neuron   = (unsigned int) nfrom;
                        conn->to_neuron     = (unsigned int) nto;
                        conn->weight        = (fann_type) weight;
                    }
                }                
            }
        }
    }
    
    fann_set_weight_array(ann, conns, aConns->count);
    
    xfree(conns);
    
    dump_ann(ann);
        
    fann_destroy(ann);
    
    CMD_FOOTER;
}

/** Get network parameters */
static int cmd_get_params(int argc, char **argv)
{
    CMD_HEADER(
            "get_params",            
            "Get ANN parameters in JSON format"            
            );
    
    struct arg_file *aFile   = arg_file0(NULL, "ann", "filepath", "path to the ANN file. If unspecified, read from STDIN");
    struct arg_lit  *aWithConnections = arg_lit0(NULL, "with-connections", "dump also ANN's connections");    
    struct arg_lit  *aWithActivFuncs = arg_lit0(NULL, "with-activation-functions", "dump also neurons' activation functions");    
    CMD_PARSE(aFile, aWithConnections, aWithActivFuncs);    
    
    struct fann *ann;
    if (aFile->count > 0) {
        ann = fann_create_from_file(aFile->filename[0]);
    } else {
        ann = fann_create_from_fd(stdin, "STDIN");
    }
    
    assert(ann != NULL);
    
    unsigned int i, j;
    unsigned int numLayers = fann_get_num_layers(ann);
    unsigned int *layers = (unsigned int *) xmalloc(sizeof(unsigned int) * numLayers);
    unsigned int *biases = (unsigned int *) xmalloc(sizeof(unsigned int) * numLayers);
    
    fann_get_layer_array(ann, layers);
    fann_get_bias_array(ann, biases);
    
    printf("{\n");
    printf("  type: \"%s\",\n", FANN_NETTYPE_NAMES[fann_get_network_type(ann)]);
    printf("  layers: {\n");
    printf("    input:  {neurons: %u, bias: %u},\n", layers[0], biases[0]);
    printf("    output: {neurons: %u},\n", layers[numLayers-1]);
    printf("    hidden: [\n");
    for (i = 1; i < numLayers - 1; i++) {
        printf("      {neurons: %u, bias: %u},\n", layers[i], biases[i]);
    }  
    
    
    printf("    ]\n");
    printf("  },\n"); 
    printf("  connectionRate: %f,\n", fann_get_connection_rate(ann));
    if (aWithConnections->count > 0) {
        printf("  connections: [");
        unsigned int totalConnections = fann_get_total_connections(ann);
        struct fann_connection *conn, *conns = (struct fann_connection *) xmalloc(sizeof(struct fann_connection) * totalConnections);
        fann_get_connection_array(ann, conns);
        for (i = 0, conn = conns; i < totalConnections; i++, conn++) {
            if (i > 0) putchar(',');
            printf("[%u,%u,%0.20e]", conn->from_neuron, conn->to_neuron, conn->weight);
        }
        xfree(conns);
        printf("],\n");
    }
    
    printf("  training: {\n");
    printf("    algorithm: \"%s\",\n", FANN_TRAIN_NAMES[fann_get_training_algorithm(ann)]);
    if (aWithActivFuncs->count > 0) {
        int total_dumped = 0;
        printf("    activationF: [\n");
        for (i = 1; i < numLayers; i++) { //start from first hidden layer
            for (j = 0; j < layers[i]; j++, total_dumped++) {
                if (total_dumped > 0) putchar(',');
                printf("{l: %u, n: %u, f: \"%s\", s: %f}", i, j, FANN_ACTIVATIONFUNC_NAMES[fann_get_activation_function(ann, i, j)], fann_get_activation_steepness(ann, i, j));
            }
        }
        printf("],\n");
    }
    printf("    errorF: \"%s\",\n", FANN_ERRORFUNC_NAMES[fann_get_train_error_function(ann)]);
    printf("    stopF: \"%s\",\n", FANN_STOPFUNC_NAMES[fann_get_train_stop_function(ann)]);
    printf("    stopParams: {\n");
    printf("      bitFailLimit: %f,\n", (double) fann_get_bit_fail_limit(ann));
    printf("    },\n");
    printf("    learningRate: %f,\n", (double) fann_get_learning_rate(ann));
    printf("    learningMomentum: %f,\n", (double) fann_get_learning_momentum(ann));
    printf("    quickPropParams: {\n");
    printf("      decay: %f,\n", (double) fann_get_quickprop_decay(ann));
    printf("      mu: %f\n", (double) fann_get_quickprop_mu(ann));
    printf("    },\n");
    printf("    rPropParams: {\n");
    printf("      increaseFactor: %f,\n", (double) fann_get_rprop_increase_factor(ann));
    printf("      decreaseFactor: %f,\n", (double) fann_get_rprop_decrease_factor(ann));
    printf("      deltaMin: %f,\n", (double) fann_get_rprop_delta_min(ann));
    printf("      deltaMax: %f,\n", (double) fann_get_rprop_delta_max(ann));
    printf("      deltaZero: %f\n", (double) fann_get_rprop_delta_zero(ann));
    printf("    },\n");   
    printf("    sarPropParams: {\n");
    printf("      weightDecayShift: %f,\n", (double) fann_get_sarprop_weight_decay_shift(ann));
    printf("      stepErrorThresholdFactor: %f,\n", (double) fann_get_sarprop_step_error_threshold_factor(ann));
    printf("      stepErrorShift: %f,\n", (double) fann_get_sarprop_step_error_shift(ann));
    printf("      temperature: %f\n", (double) fann_get_sarprop_temperature(ann));    
    printf("    },\n");
    printf("    cascadeParams: {\n");
    printf("      output: {\n");
    printf("        changeFraction: %f,\n", (double) fann_get_cascade_output_change_fraction(ann));    
    printf("        stagnationEpochs: %u,\n", fann_get_cascade_output_stagnation_epochs(ann));    
    printf("        maxEpochs: %u,\n", fann_get_cascade_max_out_epochs(ann));    
    printf("        minEpochs: %u\n", fann_get_cascade_min_out_epochs(ann));    
    printf("      },\n");
    printf("      candidates: {\n");
    printf("        count: %u,\n", fann_get_cascade_num_candidates(ann));        
    printf("        groups: %u,\n", fann_get_cascade_num_candidate_groups(ann));        
    printf("        trainingLimit: %f,\n", (double) fann_get_cascade_candidate_limit(ann));        
    printf("        changeFraction: %f,\n", (double) fann_get_cascade_candidate_change_fraction(ann));        
    printf("        stagnationEpochs: %u,\n", fann_get_cascade_candidate_stagnation_epochs(ann));        
    printf("        maxEpochs: %u,\n", fann_get_cascade_max_cand_epochs(ann));    
    printf("        minEpochs: %u\n", fann_get_cascade_min_cand_epochs(ann));    
    printf("      },\n");
    printf("      weightMultiplier: %f,\n", (double) fann_get_cascade_weight_multiplier(ann));      
    printf("      activationParams: {\n");    
    do {
        unsigned int nfunc = fann_get_cascade_activation_functions_count(ann);
        unsigned int nstep = fann_get_cascade_activation_steepnesses_count(ann);
        enum fann_activationfunc_enum *funcs = fann_get_cascade_activation_functions(ann);
        fann_type *stepnesses = fann_get_cascade_activation_steepnesses(ann);
        
        printf("        functions:[");
        for (i = 0; i < nfunc; i++) {
            if (i > 0) putchar(',');
            printf("\"%s\"", FANN_ACTIVATIONFUNC_NAMES[funcs[i]]);
        }
        printf("],\n");
        
        printf("        steepnesses:[");
        for (i = 0; i < nstep; i++) {
            if (i > 0) putchar(',');
            printf("%f", (double) stepnesses[i]);
        }
        printf("]\n");
    } while(0);    
    printf("      }\n");
    printf("    }\n");
    printf("  }\n");
    printf("}\n");
    
    xfree(layers);
    xfree(biases);
    
    
    fann_destroy(ann);
    
    CMD_FOOTER;
}

/** Setup training parameters */
static int cmd_setup_training(int argc, char **argv)
{
    CMD_HEADER(
            "setup_training",            
            "Set ANN training parameters"
            );
    
    struct arg_file *aFile = arg_file0(NULL, "ann", "filepath", "path to the ANN file. If unspecified, read from STDIN");    
    struct arg_str *aNeuronActivationFunc = arg_strn(NULL, "neuron-activation-function", "L:N:F", 0, argc+1, "activation function for specific neuron. The format of the string is L:N:F, where L is the layer number (being 1 the first hidden layer), N the neuron index in the layer (>=0) and F the name of the activation function. Entries with wrong layer or neuron numbers will be ignored.");
    struct arg_str *aHiddenActivationFunc = arg_str0(NULL, "hidden-activation-function", "string", "activation function for hidden layers: FANN_LINEAR, FANN_THRESHOLD, FANN_THRESHOLD_SYMMETRIC, FANN_SIGMOID, FANN_SIGMOID_STEPWISE, FANN_SIGMOID_SYMMETRIC, FANN_SIGMOID_SYMMETRIC_STEPWISE, FANN_GAUSSIAN, FANN_GAUSSIAN_SYMMETRIC, FANN_GAUSSIAN_STEPWISE, FANN_ELLIOT, FANN_ELLIOT_SYMMETRIC, FANN_LINEAR_PIECE, FANN_LINEAR_PIECE_SYMMETRIC, FANN_SIN_SYMMETRIC, FANN_COS_SYMMETRIC, FANN_SIN, FANN_COS.");
    struct arg_str *aOutputActivationFunc = arg_str0(NULL, "output-activation-function", "string", "activation function for output layer: FANN_LINEAR, FANN_THRESHOLD, FANN_THRESHOLD_SYMMETRIC, FANN_SIGMOID, FANN_SIGMOID_STEPWISE, FANN_SIGMOID_SYMMETRIC, FANN_SIGMOID_SYMMETRIC_STEPWISE, FANN_GAUSSIAN, FANN_GAUSSIAN_SYMMETRIC, FANN_GAUSSIAN_STEPWISE, FANN_ELLIOT, FANN_ELLIOT_SYMMETRIC, FANN_LINEAR_PIECE, FANN_LINEAR_PIECE_SYMMETRIC, FANN_SIN_SYMMETRIC, FANN_COS_SYMMETRIC, FANN_SIN, FANN_COS.");
    struct arg_str *aNeuronActivationSteep = arg_strn(NULL, "neuron-activation-steepness", "L:N:S", 0, argc+1, "activation steepness for specific neuron. The format of the string is L:N:S, where L is the layer number (being 1 the first hidden layer), N the neuron index in the layer (>=0) and S the steepness value. Entries with wrong layer or neuron numbers will be ignored.");
    struct arg_dbl *aHiddenActivationSteep = arg_dbl0(NULL, "hidden-activation-steepness", "float", "activation steepness for hidden layers.");
    struct arg_dbl *aOutputActivationSteep = arg_dbl0(NULL, "output-activation-steepness", "float", "activation steepness for output layer.");
    
    struct arg_str *aTrainingAlgorithm    = arg_str0(NULL, "training-algorithm", "string", "training algorithm:  FANN_TRAIN_INCREMENTAL, FANN_TRAIN_BATCH, FANN_TRAIN_RPROP, FANN_TRAIN_QUICKPROP, FANN_TRAIN_SARPROP");
    struct arg_str *aErrFunc              = arg_str0(NULL, "error-function", "string", "error function: FANN_ERRORFUNC_LINEAR, FANN_ERRORFUNC_TANH");
    struct arg_str *aStopFunc             = arg_str0(NULL, "stop-function", "string", "stop function: FANN_STOPFUNC_MSE, FANN_STOPFUNC_BIT");
    struct arg_dbl *aBitFailLimit         = arg_dbl0(NULL, "bit-fail-limit", "float", "bit fail limit when stop function is FANN_STOPFUNC_BIT. It's the maximum accepted difference between the desired output and the actual output during training.  Each output that diverges more than this limit is counted as an error bit.  This difference is divided by two when dealing with symmetric activation functions, so that symmetric and not symmetric activation functions can use the same limit.");
    struct arg_dbl *aLearningRate         = arg_dbl0(NULL, "learning-rate", "float", "used to determine how aggressive training should be for some of the training algorithms");
    struct arg_dbl *aLearningMomentum     = arg_dbl0(NULL, "learning-momentum", "float", "used to speed up FANN_TRAIN_INCREMENTAL training.  A too high momentum will however not benefit training.  Setting momentum to 0 will be the same as not using the momentum parameter.  The recommended value of this parameter is between 0.0 and 1.0.");
    struct arg_dbl *aQuickPropDecay       = arg_dbl0(NULL, "quickprop-decay", "float", "small negative valued number which is the factor that the weights should become smaller in each iteration during quickprop training. This is used to make sure that the weights do not become too high during training.");
    struct arg_dbl *aQuickPropMu          = arg_dbl0(NULL, "quickprop-mu", "float", "used to increase and decrease the step-size during quickprop training.  The mu factor should always be above 1, since it would otherwise decrease the step-size when it was suppose to increase it.");
    struct arg_dbl *aRPropIncreaseFactor  = arg_dbl0(NULL, "rprop-increase-factor", "float", "a value larger than 1, which is used to increase the step-size during RPROP training.");
    struct arg_dbl *aRPropDecreaseFactor  = arg_dbl0(NULL, "rprop-decrease-factor", "float", "a value smaller than 1, which is used to decrease the step-size during RPROP training.");
    struct arg_dbl *aRPropDeltaMin        = arg_dbl0(NULL, "rprop-delta-min", "float", "a small positive number determining how small the minimum step-size may be.");
    struct arg_dbl *aRPropDeltaMax        = arg_dbl0(NULL, "rprop-delta-max", "float", "a positive number determining how large the maximum step-size may be.");
    struct arg_dbl *aRPropDeltaZero       = arg_dbl0(NULL, "rprop-delta-zero", "float", "a positive number determining the initial step size.");
    struct arg_dbl *aSARPropWeightDecayShift       = arg_dbl0(NULL, "sarprop-weight-decay-shift", "float", "the SARPROP weight decay shift.");
    struct arg_dbl *aSARPropStepErrThresFactor     = arg_dbl0(NULL, "sarprop-step-error-threshold-factor", "float", "the SARPROP step error threshold factor.");
    struct arg_dbl *aSARPropStepErrShift           = arg_dbl0(NULL, "sarprop-step-error-shift", "float", "the SARPROP step error shift.");
    struct arg_dbl *aSARPropTemperature            = arg_dbl0(NULL, "sarprop-temperature", "float", "the SARPROP temperature.");
    struct arg_dbl *aCascadeOutputChangeFraction = arg_dbl0(NULL, "cascade-output-change-fraction", "float", "number between 0 and 1 determining how large a fraction the MSE value should change within --cascade-output-stagnation-epochs during training of the output connections, in order for the training not to stagnate.  If the training stagnates, the training of the output connections will be ended and new candidates will be prepared.");
    struct arg_int *aCascadeOutputStagEpochs     = arg_int0(NULL, "cascade-output-stagnation-epochs", "int", "number of epochs training is allowed to continue without changing the MSE by a fraction of --cascade-output-change-fraction.");
    struct arg_int *aCascadeOutputMaxEpochs      = arg_int0(NULL, "cascade-output-max-epochs", "int", "maximum number of epochs the output connections may be trained after adding a new candidate neuron.");
    struct arg_int *aCascadeOutputMinEpochs      = arg_int0(NULL, "cascade-output-min-epochs", "int", "minimum number of epochs the output connections may be trained after adding a new candidate neuron.");    
    struct arg_int *aCascadeCandidateGroups     = arg_int0(NULL, "cascade-candidate-groups", "int", "number of groups of identical candidates which will be used during training.");
    struct arg_dbl *aCascadeCandidateTrainingLimit = arg_dbl0(NULL, "cascade-candidate-training-limit", "float", "limit for how much the candidate neuron may be trained.  The limit is a limit on the proportion between the MSE and candidate score.");
    struct arg_dbl *aCascadeCandidateChangeFraction = arg_dbl0(NULL, "cascade-candidate-change-fraction", "float", "number between 0 and 1 determining how large a fraction the MSE value should change within --cascade-candidate-stagnation-epochs during training of the candidate connections, in order for the training not to stagnate.  If the training stagnates, the training of the candidate connections will be ended and new candidates will be prepared.");
    struct arg_int *aCascadeCandidateStagEpochs     = arg_int0(NULL, "cascade-candidate-stagnation-epochs", "int", "number of epochs training is allowed to continue without changing the MSE by a fraction of --cascade-candidate-change-fraction.");
    struct arg_int *aCascadeCandidateMaxEpochs      = arg_int0(NULL, "cascade-candidate-max-epochs", "int", "maximum number of epochs the candidate connections may be trained after adding a new candidate neuron.");
    struct arg_int *aCascadeCandidateMinEpochs      = arg_int0(NULL, "cascade-candidate-min-epochs", "int", "minimum number of epochs the candidate connections may be trained after adding a new candidate neuron.");
    struct arg_dbl *aCascadeWeightMultiplier        = arg_dbl0(NULL, "cascade-weight-multiplier", "float", "parameter which is used to multiply the weights from the candidate neuron before adding the neuron to the neural network.  This parameter is usually between 0 and 1, and is used to make the training a bit less aggressive.");
    struct arg_str *aCascadeActivationFunction      = arg_strn(NULL, "cascade-activation-function", "string", 0, argc+1, "activation function for cascade training. Specify as many functions as desired by repeating this argument.");
    struct arg_dbl *aCascadeActivationSteep         = arg_dbln(NULL, "cascade-activation-steepness", "float", 0, argc+1, "activation steepness for cascade training. Specify as many steepnesses as desired by repeating this argument.");
    
    
    CMD_PARSE(aFile, aNeuronActivationFunc, aHiddenActivationFunc, aOutputActivationFunc, aNeuronActivationSteep, aHiddenActivationSteep, aOutputActivationSteep, aTrainingAlgorithm, aErrFunc, aStopFunc, aBitFailLimit,
            aLearningRate, aLearningMomentum, aQuickPropDecay, aQuickPropMu, aRPropIncreaseFactor, aRPropDecreaseFactor, 
            aRPropDeltaMin, aRPropDeltaMax, aRPropDeltaZero, aSARPropWeightDecayShift, aSARPropStepErrThresFactor, aSARPropStepErrShift,
            aSARPropTemperature, aCascadeOutputChangeFraction, aCascadeOutputStagEpochs, aCascadeOutputMaxEpochs, aCascadeOutputMinEpochs,
            aCascadeCandidateGroups, aCascadeCandidateTrainingLimit, aCascadeCandidateChangeFraction, aCascadeCandidateStagEpochs,
            aCascadeCandidateMaxEpochs, aCascadeCandidateMinEpochs, aCascadeWeightMultiplier, aCascadeActivationFunction, aCascadeActivationSteep);    
    
    struct fann *ann;
    if (aFile->count > 0) {
        ann = fann_create_from_file(aFile->filename[0]);
    } else {
        ann = fann_create_from_fd(stdin, "STDIN");
    }
    
    assert(ann != NULL);
    
    if (aHiddenActivationFunc->count > 0) {
        enum fann_activationfunc_enum af = decode_activation_func(aHiddenActivationFunc->sval[0]);
        if (af == -1) {
            fprintf(stderr, "Unknown activation function: %s\n", aHiddenActivationFunc->sval[0]);
            CMD_ERR(ERR);
        }
        fann_set_activation_function_hidden(ann, af);
    }
    
    if (aHiddenActivationSteep->count > 0) {        
        fann_set_activation_steepness_hidden(ann, (fann_type) aHiddenActivationSteep->dval[0]);
    }
    
    if (aOutputActivationFunc->count > 0) {
        enum fann_activationfunc_enum af = decode_activation_func(aOutputActivationFunc->sval[0]);
        if (af == -1) {
            fprintf(stderr, "Unknown activation function: %s\n", aOutputActivationFunc->sval[0]);
            CMD_ERR(ERR);
        }
        fann_set_activation_function_output(ann, af);
    }
    
    if (aOutputActivationSteep->count > 0) {        
        fann_set_activation_steepness_output(ann, (fann_type) aOutputActivationSteep->dval[0]);
    }
    
    if (aNeuronActivationFunc->count > 0) {
        int i;
        for (i = 0; i < aNeuronActivationFunc->count; i++) {
            char *e;            
            unsigned int layer = (unsigned int) strtoul(aNeuronActivationFunc->sval[i], &e, 10);
            if (*e == ':') {
                e++;
                unsigned int neuron = (unsigned int) strtoul(e, &e, 10);
                if (*e == ':') {
                    e++;
                    enum fann_activationfunc_enum af = decode_activation_func(e);         
                    if (af == -1) {
                        fprintf(stderr, "Unknown activation function: %s\n", e);
                        CMD_ERR(ERR);
                    }
                    fann_set_activation_function(ann, af, layer, neuron);
                }                
            }            
        }
    }
    
    if (aNeuronActivationSteep->count > 0) {
        int i;
        for (i = 0; i < aNeuronActivationSteep->count; i++) {
            char *e;            
            unsigned int layer = (unsigned int) strtoul(aNeuronActivationSteep->sval[i], &e, 10);
            if (*e == ':') {
                e++;
                unsigned int neuron = (unsigned int) strtoul(e, &e, 10);
                if (*e == ':') {
                    e++;
                    double s = strtod(e, &e);
                    fann_set_activation_steepness(ann, (fann_type) s, layer, neuron);
                }                
            }            
        }
    }
    
    
    if (aTrainingAlgorithm->count > 0) {
        enum fann_train_enum ta = decode_training_algorithm(aTrainingAlgorithm->sval[0]);
        if (ta == -1) {
            fprintf(stderr, "Unknown training algorithm: %s\n", aTrainingAlgorithm->sval[0]);
            CMD_ERR(ERR);
        }
        fann_set_training_algorithm(ann, ta);
    }
    
    if (aErrFunc->count > 0) {
        enum fann_errorfunc_enum ef = decode_error_function(aErrFunc->sval[0]);
        if (ef == -1) {
            fprintf(stderr, "Unknown error function: %s\n", aErrFunc->sval[0]);
            CMD_ERR(ERR);
        }
        fann_set_train_error_function(ann, ef);
    }
    
    if (aStopFunc->count > 0) {
        enum fann_stopfunc_enum sf = decode_stop_function(aStopFunc->sval[0]);
        if (sf == -1) {
            fprintf(stderr, "Unknown stop function: %s\n", aStopFunc->sval[0]);
            CMD_ERR(ERR);
        }
        fann_set_train_stop_function(ann, sf);
    }
    
    if (aBitFailLimit->count > 0) fann_set_bit_fail_limit(ann, (fann_type) aBitFailLimit->dval[0]);
    if (aLearningRate->count > 0) fann_set_learning_rate(ann, (fann_type) aLearningRate->dval[0]);
    if (aLearningMomentum->count > 0) fann_set_learning_momentum(ann, (fann_type) aLearningMomentum->dval[0]);
    if (aQuickPropDecay->count > 0) fann_set_quickprop_decay(ann, (fann_type) aQuickPropDecay->dval[0]);
    if (aQuickPropMu->count > 0) fann_set_quickprop_mu(ann, (fann_type) aQuickPropMu->dval[0]);
    if (aRPropIncreaseFactor->count > 0) fann_set_rprop_increase_factor(ann, (fann_type) aRPropIncreaseFactor->dval[0]);
    if (aRPropDecreaseFactor->count > 0) fann_set_rprop_decrease_factor(ann, (fann_type) aRPropDecreaseFactor->dval[0]);
    if (aRPropDeltaMin->count > 0) fann_set_rprop_delta_min(ann, (fann_type) aRPropDeltaMin->dval[0]);
    if (aRPropDeltaMax->count > 0) fann_set_rprop_delta_max(ann, (fann_type) aRPropDeltaMax->dval[0]);
    if (aRPropDeltaZero->count > 0) fann_set_rprop_delta_zero(ann, (fann_type) aRPropDeltaZero->dval[0]);
    if (aSARPropWeightDecayShift->count > 0) fann_set_sarprop_weight_decay_shift(ann, (fann_type) aSARPropWeightDecayShift->dval[0]);
    if (aSARPropStepErrThresFactor->count > 0) fann_set_sarprop_step_error_threshold_factor(ann, (fann_type) aSARPropStepErrThresFactor->dval[0]);
    if (aSARPropStepErrShift->count > 0) fann_set_sarprop_step_error_shift(ann, (fann_type) aSARPropStepErrShift->dval[0]);
    if (aSARPropTemperature->count > 0) fann_set_sarprop_temperature(ann, (fann_type) aSARPropTemperature->dval[0]);
    if (aCascadeOutputChangeFraction->count > 0) fann_set_cascade_output_change_fraction(ann, (fann_type) aCascadeOutputChangeFraction->dval[0]);
    if (aCascadeOutputStagEpochs->count > 0) fann_set_cascade_output_stagnation_epochs(ann, aCascadeOutputStagEpochs->ival[0]);
    if (aCascadeOutputMaxEpochs->count > 0) fann_set_cascade_max_out_epochs(ann, aCascadeOutputMaxEpochs->ival[0]);
    if (aCascadeOutputMinEpochs->count > 0) fann_set_cascade_min_out_epochs(ann, aCascadeOutputMinEpochs->ival[0]);
    if (aCascadeCandidateGroups->count > 0) fann_set_cascade_num_candidate_groups(ann, aCascadeCandidateGroups->ival[0]);
    if (aCascadeCandidateTrainingLimit->count > 0) fann_set_cascade_candidate_limit(ann, (fann_type) aCascadeCandidateTrainingLimit->dval[0]);
    if (aCascadeCandidateChangeFraction->count > 0) fann_set_cascade_candidate_change_fraction(ann, (fann_type) aCascadeCandidateChangeFraction->dval[0]);
    if (aCascadeCandidateStagEpochs->count > 0) fann_set_cascade_candidate_stagnation_epochs(ann, aCascadeCandidateStagEpochs->ival[0]);
    if (aCascadeCandidateMaxEpochs->count > 0) fann_set_cascade_max_cand_epochs(ann, aCascadeCandidateMaxEpochs->ival[0]);
    if (aCascadeCandidateMinEpochs->count > 0) fann_set_cascade_min_cand_epochs(ann, aCascadeCandidateMinEpochs->ival[0]);
    if (aCascadeWeightMultiplier->count > 0) fann_set_cascade_weight_multiplier(ann, (fann_type) aCascadeWeightMultiplier->dval[0]);
    
    if (aCascadeActivationFunction->count > 0) {
        int n = aCascadeActivationFunction->count;
        int i;
        enum fann_activationfunc_enum *afs = (enum fann_activationfunc_enum *)xmalloc(sizeof(enum fann_activationfunc_enum) * n);
        for (i = 0; i < n; i++) {
            afs[i] = decode_activation_func(aCascadeActivationFunction->sval[i]);
            if (afs[i] == -1) {
                fprintf(stderr, "Unknown activation function: %s\n", aCascadeActivationFunction->sval[i]);
                CMD_ERR(ERR);
            }
        }
        fann_set_cascade_activation_functions(ann, afs, n);
        xfree(afs);
    }
    
    if (aCascadeActivationSteep->count > 0) {
        int n = aCascadeActivationSteep->count;
        int i;
        fann_type *steeps = (fann_type *)xmalloc(sizeof(fann_type) * n);
        for (i = 0; i < n; i++) {
            steeps[i] = (fann_type) aCascadeActivationSteep->dval[i];
        }
        fann_set_cascade_activation_steepnesses(ann, steeps, n);
        xfree(steeps);
    }
    
    
    
    dump_ann(ann);

    ERR:
    fann_destroy(ann);
    
    CMD_FOOTER;
}



//Callback function for train command
static int cmd_train_callback(struct fann *ann, struct fann_train_data *train, unsigned int max_epochs, unsigned int epochs_between_reports, float desired_error, unsigned int epochs)
{
   FILE *fp = (FILE *) fann_get_user_data(ann);
   fprintf(fp, "Epochs     %8d. MSE: %.5f. Desired-MSE: %.5f\n", epochs, fann_get_MSE(ann), desired_error);
   return 0;
}

/** Train network */
static int cmd_train(int argc, char **argv)
{
    CMD_HEADER(
            "train",            
            "Trains an ANN"
            );
    
    struct arg_file *aFile = arg_file0(NULL, "ann", "filepath", "path to the ANN file. If unspecified, read from STDIN");
    struct arg_file *aTrainingFile = arg_file1(NULL, "training-data", "filepath", "path to the training data file.");
    struct arg_int  *aMaxEpochs = arg_int1(NULL, "max-epochs", "int", "maximum number of epochs the training should continue");
    struct arg_int  *aReportPeriod = arg_int0(NULL, "report-period", "int", "the number of epochs between printing a status report to stderr. If omitted, no reports should be printed.");
    struct arg_dbl  *aDesiredError = arg_dbl1(NULL, "target-error", "float", "the desired target error");
    struct arg_file *aReport = arg_file0(NULL, "report-file", "filepath", "path to report file. If omitted, STDERR is used.");
    
    CMD_PARSE(aFile, aTrainingFile, aMaxEpochs, aReportPeriod, aDesiredError, aReport);    
    
    struct fann *ann;
    if (aFile->count > 0) {
        ann = fann_create_from_file(aFile->filename[0]);
    } else {
        ann = fann_create_from_fd(stdin, "STDIN");
    }
    
    assert(ann != NULL);
    
    FILE *reportFP = stderr;
    if (aReport->count > 0) {
        FILE *fp = fopen(aReport->filename[0], "w");
        if (fp == NULL) CMD_ERR(ERR);
        reportFP = fp;
    } 
    
    fann_set_user_data(ann, reportFP);
    
    //Set callback function
    fann_set_callback(ann, cmd_train_callback);
        
    //Train
    fann_train_on_file(ann, aTrainingFile->filename[0], aMaxEpochs->ival[0], (aReportPeriod->count == 0) ? 0 : aReportPeriod->ival[0], (float) aDesiredError->dval[0]);     
    
    dump_ann(ann);
    
    if (reportFP != stderr) fclose(reportFP);
    
ERR:    
    fann_destroy(ann);
    
    CMD_FOOTER;
}

/** Run network */
static int cmd_run(int argc, char **argv)
{
    CMD_HEADER(
            "run",            
            "Run an ANN. This command either reads the input values from a file that contains values separated with spaces (using --input-file) or from the command line (using -i option as many times as inputs). The command prints to STDOUT the output values separated with spaces."
            );
    
    struct arg_file *aFile = arg_file0(NULL, "ann", "filepath", "path to the ANN file. If unspecified, read from STDIN");
    struct arg_file *aInputFile = arg_file0(NULL, "input-file", "filepath", "path to the input file. If omitted input values are read from the command line");
    struct arg_dbl  *aInputValues = arg_dbln("i", NULL, "float", 0, argc+1, "input values");
    CMD_PARSE(aFile, aInputFile, aInputValues);    
    
    if (aInputFile->count == 0 && aInputValues->count == 0) {
        fprintf(stderr, "You must specify either a file with input data or pass data through the command line. See --help for further information");
        CMD_ABORT;
    }
    
    struct fann *ann;
    if (aFile->count > 0) {
        ann = fann_create_from_file(aFile->filename[0]);
    } else {
        ann = fann_create_from_fd(stdin, "STDIN");
    }
    
    assert(ann != NULL);
    
    unsigned int nInputs = fann_get_num_input(ann);
    
       
    fann_type *inputs = NULL;
    if (aInputFile->count > 0) {
        FILE *fp = fopen(aInputFile->filename[0], "r");
        if (fp == NULL) {
            fprintf(stderr, "Could not open input file\n");
            CMD_ERR(RUN_ERR);
        }
        
        
        int i;
        
        inputs = (fann_type *) xmalloc(sizeof(fann_type) * nInputs);
        i = 0;
        while(i < nInputs && !feof(fp)) {
            double v;
            fscanf(fp, "%f", &v);
            inputs[i] = (fann_type) v;            
            i++;
        }
               
        fclose(fp);
        
        if (i < nInputs) {
            fprintf(stderr, "End of file reached! There are missing values in the file.\n");            
            CMD_ERR(RUN_ERR);
        }
                
    } else {
        int i;
        if ((unsigned int) aInputValues->count != nInputs) {
            fprintf(stderr, "Input dimension error. Expected %u inputs but %d were supplied\n", nInputs, aInputValues->count);
            CMD_ERR(RUN_ERR);
        }
        inputs = (fann_type *) xmalloc(sizeof(fann_type) * nInputs);
        
        for (i = 0; i < nInputs; i++) {
            inputs[i] = (fann_type) aInputValues->dval[i];
        }
    }
    
    fann_type *output = fann_run(ann, inputs);
    unsigned int iOutput;
    
    for (iOutput = 0; iOutput < fann_get_num_output(ann); iOutput++) {
        if (iOutput > 0) fputc(' ', stdout);
        fprintf(stdout, "%f", output[iOutput]);
    }
    fputc('\n', stdout);
    
    
        
    RUN_ERR:
    
    fann_destroy(ann);
    if (inputs != NULL) xfree(inputs);
    
    CMD_FOOTER;
}

/** Test network */
static int cmd_test(int argc, char **argv)
{
    CMD_HEADER(
            "test",            
            "Test an ANN. This command either reads the test data from a file (using --test-data) or performs a single test reading the input and output values from the command line (using -i and -o options as many times as inputs and outputs). The command prints to STDOUT the resulting MSE."
            );
    
    struct arg_file *aFile = arg_file0(NULL, "ann", "filepath", "path to the ANN file. If unspecified, read from STDIN");
    struct arg_file *aTestData = arg_file0(NULL, "test-data", "filepath", "path to the input test file. If omitted input and output test values are read from the command line");
    struct arg_dbl  *aInputValues = arg_dbln("i", NULL, "float", 0, argc+1, "input values");
    struct arg_dbl  *aOutputValues = arg_dbln("o", NULL, "float", 0, argc+1, "output values");
    CMD_PARSE(aFile, aTestData, aInputValues, aOutputValues);    
    
    if (aTestData->count == 0 && (aInputValues->count == 0 || aOutputValues->count == 0)) {
        fprintf(stderr, "You must specify either a file with test data or pass data through the command line. See --help for further information");
        CMD_ABORT;
    }
    
    struct fann *ann;
    if (aFile->count > 0) {
        ann = fann_create_from_file(aFile->filename[0]);
    } else {
        ann = fann_create_from_fd(stdin, "STDIN");
    }
    
    assert(ann != NULL);
    
    struct fann_train_data *testData = NULL;
    
    if (aTestData->count > 0) {
        testData = fann_read_train_from_file(aTestData->filename[0]);        
        if (testData == NULL) {
            fprintf(stderr, "Could not open test file\n");
            CMD_ERR(ERR);
        }

    } else {        
        unsigned int nInputs = fann_get_num_input(ann), nOutputs = fann_get_num_output(ann);
        int i;
        if ((unsigned int) aInputValues->count != nInputs || (unsigned int) aOutputValues->count != nOutputs) {
            fprintf(stderr, "Input or output dimension error. Expected %u inputs and %u outputs, but %d inputs and %d outputs were supplied\n", nInputs, nOutputs, aInputValues->count, aOutputValues->count);
            CMD_ERR(ERR);
        }
                
        testData = fann_create_train(1, nInputs, nOutputs);
                
        for (i = 0; i < nInputs; i++) {
            testData->input[0][i]  = (fann_type) aInputValues->dval[i];
        }
        for (i = 0; i < nOutputs; i++) {
            testData->output[0][i] = (fann_type) aOutputValues->dval[i];
        }
    }
        
    
    fprintf(stdout, "%f\n", (double) fann_test_data(ann, testData));
    
ERR:
    
    fann_destroy(ann);
    if (testData != NULL) fann_destroy_train(testData);
    
    CMD_FOOTER;
}



static int cmd_help(int argc, char **argv);

/** Command table */
static struct cmd_tab_entry {
    const char *name;       /**< Command name */  
    int (*f)(int, char**);  /**< Command handler */
    const char *brief;      /**< Command brief */
} CMDTAB[] = {
    {.name = "help", .f = cmd_help, .brief = "show a list of supported commands"},
    {.name = "create_std", .f = cmd_create_std, .brief = "create a standard fully connected backpropagation neural network"},
    {.name = "create_sparse", .f = cmd_create_sparse, .brief = "create a standard backpropagation neural network, which is not fully connected"},
    {.name = "create_shortcut", .f = cmd_create_shortcut, .brief = "creates a standard backpropagation neural network, which is not fully connected and which also has shortcut connections"},
    {.name = "set_weights", .f = cmd_set_weights, .brief = "set weights of the connections in a neural network"},
    {.name = "get_params", .f = cmd_get_params, .brief = "get ANN's parameters"},
    {.name = "setup_training", .f = cmd_setup_training, .brief = "set ANN's training parameters"},
    {.name = "train", .f = cmd_train, .brief = "Train ANN from data file"},
    {.name = "run", .f = cmd_run, .brief = "Run an ANN"},
    {.name = "test", .f = cmd_test, .brief="Test an ANN"},
    ///////////////////////////
    {.name = NULL} //Last item
};

/** Main help */
static int cmd_help(int argc, char **argv)
{
    struct cmd_tab_entry *entry;
    
    banner();
    printf("Command list:\n");
    for (entry = CMDTAB; entry->name != NULL; entry++) {
        printf(" %-20s :%s\n", entry->name, entry->brief);
    }
}

/**
 * Run command
 * @param argc Argument count
 * @param argv Arguments
 * @return Return code
 */
int runCommand(int argc, char **argv)
{
    struct cmd_tab_entry *entry;
    const char *cmdname = argv[0];
    for (entry = CMDTAB; entry->name != NULL; entry++) {
        if (strcmp(entry->name, cmdname) == 0) {
            return entry->f(argc, argv);
        }
    }
    
    fprintf(stderr, "Invalid command: %s. Type `fannc help`.\n", cmdname);
    return 1;
}

/**
 * Print banner
 */
void banner()
{
    printf("Copyright (C) 2014 Claudi Martinez <claudix.kernel@gmail.com>.\nThis is free software; see the source for copying conditions\n");
}