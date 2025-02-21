/// file      : std/container/vec.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2025, Siddharth Mishra, All rights reserved.
///
/// Provides a type-safe vector implementation in C

#ifndef MISRA_STD_CONTAINER_VEC_H
#define MISRA_STD_CONTAINER_VEC_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// beam
#include <Misra/Std/Container/Common.h>

typedef struct {
    size_t            length;
    size_t            capacity;
    GenericCopyInit   copy_init;
    GenericCopyDeinit copy_deinit;
    void             *data;
} GenericVec;

///
/// Cast any vector to a generic vector
///
#define GENERIC_VEC(x) ((GenericVec *)(void *)(x))

///
/// Typesafe vector definition.
/// This is much like C++ template std::vector<T>
///
/// USAGE:
///   Vec(int) integers; // Vector of integers
///   Vec(CustomStruct) my_data; // Vector of CustomStruct
///   Vec(float) real_numbers; // Vector of float values
///   Vec(const char*) names; Vector of c-style null-terminated strings
///
#define Vec(T)                                                                                     \
    struct {                                                                                       \
        size_t            length;                                                                  \
        size_t            capacity;                                                                \
        GenericCopyInit   copy_init;                                                               \
        GenericCopyDeinit copy_deinit;                                                             \
        T                *data;                                                                    \
    }

#define VEC_DATA_TYPE(v) __typeof__ ((v)->data[0])

///
/// Initialize given vector.
///
/// USAGE:
///   Vec(HttpRequest) requests;
///   InitVec(&requests);
///
/// v[in,out] : Pointer to vector memory that needs to be initialized.
/// ci[in]    : Copy init method.
/// cd[in]    : Copy deinit method.
///
/// SUCCESS : Returns `v` on success
/// FAILURE : Returns NULL otherwise
///
#define VecInit(v, ci, cd)                                                                         \
    (__typeof__ (v))(init_vec (                                                                    \
        GENERIC_VEC (v),                                                                           \
        sizeof ((v)->data[0]),                                                                     \
        (GenericCopyInit)(void *)(ci),                                                             \
        (GenericCopyDeinit)(void *)(cd)                                                            \
    ))

///
/// Deinit vec by freeing all allocations.
///
/// USAGE:
///   Vec(Model)* models = GetAllModels(...);
///   ... // use vector
///   DeinitVec(models)
///
/// v[in,out] : Pointer to vector to be destroyed
///
#define VecDeinit(v) deinit_vec (GENERIC_VEC (v), sizeof ((v)->data[0]))

///
/// Insert item into vector of it's type.
/// Insertion index must not exceed vector length.
/// This preserves the ordering of elements. Best to be used with sorted vectors,
/// if the sorted property is to be preserved.
///
/// In worst case this would to to O(n)
///
/// USAGE:
///   // the data
///   int x = 10;
///   int y = 20;
///
///   // vector
///   Vec(int) integers;
///   InitVec(&integers);
///
///   // insert items
///   VecInsert(&integers, &x, 0); // x inserted at position 0
///   VecInsert(&integers, &y, 0); // x shifted one position and y is inserted
///   VecInsert(&integers, ((int[]){5}), 1); // x shifted one position and 5 is inserted at index 1
///
/// v[in,out]   : Vector to insert item into
/// val[in] : Pointer to value to be inserted
/// idx[in] : Index to insert item at.
///
/// SUCCESS : Returns `v` the vector itself on success.
/// FAILURE : Returns `NULL` otherwise.
///
#define VecInsert(v, val, idx)                                                                     \
    ((__typeof__ (v))(insert_into_vec (GENERIC_VEC (v), (val), sizeof ((v)->data[0]), (idx))))

///
/// Quickly insert item into vector. Ordering of elements is not guaranteed
/// to be preserved. This call makes significant difference only for sufficiently
/// large vectors and when `idx` is quite less than `(v)->length`.
///
/// Insertion time is guaranteed to be constant for same data types.
///
/// Usage is exactly same as `VecInsert`, just the internal implementation is
/// different.
///
/// v[in,out]   : Vector to insert item into
/// val[in] : Pointer to value to be inserted
/// idx[in] : Index to insert item at.
///
/// SUCCESS : Returns `v` the vector itself on success.
/// FAILURE : Returns `NULL` otherwise.
///
#define VecInsertFast(v, val, idx)                                                                 \
    ((__typeof__ (v))(insert_fast_into_vec (GENERIC_VEC (v), (val), sizeof ((v)->data[0]), (idx))))

