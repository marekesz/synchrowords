#pragma once

#include <sys/mman.h>

#include "allocator_base.hpp"
#include "../common/memory.hpp"

namespace uwr::mem {

using std::true_type;
using std::false_type;

template<class T>
class hybrid_allocator : public allocator_base<hybrid_allocator<T>, T> {
public:
   using base_t = allocator_base<hybrid_allocator<T>, T>;
   using size_type = typename base_t::size_type;
   using value_type = T;
   using pointer = typename base_t::pointer;

   constexpr hybrid_allocator() noexcept;
   constexpr explicit hybrid_allocator(size_type n);

   UWR_FORCEINLINE constexpr size_type fix_capacity(size_type n) const;
   UWR_FORCEINLINE constexpr pointer alloc(size_type n) const;
   UWR_FORCEINLINE constexpr void dealloc(pointer data, size_type n) const;
   UWR_FORCEINLINE constexpr void expand(size_type req);
   UWR_FORCEINLINE constexpr void shrink(size_type req);
   template<class GF>
   UWR_FORCEINLINE constexpr void grow(size_type req);
   UWR_FORCEINLINE constexpr bool expand_or_dealloc_and_alloc_raw(size_type req);

private:
   UWR_FORCEINLINE constexpr int is_big_size(size_type n) const;
   UWR_FORCEINLINE constexpr int is_big_size(size_type n, true_type) const;
   UWR_FORCEINLINE constexpr int is_big_size(size_type n, false_type) const;
   UWR_FORCEINLINE constexpr size_type fix_big_capacity(size_type n) const;
   UWR_FORCEINLINE constexpr size_type fix_small_capacity(size_type n) const;
   UWR_FORCEINLINE constexpr pointer big_alloc(size_type n) const;
   UWR_FORCEINLINE constexpr pointer small_alloc(size_type n) const;
   UWR_FORCEINLINE constexpr void big_dealloc(pointer data, size_type n) const;
   UWR_FORCEINLINE constexpr void small_dealloc(pointer data, size_type n) const;

   template<class GF>
   UWR_FORCEINLINE constexpr void grow(size_type req, true_type);
   template<class GF>
   constexpr void grow(size_type req, false_type);
   constexpr pointer expand(size_type req, true_type);
   constexpr pointer expand(size_type req, false_type);
   constexpr pointer shrink(size_type req, true_type);
   constexpr pointer shrink(size_type req, false_type);
   constexpr bool expand_or_dealloc_and_alloc_raw(size_type req, true_type);
   constexpr bool expand_or_dealloc_and_alloc_raw(size_type req, false_type);

