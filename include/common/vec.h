#ifndef VEC_H
#define VEC_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


///
/// +--------+--------+--------+--------+--------+--------+
/// | header | vec[0] | vec[1] | vec[2] |  ....  | vec[N] |
/// +--------+--------+--------+--------+--------+--------+
/// ^         ^ Vec(T) given to you
/// |
/// Start of allocated memory
/// 
/// Every `Vec(T)` has a `VecHeader` that sits behind it in memory.
/// This header is where the allocated memory really starts, and
/// `Vec(T)` is just a pointer to where the element array starts.

/// Information about a vector that sits behind the element array.
typedef struct VecHeader {
    /// The vector's length (currently used space).
    uint32_t len;
    /// The vector's allocated capacity.
    uint32_t cap;
} VecHeader;

/// Small utility to catch errors where someone passes
/// the vector by value instead of by reference
#define _vec_assert_ptr_ptr(pp) (((void)**(pp)), (pp))

/// Vector (array list) of type T.
#define Vec(T) T*
/// Width of the elements of a vector.
#define vec_stride(vec) sizeof(*(vec))
/// This vector's `VecHeader`.
#define vec_header(vec) ((VecHeader*)((char*)(vec) - sizeof(VecHeader)))
/// Get the pointer to a vec's elements from the header.
#define vec_elems_from_header(header) ((void*)((char*)(header) + sizeof(VecHeader)))
/// This vector's length.
#define vec_len(vec) vec_header(vec)->len
/// This vector's capacity.
#define vec_cap(vec) vec_header(vec)->cap
/// Clear the vector (set it's length to zero).
#define vec_clear(vecptr) (vec_len(*_vec_assert_ptr_ptr(vecptr)) = 0)
/// Create a new vector, storing type `T` with an initial capacity `initial_cap`.
#define vec_new(T, initial_cap) ((Vec(T)) _vec_new(sizeof(T), initial_cap))
/// Ensure the vector `vecptr` can append `slots` items without allocating.
#define vec_reserve(vecptr, slots) _vec_reserve((void**)_vec_assert_ptr_ptr(vecptr), vec_stride(*vecptr), slots)
/// Append `item` to vector `vecptr`.
#define vec_append(vecptr, item) do { \
    _vec_reserve1((void**)_vec_assert_ptr_ptr(vecptr), vec_stride(*vecptr)); \
    (*vecptr)[vec_len(*vecptr)++] = (item); \
} while (0)
/// reallocate the vector `vecptr` such that its capapcity is equal to its length. 
#define vec_shrink(vecptr) _vec_shrink((void**)_vec_assert_ptr_ptr(vecptr), vec_stride(*vecptr))
/// Destroy and deallocate vector `vecptr`
#define vec_destroy(vecptr) _vec_destroy((void**)_vec_assert_ptr_ptr(vecptr))
/// Peek at the last element of a vector.
#define vec_peek(vec) (vec)[vec_len(vec)-1]

Vec(void) _vec_new(size_t stride, size_t initial_cap);
void _vec_reserve(Vec(void)* v, size_t stride, size_t slots);
void _vec_reserve1(Vec(void)* v, size_t stride);
void _vec_shrink(Vec(void)* v, size_t stride);
void _vec_destroy(Vec(void)* v);

#endif // VEC_H