///
/// Remove item from vector at given index and store in given pointer.
/// Order of elements is guaranteed to be preserved.
///
/// v[in,out] : Vector to remove item from.
/// val[out]  : Where removed item will be stored. If not provided then it's equivalent to
///             deleting the item at specified index.
/// idx[in]   : Index in vector to remove item from.
///
/// SUCCESS : Returns `v` on success.
/// FAILURE : Returns NULL otherwise.
///
#define VecRemove(v, val, idx)                                                                     \
    ((__typeof__ (v))remove_range_vec (GENERIC_VEC (v), (val), sizeof ((v)->data[0]), (idx), 1))

///
/// Remove item from vector at given index and store in given pointer.
/// Order of elements inside vector is not guaranteed to be preserved.
/// The implementation is faster in some scenarios that `VecRemove`
///
/// v[in,out] : Vector to remove item from.
/// val[out]  : Where removed item will be stored. If not provided then it's equivalent to
///             deleting the item at specified index.
/// idx[in]   : Index in vector to remove item from.
///
/// SUCCESS : Returns `v` on success.
/// FAILURE : Returns NULL otherwise.
///
#define VecRemoveFast(v, val, idx)                                                                 \
    ((__typeof__ (v)                                                                               \
    )fast_remove_range_vec (GENERIC_VEC (v), (val), sizeof ((v)->data[0]), (idx), 1))

///
/// Remove data from vector in given range [start, start + count)
/// Order of elements is guaranteed to be preserved.
///
/// v[in,out] : Vector to remove item from.
/// rd[out]   : Where removed data will be stored. If not provided then it's equivalent to
///             deleting the items in specified range.
/// start[in] : Index in vector to removing items from.
/// count[in] : Number of items from starting index.
///
/// SUCCESS : Returns `v` on success.
/// FAILURE : Returns NULL otherwise.
///
#define VecRemoveRange(v, rd, start, count)                                                        \
    ((__typeof__ (v)                                                                               \
    )remove_range_vec (GENERIC_VEC (v), (rd), sizeof (*(v)->data), (start), (count)))


///
/// Remove item from vector at given index and store in given pointer.
/// Order of elements inside vector is not guaranteed to be preserved.
/// The implementation is faster in some scenarios that `VecRemove`
///
/// v[in,out] : Vector to remove item from.
/// rd[out]   : Where removed data will be stored. If not provided then it's equivalent to
///             deleting the items in specified range.
/// start[in] : Index in vector to removing items from.
/// count[in] : Number of items from starting index.
///
/// SUCCESS : Returns `v` on success.
/// FAILURE : Returns NULL otherwise.
///
#define VecRemoveRangeFast(v, rd, start, count)                                                    \
    ((__typeof__ (v)                                                                               \
    )fast_remove_range_vec (GENERIC_VEC (v), (rd), sizeof (*(v)->data), (start), (count)))

///
/// Push item into vector back.
///
/// v[in,out]   : Vector to push item into
/// val[in] : Pointer to value to be pushed
///
/// SUCCESS : Returns `v` the vector itself on success.
/// FAILURE : Returns `NULL` otherwise.
///
#define VecPushBack(v, val) VecInsert ((v), (val), ((v) ? (v)->length : (size_t)-1))

///
/// Pop item from vector back.
///
/// v[in,out]  : Vector to pop item from.
/// val[out]   : Popped item will be stored here. Make sure this has sufficient memory
///              to store memcopied data. If no pointer is provided, then it's equivalent
///              to deleting item from last position.
///
/// SUCCESS : Returns `v` on success
/// FAILURE : Returns NULL otherwise.
///
#define VecPopBack(v, val) VecRemove ((v), (val), ((v) ? (v)->length - 1 : (size_t)-1))

///
/// Push item into vector front.
///
/// v[in,out]   : Vector to push item into
/// val[in] : Pointer to value to be pushed
///
/// SUCCESS : Returns `v` the vector itself on success.
/// FAILURE : Returns `NULL` otherwise.
///
#define VecPushFront(v, val) VecInsert ((v), (val), 0)

///
/// Pop item from vector front.
///
/// v[in,out]  : Vector to pop item from.
/// val[out]   : Popped item will be stored here. Make sure this has sufficient memory
///              to store memcopied data. If no pointer is provided, then it's equivalent
///              to deleting item from last position.
///
/// SUCCESS : Returns `v` on success
/// FAILURE : Returns NULL otherwise.
///
#define VecPopFront(v, val) VecRemove ((v), (val), 0)

