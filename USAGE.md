# fannc - Usage
<style>
td:first-child {
   white-space:nowrap !important;
   vertical-align: top !important;
}
</style>
## Overview

Fannc is a command line utility for creating, training and testing **artificial neural networks** (ANN) using the [FANN library](http://leenissen.dk/fann/wp/). The general syntax of this command is:

```
fannc COMMAND ARGS...
```
Where COMMAND is the command to execute and ARGS its particular arguments. Type `fannc help` for showing a list of all supported commands:

```
$ fannc help
Command list:
 help                 :show a list of supported commands
 create_std           :create a standard fully connected backpropagation neural network
 create_sparse        :create a standard backpropagation neural network, which is not fully connected
 create_shortcut      :creates a standard backpropagation neural network, which is not fully connected and which also has shortcut connections
 set_weights          :set weights of the connections in a neural network
 get_params           :get ANN's parameters
 setup_training       :set ANN's training parameters
 train                :Train ANN from data file
 run                  :Run an ANN
```


Most of the commands that deal with ANNs are designed to get a network file from STDIN (or from a file) and to dump it to STDOUT. This way one can take advantage of the shell's ability to pass data through pipes from one command to another (using the vertical bar | symbol) as well as to redirect STDOUT to a file. So, for example, the shell command below creates a standard ANN with 2 input neurons, 3 neurons in a hidden layer and 1 output neuron; then sets some of its training parameters and finally stores the resulting network into a file, all in one single line:

```
$ fannc create_std 2 3 1 | fannc config_training --training-algorithm=FANN_TRAIN_QUICKPROP > ann1.net
```

Now if we wanted to create a second network based on the previous one but with some different parameters we'll do:

```
$ fannc config_training --ann=ann1.net --training-algorithm=FANN_TRAIN_RPROP > ann2.net
```

To get help about a specific command just type: `fannc COMMAND --help`.

### Note about this document
Most of the text of this document is an excerpt of the [FANN library's reference manual](http://leenissen.dk/fann/html/files/fann-h.html). Please, refer to that manual for further information about the concepts and implementation issues behind the way fannc works.

## Command reference

### create_std
Creates a standard fully connected backpropagation neural network. There will be a bias neuron in each layer (except the output layer), and this bias neuron will be connected to all neurons in the next layer.  When running the network, the bias nodes always emit 1.

The created network's structure will be dumped to STDOUT.

**Usage:**
```
fannc create_std [--min-random-weight=float] [--max-random-weight=float] [--init-weights] n n [n]... 
                 [--help]
```

Argument                       | Description
-------------------------------|-------------
`--min-random-weight=float`    | `minimum random value for initializing weights. If omitted, -0.1 is taken.`
`--max-random-weight=float`    | `maximum random value for initializing weights. If omitted, 0.1 is taken.`
`--init-weights`               | `if specified, initialize weights using Widrow + Nguyen’s algorithm from training data passed from STDIN`
`n`                            | `integer values determining the number of neurons in each layer starting with the input layer and ending with the output layer.`
`--help`                       | `print this help and exit`

<hr>
### create_sparse
Creates a standard backpropagation neural network, which is not fully connected.

The created network's structure will be dumped to STDOUT.

**Usage:**
```
fannc create_sparse [--min-random-weight=float] [--max-random-weight=float] [--init-weights] 
                    --rate=int n n [n]... [--help]
```

Argument                       | Description
-------------------------------|-------------
`--min-random-weight=float`    | `minimum random value for initializing weights. If omitted, -0.1 is taken.`
`--max-random-weight=float`    | `maximum random value for initializing weights. If omitted, 0.1 is taken.`
`--init-weights`               | `if specified, initialize weights using Widrow + Nguyen's algorithm from training data passed from STDIN`
`--rate=int`                   | `connection rate expressed as a percent. It controls how many connections there will be in the network. If the connection rate is set to 100, the network will be fully connected, but if it is set to 50 only half of the connections will be set. A connection rate of 100 will yield the same result as create_std.`
`n`                            | `integer values determining the number of neurons in each layer starting with the input layer and ending with the output layer.`
`--help`                       | `print this help and exit`

<hr>
### create_shortcut
Creates a standard backpropagation neural network, which is not fully connected and which also has shortcut connections. Shortcut connections are connections that skip layers.  A fully connected network with shortcut connections, is a network where all neurons are connected to all neurons in later layers.  Including direct connections from the input layer to the output layer.

The created network's structure will be dumped to STDOUT.

**Usage:**
```
fannc create_shortcut [--min-random-weight=float] [--max-random-weight=float] [--init-weights] n n [n]... 
                      [--help]
```

Argument                       | Description
-------------------------------|-------------
`--min-random-weight=float`    | `minimum random value for initializing weights. If omitted, -0.1 is taken.`
`--max-random-weight=float`    | `maximum random value for initializing weights. If omitted, 0.1 is taken.`
`--init-weights`               | `if specified, initialize weights using Widrow + Nguyen’s algorithm from training data passed from STDIN`
`n`                            | `integer values determining the number of neurons in each layer starting with the input layer and ending with the output layer.`
`--help`                       | `print this help and exit`

<hr>
### set_weights
Set weights of the connections in a neural network.

**Usage**
```
fannc set_weights [--ann=filepath] conn [conn]... [--help]
```

Argument                       | Description
-------------------------------|-------------
`--ann=filepath`               |`path to the ANN file. If unspecified, read from STDIN.`
`conn`                         |`connection string. Syntax: SRC:DST:WEIGHT, where SRC is the number of the source neuron, DST the destination neuron and WEIGHT the connection's weight. Bad formatted or unexisting connections will be ignored.`
`--help`                       | `print this help and exit`


<hr>
### get_params
Get ANN parameters dumping them to STDOUT in JSON format.

**Usage**
```
fannc get_params [--ann=filepath] [--with-connections] [--with-activation-functions] [--help]
```

Argument                       | Description
-------------------------------|-------------
`--ann=filepath`               |`path to the ANN file. If unspecified, read from STDIN.`
`--with-connections`           |`dump also ANN's connections`
`--with-activation-functions`  |`dump also neurons' activation functions`
`--help`                       | `print this help and exit`

**Output example**
```
$ fannc get_params --ann=ann1.net
{
  type: "FANN_NETTYPE_LAYER",
  layers: {
    input:  {neurons: 2, bias: 1},
    output: {neurons: 1},
    hidden: [
      {neurons: 3, bias: 1},
    ]
  },
  connectionRate: 1.000000,
  training: {
    algorithm: "FANN_TRAIN_QUICKPROP",
    errorF: "FANN_ERRORFUNC_TANH",
    stopF: "FANN_STOPFUNC_MSE",
    stopParams: {
      bitFailLimit: 0.350000,
    },
    learningRate: 0.700000,
    learningMomentum: 0.000000,
    quickPropParams: {
      decay: -0.000100,
      mu: 1.750000
    },
    rPropParams: {
      increaseFactor: 1.200000,
      decreaseFactor: 0.500000,
      deltaMin: 0.000000,
      deltaMax: 50.000000,
      deltaZero: 0.100000
    },
    sarPropParams: {
      weightDecayShift: -6.644000,
      stepErrorThresholdFactor: 0.100000,
      stepErrorShift: 1.385000,
      temperature: 0.015000
    },
    cascadeParams: {
      output: {
        changeFraction: 0.010000,
        stagnationEpochs: 12,
        maxEpochs: 150,
        minEpochs: 50
      },
      candidates: {
        count: 80,
        groups: 2,
        trainingLimit: 1000.000000,
        changeFraction: 0.010000,
        stagnationEpochs: 12,
        maxEpochs: 150,
        minEpochs: 50
      },
      weightMultiplier: 0.400000,
      activationParams: {
        functions:["FANN_SIGMOID","FANN_SIGMOID_SYMMETRIC","FANN_GAUSSIAN","FANN_GAUSSIAN_SYMMETRIC","FANN_ELLIOT","FANN_ELLIOT_SYMMETRIC","FANN_SIN_SYMMETRIC","FANN_COS_SYMMETRIC","FANN_SIN","FANN_COS"],
        steepnesses:[0.250000,0.500000,0.750000,1.000000]
      }
    }
  }
}
```
<hr>
### setup_training
Set ANN training parameters

**Usage**
```
fannc setup_training [--ann=filepath] [--neuron-activation-function=L:N:F]... 
                     [--hidden-activation-function=string] 
                     [--output-activation-function=string] [--neuron-activation-steepness=L:N:S]... 
                     [--hidden-activation-steepness=float] [--output-activation-steepness=float] 
                     [--training-algorithm=string] [--error-function=string] [--stop-function=string] 
                     [--bit-fail-limit=float] [--learning-rate=float] [--learning-momentum=float] 
                     [--quickprop-decay=float] [--quickprop-mu=float] [--rprop-increase-factor=float] 
                     [--rprop-decrease-factor=float] [--rprop-delta-min=float] [--rprop-delta-max=float] 
                     [--rprop-delta-zero=float] [--sarprop-weight-decay-shift=float] 
                     [--sarprop-step-error-threshold-factor=float] [--sarprop-step-error-shift=float] 
                     [--sarprop-temperature=float] [--cascade-output-change-fraction=float] 
                     [--cascade-output-stagnation-epochs=int] [--cascade-output-max-epochs=int] 
                     [--cascade-output-min-epochs=int] [--cascade-candidate-groups=int] 
                     [--cascade-candidate-training-limit=float] [--cascade-candidate-change-fraction=float] 
                     [--cascade-candidate-stagnation-epochs=int] [--cascade-candidate-max-epochs=int] 
                     [--cascade-candidate-min-epochs=int] [--cascade-weight-multiplier=float] 
                     [--cascade-activation-function=string]... [--cascade-activation-steepness=float]... [--help]
```

Argument                                       | Description
-----------------------------------------------|-------------
`--ann=filepath`                               |`path to the ANN file. If unspecified, read from STDIN`
`--hidden-activation-function=string`          |`activation function for hidden layers: FANN_LINEAR, FANN_THRESHOLD, FANN_THRESHOLD_SYMMETRIC, FANN_SIGMOID, FANN_SIGMOID_STEPWISE, FANN_SIGMOID_SYMMETRIC, FANN_SIGMOID_SYMMETRIC_STEPWISE, FANN_GAUSSIAN, FANN_GAUSSIAN_SYMMETRIC, FANN_GAUSSIAN_STEPWISE, FANN_ELLIOT, FANN_ELLIOT_SYMMETRIC, FANN_LINEAR_PIECE, FANN_LINEAR_PIECE_SYMMETRIC, FANN_SIN_SYMMETRIC, FANN_COS_SYMMETRIC, FANN_SIN, FANN_COS.`
`--output-activation-function=string`          |`activation function for output layer: FANN_LINEAR, FANN_THRESHOLD, FANN_THRESHOLD_SYMMETRIC, FANN_SIGMOID, FANN_SIGMOID_STEPWISE, FANN_SIGMOID_SYMMETRIC, FANN_SIGMOID_SYMMETRIC_STEPWISE, FANN_GAUSSIAN, FANN_GAUSSIAN_SYMMETRIC, FANN_GAUSSIAN_STEPWISE, FANN_ELLIOT, FANN_ELLIOT_SYMMETRIC, FANN_LINEAR_PIECE, FANN_LINEAR_PIECE_SYMMETRIC, FANN_SIN_SYMMETRIC, FANN_COS_SYMMETRIC, FANN_SIN, FANN_COS.`
`--neuron-activation-function=L:N:F`           |`activation function for specific neuron. The format of the string is L:N:F, where L is the layer number (being 1 the first hidden layer), N the neuron index in the layer (>=0) and F the name of the activation function. Entries with wrong layer or neuron numbers will be ignored.`
`--neuron-activation-steepness=L:N:S`          |`activation steepness for specific neuron. The format of the string is L:N:S, where L is the layer number (being 1 the first hidden layer), N the neuron index in the layer (>=0) and S the steepness value. Entries with wrong layer or neuron numbers will be ignored.`
`--hidden-activation-steepness=float`          |`activation steepness for hidden layers.`
`--output-activation-steepness=float`          |`activation steepness for output layer.`
`--training-algorithm=string`                  |`training algorithm: FANN_TRAIN_INCREMENTAL, FANN_TRAIN_BATCH, FANN_TRAIN_RPROP, FANN_TRAIN_QUICKPROP, FANN_TRAIN_SARPROP`
`--error-function=string`                      |`error function: FANN_ERRORFUNC_LINEAR, FANN_ERRORFUNC_TANH`
`--stop-function=string`                       |`stop function: FANN_STOPFUNC_MSE, FANN_STOPFUNC_BIT`
`--bit-fail-limit=float`                       |`bit fail limit when stop function is FANN_STOPFUNC_BIT. It's the maximum accepted difference between the desired output and the actual output during training.  Each output that diverges more than this limit is counted as an error bit.  This difference is divided by two when dealing with symmetric activation functions, so that symmetric and not symmetric activation functions can use the same limit.`
`--learning-rate=float`                        |`used to determine how aggressive training should be for some of the training algorithms`
`--learning-momentum=float`                    |`used to speed up FANN_TRAIN_INCREMENTAL training.  A too high momentum will however not benefit training.  Setting momentum to 0 will be the same as not using the momentum parameter.  The recommended value of this parameter is between 0.0 and 1.0.`
`--quickprop-decay=float`                      |`small negative valued number which is the factor that the weights should become smaller in each iteration during quickprop training. This is used to make sure that the weights do not become too high during training.`
`--quickprop-mu=float`                         |`used to increase and decrease the step-size during quickprop training.  The mu factor should always be above 1, since it would otherwise decrease the step-size when it was suppose to increase it.`
`--rprop-increase-factor=float`                |`a value larger than 1, which is used to increase the step-size during RPROP training.`
`--rprop-decrease-factor=float`                |`a value smaller than 1, which is used to decrease the step-size during RPROP training.`
`--rprop-delta-min=float`                      |`a small positive number determining how small the minimum step-size may be.`
`--rprop-delta-max=float`                      |`a positive number determining how large the maximum step-size may be.`
`--rprop-delta-zero=float`                     |`a positive number determining the initial step size.`
`--sarprop-weight-decay-shift=float`           |`the SARPROP weight decay shift.`
`--sarprop-step-error-threshold-factor=float`  |`the SARPROP step error threshold factor.`
`--sarprop-step-error-shift=float`             |`the SARPROP step error shift.`
`--sarprop-temperature=float`                  |`the SARPROP temperature.`
`--cascade-output-change-fraction=float`       |`number between 0 and 1 determining how large a fraction the MSE value should change within --cascade-output-stagnation-epochs during training of the output connections, in order for the training not to stagnate.  If the training stagnates, the training of the output connections will be ended and new candidates will be prepared.`
`--cascade-output-stagnation-epochs=int`       |`number of epochs training is allowed to continue without changing the MSE by a fraction of --cascade-output-change-fraction.`
`--cascade-output-max-epochs=int`              |`maximum number of epochs the output connections may be trained after adding a new candidate neuron.`
`--cascade-output-min-epochs=int`              |`minimum number of epochs the output connections may be trained after adding a new candidate neuron.`
`--cascade-candidate-groups=int`               |`number of groups of identical candidates which will be used during training.`
`--cascade-candidate-training-limit=float`     |`limit for how much the candidate neuron may be trained.  The limit is a limit on the proportion between the MSE and candidate score.`
`--cascade-candidate-change-fraction=float`    |`number between 0 and 1 determining how large a fraction the MSE value should change within --cascade-candidate-stagnation-epochs during training of the candidate connections, in order for the training not to stagnate.  If the training stagnates, the training of the candidate connections will be ended and new candidates will be prepared.`
`--cascade-candidate-stagnation-epochs=int`    |`number of epochs training is allowed to continue without changing the MSE by a fraction of --cascade-candidate-change-fraction.`
`--cascade-candidate-max-epochs=int`           |`maximum number of epochs the candidate connections may be trained after adding a new candidate neuron.`
`--cascade-candidate-min-epochs=int`           |`minimum number of epochs the candidate connections may be trained after adding a new candidate neuron.`
`--cascade-weight-multiplier=float`            |`parameter which is used to multiply the weights from the candidate neuron before adding the neuron to the neural network.  This parameter is usually between 0 and 1, and is used to make the training a bit less aggressive.`
`--cascade-activation-function=string`         |`activation function for cascade training. Specify as many functions as desired by repeating this argument.`
`--cascade-activation-steepness=float`         |`activation steepness for cascade training. Specify as many steepnesses as desired by repeating this argument.`
`--help`                                       |`print this help and exit`


<hr>
### train
Train an ANN

**Usage**
```
fannc train [--ann=filepath] --training-data=filepath --max-epochs=int [--report-period=int] --target-error=float 
            [--report-file=filepath] [--help]
```

Argument                                       | Description
-----------------------------------------------|-------------
`--ann=filepath`                               |`path to the ANN file. If unspecified, read from STDIN`
`--training-data=filepath`                     |`path to the training data file.`
`--max-epochs=int`                             |`maximum number of epochs the training should continue`
`--report-period=int`                          |`the number of epochs between printing a status report to stderr. If omitted, no reports should be printed.`
`--target-error=float`                         |`the desired target error`
`--report-file=filepath`                       |`path to report file. If omitted, STDERR is used.`
`--help`                                       |`print this help and exit`


<hr>
### run
Run an ANN. This command either reads the input values from a file that contains values separated with spaces (using --input-file) or from the command line (using -i option as many times as inputs).

The command prints to STDOUT the output values separated with spaces.

**Usage**
```
fannc run [--ann=filepath] [--input-file=filepath] [-i float]... [--help]
```

Argument                                       | Description
-----------------------------------------------|-------------
`--ann=filepath`                               |`path to the ANN file. If unspecified, read from STDIN`
`--input-file=filepath`                        |`path to the input file. If omitted input values are read from the command line`
`-i float`                                     |`input values`
`--help`                                       |`print this help and exit`

<hr>
### test
Test an ANN. This command either reads the test data from a file (using --test-data) or performs a single test reading the input and output values from the command line (using -i and -o options as many times as inputs and outputs).

The command prints to STDOUT the resulting MSE.

**Usage**
```
fannc test [--ann=filepath] [--input-file=filepath] [-i float]... [-o float]... [--help]
```

Argument                                       | Description
-----------------------------------------------|-------------
`--ann=filepath`                               |`path to the ANN file. If unspecified, read from STDIN`
`--test-data=filepath`                         |`path to the input test file. If omitted input and output test values are read from the command line`
`-i float`                                     |`input values`
`-o float`                                     |`output values`
`--help`                                       |`print this help and exit`





