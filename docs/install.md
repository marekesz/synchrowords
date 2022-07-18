# Prerequisites

To run the program you first need to install `g++` (with `-pthread` support) and `make`.
On Ubuntu 20.04 you can install all of these with:
```
sudo apt install g++ make libtbb-dev
```

If you want to enable GPU utilization, [cuda](https://developer.nvidia.com/cuda-toolkit)
and [nvcc](https://docs.nvidia.com/cuda/cuda-compiler-driver-nvcc/index.html)  must also be installed.

The following versions were used in testing (higher should be fine):
* g++ 9.3.0
* CUDA 11.4
* nvcc 10.1.243

# Installation

To compile the program call `make`.

Because the program uses just-in-time compilation, even though the first `make` succeeds, it might still fail to run in some/all configurations. The `-v/--verbose` option might be useful for checking what went wrong during runtime.

Failed runs might cause compilation artefacts to be left in the `build/` folder. Delete them with `make clean`.

To use the program from everywhere on your system, consider adding `alias synchro='/path/to/repo/synchro'` to your `~/.bashrc`.

# Usage
Call `synchro --help` to see the following message.
```
synchro [OPTION...]
  -f, --file arg          Path to the input file
  -c, --config arg        Path to the config file
  -o, --output arg        Path to the output file
  -b, --build-suffix arg  Suffix of the build folder (default: )
      --continue          Do not overwrite the output file and run algorithms
                          only for remaining automata
  -v, --verbose           Verbose output
  -q, --quiet             Quiet output (only warnings and errors)
  -d, --debug             Debug output (all messages and timers)
  -h, --help              Print usage
```

### Input file

The input file must contain automata in the following format
```
K N
A_{0, 0} A_{0, 1} ... A_{0, K-1} A{1, 0} A{1, 1} ...
```
where `A_{i, j}` is the result of the transition function on `i`-th state and `j`-th letter. The states and letters indices are zero-based.

The file can contain many inputs.

E.g. valid input (`data/readme_input.txt`):
```
4 1
0 0 0 0
3 4
0 0 0 1 1 1 2 2 2 3 3 3
3 4
0 0 0 1 1 0 2 0 2 0 3 3
3 100
75 66 11 48 17 34 73 59 78 58 36 14 75 46 22 14 66 90 4 72 99 18 71 56 48 11 77 64 75 87 81 87 11 67 3 42 77 49 42 54 68 94 47 35 85 69 50 38 93 61 60 93 66 80 79 26 70 83 33 46 0 65 92 58 20 50 93 87 18 27 99 17 18 44 60 23 76 68 17 10 87 10 14 78 32 3 61 14 56 64 45 91 86 62 66 54 85 24 28 62 96 96 61 61 19 9 38 85 29 3 31 62 99 32 84 70 63 68 73 27 86 23 3 26 98 76 17 26 47 83 45 79 92 16 31 21 73 1 1 18 1 26 62 87 68 36 50 81 3 47 84 97 90 37 2 85 30 65 86 0 36 83 43 41 54 74 81 49 96 91 39 63 96 47 82 82 14 32 47 42 12 49 4 51 80 80 38 18 65 33 94 39 44 75 35 8 85 22 35 72 94 60 31 2 80 57 10 19 35 43 36 98 11 64 20 27 25 5 43 23 50 82 27 82 78 36 24 11 82 38 16 19 77 29 38 27 5 58 72 14 43 25 93 71 10 42 23 45 46 93 84 24 8 78 87 9 88 24 95 1 17 35 10 35 2 21 10 8 10 85 68 66 2 24 63 40 11 11 72 99 99 70 85 72 68 83 21 80 58 83 88 5 53 81 84 22 81 60 92 57
```

### Config
* [Config documentation](docs/config.md)

### Example run

After calling `synchro --config configs/readme_config.json --file data/readme_input.txt -o save.txt` you should see

```
[12:44:47.472] [INFO] Read 4 automata
[12:44:47.472] [INFO] Recompiling for N = 1, K = 4
[12:44:51.991] [INFO] Minimum synchronizing word length: [0, 0]
[12:44:51.991] [INFO] Recompiling for N = 4, K = 3
[12:44:57.479] [WARNING@brute] Automaton is non-synchronizing
[12:44:57.479] [INFO] NON SYNCHRO
[12:44:57.479] [INFO] Loading precompiled library
[12:44:57.479] [INFO@brute] mlsw: 3
[12:44:57.479] [INFO] Minimum synchronizing word length: [3, 3]
[12:44:57.479] [INFO] Recompiling for N = 100, K = 3
[12:45:03.497] [INFO@brute] N > 20, exiting...
[12:45:03.497] [INFO@eppstein] Upper bound: 25
[12:45:03.498] [INFO@beam] Upper bound: 19
[12:45:03.508] [INFO@reduce] Reduced to N = 90 in 8 bfs steps
[12:45:03.508] [INFO] Recompiling for N = 90, K = 3
[12:45:09.605] [INFO@exact] mlsw: 18
[12:45:09.605] [INFO] Minimum synchronizing word length: [18, 18]
[12:45:09.605] [INFO] Saving synchronizing word of length 25
```

The output file will contain information about synchronizing words in the following format
```
ID: [lower_bound, upper_bound] ((algorithm #1, time in ms), (algorithm #2, time in ms), ...) {w_0, w_1, ..., w_k}
```

In our example, the `save.txt` file should look like this
```
0: [0, 0] ((Brute, 0), (Eppstein, 0), (Beam, 0), (Reduce, 0), (Exact, 0))
1: NON SYNCHRO
2: [3, 3] ((Brute, 0), (Eppstein, 0), (Beam, 0), (Reduce, 0), (Exact, 0))
3: [18, 18] ((Brute, 0), (Eppstein, 0), (Beam, 0), (Reduce, 9), (Exact, 62)) {0 0 1 0 1 0 1 0 2 2 1 0 1 2 2 0 0 2 2 0 0 2 0 1 2}
```

The default behavior of every algorithm is to exit if it cannot find a shorter synchronizing word than the predecessors.
That is why in the first three cases, the Eppstein algorithm (which has the `find_word` parameter set to `true`) did not even run and the word was not saved.
In the last case, the saved word has a length greater than 18, because `Beam` and `Exact` do not support the `find_word` parameter.
