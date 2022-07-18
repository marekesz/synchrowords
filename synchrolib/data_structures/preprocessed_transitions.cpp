#include "preprocessed_transition.hpp"

#include <synchrolib/data_structures/automaton.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/bits.hpp>
#include <synchrolib/utils/general.hpp>
#include <synchrolib/utils/memory.hpp>
#include <synchrolib/utils/thread_pool.hpp>

#if GPU
#include <synchrolib/data_structures/cuda/preprocessed_transition_kernel.hpp>
#endif

namespace synchrolib {

template <uint N, uint K>
PreprocessedTransition<N, K>::PreprocessedTransition() {}

template <uint N, uint K>
PreprocessedTransition<N, K>::PreprocessedTransition(const Automaton<N, K> &aut, const uint k) {
  for (uint i = 0; i < slices(); i++) {
    for (uint s = 0; s < (1 << PREPROCESSED_TRANSITION_SLICE); s++) {
      Subset<N> set = Subset<N>::Empty();
      uint st = s;
      for (uint j = 0; j < PREPROCESSED_TRANSITION_SLICE; j++) {
        uint n = PREPROCESSED_TRANSITION_SLICE * i + j;
        if (n >= N) break;
        if (st & 1) set.set(aut[n][k]);
        st >>= 1;
      }
      trans[i][s] = set;
    }
  }
}

template <uint N, uint K>
PreprocessedTransition<N, K>::PreprocessedTransition(const InverseAutomaton<N, K> &invaut, const uint k) {
  for (uint i = 0; i < slices(); i++) {
    for (uint s = 0; s < (1 << PREPROCESSED_TRANSITION_SLICE); s++) {
      Subset<N> set = Subset<N>::Empty();
      uint st = s;
      for (uint j = 0; j < PREPROCESSED_TRANSITION_SLICE; j++) {
        uint n = PREPROCESSED_TRANSITION_SLICE * i + j;
        if (n >= N) break;
        if (st & 1)
          for (uint e = invaut.begin(n, k); e < invaut.end(n, k); e++)
            set.set(invaut.edge(e, k));
        st >>= 1;
      }
      trans[i][s] = set;
    }
  }
}

template <uint N, uint K>
void PreprocessedTransition<N, K>::apply(const Subset<N>* from, Subset<N>* to, size_t cnt) const {
  if (!cnt) {
    return;
  }

#if GPU && (GPU_MEMORY > 64)
  size_t bucket_size = ((static_cast<size_t>(GPU_MEMORY) - 32) * 1024 * 1024 - sizeof(*this)) / (2 * sizeof(Subset<N>));
  while (cnt) {
    size_t now = std::min(cnt, bucket_size);
    preprocessed_transition_kernel(*this, from, to, now);
    from += now;
    to += now;
    cnt -= now;
  }
#else
  if (THREADS > 1 && cnt > 256) {
    ThreadPool pool;
    pool.start(THREADS);
    for (size_t t = 0; t < THREADS; ++t) {
      auto it = from + (cnt * t / THREADS);
      auto end = from + (cnt * (t + 1) / THREADS);
      auto out = to + (cnt * t / THREADS);

      pool.add_job([this, it, end, out] () mutable {
        while (it != end) {
          this->apply(*it, *out);
          ++it;
          ++out;
        }
      });
    }
    pool.wait();
  } else {
    for (size_t i = 0; i < cnt; ++i, ++from, ++to) {
      apply(*from, *to);
    }
  }
#endif
}

template class PreprocessedTransition<AUT_N, AUT_K>;

}  // namespace synchrolib
