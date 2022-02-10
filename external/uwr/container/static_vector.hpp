#pragma once

#include <iterator>

#include "../common/memory.hpp"
#include "../common/minimal_size_type.hpp"
#include "../common/define.hpp"
#include "../common/proxy.hpp"
#if CPP_ABOVE_17
#include "../common/synth_three_way.hpp"
#endif

/* 
* push_back    = fast_push_back
* emplace_back = fast_emplace_back
* by default
*  
* #define UWR_STATIC_VECTOR_OPTIM_ENABLE 0
* to disable this behaviour
*/
#ifndef UWR_STATIC_VECTOR_OPTIM_ENABLE
#define UWR_STATIC_VECTOR_OPTIM_ENABLE 1
#endif

namespace uwr {

using len_t = mem::len_t;

template<class T, len_t C>
class static_vector {
public:
    using value_type = T;
    using size_type = typename mem::minimal_size_type<C>::type;
    using difference_type = std::ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr static_vector() noexcept;
    constexpr explicit static_vector(size_type n);
    constexpr static_vector(size_type n, const T& val);
    template<class InIt, class = typename std::iterator_traits<InIt>::value_type>
    constexpr static_vector(InIt first, InIt last);
    constexpr static_vector(const static_vector& x);
    constexpr static_vector(static_vector&& x) noexcept;
    constexpr static_vector(std::initializer_list<T> ilist);
    #if CPP_ABOVE_17
    constexpr
    #endif
    ~static_vector();

    UWR_FORCEINLINE constexpr static_vector& operator=(const static_vector& other) noexcept;
    UWR_FORCEINLINE constexpr static_vector& operator=(static_vector&& other) noexcept;
    UWR_FORCEINLINE constexpr static_vector& operator=(std::initializer_list<T> ilist) noexcept;

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
    UWR_FORCEINLINE constexpr void resize(size_type n);
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

    UWR_FORCEINLINE constexpr void push_back(const T& value);
    UWR_FORCEINLINE constexpr void push_back(T&& value);
    UWR_FORCEINLINE constexpr void fast_push_back(T&& value) noexcept;

    UWR_FORCEINLINE constexpr void pop_back();
    UWR_FORCEINLINE constexpr bool safe_pop_back() noexcept;

    UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, const T& value);
    UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, T&& value);
    UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, size_type count, const T& value);
    template<class InIt, class = typename std::iterator_traits<InIt>::value_type>
    UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, InIt first, InIt last);
    UWR_FORCEINLINE constexpr iterator insert(const_iterator pos, std::initializer_list<T> ilist);

    constexpr iterator erase(const_iterator pos);
    constexpr iterator erase(const_iterator first, const_iterator last);

    UWR_FORCEINLINE constexpr void swap(static_vector& other);
    UWR_FORCEINLINE constexpr void clear() noexcept;

    template<class... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args);
    template<class... Args>
    UWR_FORCEINLINE constexpr reference emplace_back(Args&&... args);
    template<class... Args>
    UWR_FORCEINLINE constexpr reference fast_emplace_back(Args&&... args) noexcept;

private:
    UWR_FORCEINLINE constexpr T* data_at(size_type n) noexcept;
    UWR_FORCEINLINE constexpr const T* data_at(size_type n) const noexcept;
    template<class InIt>
    UWR_FORCEINLINE constexpr void priv_copy_assign(InIt first, InIt last, size_type n);
    template<class InIt>
    UWR_FORCEINLINE constexpr void priv_move_assign(InIt first, InIt last, size_type n);
    template<class InIt>
    UWR_FORCEINLINE constexpr iterator priv_copy_insert(const_iterator pos, InIt first, InIt last, size_type n);

    template<class ResizeProxy>
    constexpr void priv_resize(ResizeProxy&& proxy);
    template<class AssignProxy>
    constexpr void priv_assign(AssignProxy&& proxy);
    template<class InsertProxy>
    constexpr iterator priv_insert(const_iterator pos, InsertProxy&& proxy);
    constexpr void priv_swap(static_vector& shorter, static_vector& longer) const;

private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type m_data[C];
    size_type m_size;
};

template<class T, len_t C>
constexpr
static_vector<T, C>::static_vector() noexcept
    : m_size(0) {}

template<class T, len_t C>
constexpr
static_vector<T, C>::static_vector(size_type n)
    : m_size(n) {
    mem::construct(this->data(), n);
}

