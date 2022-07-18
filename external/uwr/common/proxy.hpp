#pragma once

#include "memory.hpp"

namespace uwr {

template<class T>
class default_construct_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE default_construct_proxy(size_type n)
      : n(n) {}

   UWR_FORCEINLINE void construct(T* begin, T* end) {
      mem::construct(begin, end);
   }

public:
   size_type n;
};

template<class T>
class uninitialized_fill_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE explicit uninitialized_fill_proxy(size_type n, const T& value)
      : value(value), n(n) {}

   UWR_FORCEINLINE void construct(T* begin, T* end) {
      mem::ufill(begin, end, value);
   }

private:
   const T& value;

public:
   size_type n;
};

template<class T, class InIt>
class copy_assign_range_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE copy_assign_range_proxy(InIt first, InIt last, size_type n)
      : first(first), last(last), n(n) {
      UWR_ASSERT(n == static_cast<size_type>(std::distance(first, last)));
   }

   UWR_FORCEINLINE void assign_to_short(T* begin, size_type len) {
      UWR_ASSERT(len < n);

      InIt split = std::next(first, len);
      mem::ucopy(mem::copy(begin, first, split), split, last);
   }

   UWR_FORCEINLINE void assign_to_long(T* begin, size_type len) {
      mem::destroy(mem::copy(begin, first, last), begin + len);
   }

private:
   InIt first;
   InIt last;

public:
   size_type n;
};

template<class T, class InIt>
class move_assign_range_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE move_assign_range_proxy(InIt first, InIt last, size_type n)
      : first(first), last(last), n(n) {
      UWR_ASSERT(n == static_cast<size_type>(std::distance(first, last)));
   }

   UWR_FORCEINLINE void assign_to_short(T* begin, size_type len) {
      UWR_ASSERT(len < n);

      InIt split = std::next(first, len);
      mem::umove(mem::move(begin, first, split), split, last);
   }

   UWR_FORCEINLINE void assign_to_long(T* begin, size_type len) {
      mem::destroy(mem::move(begin, first, last), begin + len);
   }

private:
   InIt first;
   InIt last;

public:
   size_type n;
};

template<class T>
class assign_fill_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE assign_fill_proxy(size_type n, const T& value)
      : value(value), n(n) {}

   UWR_FORCEINLINE void assign_to_short(T* begin, size_type len) {
      UWR_ASSERT(len < n);

      mem::ufill(mem::fill(begin, len, value), begin + n, value);
   }

   UWR_FORCEINLINE void assign_to_long(T* begin, size_type len) {
      mem::destroy(mem::fill(begin, n, value), begin + len);
   }

private:
   const T& value;

public:
   size_type n;
};

template<class T>
class insert_fill_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE insert_fill_proxy(size_type count, const T& value)
      : value(value), count(count) {}

   UWR_FORCEINLINE void insert_without_spill(T* begin, T* end) {
      mem::fill(begin, end, value);
   }

   UWR_FORCEINLINE T* insert_with_spill(T* position, T* end, T* spill) {
      T* new_end = mem::umove_and_fill(spill, position, end, value);
      mem::ufill(end, spill, value);
      return new_end;
   }

private:
   const T& value;

public:
   size_type count;
};

template<class T, class InIt>
class insert_copy_range_proxy {
private:
   using size_type = mem::len_t;

public:
   UWR_FORCEINLINE insert_copy_range_proxy(InIt first, InIt last, size_type count)
      : first(first), last(last), count(count) {
      UWR_ASSERT(count == static_cast<size_type>(std::distance(first, last)));
   }

   UWR_FORCEINLINE void insert_without_spill(T* begin, T*) {
      mem::copy(begin, first, last);
   }

   UWR_FORCEINLINE T* insert_with_spill(T* position, T* end, T* spill) {
      size_type rest = static_cast<size_type>( std::distance(position, end));
      InIt split = std::next(first, rest);
      T* new_end = mem::umove_and_copy(spill, position, end, first, split);
      mem::ucopy(end, split, last);

      return new_end;
   }

private:
   InIt first;
   InIt last;

public:
   size_type count;
};

} // namespace uwr
