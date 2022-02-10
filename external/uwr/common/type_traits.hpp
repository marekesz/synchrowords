#pragma once

#include <type_traits>

/*
* Custom markers if class is trivially:
*     - default constructible
*     - copyable
*     - copy constructible
*     - move constructible
*     - moveable
*     - copy assignable
*     - move assignable 
*       
* By default these custom classes use corresponding
* std variants, but in some cases your type might
* actually be e.g. trivially moveable although it
* has move constructor. In that case you can partially
* specfiy is_trivially_moveable struct for that type
* and hardcode the value to be true:
* 
* ```
*
* #include <uwr/common/type_traits.hpp>
*
* class MySpecialType {
*   ...
* };
*
* namespace uwr::mem {
*       template<>
*       inline constexpr bool is_trivially_move_constructible_v<MySpecialType> = true;
*       template<>
*       inline constexpr bool is_trivially_relocatable_v<MySpecialType> = true;
*       ...
* }
*
* ```
* 
* This way you still have your constructor
* specified, you achieve better performance with
* our containers and do not influence other parts
* of your architecture as you may not want to
* override std::is_trivially_moveable as this could
* break something somewhere else.
* 
* NOTE: std trivial type traits are more subtle than
*       the name suggest. E.g. by default if you define destructor,
*       your type won't be trivially moveable (according to
*       std::is_trivially_moveable). If you really want your type to
*       be trivially moveable in that situation then override one of
*       the below struct as the above example suggest.
* 
* NOTE: Most types are actually trivial, even when they
*       have defined constructors, destructors or operators.
*       The most important thing to realize is that, when vectors
*       reallocate, which is the most standard operation for
*       standard vector they copy object from one place to another
*       and get rid off the old memory. So if your type doesn't
*       depend on the location it is placed in (which again most
*       types don't depend on), then they are in fact trivial even
*       if they have defined constructors/operators/destructors.
*       When reallocating old memory will no longer be considered
*       as array of objects, but array of plain bytes, then this memory
*       will be copied into the other region, and destructors/constructors/
*       operators will be eventually called on them. *
*  Example of non-trivial type:
*  struct non_trivial {
*      int a = &this;
*  };
*  
*  NOTE: E.g. most containers are trivial. *
*
*  NOTE: std::string is actually non-trivial beacuse of small stirng
*        optimizations. But "big" string are trivial.
*/