template<class T, len_t C>
constexpr
static_vector<T, C>::static_vector(size_type n, const T& val)
    : m_size(n) {
    mem::ufill(this->data(), n, val);
}

template<class T, len_t C>
template<class InIt, class>
constexpr
static_vector<T, C>::static_vector(InIt first, InIt last)
    : m_size(std::distance(first, last)) {
    mem::ucopy(this->data(), first, last);
}

template<class T, len_t C>
constexpr
static_vector<T, C>::static_vector(const static_vector& x)
    : m_size(x.m_size) {
    mem::ucopy(this->data(), x.begin(), x.size());
}

template<class T, len_t C>
constexpr
static_vector<T, C>::static_vector(static_vector&& x) noexcept
    : m_size(x.m_size) {
    mem::umove(this->data(), x.begin(), x.size());
}

template<class T, len_t C>
constexpr
static_vector<T, C>::static_vector(std::initializer_list<T> ilist)
    : m_size(ilist.size()) {
    mem::ucopy(this->data(), ilist.begin(), ilist.size());
}

template<class T, len_t C>
#if CPP_ABOVE_17
constexpr
#endif
static_vector<T, C>::~static_vector() {
    mem::destroy(this->data(), this->m_size);
}

template<class T, len_t C>
constexpr static_vector<T, C>&
static_vector<T, C>::operator=(const static_vector<T, C>& other) noexcept {
    if (UWR_LIKELY(this != &other))
        this->priv_copy_assign(other.begin(), other.end(), other.m_size);
    return *this;
}

template<class T, len_t C>
constexpr static_vector<T, C>&
static_vector<T, C>::operator=(static_vector<T, C>&& other) noexcept {
    if (UWR_LIKELY(this != &other))
        this->priv_move_assign(other.begin(), other.end(), other.m_size);
    return *this;
}

