#pragma once

#include <sys/mman.h>

#include "../common/memory.hpp"
#include "allocator_base.hpp"

namespace uwr::mem {

using std::true_type;
using std::false_type;

template<class T>
class big_allocator : public allocator_base<big_allocator<T>, T> {
public:
    using base_t = allocator_base<big_allocator<T>, T>;
    using size_type = typename base_t::size_type;
    using value_type = T;
    using pointer = typename base_t::pointer;

    constexpr big_allocator() noexcept;
    constexpr explicit big_allocator(size_type n);

    UWR_FORCEINLINE constexpr size_type fix_capacity(size_type n) const;
    UWR_FORCEINLINE constexpr pointer alloc(size_type n) const;
    UWR_FORCEINLINE constexpr void dealloc(pointer data, size_type n) const;
    UWR_FORCEINLINE constexpr void realloc(size_type req);
    UWR_FORCEINLINE constexpr void grow(size_type req);
    UWR_FORCEINLINE constexpr bool expand_or_dealloc_and_alloc_raw(size_type req);

private:
    UWR_FORCEINLINE constexpr size_type next_capacity(size_type req) const; 
    constexpr void do_realloc(size_type req, true_type);
    constexpr void do_realloc(size_type req, false_type);
    constexpr bool do_expand_or_dealloc_and_alloc_raw(size_type req, true_type);
    constexpr bool do_expand_or_dealloc_and_alloc_raw(size_type req, false_type);
};

template<class T>
constexpr
big_allocator<T>::big_allocator() noexcept
    : base_t() {}

template<class T>
constexpr
big_allocator<T>::big_allocator(size_type n)
    : base_t(n) {}

template<class T>
constexpr typename big_allocator<T>::size_type
big_allocator<T>::fix_capacity(size_type n) const {
#ifndef NDEBUG
    if (!n) return 0;
#endif
    return ((n * sizeof(T) + page_size - 1) / page_size)
            * page_size / sizeof(T);
}

template<class T>
constexpr typename big_allocator<T>::pointer
big_allocator<T>::alloc(size_type n) const {
    UWR_ASSERT(n == this->fix_capacity(n));

    return (pointer)mmap(NULL, n * sizeof(T),
                    PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS,
                    -1, 0);
}

template<class T>
constexpr void
big_allocator<T>::dealloc(pointer data, size_type n) const {
    UWR_ASSERT(n == this->fix_capacity(n));
    
    munmap(data, n * sizeof(T));
}

template<class T>
constexpr void
big_allocator<T>::realloc(size_type req) {
    req = this->fix_capacity(req);

    if (UWR_LIKELY(!!this->m_capacity)) {
        this->do_realloc(req,
                is_trivially_relocatable<T>());
    }
    else {
        this->m_data = this->alloc(req);
    }

    this->m_capacity = req;
}

template<class T>
constexpr void
big_allocator<T>::do_realloc(size_type req, true_type) {
    UWR_ASSERT(req > this->m_capacity);
    UWR_ASSERT(req == this->fix_capacity(req));

    this->m_data = (pointer)mremap(
            (void*)this->m_data,
            this->m_capacity * sizeof(T),
            req * sizeof(T),
            MREMAP_MAYMOVE);
}

template<class T>
constexpr void
big_allocator<T>::do_realloc(size_type req, false_type) {
    UWR_ASSERT(req > this->m_capacity);
    UWR_ASSERT(req == this->fix_capacity(req));

    pointer new_data = (pointer)mremap(
            this->m_data,
            this->m_capacity * sizeof(T),
            req * sizeof(T), 0);

    if (new_data == (pointer)-1) {
        new_data = this->alloc(req);
        umove_and_destroy(new_data, this->m_data, this->m_size);
        this->dealloc(this->m_data, this->m_capacity);
    }

    this->m_data = new_data;
}

template<class T>
constexpr void
big_allocator<T>::grow(size_type req) {
    this->realloc(this->next_capacity(req));
}

template<class T>
constexpr typename big_allocator<T>::size_type
big_allocator<T>::next_capacity(size_type req) const {
    return std::max(2 * this->m_capacity, req);
}

template<class T>
constexpr bool
big_allocator<T>::expand_or_dealloc_and_alloc_raw(size_type req) {
    UWR_ASSERT(req > this->m_capacity);

    req = this->fix_capacity(req);

    if (UWR_LIKELY(!!this->m_data)) {
        return this->do_expand_or_dealloc_and_alloc_raw(req,
                is_trivially_relocatable<T>());
    }
    else {
        this->m_data = this->alloc(req);
        this->m_capacity = req;

        return false;
    }
}

template<class T>
constexpr bool
big_allocator<T>::do_expand_or_dealloc_and_alloc_raw(size_type req, true_type) {
    UWR_ASSERT(req > this->m_capacity);
    UWR_ASSERT(req == this->fix_capacity(req));

    destroy(this->m_data, this->m_size);

    this->m_data = (pointer)mremap(
            (void*)this->m_data,
            this->m_capacity * sizeof(T),
            req * sizeof(T),
            MREMAP_MAYMOVE);

    this->m_capacity = req;

    // always return false as these objects are trivial
    // NOTE: assumption is that you don't care about
    //       objects under `data` address, so return
    //       always false so that caller thinks they
    //       are gone
    return false;
}

template<class T>
constexpr bool
big_allocator<T>::do_expand_or_dealloc_and_alloc_raw(size_type req, false_type) {
    UWR_ASSERT(req > this->m_capacity);
    UWR_ASSERT(req == this->fix_capacity(req));

    void* new_data = mremap(
            this->m_data,
            this->m_capacity * sizeof(T),
            req * sizeof(T), 0);

    if (new_data == (void*)-1) {
        destroy(this->m_data, this->m_size);
        this->dealloc(this->m_data, this->m_capacity);

        this->m_data = this->alloc(req);
        this->m_capacity = req;

        return false;
    }
    else {
        UWR_ASSERT((pointer)new_data == this->m_data);
        this->m_capacity = req;

        return true;
    }
}

} // namespace uwr::mem