namespace uwr::mem {

/*
* getters/helpers (override those)
*/

/* T() */
template<class T>
inline constexpr bool is_trivially_default_constructible_v =
    std::is_trivially_default_constructible_v<T>;

/* T(const&) */
template<class T>
inline constexpr bool is_trivially_copy_constructible_v =
    std::is_trivially_copy_constructible_v<T>;

/* T(&&) */
template<class T>
inline constexpr bool is_trivially_move_constructible_v =
    std::is_trivially_move_constructible_v<T>;

/* op(const&) */
template<class T>
inline constexpr bool is_trivially_copy_assignable_v =
    std::is_trivially_copy_assignable_v<T>;

/* op(&&) */
template<class T>
inline constexpr bool is_trivially_move_assignable_v =
    std::is_trivially_move_assignable_v<T>;

/* ~T() */
template<class T>
inline constexpr bool is_trivially_destructible_v =
    std::is_trivially_destructible_v<T>;

/* T(&&) && ~T() */
template<class T>
inline constexpr bool is_trivially_relocatable_v =
    is_trivially_move_constructible_v<T> &&
    is_trivially_destructible_v<T>;


/*
* structs definitions (do not override)
*/

/* T() */
template<class T>
struct is_trivially_default_constructible
    : std::integral_constant<bool, is_trivially_default_constructible_v<T>> {};

/* T(const&) */
template<class T>
struct is_trivially_copy_constructible
    : std::integral_constant<bool, is_trivially_copy_constructible_v<T>> {};

/* T(&&) */
template<class T>
struct is_trivially_move_constructible
    : std::integral_constant<bool, is_trivially_move_constructible_v<T>> {};

/* op(const&) */
template<class T>
struct is_trivially_copy_assignable
    : std::integral_constant<bool, is_trivially_copy_assignable_v<T>> {};

/* op(&&) */
template<class T>
struct is_trivially_move_assignable
    : std::integral_constant<bool, is_trivially_move_assignable_v<T>> {};

/* ~T() */
template<class T>
struct is_trivially_destructible
    : std::integral_constant<bool, is_trivially_destructible_v<T>> {};

/* T(&&) && ~T() */
template<class T>
struct is_trivially_relocatable
    : std::integral_constant<bool, is_trivially_relocatable_v<T>> {};


/*
* enable_if helpers (do not override)
*/

/* T() */
template<class T, class R = void>
using T_Cons_D =
    std::enable_if_t<is_trivially_default_constructible_v<T>, R>;
template<class T, class R = void>
using NT_Cons_D =
    std::enable_if_t<!is_trivially_default_constructible_v<T>, R>;

/* T(const&) */
template<class T, class R = void>
using T_Copy_C =
    std::enable_if_t<is_trivially_copy_constructible_v<T>, R>;
template<class T, class R = void>
using NT_Copy_C =
    std::enable_if_t<!is_trivially_copy_constructible_v<T>, R>;

/* T(&&) */
template<class T, class R = void>
using T_Move_C =
    std::enable_if_t<is_trivially_move_constructible_v<T>, R>;
template<class T, class R = void>
using NT_Move_C =
    std::enable_if_t<!is_trivially_move_constructible_v<T>, R>;

/* op(const&) */
template<class T, class R = void>
using T_Copy_A =
    std::enable_if_t<is_trivially_copy_assignable_v<T>, R>;
template<class T, class R = void>
using NT_Copy_A =
    std::enable_if_t<!is_trivially_copy_assignable_v<T>, R>;

/* op(&&) */
template<class T, class R = void>
using T_Move_A =
    std::enable_if_t<is_trivially_move_assignable_v<T>, R>;
template<class T, class R = void>
using NT_Move_A =
    std::enable_if_t<!is_trivially_move_assignable_v<T>, R>;

/* ~T() */
template<class T, class R = void>
using T_Dest =
    std::enable_if_t<is_trivially_destructible_v<T>, R>;
template<class T, class R = void>
using NT_Dest =
    std::enable_if_t<!is_trivially_destructible_v<T>, R>;

/* T(&&) && ~T() */
template<class T, class R = void>
using T_Reloc =
    std::enable_if_t<is_trivially_relocatable_v<T>, R>;
template<class T, class R = void>
using NT_Reloc =
    std::enable_if_t<!is_trivially_relocatable_v<T>, R>;

/* T(const&) && op=(const&) */
template<class T, class R = void>
using T_Copy =
    std::enable_if_t<is_trivially_copy_constructible_v<T> &&
                     is_trivially_copy_assignable_v<T>, R>;
template<class T, class R = void>
using NT_Copy =
    std::enable_if_t<!(is_trivially_copy_constructible_v<T> &&
                       is_trivially_copy_assignable_v<T>), R>;

/* T(&&) && op=(&&) */
template<class T, class R = void>
using T_Move =
    std::enable_if_t<is_trivially_move_constructible_v<T> &&
                     is_trivially_move_assignable_v<T>, R>;
template<class T, class R = void>
using NT_Move =
    std::enable_if_t<!(is_trivially_move_constructible_v<T> &&
                       is_trivially_move_assignable_v<T>), R>;

/* T(&&) && ~T() */
template<class T, class R = void>
using T_Move_C_Dest =
    std::enable_if_t<is_trivially_move_constructible_v<T> &&
                     is_trivially_destructible_v<T>, R>;
template<class T, class R = void>
using NT_Move_C_Dest =
    std::enable_if_t<!(is_trivially_move_constructible_v<T> &&
                       is_trivially_destructible_v<T>), R>;

/* op(&&) && ~T() */
template<class T, class R = void>
using T_Move_A_Dest =
    std::enable_if_t<is_trivially_move_assignable_v<T> &&
                     is_trivially_destructible_v<T>, R>;
template<class T, class R = void>
using NT_Move_A_Dest =
    std::enable_if_t<!(is_trivially_move_assignable_v<T> &&
                       is_trivially_destructible_v<T>), R>;

/* T(&&) && op=(const&) */
template<class T, class R = void>
using T_Move_C_Copy_A =
    std::enable_if_t<is_trivially_move_constructible_v<T> &&
                     is_trivially_copy_assignable_v<T>, R>;
template<class T, class R = void>
using NT_Move_C_Copy_A =
    std::enable_if_t<!(is_trivially_move_constructible_v<T> &&
                       is_trivially_copy_assignable_v<T>), R>;

} // namespace uwr
