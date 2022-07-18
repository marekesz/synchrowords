#pragma once

#include "../common/type_traits.hpp"
#include "../common/memory.hpp"

namespace uwr::mem {

template<class Allocator, class T>
class allocator_base {
protected:
   using allocator_type = Allocator;
   using size_type = len_t;
   using value_type = T;
   using pointer = value_type*;

protected:
   constexpr
   allocator_base() noexcept
      : m_size(0),
        m_capacity(0),
        m_data(nullptr) {}

   constexpr explicit
   allocator_base(size_type n)
      : m_size(n),
        m_capacity(deriv()->fix_capacity(m_size)),
        m_data(deriv()->alloc(m_capacity)) {}

   constexpr
   allocator_base(allocator_base&& x)
      : m_size(x.m_size),
        m_capacity(x.m_capacity),
        m_data(x.m_data) {
      x.m_size = 0;
      x.m_capacity = 0;
      x.m_data = nullptr;
   }

#if CPP_ABOVE_17
   constexpr
#endif
   ~allocator_base() {
      deriv()->dealloc(m_data, m_capacity);
   }

   UWR_FORCEINLINE constexpr
   allocator_base& operator=(allocator_base&& other) {
      swap(other);
      return *this;
   }

public:
   UWR_FORCEINLINE constexpr
   void swap(allocator_base& other) {
      std::swap(m_size, other.m_size);
      std::swap(m_capacity, other.m_capacity);
      std::swap(m_data, other.m_data);
   }

   /*
   * interface to implement in
   * derived allocators
   */
   size_type fix_capacity(size_type n);
   pointer alloc(size_type n) const;
   void dealloc(pointer data, size_type n) const;
   void expand(size_type req);
   void shrink(size_type req);
   template<class GF>
   void grow(size_type req);
   bool expand_or_dealloc_and_alloc_raw(size_type req);

private:
   UWR_FORCEINLINE constexpr
   allocator_type* deriv() { return reinterpret_cast<allocator_type*>(this); }

public:
   size_type m_size;
   size_type m_capacity;
   pointer m_data;
};

} // namespace uwr::mem
