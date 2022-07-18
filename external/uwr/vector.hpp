#pragma once

#include <iterator>
#include <limits>

#include "common/memory.hpp"
#include "common/define.hpp"
#include "common/proxy.hpp"
#include "common/type_traits.hpp"
#include "common/growth_factor.hpp"
#if CPP_ABOVE_17
#include "common/synth_three_way.hpp"
#endif
#include "allocator/hybrid_allocator.hpp"

namespace uwr {

template<class T, class GF = gf_default, class A = mem::hybrid_allocator<T>>
class vector {
public:
   using value_type = T;
   using allocator_type = A;
   using size_type = typename allocator_type::size_type;
   using difference_type = std::ptrdiff_t;
   using reference = T&;
   using const_reference = const T&;
   using pointer = T*;
   using const_pointer = const T*;
   using iterator = pointer;
   using const_iterator = const_pointer;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   constexpr vector() noexcept;
   constexpr explicit vector(size_type n);
   constexpr vector(size_type n, const T& val);
   template<class InIt, class = typename std::iterator_traits<InIt>::value_type>
   constexpr vector(InIt first, InIt last);
   constexpr vector(const vector& x);
   template<class GF_, class A_>
   constexpr vector(const vector<T, GF_, A_>& x);
   constexpr vector(vector&& x) noexcept;
   template<class GF_>
   constexpr vector(vector<T, GF_, A>&& x) noexcept;
   constexpr vector(std::initializer_list<T> ilist);
   #if CPP_ABOVE_17
   constexpr
   #endif
   ~vector();

   UWR_FORCEINLINE constexpr vector& operator=(const vector& other) noexcept;
   template<class GF_, class A_>
   UWR_FORCEINLINE constexpr vector& operator=(const vector<T, GF_, A_>& other) noexcept;
   UWR_FORCEINLINE constexpr vector& operator=(vector&& other) noexcept;
   template<class GF_>
   UWR_FORCEINLINE constexpr vector& operator=(vector<T, GF_, A>&& other) noexcept;
   UWR_FORCEINLINE constexpr vector& operator=(std::initializer_list<T> ilist) noexcept;

   UWR_FORCEINLINE constexpr iterator begin() noexcept;
   UWR_FORCEINLINE constexpr const_iterator begin() const noexcept;
   UWR_FORCEINLINE constexpr iterator end() noexcept;
   UWR_FORCEINLINE constexpr const_iterator end() const noexcept;
   UWR_FORCEINLINE constexpr reverse_iterator rbegin() noexcept;
   UWR_FORCEINLINE constexpr const_reverse_iterator rbegin() const noexcept;
   UWR_FORCEINLINE constexpr reverse_iterator rend() noexcept;
   UWR_FORCEINLINE constexpr const_reverse_iterator rend() const noexcept;
   UWR_FORCEINLINE constexpr const_iterator cbegin() const noexcept;
   UWR_FORCEINLINE constexpr const_iterator cend() const noexcept;
   UWR_FORCEINLINE constexpr const_reverse_iterator crbegin() const noexcept;
   UWR_FORCEINLINE constexpr const_reverse_iterator crend() const noexcept;

   UWR_FORCEINLINE constexpr size_type size() const noexcept;
   UWR_FORCEINLINE constexpr size_type max_size() const noexcept;
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr void resize(size_type n);
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr void resize(size_type n, const T& val);
   UWR_FORCEINLINE constexpr size_type capacity() const noexcept;
   [[nodiscard]] UWR_FORCEINLINE constexpr bool empty() const noexcept;
   UWR_FORCEINLINE constexpr void reserve(size_type n) noexcept;
   UWR_FORCEINLINE constexpr void shrink_to_fit() noexcept;

