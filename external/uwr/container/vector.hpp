#pragma once

#include <iterator>
#include <limits>

#include "../common/memory.hpp"
#include "../common/define.hpp"
#include "../common/proxy.hpp"
#if CPP_ABOVE_17
#include "../common/synth_three_way.hpp"
#endif
#include "../allocator/hybrid_allocator_bs.hpp"

namespace uwr {

template<class T, class A = mem::hybrid_allocator_bs<T>>
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
    constexpr vector(vector&& x) noexcept;
    constexpr vector(std::initializer_list<T> ilist);
    #if CPP_ABOVE_17
    constexpr
    #endif
    ~vector();

    UWR_FORCEINLINE constexpr vector& operator=(const vector& other) noexcept;
    UWR_FORCEINLINE constexpr vector& operator=(vector&& other) noexcept;
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
    UWR_FORCEINLINE constexpr void resize(size_type n);
    UWR_FORCEINLINE constexpr void resize(size_type n, const T& val);
    UWR_FORCEINLINE constexpr size_type capacity() const noexcept;
    [[nodiscard]] UWR_FORCEINLINE constexpr bool empty() const noexcept;
    constexpr void reserve(size_type n) noexcept;
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

    UWR_FORCEINLINE constexpr void swap(vector& other);
    UWR_FORCEINLINE constexpr void clear() noexcept;

    template<class... Args>
    constexpr iterator emplace(const_iterator pos, Args&&... args);
    template<class... Args>
    constexpr reference emplace_back(Args&&... args);
    template<class... Args>
    UWR_FORCEINLINE constexpr reference fast_emplace_back(Args&&... args) noexcept;

private:
    template<class InIt>
    UWR_FORCEINLINE constexpr void priv_copy_assign(InIt first, InIt last, size_type n);
    template<class InIt>
    UWR_FORCEINLINE constexpr iterator priv_copy_insert(const_iterator pos, InIt first, InIt last, size_type n);

    template<class ResizeProxy>
    constexpr void priv_resize(ResizeProxy&& proxy);
    template<class AssignProxy>
    constexpr void priv_assign(AssignProxy&& proxy);
    template<class InsertProxy>
    constexpr iterator priv_insert(const_iterator pos, InsertProxy&& proxy);

private:
    allocator_type m_alloc;
};

template<class T, class A>
constexpr
vector<T, A>::vector() noexcept
    : m_alloc() {}

template<class T, class A>
constexpr
vector<T, A>::vector(size_type n)
    : m_alloc(n) {
    mem::construct(this->data(), n);
}

template<class T, class A>
constexpr
vector<T, A>::vector(size_type n, const T& val)
    : m_alloc(n) {
    mem::ufill(this->data(), n, val);
}

template<class T, class A>
template<class InIt, class>
constexpr
vector<T, A>::vector(InIt first, InIt last)
    : m_alloc(std::distance(first, last)) {
    mem::ucopy(this->data(), first, last);
}

template<class T, class A>
constexpr
vector<T, A>::vector(const vector& x)
    : m_alloc(x.size()) {
    mem::ucopy(this->data(), x.begin(), x.size());
}

template<class T, class A>
constexpr
vector<T, A>::vector(vector&& x) noexcept
    : m_alloc(std::move(x.m_alloc)) {}

template<class T, class A>
constexpr
vector<T, A>::vector(std::initializer_list<T> ilist)
    : m_alloc(ilist.size()) {
    mem::ucopy(this->data(), ilist.begin(), ilist.size());
}

template<class T, class A>
#if CPP_ABOVE_17
constexpr
#endif
vector<T, A>::~vector() {
    mem::destroy(this->data(), this->size());
}

template<class T, class A>
constexpr
vector<T, A>& vector<T, A>::operator=(const vector<T, A>& other) noexcept {
    if (UWR_LIKELY(this != &other))
        this->priv_copy_assign(other.begin(), other.end(), other.size());
    return *this;
}

template<class T, class A>
constexpr
vector<T, A>& vector<T, A>::operator=(vector<T, A>&& other) noexcept {
    if (UWR_LIKELY(this != &other))
        this->m_alloc = std::move(other.m_alloc);
    return *this;
}

