#include <synchrolib/utils/general.hpp>
#include <synchrolib/data_structures/subset.hpp>
#include <synchrolib/utils/timer.hpp>
#include <synchrolib/utils/vector.hpp>
#include <synchrolib/data_structures/cuda/subsets_checker_kernel.hpp>

#include <memory>
#include <algorithm>


namespace synchrolib {

class SubsetsChecker {
public:
    template<uint N>
    bool check_subset_intersection(const FastVector<Subset<N>>& small, const FastVector<Subset<N>>& large) {
        Timer timer("check_subset_intersection");

        assert(!small.empty() && !large.empty());
        constexpr auto buckets = Subset<N>::buckets();

        auto small_data = get_data(small);
        auto large_data = get_data(large);

        auto results_array = check_subset_intersection_kernel(small_data.get(), large_data.get(), small.size(), large.size(), buckets, 1);
        return std::any_of(results_array.get(), results_array.get() + small.size(), [](bool x){ return x; });
    }

    template<uint N>
    void compress_subsets(FastVector<Subset<N>>& vec) {
        Timer timer("compress_subsets");

        assert(!vec.empty());
        constexpr auto buckets = Subset<N>::buckets();

        sort_keep_unique(vec);

        auto data = get_data(vec);
        auto results_array = check_subset_intersection_kernel(data.get(), data.get(), vec.size(), vec.size(), buckets, 2);

        FastVector<Subset<N>> new_vec;
        for (uint i = 0; i < vec.size(); ++i) {
            if (!results_array[i]) {
                new_vec.push_back(vec[i]);
            }
        }

        vec = std::move(new_vec);
    }

    ~SubsetsChecker() {
        reset_cuda_device_impl();
    }

private:
    std::unique_ptr<bool[]> check_subset_intersection_kernel(const uint64* small, const uint64* large, int small_cnt, int large_cnt, int buckets, int thresh) {
        return check_subset_intersection_kernel_impl(small, large, small_cnt, large_cnt, buckets, thresh);
    }

    template<uint N>
    std::unique_ptr<uint64[]> get_data(const FastVector<Subset<N>>& vec) {
        constexpr auto buckets = Subset<N>::buckets();
        auto data = std::make_unique<uint64[]>(buckets * vec.size());
        for (uint i = 0; i < vec.size(); ++i) {
            for (uint b = 0; b < buckets; ++b) {
                data[i * buckets + b] = vec[i].v[b];
            }
        }
        return data;
    }
};


} // namespace synchrolib
