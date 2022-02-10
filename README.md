This is a computational package with algorithms for finding synchronizing (reset) words of a deterministic finite (semi)automaton.
See surveys about synchronizing automata [1,5,6].

The package contains implementations of heuristic algorithms that find some (possibly short) reset word and the exact exponential algorithm.
Heuristics include algorithms like Eppstein, SynchroPL, and Beam [4].
The exact algorithm is an enhanced version of the fastest known approach based on bidirectional search [2,3].

The implementation is intented to be highly efficient and includes major improvements.
It uses just-in-time compilation and optionally multithreading and GPU (CUDA).

* [Installation and usage](docs/install.md)

* [Configuration files and available algorithms](docs/config.md)

### Main references ###

[1] Jarkko Kari and Mikhail V. Volkov. Černý’s conjecture and the road colouring problem. Handbook of automata, volume 1, pages 525--565. European Mathematical Society Publishing House, 2021. [doi](https://doi.org/10.4171/Automata)

[2] Andrzej Kisielewicz, Jakub Kowalski, and Marek Szykuła. A Fast Algorithm Finding the Shortest Reset Words. In Computing and Combinatorics (COCOON 2013), volume 7936 of LNCS, pages 182--196, Springer, 2013. [doi](https://doi.org/10.1007/978-3-642-38768-5_18)

[3] Andrzej Kisielewicz, Jakub Kowalski, and Marek Szykuła. Computing the shortest reset words of synchronizing automata. Journal of Combinatorial Optimization, 29(1):88--124, 2015. [doi](https://doi.org/10.1007/s10878-013-9682-0)

[4] Adam Roman and Marek Szykuła. Forward and backward synchronizing algorithms. Expert Systems With Applications, 42(24):9512--9527, 2015. [doi](https://doi.org/10.1016/j.eswa.2015.07.071)

[5] Sven Sandberg. Homing and Synchronizing Sequences. Model-Based Testing of Reactive Systems. volume 3472 of LCNS. Springer, 2005. [doi](https://doi.org/10.1007/11498490_2)

[6] Mikhail V. Volkov. Synchronizing automata and the Černý conjecture. In Language and Automata Theory and Applications, volume 5196 of LNCS, pages 11--27. Springer, 2008. [doi](https://doi.org/10.1007/978-3-540-88282-4_4)