template<class T, class A>
constexpr
vector<T, A>& vector<T, A>::operator=(std::initializer_list<T> ilist) noexcept {
    this->priv_copy_assign(ilist.begin(), ilist.end(), ilist.size());
    return *this;
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::begin() noexcept {
    return iterator(this->data());
}

template<class T, class A>
constexpr typename vector<T, A>::const_iterator
vector<T, A>::begin() const noexcept {
    return this->cbegin();
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::end() noexcept {
    return iterator(this->data() + this->size());
}

template<class T, class A>
constexpr typename vector<T, A>::const_iterator
vector<T, A>::end() const noexcept {
    return this->cend();
}

template<class T, class A>
constexpr typename vector<T, A>::reverse_iterator
vector<T, A>::rbegin() noexcept {
    return reverse_iterator(this->end());
}

template<class T, class A>
constexpr typename vector<T, A>::const_reverse_iterator
vector<T, A>::rbegin() const noexcept {
    return this->crbegin();
}

template<class T, class A>
constexpr typename vector<T, A>::reverse_iterator
vector<T, A>::rend() noexcept {
    return reverse_iterator(this->begin());
}

template<class T, class A>
constexpr typename vector<T, A>::const_reverse_iterator
vector<T, A>::rend() const noexcept {
    return this->crend();
}

template<class T, class A>
constexpr typename vector<T, A>::const_iterator
vector<T, A>::cbegin() const noexcept {
    return const_iterator(this->data());
}

template<class T, class A>
constexpr typename vector<T, A>::const_iterator
vector<T, A>::cend() const noexcept {
    return const_iterator(this->data() + this->size());
}

template<class T, class A>
constexpr typename vector<T, A>::const_reverse_iterator
vector<T, A>::crbegin() const noexcept {
    return const_reverse_iterator(this->end());
}

template<class T, class A>
constexpr typename vector<T, A>::const_reverse_iterator
vector<T, A>::crend() const noexcept {
    return const_reverse_iterator(this->begin());
}

template<class T, class A>
constexpr typename vector<T, A>::size_type
vector<T, A>::size() const noexcept {
    return this->m_alloc.m_size;
}

template<class T, class A>
constexpr typename vector<T, A>::size_type
vector<T, A>::max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(T);
}

template<class T, class A>
constexpr void
vector<T, A>::resize(size_type n) {
    this->priv_resize(default_construct_proxy<T>(n));
}

template<class T, class A>
constexpr void
vector<T, A>::resize(size_type n, const T& val) {
    this->priv_resize(uninitialized_fill_proxy(n, val));
}

template<class T, class A>
constexpr typename vector<T, A>::size_type
vector<T, A>::capacity() const noexcept {
    return this->m_alloc.m_capacity;
}

template<class T, class A>
constexpr bool
vector<T, A>::empty() const noexcept {
    return this->size() == 0;
}

template<class T, class A>
constexpr void
vector<T, A>::reserve(size_type n) noexcept {
    if (UWR_LIKELY(n > this->capacity()))
        this->m_alloc.realloc(n);
}

template<class T, class A>
constexpr void
vector<T, A>::shrink_to_fit() noexcept {
    // TODO: now we don't shrink, maybe in the future
}

template<class T, class A>
constexpr typename vector<T, A>::reference
vector<T, A>::operator[](size_type n) {
    return this->data()[n];
}

template<class T, class A>
constexpr typename vector<T, A>::const_reference
vector<T, A>::operator[](size_type n) const {
    return this->data()[n];
}

template<class T, class A>
constexpr typename vector<T, A>::reference
vector<T, A>::at(size_type n) {
    if (UWR_LIKELY(n < this->size()))
        return this->data()[n];
    else
        throw std::out_of_range("Index out of range: " + std::to_string(n));
}

template<class T, class A>
constexpr typename vector<T, A>::const_reference
vector<T, A>::at(size_type n) const {
    if (UWR_LIKELY(n < this->size()))
        return this->data()[n];
    else
        throw std::out_of_range("Index out of range: " + std::to_string(n));
}

template<class T, class A>
constexpr typename vector<T, A>::reference
vector<T, A>::front() {
    return *this->data();
}

template<class T, class A>
constexpr typename vector<T, A>::const_reference
vector<T, A>::front() const {
    return *this->data();
}

template<class T, class A>
constexpr typename vector<T, A>::reference
vector<T, A>::back() {
    return this->data()[this->size() - 1];
}

template<class T, class A>
constexpr typename vector<T, A>::const_reference
vector<T, A>::back() const {
    return this->data()[this->size() - 1];
}

template<class T, class A>
constexpr T*
vector<T, A>::data() noexcept {
    return this->m_alloc.m_data;
}

template<class T, class A>
constexpr const T*
vector<T, A>::data() const noexcept {
    return this->m_alloc.m_data;
}

template<class T, class A>
template<class InIt, class>
constexpr void
vector<T, A>::assign(InIt first, InIt last) {
    this->priv_copy_assign(first, last,
            static_cast<size_type>(std::distance(first, last)));
}

template<class T, class A>
constexpr void
vector<T, A>::assign(size_type n, const T& val) {
    this->priv_assign(assign_fill_proxy(n, val));
}

template<class T, class A>
constexpr void
vector<T, A>::assign(std::initializer_list<T> ilist) {
    this->priv_copy_assign(ilist.begin(), ilist.end(), ilist.size());
}

template<class T, class A>
constexpr void
vector<T, A>::push_back(const_reference value) {
    this->emplace_back(value);
}

template<class T, class A>
constexpr void
vector<T, A>::push_back(T&& value) {
    this->emplace_back(std::move(value));
}

template<class T, class A>
constexpr void
vector<T, A>::fast_push_back(T&& value) noexcept {
    this->fast_emplace_back(std::forward<T>(value));
}

template<class T, class A>
constexpr void
vector<T, A>::pop_back() {
    mem::destroy_at(this->data() + --this->m_alloc.m_size);
}

template<class T, class A>
constexpr bool
vector<T, A>::safe_pop_back() noexcept {
    if (UWR_LIKELY(!this->empty())) {
        this->pop_back();
        return true;
    }
    else {
        return false;
    }
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::insert(const_iterator pos, const T& value) {
    return this->emplace(pos, value);
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::insert(const_iterator pos, T&& value) {
    return this->emplace(pos, std::move(value));
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::insert(const_iterator pos, size_type count, const T& value) {
    return this->priv_insert(pos, insert_fill_proxy(count, value));
}

template<class T, class A>
template<class InIt, class>
constexpr typename vector<T, A>::iterator
vector<T, A>::insert(const_iterator pos, InIt first, InIt last) {
    return this->priv_copy_insert(pos, first, last,
            static_cast<size_type>(std::distance(first, last)));
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::insert(const_iterator pos, std::initializer_list<T> ilist) {
    return this->priv_copy_insert(pos, ilist.begin(), ilist.end(), ilist.size());
}

template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::erase(const_iterator pos) {
    T* const m_pos = const_cast<T* const>(pos);
    mem::shiftl(m_pos, m_pos + 1, this->end());
    this->pop_back();

    return m_pos;
}

#if 0 // TODO: check cache unfriendly
template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::erase(const_iterator first, const_iterator last) {
    T* const m_first = const_cast<T* const>(first);
    T* const m_last = const_cast<T* const>(last);

    if (UWR_LIKELY(m_first != m_last)) {
        T* const m_end = this->end();

        mem::destroy(
                mem::shiftl(m_first, m_last, m_end),
                m_end);

        this->m_alloc.m_size -= static_cast<size_type>(
                std::distance(m_first, m_last));
    }

    return m_first;
}
#else // TODO: check cache friendly
template<class T, class A>
constexpr typename vector<T, A>::iterator
vector<T, A>::erase(const_iterator first, const_iterator last) {
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

        this->m_alloc.m_size -= count;
    }

    return m_first;
}
#endif

template<class T, class A>
constexpr void
vector<T, A>::swap(vector<T, A>& other) {
    this->m_alloc.swap(other.m_alloc);
}

template<class T, class A>
constexpr void
vector<T, A>::clear() noexcept {
    mem::destroy(this->data(), this->size());
    this->m_alloc.m_size = 0;
}

template<class T, class A>
template<class... Args>
constexpr typename vector<T, A>::iterator
vector<T, A>::emplace(const_iterator pos, Args&&... args) {
    T* m_pos = const_cast<T*>(pos);

    if (this->size() == this->capacity()) {
        size_type m = m_pos - this->data();
        this->m_alloc.grow(this->size() + 1);
        m_pos = this->data() + m;
    }

    T* const m_end = this->end();

    if (m_pos != m_end) {
        mem::shiftr(m_pos + 1, m_pos, m_end);
        *m_pos = mem::create<T>(std::forward<Args>(args)...);
    }
    else {
        new (m_pos) T(std::forward<Args>(args)...);
    }

    ++this->m_alloc.m_size;

    return m_pos;
}

template<class T, class A>
template<class... Args>
constexpr typename vector<T, A>::reference
vector<T, A>::emplace_back(Args&&... args) {
    if (UWR_UNLIKELY(this->size() == this->capacity()))
        this->m_alloc.grow(this->size() + 1);
    return this->fast_emplace_back(std::forward<Args>(args)...);
}

template<class T, class A>
template<class... Args>
constexpr typename vector<T, A>::reference
vector<T, A>::fast_emplace_back(Args&&... args) noexcept {
    T* const last_end = this->data() + this->m_alloc.m_size++;
    new (last_end) T(std::forward<Args>(args)...);
    return *last_end;
}

template<class T, class A>
template<class InIt>
constexpr void
vector<T, A>::priv_copy_assign(InIt first, InIt last, size_type n) {
    this->priv_assign(copy_assign_range_proxy<T, InIt>(first, last, n));
}

template<class T, class A>
template<class InIt>
constexpr typename vector<T, A>::iterator
vector<T, A>::priv_copy_insert(const_iterator pos, InIt first, InIt last, size_type n) {
    return this->priv_insert(pos, insert_copy_range_proxy<T, InIt>(first, last, n));
}

template<class T, class A>
template<class AssignProxy>
constexpr void
vector<T, A>::priv_assign(AssignProxy&& proxy) {
    if (UWR_UNLIKELY(proxy.n > this->capacity())) {
        if (this->m_alloc.expand_or_dealloc_and_alloc_raw(proxy.n))
            proxy.assign_to_short(this->data(), this->size());
        else
            proxy.assign_to_short(this->data(), 0);
    }
    else if (proxy.n > this->size())
        proxy.assign_to_short(this->data(), this->size());
    else
        proxy.assign_to_long(this->data(), this->size());

    this->m_alloc.m_size = proxy.n;
}

template<class T, class A>
template<class ResizeProxy>
constexpr void
vector<T, A>::priv_resize(ResizeProxy&& proxy) {
    if (proxy.n > this->capacity()) {
        this->m_alloc.grow(proxy.n);
        proxy.construct(this->end(), this->data() + proxy.n);
    }
    else {
        T* const m_end = this->end();
        T* const new_end = this->data() + proxy.n;

        if (proxy.n > this->size())
            proxy.construct(m_end, new_end);
        else
            mem::destroy(new_end, m_end);
    }

    this->m_alloc.m_size = proxy.n;
}

template<class T, class A>
template<class InsertProxy>
constexpr typename vector<T, A>::iterator
vector<T, A>::priv_insert(const_iterator pos, InsertProxy&& proxy) {
    T* m_pos = const_cast<T*>(pos);

    if (UWR_UNLIKELY(!proxy.count))
        return m_pos;

    size_type new_size = this->size() + proxy.count;

    if (new_size > this->capacity()) {
        size_type m = m_pos - this->data();
        this->m_alloc.grow(new_size);
        m_pos = this->data() + m;
    }

    T* const m_end = this->end();
    T* const spill = m_pos + proxy.count;

    if (spill < m_end) {
        mem::shiftr(spill, m_pos, m_end);
        proxy.insert_without_spill(m_pos, spill);
    }
    else {
        proxy.insert_with_spill(m_pos, m_end, spill);
    }

    this->m_alloc.m_size = new_size;

    return m_pos;
}


/*
 * non-member operators 
 */
template<class T, class A>
UWR_FORCEINLINE constexpr bool operator==(const vector<T, A>& lhs, const vector<T, A>& rhs);
template<class T, class A>
UWR_FORCEINLINE constexpr bool operator!=(const vector<T, A>& lhs, const vector<T, A>& rhs);
template<class T, class A>
constexpr bool operator<(const vector<T, A>& lhs, const vector<T, A>& rhs);
template<class T, class A>
UWR_FORCEINLINE constexpr bool operator<=(const vector<T, A>& lhs, const vector<T, A>& rhs);
template<class T, class A>
UWR_FORCEINLINE constexpr bool operator>(const vector<T, A>& lhs, const vector<T, A>& rhs);
template<class T, class A>
UWR_FORCEINLINE constexpr bool operator>=(const vector<T, A>& lhs, const vector<T, A>& rhs);
template<class T, class A>
constexpr std::ostream& operator<<(std::ostream& out, const vector<T, A>& v);

#if CPP_ABOVE_17

template<class T, class A>
constexpr auto operator<=>(const vector<T, A>& lhs, const vector<T, A>& rhs);

#endif


/*
 * non-member operators' implementations
 */
template<class T, class A>
constexpr bool
operator==(const vector<T, A>& lhs, const vector<T, A>& rhs) {
    return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<class T, class A>
constexpr bool
operator!=(const vector<T, A>& lhs, const vector<T, A>& rhs) {
    return !(lhs == rhs);
}

template<class T, class A>
constexpr bool
operator<(const vector<T, A>& lhs, const vector<T, A>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<class T, class A>
constexpr bool
operator<=(const vector<T, A>& lhs, const vector<T, A>& rhs) {
    return !(rhs < lhs);
}

template<class T, class A>
constexpr bool
operator>(const vector<T, A>& lhs, const vector<T, A>& rhs) {
    return rhs < lhs;
}

template<class T, class A>
constexpr bool
operator>=(const vector<T, A>& lhs, const vector<T, A>& rhs) {
    return !(lhs < rhs);
}

template<class T, class A>
constexpr std::ostream&
operator<<(std::ostream& out, const vector<T, A>& v) {
    for (typename vector<T, A>::size_type i = 1; i < v.size(); ++i)
        out << v[i - 1] << ' ';
    if (!v.empty())
        out << v.back();
    return out;
}

#if CPP_ABOVE_17

template<class T, class A>
constexpr auto
operator<=>(const vector<T, A>& lhs, const vector<T, A>& rhs) {
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

template<class T, class A>
constexpr void swap(uwr::vector<T, A>& x, uwr::vector<T, A>& y);

#if CPP_ABOVE_17

template<class T, class A, class U>
constexpr typename uwr::vector<T, A>::size_type
erase(uwr::vector<T, A>& c, const U& value);
template<class T, class A, class Pred>
constexpr typename uwr::vector<T, A>::size_type
erase_if(uwr::vector<T, A>& c, Pred pred);

#endif


/*
 * non-member functions' implementations
 */
template<class T, class A>
constexpr void
swap(uwr::vector<T, A>& x, uwr::vector<T, A>& y) {
    x.swap(y);
}

#if CPP_ABOVE_17

template<class T, class A, class U>
constexpr typename uwr::vector<T, A>::size_type
erase(uwr::vector<T, A>& c, const U& value) {
    T* const c_end = c.end();
    T* const mid = std::remove(c.begin(), c_end, value);
    c.erase(mid, c_end);

    return static_cast<size_t>(std::distance(mid, c_end));
}

template<class T, class A, class Pred>
constexpr typename uwr::vector<T, A>::size_type
erase_if(uwr::vector<T, A>& c, Pred pred) {
    T* const c_end = c.end();
    T* const mid = std::remove_if(c.begin(), c_end, pred);
    c.erase(mid, c_end);

    return static_cast<size_t>(std::distance(mid, c_end));
}

#endif // CPP_ABOVE_17

} // namespace uwr