///
/// Delete last item from vec
///
#define VecDeleteLast(v) VecPopBack ((v), NULL)

///
/// Delete item at given index
///
#define VecDelete(v, idx) VecRemove ((v), NULL, (idx))

///
/// Delete item at given index using faster implementation.
/// Order preservation is not guaranteed
///
#define VecDeleteFast(v, idx) VecRemoveFast ((v), NULL, (idx))

///
/// Delete items in given range [start, start + count)
///
#define VecDeleteRange(v, start, count) VecRemoveRange ((v), NULL, (start), (count))

///
/// Delete items in given range [start, start + count) using faster implementation.
/// Order preservation is not guaranteed
///
#define VecDeleteRangeFast(v, start, count) VecRemoveRangeFast ((v), NULL, (start), (count))

///
/// Sort given vector with given comparator using quicksort algorithm.
///
/// v[in,out]  : Vector to be sorted.
/// compare[in] : Compare function. Signature and behaviour must be similar to that of `strcmp`.
///
/// SUCCESS : Returns `v` on success.
/// FAILURE : Returns NULL otherwise.
///
#define VecSort(v, compare)                                                                        \
    ((__typeof__ (v))qsort_vec (GENERIC_VEC (v), sizeof ((v)->data[0]), (GenericCompare)(compare)))

///
/// Try reducing memory footprint of vector.
/// This is to be used when we know actual allocated memory for vec is large,
/// and we won't need it in future, so we can reduce it to whatever's required at
/// the moment.
///
/// v[in,out] : Vector
///
/// SUCCESS : `v` on success
/// FAILURE : NULL
///
#define VecTryReduceSpace(v) ((__typeof__ (v))reduce_space_vec (GENERIC_VEC (v)))

///
/// Swap items at given indices.
///
/// v[in,out] : Vector to swap items in.
/// idx1[in]  : Index/Position of first item.
/// idx1[in]  : Index/Position of second item.
///
/// SUCCESS : `v` on success
/// FAILURE : NULL
///
#define VecSwapItems(v, idx1, idx2)                                                                \
    ((__typeof__ (v))swap_vec (GENERIC_VEC (v), sizeof ((v)->data[0]), (idx1), (idx2)))

///
/// Resize vector.
/// If length is smaller than current capacity, vector length is shrinked.
/// If length is greater than current capacity, space is reserved and vector is expanded.
///
/// vec[in,out] : Vector to be resized.
/// len[in]     : New length of vector.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecResize(v, len)                                                                          \
    ((__typeof__ (v))resize_vec (GENERIC_VEC (v), sizeof (VEC_DATA_TYPE (v)), (len)))

///
/// Reserve space for vector.
///
/// vec[in,out] : Vector to be resized.
/// len[in]     : New capacity of vector.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecReserve(v, n)                                                                           \
    ((__typeof__ (v))reserve_vec (GENERIC_VEC (v), sizeof (VEC_DATA_TYPE (v)), (n)))

///
/// Clear vec contents.
///
/// vec[in,out] : Vector to be cleared.
///
/// SUCCESS :
/// FAILURE : NULL
///
#define VecClear(v) ((__typeof__ (v))clear_vec (GENERIC_VEC (v), sizeof ((v)->data[0])))


#define VecFirst(v)     (v)->data[0]
#define VecLast(v)      (v)->data[(v)->length - 1]
#define VecBegin(v)     (v)->data
#define VecEnd(v)       ((v)->data + (v)->length)
#define VecIter(v, idx) ((v)->data + (idx))
#define VecAt(v, idx)   ((v)->data[idx])

///
/// Push a complete array into this vector.
///
/// v[in,out] : Vector to insert array items into.
/// arr[in]   : Array to be inserted.
/// count[in] : Number (non-zero) of items in array.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecPushArr(v, arr, count, pos)                                                             \
    ((__typeof__ (v)                                                                               \
    )push_arr_vec (GENERIC_VEC (v), sizeof ((v)->data[0]), (void *)(arr), (count), (pos)))

///
/// Push a complete array into this vector.
///
/// v[in,out] : Vector to insert array items into.
/// arr[in]   : Array to be inserted.
/// count[in] : Number (non-zero) of items in array.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecPushBackArr(v, arr, count) VecPushArr ((v), (arr), (count), (v)->length)

///
/// Push a complete array into this vector.
///
/// v[in,out] : Vector to insert array items into.
/// arr[in]   : Array to be inserted.
/// count[in] : Number (non-zero) of items in array.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecPushFrontArr(v, arr, count) VecPushArr ((v), (arr), (count), 0)