   UWR_FORCEINLINE constexpr reference operator[](size_type n);
   UWR_FORCEINLINE constexpr const_reference operator[](size_type n) const;
   UWR_FORCEINLINE constexpr reference at(size_type n);
   UWR_FORCEINLINE constexpr const_reference at(size_type n) const;
   UWR_FORCEINLINE constexpr reference front();
   UWR_FORCEINLINE constexpr const_reference front() const;
   UWR_FORCEINLINE constexpr reference back();
   UWR_FORCEINLINE constexpr const_reference back() const;
   UWR_FORCEINLINE constexpr T* data() noexcept;
   UWR_FORCEINLINE constexpr const T* data() const noexcept;

   template<class InIt, class = typename std::iterator_traits<InIt>::value_type>
   UWR_FORCEINLINE constexpr void assign(InIt first, InIt last);
   UWR_FORCEINLINE constexpr void assign(size_type n, const T& val);
   UWR_FORCEINLINE constexpr void assign(std::initializer_list<T> ilist);
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr void push_back(const T& value);
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr void push_back(T&& value);
   UWR_FORCEINLINE constexpr void fast_push_back(T&& value) noexcept;
   UWR_FORCEINLINE constexpr void pop_back();
   UWR_FORCEINLINE constexpr bool safe_pop_back() noexcept;

   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, const T& value);
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, T&& value);
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, size_type count, const T& value);
   template<class GF_ = GF, class InIt, class = typename std::iterator_traits<InIt>::value_type>
   UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, InIt first, InIt last);
   template<class GF_ = GF>
   UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist);
   constexpr iterator erase(const_iterator pos);
   constexpr iterator erase(const_iterator first, const_iterator last);

   template<class GF_>
   UWR_FORCEINLINE constexpr void swap(vector<T, GF_, A>& other);
   UWR_FORCEINLINE constexpr void clear() noexcept;

   template<class GF_ = GF, class... Args>
   constexpr iterator emplace(const_iterator pos, Args&&... args);
   template<class GF_ = GF, class... Args>
   constexpr reference emplace_back(Args&&... args);
   template<class... Args>
   UWR_FORCEINLINE constexpr reference fast_emplace_back(Args&&... args) noexcept;

private:
   template<class InIt>
   UWR_FORCEINLINE constexpr void priv_copy_assign(InIt first, InIt last, size_type n);
   template<class GF_, class InIt>
   UWR_FORCEINLINE constexpr iterator priv_copy_insert(const_iterator pos, InIt first, InIt last, size_type n);

   template<class GF_, class ResizeProxy>
   constexpr void priv_resize(ResizeProxy&& proxy);
   template<class AssignProxy>
   constexpr void priv_assign(AssignProxy&& proxy);
   template<class GF_, class InsertProxy>
   constexpr iterator priv_insert(const_iterator pos, InsertProxy&& proxy);

   template<class T_, class GF_, class A_>
   friend class vector;

private:
   allocator_type m_alloc;
};

template<class T, class GF, class A>
constexpr
vector<T, GF, A>::vector() noexcept
   : m_alloc() {}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>::vector(size_type n)
   : m_alloc(n) {
   mem::construct(data(), n);
}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>::vector(size_type n, const T& val)
   : m_alloc(n) {
   mem::ufill(data(), n, val);
}

template<class T, class GF, class A>
template<class InIt, class>
constexpr
vector<T, GF, A>::vector(InIt first, InIt last)
   : m_alloc(std::distance(first, last)) {
   mem::ucopy(data(), first, last);
}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>::vector(const vector& x)
   : m_alloc(x.size()) {
   mem::ucopy(data(), x.begin(), x.size());
}

template<class T, class GF, class A>
template<class GF_, class A_>
constexpr
vector<T, GF, A>::vector(const vector<T, GF_, A_>& x)
   : m_alloc(x.size()) {
   mem::ucopy(data(), x.begin(), x.size());
}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>::vector(vector&& x) noexcept
   : m_alloc(std::move(x.m_alloc)) {}

