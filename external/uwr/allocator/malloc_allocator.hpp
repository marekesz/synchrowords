#pragma once

#include "allocator_base.hpp"
#include "../common/memory.hpp"

namespace uwr::mem {

using std::true_type;
using std::false_type;

template<class T>
class malloc_allocator : public allocator_base<malloc_allocator<T>, T> {
public:
   using base_t = allocator_base<malloc_allocator<T>, T>;
   using size_type = typename base_t::size_type;
   using value_type = T;
   using pointer = typename base_t::pointer;

   constexpr malloc_allocator() noexcept;
   constexpr explicit malloc_allocator(size_type n);

   UWR_FORCEINLINE constexpr size_type fix_capacity(size_type n) const;
   UWR_FORCEINLINE constexpr pointer alloc(size_type n) const;
   UWR_FORCEINLINE constexpr void dealloc(pointer data, size_type n) const;
   UWR_FORCEINLINE constexpr void expand(size_type req);
   UWR_FORCEINLINE constexpr void shrink(size_type req);
   template<class GF>
   UWR_FORCEINLINE constexpr void grow(size_type req);
   constexpr bool expand_or_dealloc_and_alloc_raw(size_type req);

private:
   UWR_FORCEINLINE constexpr T* expand(size_type req, true_type);
   constexpr T* expand(size_type req, false_type);
};

template<class T>
constexpr
malloc_allocator<T>::malloc_allocator() noexcept
   : base_t() {}

template<class T>
constexpr
malloc_allocator<T>::malloc_allocator(size_type n)
   : base_t(n) {}

template<class T>
constexpr typename malloc_allocator<T>::size_type
malloc_allocator<T>::fix_capacity(size_type n) const {
#ifndef NDEBUG
   if (!n) return 0;
#endif
   return std::max((64 + sizeof(T) - 1) / sizeof(T), n);
}

template<class T>
constexpr typename malloc_allocator<T>::pointer
malloc_allocator<T>::alloc(size_type n) const {
   UWR_ASSERT(n == fix_capacity(n));

   return static_cast<T*>(malloc(n * sizeof(T)));
}

template<class T>
constexpr void
malloc_allocator<T>::dealloc(pointer data, UWR_UNUSED size_type n) const {
   UWR_ASSERT(n == fix_capacity(n));

   free(data);
}

template<class T>
constexpr void
malloc_allocator<T>::expand(size_type req) {
   req = fix_capacity(req);
   this->m_data = expand(req, is_trivially_relocatable<T>());
   this->m_capacity = req;
}

template<class T>
constexpr void
malloc_allocator<T>::shrink(size_type req) {
   expand(req);
}

template<class T>
constexpr T*
malloc_allocator<T>::expand(size_type req, true_type) {
   return (pointer)::realloc((void*)this->m_data, req * sizeof(T));
}

template<class T>
constexpr T*
malloc_allocator<T>::expand(size_type req, false_type) {
   pointer new_data = alloc(req);
   umove_and_destroy(new_data, this->m_data, this->m_size);
   dealloc(this->m_data, this->m_capacity);

   return new_data;
}

template<class T>
template<class GF>
constexpr void
malloc_allocator<T>::grow(size_type req) {
   expand(std::max(GF::num * this->m_capacity / GF::den, req));
}

template<class T>
constexpr bool
malloc_allocator<T>::expand_or_dealloc_and_alloc_raw(size_type req) {
   UWR_ASSERT(req > this->m_capacity);

   destroy(this->m_data, this->m_size);
   dealloc(this->m_data, this->m_capacity);

   this->m_capacity = fix_capacity(req);
   this->m_data = alloc(this->m_capacity);

   return false;
}

} // namespace uwr::mem