///
/// Merge two vectors and store the result in first vector.
///
/// v[in,out] : Vector to insert array items into.
/// v2[in]   : Array to be inserted.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecMerge(v, v2) VecPushBackArr ((v), (v2)->data, (v2)->length)

///
/// Reverse contents of this vector.
///
/// v[in,out] : Vector to be reversed.
///
/// SUCCESS : `v`
/// FAILURE : NULL
///
#define VecReverse(v) ((__typeof__ (v))reverse_vec (GENERIC_VEC (v), sizeof ((v)->data[0])))

#define VecForeach(v, var, body)                                                                   \
    do {                                                                                           \
        size_t ___iter___     = 0;                                                                 \
        VEC_DATA_TYPE (v) var = {0};                                                               \
        if ((v) && (v)->length) {                                                                  \
            for ((___iter___) = 0; (___iter___) < (v)->length; ++(___iter___)) {                   \
                var = (v)->data[(___iter___)];                                                     \
                { body }                                                                           \
            }                                                                                      \
        }                                                                                          \
    } while (0)

#define VecForeachReverse(v, var, body)                                                            \
    do {                                                                                           \
        size_t ___iter___     = 0;                                                                 \
        VEC_DATA_TYPE (v) var = {0};                                                               \
        if ((v) && (v)->length) {                                                                  \
            for ((___iter___) = (v)->length - 1; (___iter___) >= 0; --(___iter___)) {              \
                var = (v)->data[(___iter___)];                                                     \
                { body }                                                                           \
            }                                                                                      \
        }                                                                                          \
    } while (0)

#define VecForeachPtr(v, var, body)                                                                \
    do {                                                                                           \
        size_t ___iter___      = 0;                                                                \
        VEC_DATA_TYPE (v) *var = {0};                                                              \
        if ((v) && (v)->length) {                                                                  \
            for ((___iter___) = 0; (___iter___) < (v)->length; ++(___iter___)) {                   \
                var = &(v)->data[(___iter___)];                                                    \
                { body }                                                                           \
            }                                                                                      \
        }                                                                                          \
    } while (0)


#define VecForeachPtrReverse(v, var, body)                                                         \
    do {                                                                                           \
        size_t ___iter___      = 0;                                                                \
        VEC_DATA_TYPE (v) *var = {0};                                                              \
        if ((v) && (v)->length) {                                                                  \
            for ((___iter___) = (v)->length - 1; (___iter___) >= 0; --(___iter___)) {              \
                var = &(v)->data[(___iter___)];                                                    \
                { body }                                                                           \
            }                                                                                      \
        }                                                                                          \
    } while (0)

GenericVec *init_vec (
    GenericVec       *vec,
    size_t            item_size,
    GenericCopyInit   copy_init,
    GenericCopyDeinit copy_deinit
);
void        deinit_vec (GenericVec *vec, size_t item_size);
GenericVec *clear_vec (GenericVec *vec, size_t item_size);
GenericVec *expand_vec (GenericVec *vec, size_t item_size);
GenericVec *resize_vec (GenericVec *vec, size_t item_size, size_t new_size);
GenericVec *reserve_vec (GenericVec *vec, size_t item_size, size_t n);
GenericVec *reserve_pow2_vec (GenericVec *vec, size_t item_size, size_t n);
GenericVec *reduce_space_vec (GenericVec *vec, size_t item_size);
GenericVec *insert_into_vec (GenericVec *vec, void *item_data, size_t item_size, size_t idx);
GenericVec *insert_fast_into_vec (GenericVec *vec, void *item_data, size_t item_size, size_t idx);
GenericVec *remove_range_vec (
    GenericVec *vec,
    void       *removed_data,
    size_t      item_size,
    size_t      start,
    size_t      count
);
GenericVec *fast_remove_range_vec (
    GenericVec *vec,
    void       *removed_data,
    size_t      item_size,
    size_t      start,
    size_t      count
);
GenericVec *qsort_vec (GenericVec *vec, size_t item_size, GenericCompare comp);
GenericVec *swap_vec (GenericVec *vec, size_t item_size, size_t idx1, size_t idx2);
GenericVec *reverse_vec (GenericVec *vec, size_t item_size);
GenericVec *push_arr_vec (GenericVec *vec, size_t item_size, void *arr, size_t count, size_t pos);

#endif // MISRA_STD_CONTAINER_VEC_H