template<class T, len_t C>
constexpr static_vector<T, C>&
static_vector<T, C>::operator=(std::initializer_list<T> ilist) noexcept {
    this->priv_copy_assign(ilist.begin(), ilist.end(), ilist.size());
    return *this;
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::begin() noexcept {
    return iterator(this->data());
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_iterator
static_vector<T, C>::begin() const noexcept {
    return this->cbegin();
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::end() noexcept {
    return iterator(this->data() + this->m_size);
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_iterator
static_vector<T, C>::end() const noexcept {
    return this->cend();
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::reverse_iterator
static_vector<T, C>::rbegin() noexcept {
    return reverse_iterator(this->end());
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_reverse_iterator
static_vector<T, C>::rbegin() const noexcept {
    return this->crbegin();
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::reverse_iterator
static_vector<T, C>::rend() noexcept {
    return reverse_iterator(this->begin());
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_reverse_iterator
static_vector<T, C>::rend() const noexcept {
    return this->crend();
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_iterator
static_vector<T, C>::cbegin() const noexcept {
    return const_iterator(this->data());
}

template<class T, len_t C>
typename static_vector<T, C>::const_iterator
constexpr static_vector<T, C>::cend() const noexcept {
    return const_iterator(this->data() + this->m_size);
}

template<class T, len_t C>
typename static_vector<T, C>::const_reverse_iterator
constexpr static_vector<T, C>::crbegin() const noexcept {
    return const_reverse_iterator(this->end());
}

template<class T, len_t C>
typename static_vector<T, C>::const_reverse_iterator
constexpr static_vector<T, C>::crend() const noexcept {
    return const_reverse_iterator(this->begin());
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::size_type
static_vector<T, C>::size() const noexcept {
    return this->m_size;
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::size_type
static_vector<T, C>::max_size() const noexcept {
    return C;
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::resize(size_type n) {
    this->priv_resize(default_construct_proxy<T>(n));
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::resize(size_type n, const T& val) {
    this->priv_resize(uninitialized_fill_proxy(n, val));
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::size_type
static_vector<T, C>::capacity() const noexcept {
    return C;
}

template<class T, len_t C>
constexpr bool
static_vector<T, C>::empty() const noexcept {
    return this->m_size == 0;
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::reserve(size_type) noexcept {
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::shrink_to_fit() noexcept {
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::reference
static_vector<T, C>::operator[](size_type n) {
    return *this->data_at(n);
}

template<class T, len_t C>
typename static_vector<T, C>::const_reference
constexpr static_vector<T, C>::operator[](size_type n) const {
    return *this->data_at(n);
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::reference
static_vector<T, C>::at(size_type n) {
    if (n < this->m_size)
        return *this->data_at(n);
    else
        throw std::out_of_range("Index out of range: " + std::to_string(n));
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_reference
static_vector<T, C>::at(size_type n) const {
    if (n < this->m_size)
        return *this->data_at(n);
    else
        throw std::out_of_range("Index out of range: " + std::to_string(n));
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::reference
static_vector<T, C>::front() {
    return *this->data_at(0);
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::const_reference
static_vector<T, C>::front() const {
    return *this->data_at(0);
}

template<class T, len_t C>
typename static_vector<T, C>::reference
constexpr static_vector<T, C>::back() {
    return *this->data_at(this->m_size - 1);
}

template<class T, len_t C>
typename static_vector<T, C>::const_reference
constexpr static_vector<T, C>::back() const {
    return *this->data_at(this->m_size - 1);
}

template<class T, len_t C>
constexpr T*
static_vector<T, C>::data() noexcept {
    return this->data_at(0);
}

template<class T, len_t C>
constexpr const T*
static_vector<T, C>::data() const noexcept {
    return this->data_at(0);
}

template<class T, len_t C>
template<class InIt, class>
constexpr void
static_vector<T, C>::assign(InIt first, InIt last) {
    this->priv_copy_assign(first, last,
            static_cast<size_type>(std::distance(first, last)));
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::assign(size_type n, const T& val) {
    this->priv_assign(assign_fill_proxy(n, val));
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::assign(std::initializer_list<T> ilist) {
    this->priv_copy_assign(ilist.begin(), ilist.end(), ilist.size());
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::push_back(const_reference value) {
    this->emplace_back(value);
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::push_back(T&& value) {
    this->emplace_back(std::move(value));
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::fast_push_back(T&& value) noexcept {
    this->fast_emplace_back(std::forward<T>(value));
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::pop_back() {
    mem::destroy_at(this->data() + --this->m_size);
}

template<class T, len_t C>
constexpr bool
static_vector<T, C>::safe_pop_back() noexcept {
    if (UWR_LIKELY(this->m_size != 0)) {
        this->pop_back();
        return true;
    }
    else {
        return false;
    }
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::insert(const_iterator pos, const T& value) {
    return this->emplace(pos, value);
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::insert(const_iterator pos, T&& value) {
    return this->emplace(pos, std::move(value));
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::insert(const_iterator pos, size_type count, const T& value) {
    return this->priv_insert(pos, insert_fill_proxy(count, value));
}

template<class T, len_t C>
template<class InIt, class>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::insert(const_iterator pos, InIt first, InIt last) {
    return this->priv_copy_insert(pos, first, last,
            static_cast<size_type>(std::distance(first, last)));
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::insert(const_iterator pos, std::initializer_list<T> ilist) {
    return this->priv_copy_insert(pos, ilist.begin(), ilist.end(), ilist.size());
}

template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::erase(const_iterator pos) {
    T* const m_pos = const_cast<T* const>(pos);
    mem::shiftl(m_pos, m_pos + 1, this->end());
    this->pop_back();

    return m_pos;
}

#if 0 // TODO: check cache unfriendly version
template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::erase(const_iterator first, const_iterator last) {
    T* const m_first = const_cast<T* const>(first);
    T* const m_last = const_cast<T* const>(last);

    if (UWR_LIKELY(m_first != m_last)) {
        T* const m_end = this->end();

        mem::destroy(
                mem::shiftl(m_first, m_last, m_end),
                m_end);

        this->m_size -= static_cast<size_type>(
                std::distance(m_first, m_last));
    }

    return m_first;
}
#else // TODO: check cache friendly version
template<class T, len_t C>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::erase(const_iterator first, const_iterator last) {
    T* const m_first = const_cast<T* const>(first);
    T* const m_last = const_cast<T* const>(last);

    if (UWR_LIKELY(m_first != m_last)) {
        T* const m_end = this->end();
        size_type count = static_cast<size_type>(
                std::distance(m_first, m_last));
        T* const new_end = m_end - count;

        if (new_end >= m_last) {
            mem::move_and_destroy(
                mem::move(m_first, m_last, new_end),
                new_end, m_end);
        }
        else {
            mem::destroy(
                mem::shiftl(m_first, m_last, m_end),
                m_end); 
        };

        this->m_size -= count;
    }

    return m_first;
}
#endif

template<class T, len_t C>
constexpr void
static_vector<T, C>::swap(static_vector<T, C>& other) {
    if (this->m_size < other.m_size)
        this->priv_swap(*this, other);
    else
        this->priv_swap(other, *this);
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::clear() noexcept {
    mem::destroy(this->data(), this->m_size);
    this->m_size = 0;
}

template<class T, len_t C>
template<class... Args>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::emplace(const_iterator pos, Args&&... args) {
    T* const m_pos = const_cast<T* const>(pos);
    T* const m_end = this->end();

    if (m_pos != m_end) {
        mem::shiftr(m_pos + 1, m_pos, m_end);
        // this strange construction is caused by the fact
        // args can be either "proper" constructor arguments
        // or it can be an object of type T, if it is, we don't want to do
        // *m_pos = T(args), because that would create unnecessary object,
        // instead we would like to write *m_pos = args
        // note: this problem only happens when we use opeartor= (so we copy
        // into initialized memory), when we copy into unitialized memory
        // we have to call constructor in both situation so
        // T(std::forward<Args>(args)...) will do
        *m_pos = mem::create<T>(std::forward<Args>(args)...);
    }
    else {
        new (m_pos) T(std::forward<Args>(args)...);
    }
    
    ++this->m_size;

    return m_pos;
}

template<class T, len_t C>
template<class... Args>
constexpr typename static_vector<T, C>::reference
static_vector<T, C>::emplace_back(Args&&... args) {
    #if UWR_STATIC_VECTOR_OPTIM_ENABLE
    return this->fast_emplace_back(std::forward<Args>(args)...);
    #else
    if (UWR_LIKELY(this->m_size != C))
        return this->fast_emplace_back(std::forward<Args>(args)...);
    else
        throw std::out_of_range("Out of bounds");
    #endif
}

template<class T, len_t C>
template<class... Args>
constexpr typename static_vector<T, C>::reference
static_vector<T, C>::fast_emplace_back(Args&&... args) noexcept {
    T* const last_end = this->data() + this->m_size++;
    new (last_end) T(std::forward<Args>(args)...);
    return *last_end;
}

template<class T, len_t C>
constexpr T*
static_vector<T, C>::data_at(size_type n) noexcept {
    return reinterpret_cast<T*>(&this->m_data[n]);
}

template<class T, len_t C>
constexpr const T*
static_vector<T, C>::data_at(size_type n) const noexcept {
    return reinterpret_cast<const T*>(&this->m_data[n]);
}

template<class T, len_t C>
template<class ResizeProxy>
constexpr void
static_vector<T, C>::priv_resize(ResizeProxy&& proxy) {
    T* const m_end = this->end();
    T* const new_end = this->data() + proxy.n;

    if (proxy.n > this->m_size)
        proxy.construct(m_end, new_end);
    else
        mem::destroy(new_end, m_end);

    this->m_size = proxy.n;
}

template<class T, len_t C>
template<class InIt>
constexpr void
static_vector<T, C>::priv_copy_assign(InIt first, InIt last, size_type n) {
    this->priv_assign(copy_assign_range_proxy<T, InIt>(first, last, n));
}

template<class T, len_t C>
template<class InIt>
constexpr void
static_vector<T, C>::priv_move_assign(InIt first, InIt last, size_type n) {
    this->priv_assign(move_assign_range_proxy<T, InIt>(first, last, n));
}

template<class T, len_t C>
template<class AssignProxy>
constexpr void
static_vector<T, C>::priv_assign(AssignProxy&& proxy) {
    T* const m_data = this->data();

    if (proxy.n > this->m_size)
        proxy.assign_to_short(m_data, this->m_size);
    else
        proxy.assign_to_long(m_data, this->m_size);

    this->m_size = proxy.n;
}

template<class T, len_t C>
template<class InIt>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::priv_copy_insert(const_iterator pos, InIt first, InIt last, size_type n) {
    return this->priv_insert(pos, insert_copy_range_proxy<T, InIt>(first, last, n));
}

template<class T, len_t C>
template<class InsertProxy>
constexpr typename static_vector<T, C>::iterator
static_vector<T, C>::priv_insert(const_iterator pos, InsertProxy&& proxy) {
    T* const m_pos = const_cast<T* const>(pos);

    if (UWR_UNLIKELY(!proxy.count))
        return m_pos;

    T* const m_end = this->end();
    T* const spill = m_pos + proxy.count;

    if (spill < m_end) {
        mem::shiftr(spill, m_pos, m_end);
        proxy.insert_without_spill(m_pos, spill);
    }
    else {
        proxy.insert_with_spill(m_pos, m_end, spill);
    }

    this->m_size += proxy.count;

    return m_pos;
}

template<class T, len_t C>
constexpr void
static_vector<T, C>::priv_swap(static_vector<T, C>& shorter, static_vector<T, C>& longer) const {
    T* const s_begin = shorter.begin();
    T* const s_end = shorter.end();
    T* const l_begin = longer.begin();
    T* const l_end = longer.end();

    T* const l_split = std::swap_ranges(s_begin, s_end, l_begin);
    mem::umove_and_destroy(s_end, l_split, l_end);

    std::swap(shorter.m_size, longer.m_size);
}


/*
 * non-member operators 
 */
template<class T, len_t C>
UWR_FORCEINLINE constexpr bool operator==(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);
template<class T, len_t C>
UWR_FORCEINLINE constexpr bool operator!=(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);
template<class T, len_t C>
constexpr inline bool operator<(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);
template<class T, len_t C>
UWR_FORCEINLINE constexpr bool operator<=(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);
template<class T, len_t C>
UWR_FORCEINLINE constexpr bool operator>(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);
template<class T, len_t C>
UWR_FORCEINLINE constexpr bool operator>=(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);
template<class T, len_t C>
constexpr std::ostream& operator<<(std::ostream& out, const static_vector<T, C>& v);

#if CPP_ABOVE_17

template<class T, len_t C>
constexpr auto operator<=>(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs);

#endif


/*
 * non-member operators' implementations
 */
template<class T, len_t C>
constexpr bool
operator==(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<class T, len_t C>
constexpr bool
operator!=(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return !(lhs == rhs);
}

template<class T, len_t C>
constexpr bool
operator<(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<class T, len_t C>
constexpr bool
operator<=(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return !(rhs < lhs);
}

template<class T, len_t C>
constexpr bool
operator>(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return rhs < lhs;
}

template<class T, len_t C>
constexpr bool
operator>=(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return !(lhs < rhs);
}

template<class T, len_t C>
constexpr std::ostream&
operator<<(std::ostream& out, const static_vector<T, C>& v) {
    for (typename static_vector<T, C>::size_type i = 1; i < v.size(); ++i)
        out << v[i - 1] << ' ';
    if (!v.empty())
        out << v.back();
    return out;
}

#if CPP_ABOVE_17

template<class T, len_t C>
constexpr auto
operator<=>(const static_vector<T, C>& lhs, const static_vector<T, C>& rhs) {
    return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(),
                                                  rhs.begin(), rhs.end(),
                                                  mem::synth_three_way);
}

#endif

} // namespace uwr


/*
 * non-member functions
 */
namespace std {

template<class T, uwr::len_t C>
constexpr void swap(uwr::static_vector<T, C>& x, uwr::static_vector<T, C>& y);

#if CPP_ABOVE_17

template<class T, uwr::len_t C, class U>
constexpr typename uwr::static_vector<T, C>::size_type
erase(uwr::static_vector<T, C>& c, const U& value);
template<class T, uwr::len_t C, class Pred>
constexpr typename uwr::static_vector<T, C>::size_type
erase_if(uwr::static_vector<T, C>& c, Pred pred);

#endif


/*
 * non-member functions' implementations
 */
template<class T, uwr::len_t C>
constexpr void
swap(uwr::static_vector<T, C>& x, uwr::static_vector<T, C>& y) {
    x.swap(y);
}

#if CPP_ABOVE_17

template<class T, uwr::len_t C, class U>
constexpr typename uwr::static_vector<T, C>::size_type
erase(uwr::static_vector<T, C>& c, const U& value) {
    using size_type = typename uwr::static_vector<T, C>::size_type;

    T* const c_end = c.end();
    T* const mid = std::remove(c.begin(), c_end, value);
    c.erase(mid, c_end);

    return static_cast<size_type>(std::distance(mid, c_end));
}

template<class T, uwr::len_t C, class Pred>
constexpr typename uwr::static_vector<T, C>::size_type
erase_if(uwr::static_vector<T, C>& c, Pred pred) {
    using size_type = typename uwr::static_vector<T, C>::size_type;

    T* const c_end = c.end();
    T* const mid = std::remove_if(c.begin(), c_end, pred);
    c.erase(mid, c_end);

    return static_cast<size_type>(std::distance(mid, c_end));
}

#endif // CPP_ABOVE_17

} // namespace std
