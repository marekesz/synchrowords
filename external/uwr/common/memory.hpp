#pragma once

#include <algorithm>
#include <cstring>

#include "type_traits.hpp"
#include "define.hpp"
#include "types.hpp"

namespace uwr::mem {

// using len_t = std::u32;
using len_t = uwr::u64;

// should use getpagesize() but want constexpr
inline constexpr len_t page_size = 4096;

/*
 * construct (so should be uninitialized) continuous memory
 */
template<class T>
UWR_FORCEINLINE constexpr
T_Cons_D<T> construct(T* begin, T* end) {
   std::memset((void*)begin, 0, (end - begin) * sizeof(T));
}

template<class T>
constexpr
NT_Cons_D<T> construct(T* begin, T* end) {
   while (begin != end)
      new (begin++) T;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Cons_D<T> construct(T* begin, len_t n) {
   std::memset((void*)begin, 0, n * sizeof(T));
}

template<class T>
constexpr
NT_Cons_D<T> construct(T* begin, len_t n) {
   while (n--)
      new (begin++) T;
}

/*
 * fill initialized, continuous memory
 */
template<class T>
constexpr
void fill(T* begin, T* end, const T& val) {
   while (begin != end)
      *begin++ = val;
}

template<class T>
constexpr
T* fill(T* begin, len_t n, const T& val) {
   while (n--)
      *begin++ = val;
   return begin;
}

/*
 * fill unitialized continuous memory
 */
template<class T>
constexpr
void ufill(T* begin, T* end, const T& val) {
   while (begin != end)
      new (begin++) T(val);
}

template<class T>
constexpr
T* ufill(T* begin, len_t n, const T& val) {
   while (n--)
      new (begin++) T(val);
   return begin;
}

/*
 * copy into initialized memory
 */
template<class T>
UWR_FORCEINLINE static constexpr
T* do_copy(T* dest, const T* begin, const T* end) {
   return std::copy(begin, end, dest);
}

template<class T>
UWR_FORCEINLINE static constexpr
T* do_copy(T* dest, const T* begin, len_t n) {
   return std::copy_n(begin, n, dest);
}

template<class T, class InIt>
constexpr
T* copy(T* dest, InIt begin, InIt end) {
   while (begin != end)
      *dest++ = *begin++;
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_A<T, T*> copy(T* dest, T* begin, T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_A<T, T*> copy(T* dest, const T* begin, const T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T, class InIt>
constexpr
T* copy(T* dest, InIt begin, len_t n) {
   while (n--)
      *dest++ = *begin++;
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_A<T, T*> copy(T* dest, T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_A<T, T*> copy(T* dest, const T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

/*
 * copy into uninitialized, continuous memory
 */
template<class T, class InIt>
constexpr
T* ucopy(T* dest, InIt begin, InIt end) {
   while (begin != end)
      new (dest++) T(*begin++);
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_C<T, T*> ucopy(T* dest, T* begin, T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_C<T, T*> ucopy(T* dest, const T* begin, const T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T, class InIt>
constexpr
T* ucopy(T* dest, InIt begin, len_t n) {
   while (n--)
      new (dest++) T(*begin++);
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_C<T, T*> ucopy(T* dest, T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Copy_C<T, T*> ucopy(T* dest, const T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

/*
 * move into initialized, continuous memory
 */
template<class T, class InIt>
constexpr
T* move(T* dest, InIt begin, InIt end) {
   while (begin != end)
      *dest++ = std::move(*begin++);
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_A<T, T*> move(T* dest, T* begin, T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_A<T, T*> move(T* dest, const T* begin, const T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T, class InIt>
constexpr
T* move(T* dest, InIt begin, len_t n) {
   while (n--)
      *dest++ = std::move(*begin++);
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_A<T, T*> move(T* dest, T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_A<T, T*> move(T* dest, const T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

/*
 * move into uninitialized memory
 */
template<class T, class InIt>
constexpr
T* umove(T* dest, InIt begin, InIt end) {
   while (begin != end)
      new (dest++) T(std::move(*begin++));
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_C<T, T*> umove(T* dest, T* begin, T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_C<T, T*> umove(T* dest, const T* begin, const T* end) {
   return mem::do_copy(dest, begin, end);
}

template<class T, class InIt>
constexpr
T* umove(T* dest, InIt begin, len_t n) {
   while (n--)
      new (dest++) T(std::move(*begin++));
   return dest;
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_C<T, T*> umove(T* dest, T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_C<T, T*> umove(T* dest, const T* begin, len_t n) {
   return mem::do_copy(dest, begin, n);
}

/*
 * destroy at specified address
 */
template<class T>
constexpr
T_Dest<T> destroy_at(T*) {}

template<class T>
constexpr
NT_Dest<T> destroy_at(T* addr) {
   addr->~T();
}

/*
 * destroy range of objects (so are initialized)
 */
template<class T>
UWR_FORCEINLINE constexpr
T_Dest<T> destroy(T*, T*) {}

template<class T>
constexpr
NT_Dest<T> destroy(T* begin, T* end) {
   while (begin != end)
      mem::destroy_at(begin++);
}

template<class T>
UWR_FORCEINLINE constexpr
T_Dest<T, T*> destroy(T* begin, len_t n) {
   return begin + n;
}

template<class T>
constexpr
NT_Dest<T, T*> destroy(T* begin, len_t n) {
   while (n--)
      mem::destroy_at(begin++);
   return begin;
}

/*
* do umove and destroy at the same time (cache friendly)
*/
template<class T>
UWR_FORCEINLINE constexpr
T_Move_C_Dest<T> umove_and_destroy(T* dest, T* begin, T* end) {
   mem::umove(dest, begin, end);
}

template<class T>
constexpr
NT_Move_C_Dest<T> umove_and_destroy(T* dest, T* begin, T* end) {
   while (begin != end) {
      new (dest++) T(std::move(*begin));
      mem::destroy_at(begin++);
   }
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_C_Dest<T> umove_and_destroy(T* dest, T* begin, len_t n) {
   mem::umove(dest, begin, n);
}

template<class T>
constexpr
NT_Move_C_Dest<T> umove_and_destroy(T* dest, T* begin, len_t n) {
   while (n--) {
      new (dest++) T(std::move(*begin));
      std::destroy_at(begin++);
   }
}

/*
* do move and destroy at the same time (cache friendly)
*/
template<class T>
UWR_FORCEINLINE constexpr
T_Move_A_Dest<T> move_and_destroy(T* dest, T* begin, T* end) {
   mem::move(dest, begin, end);
}

template<class T>
constexpr
NT_Move_A_Dest<T> move_and_destroy(T* dest, T* begin, T* end) {
   while (begin != end) {
      *dest++ = std::move(*begin);
      mem::destroy_at(begin++);
   }
}

template<class T>
UWR_FORCEINLINE constexpr
T_Move_A_Dest<T> move_and_destroy(T* dest, T* begin, len_t n) {
   mem::move(dest, begin, n);
}

template<class T>
constexpr
NT_Move_A_Dest<T> move_and_destroy(T* dest, T* begin, len_t n) {
   while (n--) {
      *dest++ = std::move(*begin);
      mem::destroy_at(begin++);
   }
}

/*
* do move and fill at the same time (cache friendly)
*/
template<class T>
UWR_FORCEINLINE constexpr
T_Move_C_Copy_A<T, T*> umove_and_fill(T* dest, T* begin, T* end, const T& val) {
   // NOTE: for trivial types it seems umove and fill is faster
   T* ret = mem::umove(dest, begin, end);
   mem::fill(begin, end, val);
   return ret;
}

template<class T>
constexpr
NT_Move_C_Copy_A<T, T*> umove_and_fill(T* dest, T* begin, T* end, const T& val) {
   while (begin != end) {
      new (dest++) T(std::move(*begin));
      *begin++ = val;
   }
   return dest;
}

/*
* do move and copy at the same time (cache friendly)
*/
template<class T, class InIt>
UWR_FORCEINLINE constexpr
T_Move_C_Copy_A<T, T*> umove_and_copy(T* dest, T* begin, T* end, InIt first, InIt split) {
   T* ret = mem::umove(dest, begin, end);
   mem::copy(begin, first, split);
   return ret;
}

template<class T, class InIt>
UWR_FORCEINLINE constexpr
NT_Move_C_Copy_A<T, T*> umove_and_copy(T* dest, T* begin, T* end, InIt first, InIt) {
   while (begin != end) {
      new (dest++) T(std::move(*begin));
      *begin++ = *first++;
   }
   return dest;
}

/*
 * move to destination moving in backward direction
 */
template<class T>
UWR_FORCEINLINE constexpr
T_Move_A<T> move_backward(T* dest, T* begin, T* end) {
   len_t n = end - begin;
   std::memmove((void*)(dest - n), (void*)begin, n * sizeof(T));
}

template<class T>
constexpr
NT_Move_A<T> move_backward(T* dest, T* begin, T* end) {
   while (begin != end)
      *(--dest) = std::move(*(--end));
}

/*
 * shifts data to the right,
 * assumption: begin < dest <= end 
 * and addresses >= end are uninitialized,
 */
template<class T>
UWR_FORCEINLINE constexpr
T_Move<T> shiftr(T* dest, T* begin, T* end) {
   std::memmove((void*)dest, (void*)begin, (end - begin) * sizeof(T));
}

template<class T>
UWR_FORCEINLINE constexpr
NT_Move<T> shiftr(T* dest, T* begin, T* end) {
   T* seg = end - (dest - begin);
   mem::umove(end, seg, end);
   mem::move_backward(end, begin, seg);
}

/*
 * shifts data to the left,
 * assumption: dest < begin,
 * returns dest + (end - begin)
 */
template<class T>
UWR_FORCEINLINE constexpr
T* shiftl(T* dest, T* begin, T* end) {
   return mem::move(dest, begin, end);
}

/*
 * create object of type T (call its constructor)
 * only if args are proper constructor arguemnts,
 * if args is one object of type T, just return that
 * object and don't call constructor
 * NOTE: would hapilly remove that function
 */
template<class T, class V, class... Args>
UWR_FORCEINLINE constexpr
T create(V&& x, Args&&... args) {
   return T(std::forward<V>(x), std::forward<Args>(args)...);
}

template<class T>
UWR_FORCEINLINE constexpr
const T& create(const T& x) {
   return x;
}

template<class T>
UWR_FORCEINLINE constexpr
T&& create(T&& x) {
   return std::forward<T>(x);
}

template<class T>
UWR_FORCEINLINE constexpr
T create() {
   return T();
}

} // namespace uwr::mem