template<class T, class GF, class A>
template<class GF_>
constexpr
vector<T, GF, A>::vector(vector<T, GF_, A>&& x) noexcept
   : m_alloc(std::move(x.m_alloc)) {}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>::vector(std::initializer_list<T> ilist)
   : m_alloc(ilist.size()) {
   mem::ucopy(data(), ilist.begin(), ilist.size());
}

template<class T, class GF, class A>
#if CPP_ABOVE_17
constexpr
#endif
vector<T, GF, A>::~vector() {
   mem::destroy(data(), size());
}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>& vector<T, GF, A>::operator=(const vector& other) noexcept {
   if (UWR_LIKELY(this != &other))
      priv_copy_assign(other.begin(), other.end(), other.size());
   return *this;
}

template<class T, class GF, class A>
template<class GF_, class A_>
constexpr
vector<T, GF, A>& vector<T, GF, A>::operator=(const vector<T, GF_, A_>& other) noexcept {
   if (UWR_LIKELY(this != reinterpret_cast<const vector*>(&other)))
      priv_copy_assign(other.begin(), other.end(), other.size());
   return *this;
}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>& vector<T, GF, A>::operator=(vector&& other) noexcept {
   if (UWR_LIKELY(this != &other))
      m_alloc = std::move(other.m_alloc);
   return *this;
}

template<class T, class GF, class A>
template<class GF_>
constexpr
vector<T, GF, A>& vector<T, GF, A>::operator=(vector<T, GF_, A>&& other) noexcept {
   if (UWR_LIKELY(this != reinterpret_cast<vector*>(&other)))
      m_alloc = std::move(other.m_alloc);
   return *this;
}