   UWR_FORCEINLINE constexpr size_type npages(size_type x);
};

template<class T>
constexpr
hybrid_allocator<T>::hybrid_allocator() noexcept
   : base_t() {}

template<class T>
constexpr
hybrid_allocator<T>::hybrid_allocator(size_type n)
   : base_t(n) {}

template<class T>
constexpr typename hybrid_allocator<T>::size_type
hybrid_allocator<T>::fix_capacity(size_type n) const {
#ifndef NDEBUG
   if (!n) return 0;
#endif
   if (is_big_size(n))
      return fix_big_capacity(n);
   else
      return fix_small_capacity(n);
}

template<class T>
constexpr typename hybrid_allocator<T>::size_type
hybrid_allocator<T>::fix_big_capacity(size_type n) const {
   UWR_ASSERT(is_big_size(n));

   return ((n * sizeof(T) + page_size - 1) / page_size)
      * page_size / sizeof(T);
}

template<class T>
constexpr typename hybrid_allocator<T>::size_type
hybrid_allocator<T>::fix_small_capacity(size_type n) const {
   UWR_ASSERT(!is_big_size(n));

   return std::max((64 + sizeof(T) - 1) / sizeof(T), n);
}

template<class T>
constexpr int
hybrid_allocator<T>::is_big_size(size_type n) const {
   return is_big_size(n, is_trivially_relocatable<T>());
}

template<class T>
constexpr int
hybrid_allocator<T>::is_big_size(size_type n, true_type) const {
   return n * sizeof(T) >= page_size;
}

template<class T>
constexpr int
hybrid_allocator<T>::is_big_size(size_type n, false_type) const {
   return n * sizeof(T) >= 32 * 1024 * 1024;
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::alloc(size_type n) const {
   UWR_ASSERT(n == fix_capacity(n));

   if (is_big_size(n))
      return big_alloc(n);
   else
      return small_alloc(n);
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::big_alloc(size_type n) const {
   UWR_ASSERT(is_big_size(n));
   UWR_ASSERT(n == fix_capacity(n));

   return (pointer)mmap(NULL, n * sizeof(T),
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1, 0);
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::small_alloc(size_type n) const {
   UWR_ASSERT(!is_big_size(n));
   UWR_ASSERT(n == fix_capacity(n));

   return (pointer)malloc(n * sizeof(T));
}

template<class T>
constexpr void
hybrid_allocator<T>::dealloc(pointer data, size_type n) const {
   UWR_ASSERT(n == fix_capacity(n));

   if (is_big_size(n))
      big_dealloc(data, n);
   else
      small_dealloc(data, n);
}

template<class T>
constexpr void
hybrid_allocator<T>::big_dealloc(pointer data, size_type n) const {
   UWR_ASSERT(is_big_size(n));
   UWR_ASSERT(n == fix_capacity(n));

   munmap(data, n * sizeof(T));
}

template<class T>
constexpr void
hybrid_allocator<T>::small_dealloc(pointer data, UWR_UNUSED size_type n) const {
   UWR_ASSERT(!is_big_size(n));
   UWR_ASSERT(n == fix_capacity(n));

   free(data);
}

template<class T>
constexpr void
hybrid_allocator<T>::expand(size_type req) {
   req = fix_capacity(req);
   if (UWR_LIKELY(!!this->m_data))
      this->m_data = expand(req, is_trivially_relocatable<T>());
   else
      this->m_data = alloc(req);
   this->m_capacity = req;
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::expand(size_type req, true_type) {
   UWR_ASSERT(req > this->m_capacity);
   UWR_ASSERT(req == fix_capacity(req));

   u8 cond = is_big_size(this->m_capacity) | is_big_size(req) << 1;
   switch (cond) {
      case 0b00: { /* both are small sizes */
         return (pointer)::realloc((void*)this->m_data, req * sizeof(T));
      }
      case 0b10: { /* new size is big, old is small */
         pointer new_data = big_alloc(req);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         return new_data;
      }
      case 0b11: { /* both are big sizes */
         return (pointer)mremap((void*)this->m_data,
                                this->m_capacity * sizeof(T),
                                req * sizeof(T),
                                MREMAP_MAYMOVE);
      }
      default: /* impossible */
         UWR_ASSERT(false);
         return nullptr; // keep compiler happy
   }
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::expand(size_type req, false_type) {
   UWR_ASSERT(req > this->m_capacity);
   UWR_ASSERT(req == fix_capacity(req));

   u8 cond = is_big_size(this->m_capacity) | is_big_size(req) << 1;
   switch (cond) {
      case 0b00: { /* both are small sizes */
         pointer new_data = small_alloc(req);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         return new_data;
      }
      case 0b10: { /* new size is big, old is small */
         pointer new_data = big_alloc(req);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         return new_data;
      }
      case 0b11: { /* both are big sizes */
         pointer new_data = (pointer)mremap(this->m_data,
                                            this->m_capacity * sizeof(T),
                                            req * sizeof(T), 0);
         if (new_data == (pointer)-1) {
            new_data = big_alloc(req);
            umove_and_destroy(new_data, this->m_data, this->m_size);
            big_dealloc(this->m_data, this->m_capacity);
         }
         return new_data;
      }
      default: /* impossible */
         UWR_ASSERT(false);
         return nullptr; // keep compiler happy
   }
}

template<class T>
constexpr void
hybrid_allocator<T>::shrink(size_type req) {
   UWR_ASSERT(req < this->m_capacity);

   req = fix_capacity(req);
   this->m_data = shrink(req, is_trivially_relocatable<T>());
   this->m_capacity = req;
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::shrink(size_type req, true_type) {
   u8 cond = is_big_size(req)<<1 | is_big_size(this->m_capacity);
   switch (cond) {
      case 0b00: { /* both are small sizes */
         return (pointer)::realloc((void*)this->m_data, req * sizeof(T));
      } break;
      case 0b01: { /* new size is small, old is big */
         pointer new_data = small_alloc(req);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         big_dealloc(this->m_data, this->m_capacity);
         return new_data;
      } break;
      case 0b11: { /* both are big sizes */
         return (pointer)mremap((void*)this->m_data,
                                this->m_capacity * sizeof(T),
                                req * sizeof(T),
                                MREMAP_MAYMOVE);
      } break;
      default: /* impossible */
         UWR_ASSERT(false);
         return nullptr; // keep compiler happy
   }
}

template<class T>
constexpr typename hybrid_allocator<T>::pointer
hybrid_allocator<T>::shrink(size_type req, false_type) {
   u8 cond = is_big_size(this->m_capacity) | is_big_size(req) << 1;
   switch (cond) {
      case 0b00: { /* both are small sizes */
         pointer new_data = small_alloc(req);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         return new_data;
      } break;
      case 0b10: { /* new size is small, old is big */
         pointer new_data = small_alloc(req);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         big_dealloc(this->m_data, this->m_capacity);
         return new_data;
      } break;
      case 0b11: { /* both are big sizes */
         pointer new_data = (pointer)mremap((void*)this->m_data,
                                            this->m_capacity * sizeof(T),
                                            req * sizeof(T), 0);
         if (UWR_UNLIKELY(new_data == (pointer)-1)) {
            new_data = big_alloc(req);
            umove_and_destroy(new_data, this->m_data, this->m_size);
            big_dealloc(this->m_data, this->m_capacity);
         }
         return new_data;
      } break;
      default: /* impossible */
         UWR_ASSERT(false);
         return nullptr; // keep compiler happy
   }
}

template<class T>
template<class GF>
constexpr void
hybrid_allocator<T>::grow(size_type req) {
   UWR_ASSERT(req > this->m_capacity);

   if (UWR_LIKELY(!!this->m_data)) {
      grow<GF>(req, is_trivially_relocatable<T>());
   }
   else {
      req = fix_capacity(req);
      this->m_data = alloc(req);
      this->m_capacity = req;
   }
}

template<class T>
template<class GF>
constexpr void
hybrid_allocator<T>::grow(size_type req, true_type) {
   req = std::max(GF::num * this->m_capacity / GF::den, req);
   req = fix_capacity(req);
   this->m_data = expand(req, true_type());
   this->m_capacity = req;
}

template<class T>
template<class GF>
constexpr void
hybrid_allocator<T>::grow(size_type req, false_type) {
   size_type next = std::max(GF::num * this->m_capacity / GF::den, req);
   u8 cond = is_big_size(this->m_capacity) | is_big_size(next) << 1;
   switch (cond) {
      case 0b00: { /* both are small sizes */
         next = fix_small_capacity(next);
         pointer new_data = small_alloc(next);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         this->m_data = new_data;
         this->m_capacity = next;
      } break;
      case 0b10: { /* new size is big, old is small */
         next = fix_big_capacity(next);
         pointer new_data = big_alloc(next);
         umove_and_destroy(new_data, this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         this->m_data = new_data;
         this->m_capacity = next;
      } break;
      case 0b11: { /* both are big sizes */
         size_type range_l = npages(req);
         size_type range_r = npages(next) + 1;
         size_type cur_pages = npages(this->m_capacity);
         bool success = false;
         while (range_l + 1 < range_r) {
            size_type range_m = (range_l + range_r) >> 1;
            pointer new_data = (pointer)mremap(this->m_data,
                                               cur_pages * page_size,
                                               range_m * page_size, 0);
            if (new_data != (pointer)-1) {
               UWR_ASSERT(new_data == this->m_data);
               range_l = range_m;
               cur_pages = range_m;
               success = true;
            }
            else {
               range_r = range_m;
            }
         }

         if (success) {
            this->m_capacity = range_l * page_size / sizeof(T);
         }
         else {
            this->m_capacity = cur_pages * page_size / sizeof(T);
            next = fix_big_capacity(next);
            pointer new_data = big_alloc(next);
            umove_and_destroy(new_data, this->m_data, this->m_size);
            big_dealloc(this->m_data, this->m_capacity);
            this->m_data = new_data;
            this->m_capacity = next;
         }
      } break;
      default: /* impossible */
         UWR_ASSERT(false);
   }
}

template<class T>
constexpr bool
hybrid_allocator<T>::expand_or_dealloc_and_alloc_raw(size_type req) {
   UWR_ASSERT(req > this->m_capacity);

   req = fix_capacity(req);
   if (UWR_LIKELY(!!this->m_data)) {
      return expand_or_dealloc_and_alloc_raw(req,
            is_trivially_relocatable<T>());
   }
   else {
      this->m_data = alloc(req);
      this->m_capacity = req;
      return false;
   }
}

template<class T>
constexpr bool
hybrid_allocator<T>::expand_or_dealloc_and_alloc_raw(size_type req, true_type) {
   UWR_ASSERT(req > this->m_capacity);
   UWR_ASSERT(req == fix_capacity(req));

   u8 cond = is_big_size(this->m_capacity) | is_big_size(req) << 1;
   destroy(this->m_data, this->m_size);
   switch (cond) {
      case 0b00: { /* both are small sizes */
         small_dealloc(this->m_data, this->m_capacity);
         this->m_data = small_alloc(req);
      } break;
      case 0b10: { /* new size is big, old is small */
         small_dealloc(this->m_data, this->m_capacity);
         this->m_data = big_alloc(req);
      } break;
      case 0b11: { /* both are big sizes */
         this->m_data = (pointer)mremap(this->m_data,
                                        this->m_capacity * sizeof(T),
                                        req * sizeof(T),
                                        MREMAP_MAYMOVE);
      } break;
      default: /* impossible */
         UWR_ASSERT(false);
   }
   this->m_capacity = req;

   // always return false as these objects are trivial
   // NOTE: assumption is that you don't care about
   //   objects under `data` address, so return
   //   always false so that caller thinks they
   //   are gone
   return false;
}

template<class T>
constexpr bool
hybrid_allocator<T>::expand_or_dealloc_and_alloc_raw(size_type req, false_type) {
   UWR_ASSERT(req > this->m_capacity);
   UWR_ASSERT(req == fix_capacity(req));

   u8 cond = is_big_size(this->m_capacity) | is_big_size(req) << 1;
   switch (cond) {
      case 0b00: { /* both are small */
         destroy(this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         this->m_data = small_alloc(req);
         this->m_capacity = req;
         return false;
      }
      case 0b10: { /* new size is big, old is small */
         destroy(this->m_data, this->m_size);
         small_dealloc(this->m_data, this->m_capacity);
         this->m_data = big_alloc(req);
         this->m_capacity = req;
         return false;
      }
      case 0b11: { /* both are big sizes */
         pointer new_data = (pointer)mremap(this->m_data,
                                            this->m_capacity * sizeof(T),
                                            req * sizeof(T), 0);
         if (new_data == (pointer)-1) {
            // first alloc, then dealloc - why?
            // mremap seems to behave strange
            // if you do the other way around
            // and dramatically reduces in-place
            // reallocations in the future
            new_data = big_alloc(req);
            destroy(this->m_data, this->m_size);
            big_dealloc(this->m_data, this->m_capacity);
            this->m_data = new_data;
            this->m_capacity = req;
            return false;
         }
         else {
            UWR_ASSERT((pointer)new_data == this->m_data);
            this->m_capacity = req;
            return true;
         }
      }
      default: /* impossible */
         UWR_ASSERT(false);
         return false; // keep compiler happy
   }
}

template<class T>
constexpr typename hybrid_allocator<T>::size_type
hybrid_allocator<T>::npages(size_type x) {
   return (x * sizeof(T) + page_size - 1) / page_size;
}

} // namespace uwr::mem
