# fannc


Fannc is a complete and well documented command line utility for creating, training and testing **artificial neural networks** using the [FANN library](http://leenissen.dk/fann/wp/). Fannc implements all the library features by exposing its functions through an extensive command set.

### Limitations

The fixed point library version is not supported. All values involved are floating point numbers.

### Dependencies

FANNC depends on the following libraries:
- **argtable2**: a library for parsing command line arguments. You can download it from http://argtable.sourceforge.net.
- **fann**: a library that implements multilayer artificial neural networks. It is the core of **fannc** and you can download it from http://leenissen.dk/fann/wp.



### Building on Linux

To build **fannc** just run the following commands from a terminal shell in the source directory:

```bash
mkdir build
cd build
cmake ..
make
```

### Building on other platforms

**fannc** has not been tested in other platforms so far. Any contribution for porting it to them will be highly appreciated.

### Documentation

You can find documentation about the usage of fannc in [**USAGE.md**](https://github.com/claudix/fannc/blob/master/USAGE.md).


### License

**fannc** is licensed under the terms of the [LGPL license](https://www.gnu.org/licenses/lgpl-3.0.txt).

<hr>
### Example

Let's create an ANN which learns the XOR operation. The ANN has 2 inputs and 1 output, and also a hidden layer with 3 neurons. First, we create a file called `xor.data` that contains the training data. The format of this file is the same as the one described by the FANN library:

```
<NUMBER_OF_EXPERIMENTS> <NUMBER_OF_INPUTS> <NUMBER_OF_OUTPUTS>
<EXPERIMENT_1_INPUT_1> <EXPERIMENT_1_INPUT_2> ... <EXPERIMENT_1_INPUT_N>
<EXPERIMENT_1_OUTPUT_1> <EXPERIMENT_1_OUTPUT_2> ... <EXPERIMENT_1_OUTPUT_M>
<EXPERIMENT_2_INPUT_1> <EXPERIMENT_2_INPUT_2> ... <EXPERIMENT_2_INPUT_N>
<EXPERIMENT_2_OUTPUT_1> <EXPERIMENT_2_OUTPUT_2> ... <EXPERIMENT_2_OUTPUT_M>
...
<EXPERIMENT_S_INPUT_1> <EXPERIMENT_S_INPUT_2> ... <EXPERIMENT_S_INPUT_N>
<EXPERIMENT_S_OUTPUT_1> <EXPERIMENT_S_OUTPUT_2> ... <EXPERIMENT_S_OUTPUT_M>
```

In our example, the `xor.data` file's contents is shown below. The symbol `-1` represents the logical value `0` whereas `1` represents a logical `1`:

```
4 2 1
-1 -1
-1
-1 1
1
1 -1
1
1 1
-1
```

To train our network we will use the sigmoid symmetric activation function (FANN_SIGMOID_SYMMETRIC) in both hidden and output layers. The desired error is set to 0.001 and the maximum number of training epochs is set to 500000. On the other hand, we will be reported about the training status every 1000 epochs.

```
$ fannc create_std 2 3 1 | fannc setup_training --hidden-activation-function=FANN_SIGMOID_SYMMETRIC --output-activation-function=FANN_SIGMOID_SYMMETRIC | fannc train --training-data=xor.data --max-epochs=500000 --target-error=0.001 --report-period=1000 > xor.net

Epochs            1. MSE: 0.25060. Desired-MSE: 0.00100
Epochs           21. MSE: 0.00053. Desired-MSE: 0.00100
```

At this point the file `xor.net` contains an ANN able to compute the XOR of two values. Let's test the ANN with the same training data file to see the resulting MSE:

```
$ fannc test --ann=xor.net --test-data=xor.data
0.000730
```

We can also test for a single experiment by passing values through the command line:

```
$ fannc test --ann=xor.net -i -1 -i -1 -o -1
0.000050
```

Finally, let's run the ANN for values passed through the command line. The command dumps the computed output for the given values (we also can supply input values through a file; please refer to usage manual).

```
$ fannc run --ann=xor.net -i -1 -i -1
-0.985807

$ fannc run --ann=xor.net -i -1 -i 1
0.985583
```

### Acknowledgements
(English)
Fannc is extensively used by the processing engine of time and attendance software by [Imesd Electronica](https://imesd.es), so I appreciate they used this program and helped for improving it.

(Spanish)
Fannc se usa extensivamente en el motor de procesado del software de control horario de [Imesd Electronica](https://imesd.es), a la que agradezco que usaran este programa y me ayudaran a mejorarlo