template<class T, class GF, class A>
constexpr
vector<T, GF, A>& vector<T, GF, A>::operator=(std::initializer_list<T> ilist) noexcept {
   priv_copy_assign(ilist.begin(), ilist.end(), ilist.size());
   return *this;
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::begin() noexcept {
   return iterator(data());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_iterator
vector<T, GF, A>::begin() const noexcept {
   return cbegin();
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::end() noexcept {
   return iterator(data() + size());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_iterator
vector<T, GF, A>::end() const noexcept {
   return cend();
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::reverse_iterator
vector<T, GF, A>::rbegin() noexcept {
   return reverse_iterator(end());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reverse_iterator
vector<T, GF, A>::rbegin() const noexcept {
   return crbegin();
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::reverse_iterator
vector<T, GF, A>::rend() noexcept {
   return reverse_iterator(begin());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reverse_iterator
vector<T, GF, A>::rend() const noexcept {
   return crend();
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_iterator
vector<T, GF, A>::cbegin() const noexcept {
   return const_iterator(data());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_iterator
vector<T, GF, A>::cend() const noexcept {
   return const_iterator(data() + size());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reverse_iterator
vector<T, GF, A>::crbegin() const noexcept {
   return const_reverse_iterator(end());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reverse_iterator
vector<T, GF, A>::crend() const noexcept {
   return const_reverse_iterator(begin());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::size_type
vector<T, GF, A>::size() const noexcept {
   return m_alloc.m_size;
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::size_type
vector<T, GF, A>::max_size() const noexcept {
   return std::numeric_limits<size_type>::max() / sizeof(T);
}

template<class T, class GF, class A>
template<class GF_>
constexpr void
vector<T, GF, A>::resize(size_type n) {
   priv_resize<GF_>(default_construct_proxy<T>(n));
}

template<class T, class GF, class A>
template<class GF_>
constexpr void
vector<T, GF, A>::resize(size_type n, const T& val) {
   priv_resize<GF_>(uninitialized_fill_proxy(n, val));
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::size_type
vector<T, GF, A>::capacity() const noexcept {
   return m_alloc.m_capacity;
}

template<class T, class GF, class A>
constexpr bool
vector<T, GF, A>::empty() const noexcept {
   return size() == 0;
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::reserve(size_type n) noexcept {
   if (UWR_LIKELY(n > capacity()))
      m_alloc.expand(n);
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::shrink_to_fit() noexcept {
   if (UWR_LIKELY(size() != capacity()))
      m_alloc.shrink(size());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::reference
vector<T, GF, A>::operator[](size_type n) {
   return data()[n];
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reference
vector<T, GF, A>::operator[](size_type n) const {
   return data()[n];
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::reference
vector<T, GF, A>::at(size_type n) {
   if (UWR_LIKELY(n < size()))
      return data()[n];
   else
      throw std::out_of_range("Index out of range: " + std::to_string(n));
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reference
vector<T, GF, A>::at(size_type n) const {
   if (UWR_LIKELY(n < size()))
      return data()[n];
   else
      throw std::out_of_range("Index out of range: " + std::to_string(n));
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::reference
vector<T, GF, A>::front() {
   return *data();
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reference
vector<T, GF, A>::front() const {
   return *data();
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::reference
vector<T, GF, A>::back() {
   return data()[size() - 1];
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::const_reference
vector<T, GF, A>::back() const {
   return data()[size() - 1];
}

template<class T, class GF, class A>
constexpr T*
vector<T, GF, A>::data() noexcept {
   return m_alloc.m_data;
}

template<class T, class GF, class A>
constexpr const T*
vector<T, GF, A>::data() const noexcept {
   return m_alloc.m_data;
}

template<class T, class GF, class A>
template<class InIt, class>
constexpr void
vector<T, GF, A>::assign(InIt first, InIt last) {
   priv_copy_assign(first, last,
         static_cast<size_type>(std::distance(first, last)));
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::assign(size_type n, const T& val) {
   priv_assign(assign_fill_proxy(n, val));
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::assign(std::initializer_list<T> ilist) {
   priv_copy_assign(ilist.begin(), ilist.end(), ilist.size());
}

template<class T, class GF, class A>
template<class GF_>
constexpr void
vector<T, GF, A>::push_back(const_reference value) {
   emplace_back<GF_>(value);
}

template<class T, class GF, class A>
template<class GF_>
constexpr void
vector<T, GF, A>::push_back(T&& value) {
   emplace_back<GF_>(std::move(value));
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::fast_push_back(T&& value) noexcept {
   fast_emplace_back(std::forward<T>(value));
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::pop_back() {
   mem::destroy_at(data() + --m_alloc.m_size);
}

template<class T, class GF, class A>
constexpr bool
vector<T, GF, A>::safe_pop_back() noexcept {
   if (UWR_LIKELY(!empty())) {
      pop_back();
      return true;
   }
   else {
      return false;
   }
}

template<class T, class GF, class A>
template<class GF_>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::insert(const_iterator pos, const T& value) {
   return emplace<GF_>(pos, value);
}

template<class T, class GF, class A>
template<class GF_>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::insert(const_iterator pos, T&& value) {
   return emplace<GF_>(pos, std::move(value));
}

template<class T, class GF, class A>
template<class GF_>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::insert(const_iterator pos, size_type count, const T& value) {
   return priv_insert<GF_>(pos, insert_fill_proxy(count, value));
}

template<class T, class GF, class A>
template<class GF_, class InIt, class>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::insert(const_iterator pos, InIt first, InIt last) {
   return priv_copy_insert<GF_>(pos, first, last,
       static_cast<size_type>(std::distance(first, last)));
}

template<class T, class GF, class A>
template<class GF_>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::insert(const_iterator pos, std::initializer_list<T> ilist) {
   return priv_copy_insert<GF_>(pos, ilist.begin(), ilist.end(), ilist.size());
}

template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::erase(const_iterator pos) {
   T* const m_pos = const_cast<T* const>(pos);
   mem::shiftl(m_pos, m_pos + 1, end());
   pop_back();

   return m_pos;
}

#if 0 // TODO: check cache unfriendly
template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::erase(const_iterator first, const_iterator last) {
   T* const m_first = const_cast<T* const>(first);
   T* const m_last = const_cast<T* const>(last);

   if (UWR_LIKELY(m_first != m_last)) {
      T* const m_end = end();
      mem::destroy(mem::shiftl(m_first, m_last, m_end),
                   m_end);
      m_alloc.m_size -= static_cast<size_type>(std::distance(m_first, m_last));
   }

   return m_first;
}
#else // TODO: check cache friendly
template<class T, class GF, class A>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::erase(const_iterator first, const_iterator last) {
   T* const m_first = const_cast<T* const>(first);
   T* const m_last = const_cast<T* const>(last);
   if (UWR_LIKELY(m_first != m_last)) {
      T* const m_end = end();
      size_type count = static_cast<size_type>(std::distance(m_first, m_last));
      T* const new_end = m_end - count;

      if (new_end >= m_last) {
         mem::move_and_destroy(mem::move(m_first, m_last, new_end),
                               new_end, m_end);
      }
      else {
         mem::destroy(mem::shiftl(m_first, m_last, m_end),
                      m_end);
      };

      m_alloc.m_size -= count;
   }

   return m_first;
}
#endif

template<class T, class GF, class A>
template<class GF_>
constexpr void
vector<T, GF, A>::swap(vector<T, GF_, A>& other) {
   m_alloc.swap(other.m_alloc);
}

template<class T, class GF, class A>
constexpr void
vector<T, GF, A>::clear() noexcept {
   mem::destroy(data(), size());
   m_alloc.m_size = 0;
}

template<class T, class GF, class A>
template<class GF_, class... Args>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::emplace(const_iterator pos, Args&&... args) {
   T* m_pos = const_cast<T*>(pos);
   if (size() == capacity()) {
      size_type m = m_pos - data();
      m_alloc.template grow<GF_>(size() + 1);
      m_pos = data() + m;
   }

   T* const m_end = end();
   if (m_pos != m_end) {
      mem::shiftr(m_pos + 1, m_pos, m_end);
      *m_pos = mem::create<T>(std::forward<Args>(args)...);
   }
   else {
      new (m_pos) T(std::forward<Args>(args)...);
   }

   ++m_alloc.m_size;

   return m_pos;
}

template<class T, class GF, class A>
template<class GF_, class... Args>
constexpr typename vector<T, GF, A>::reference
vector<T, GF, A>::emplace_back(Args&&... args) {
   if (UWR_UNLIKELY(size() == capacity()))
      m_alloc.template grow<GF_>(size() + 1);
   return fast_emplace_back(std::forward<Args>(args)...);
}

template<class T, class GF, class A>
template<class... Args>
constexpr typename vector<T, GF, A>::reference
vector<T, GF, A>::fast_emplace_back(Args&&... args) noexcept {
   T* const last_end = data() + m_alloc.m_size++;
   new (last_end) T(std::forward<Args>(args)...);
   return *last_end;
}

template<class T, class GF, class A>
template<class InIt>
constexpr void
vector<T, GF, A>::priv_copy_assign(InIt first, InIt last, size_type n) {
   priv_assign(copy_assign_range_proxy<T, InIt>(first, last, n));
}

template<class T, class GF, class A>
template<class GF_, class InIt>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::priv_copy_insert(const_iterator pos, InIt first, InIt last, size_type n) {
   return priv_insert<GF_>(pos, insert_copy_range_proxy<T, InIt>(first, last, n));
}

template<class T, class GF, class A>
template<class AssignProxy>
constexpr void
vector<T, GF, A>::priv_assign(AssignProxy&& proxy) {
   if (UWR_UNLIKELY(proxy.n > capacity())) {
      if (m_alloc.expand_or_dealloc_and_alloc_raw(proxy.n))
         proxy.assign_to_short(data(), size());
      else
         proxy.assign_to_short(data(), 0);
   }
   else if (proxy.n > size())
      proxy.assign_to_short(data(), size());
   else
      proxy.assign_to_long(data(), size());
   m_alloc.m_size = proxy.n;
}

template<class T, class GF, class A>
template<class GF_, class ResizeProxy>
constexpr void
vector<T, GF, A>::priv_resize(ResizeProxy&& proxy) {
   if (proxy.n > capacity()) {
      m_alloc.template grow<GF_>(proxy.n);
      proxy.construct(end(), data() + proxy.n);
   }
   else {
      T* const m_end = end();
      T* const new_end = data() + proxy.n;

      if (proxy.n > size())
         proxy.construct(m_end, new_end);
      else
         mem::destroy(new_end, m_end);
   }
   m_alloc.m_size = proxy.n;
}

template<class T, class GF, class A>
template<class GF_, class InsertProxy>
constexpr typename vector<T, GF, A>::iterator
vector<T, GF, A>::priv_insert(const_iterator pos, InsertProxy&& proxy) {
   T* m_pos = const_cast<T*>(pos);
   if (UWR_UNLIKELY(!proxy.count))
      return m_pos;

   size_type new_size = size() + proxy.count;
   if (new_size > capacity()) {
      size_type m = m_pos - data();
      m_alloc.template grow<GF_>(new_size);
      m_pos = data() + m;
   }

   T* const m_end = end();
   T* const spill = m_pos + proxy.count;
   if (spill < m_end) {
      mem::shiftr(spill, m_pos, m_end);
      proxy.insert_without_spill(m_pos, spill);
   }
   else {
      proxy.insert_with_spill(m_pos, m_end, spill);
   }

   m_alloc.m_size = new_size;

   return m_pos;
}

// uwr::vector is trivially relocatable
template<class T, class GF, class A>
inline constexpr bool
uwr::mem::is_trivially_relocatable_v<uwr::vector<T, GF, A>> = true;


/*
 * non-member operators 
 */
template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr bool
operator==(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return lhs.size() == rhs.size() &&
      std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr bool
operator!=(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return !(lhs == rhs);
}

template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr bool
operator<(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return std::lexicographical_compare(lhs.begin(), lhs.end(),
                                       rhs.begin(), rhs.end());
}

template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr bool
operator<=(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return !(rhs < lhs);
}

template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr bool
operator>(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return rhs < lhs;
}

template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr bool
operator>=(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return !(lhs < rhs);
}

#if CPP_ABOVE_17

template<class T, class GF1, class GF2, class A1, class A2>
UWR_FORCEINLINE constexpr auto
operator<=>(const vector<T, GF1, A1>& lhs, const vector<T, GF2, A2>& rhs) {
   return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(),
                                                 rhs.begin(), rhs.end(),
                                                 mem::synth_three_way{});
}

#endif

} // namespace uwr


/*
 * non-member functions
 */
namespace std {

template<class T, class GF, class A>
constexpr void
swap(uwr::vector<T, GF, A>& x, uwr::vector<T, GF, A>& y) {
   x.swap(y);
}

template<class T, class GF1, class GF2, class A>
constexpr void
swap(uwr::vector<T, GF1, A>& x, uwr::vector<T, GF2, A>& y) {
   x.swap(y);
}

#if CPP_ABOVE_17

template<class T, class GF, class A, class U>
constexpr typename uwr::vector<T, GF, A>::size_type
erase(uwr::vector<T, GF, A>& c, const U& value) {
   T* const c_end = c.end();
   T* const mid = std::remove(c.begin(), c_end, value);
   c.erase(mid, c_end);

   return static_cast<size_t>(std::distance(mid, c_end));
}

template<class T, class GF, class A, class Pred>
constexpr typename uwr::vector<T, GF, A>::size_type
erase_if(uwr::vector<T, GF, A>& c, Pred pred) {
   T* const c_end = c.end();
   T* const mid = std::remove_if(c.begin(), c_end, pred);
   c.erase(mid, c_end);

   return static_cast<size_t>(std::distance(mid, c_end));
}

#endif // CPP_ABOVE_17

} // namespace std

