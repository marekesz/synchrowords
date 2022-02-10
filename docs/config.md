# Configuration files

Json configuration files specify computational plans.
A plan describes a sequence of algorithms with their parameters that are run sequentially.

Parameters are either boolean, integer or string.

Integer and boolean parameters can appear in json as a value of their respective type (e.g. `"find_word": true`)
or as a string containing a valid C++ expression (e.g. `"find_word": "AUT_N < 1000 * 1000"`).
The C++ expressions can use `<cmath>` functions and predefined `AUT_N`, `AUT_K` values, which respectively denote the number of states and the size of the alphabet of the given automaton.

The only two exceptions to these rules are the `threads` and `gpu` global parameters, whose values **can not** be C++ expressions.

## Global parameters

* `upper_bound` (integer) (default `"1ULL * AUT_N * AUT_N * AUT_N / 6"`) -- Specifies the initial upper bound on reset threshold.
Algorithms terminate if they do not find a reset word of length within this bound.
Otherwise, the upper bound can be decreased by the algorithms.

* `threads` (integer) (default `1`) -- Specifies the number of threads for parallel computation.

* `gpu` (boolean) (default `false`) -- Enables gpu. Requies CUDA library and nvcc (see [installation guide](docs/install.md))

* `algorithms` (list) -- Specifies the list of algorithms that the plan consists of. Algorithms will be run in the given order.

## Algorithms

Each algorithm in the `algorithms` list is a dictionary with keys:
* `name` -- One of the available algorithms (see the list below).
* `config` -- Dictionary (possibly empty) containing algorithm parameters.

By default algorithms only improve upper and lower bounds on the shortest reset threshold.
Some of them also support the `find_word` parameter, which makes them output the synchronizing word. 

#### `Brute`

This is an exact algorithm for small automata.
It runs BFS in the power automaton, using a bit array for visited sets. Thus, it requires O(2^N) memory.

* `max_n` (integer) (default `20`) -- Specifies the maximum supported number of states.
If the given automaton has more states, the algorithm does nothing.
Must be less or equal to `32`.

#### `Eppstein`

The classic Eppstein algorithm works by running at most (n-1) iterations of a subprocedure, which chooses an arbitrary pair of not yet synchronized states and appends a word that synchronizes them to the result.
The known upper bound on the length of the resulting reset word is cubic in the number of states.

* `transition_tables` (boolean) (default `false`) --   Enables the precomputation of shortcuts in the pairs
  tree. Must be set to `false` if `find_word` is set to true.

* `find_word` (boolean) (default `false`) -- Returns a reset word. If `false`, the algorithm only returns the length and sets the upper bound.

#### `Beam`

Implementation of the beam search algorithm.
It runs inverse BFS in the power automaton.
The initial list consists of all of the singletons.
In each iteration, only `beam_size` best sets are kept (valued by their size).

* `beam_size` (integer) (default `"std::log2(AUT_N)"`)

* `max_iter` (integer) (default `-1`) -- Limits the number of iterations (`-1` for unlimited, i.e., the `upper_bound` on reset threshold).
Though very unlikely for random automata, beam search might run into a loop, especially for small `beam_size`.
In this case, the algorithm would run for `upper_bound` number of iterations, which might be large if no previous algorithm decreased this bound.

* `presort` (string) (default `"none"`) -- One of [`"none"`, `"indeg"`].
If `"indeg"` is specified, the algorithm permutes the indices of the automaton so that they are ordered from lowest to highest in-degree.
Useful if the implementation uses tries.

#### `Exact`

Finds the reset threshold (the length of the shortest reset words) or computes a lower bound.

The algorithm works by first running two BFS algorithms (one starting from the singletons, and one starting from the set of all the states) and checking in each iteration if they met.
If the answer is not found in a certain amount of steps or the memory runs out, it optionally switches to a DFS algorithm.

A good upper bound on the shortest reset threshold will decrease the running time greatly if the `dfs_shortcut` parameter is set to `true`.

* `dfs` (boolean) (default `true`) -- Enables the DFS phase. If set to `false`, after the first phase fails to find the reset threshold, a lower bound is returned.

* `dfs_shortcut` (boolean) (default `true`) -- Enables switching to the DFS phase when the algorithm calculates that it is better. If set to `false`, DFS is only entered after the memory runs out.

* `max_memory_mb` (integer) (default `2048`) -- Maximum amount of used memory in megabytes. If `gpu` is set to true, this limit also applies to the GPU memory.

* `dfs_min_list_size` (integer) (default `10000`) -- The minimum size of the list at each depth during the DFS phase.

* `strict_memory_limit` (boolean) (default `false`) -- Stops the algorithm if there's not enough memory in the DFS phase. If set to `false`, only warnings are printed.

* `bfs_small_list_size` (integer) (default `"AUT_N * 16"`) -- Until both BFS and I-BFS lists reach this size, the algorithm always picks the smaller side to expand and not consider DFS shortcut.

#### `Reduce`

Reduces the number of states of the automaton before entering the Exact algorithm (which is the only algorithm that can be run after Reduce succeeds).
Works by running a small number of BFS iterations and then trying to delete all of the states that will never be reached again (outside the sink component).

* `min_n` (integer) (default `80`) -- Specifies the minimum supported number of states. If the given automaton has fewer states, the reduction is skipped.

* `list_size_threshold` (integer) (default `"AUT_N * 16"`) -- The algorithm tries to reduce the number of states after at least `list_size_threshold` sets are on the BFS list.
