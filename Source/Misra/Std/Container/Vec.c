/// file      : std/vec.c
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2025, Siddharth Mishra, All rights reserved.
///
/// Generic vector implementation

// ct
#include <Misra/Std/Container/Vec.h>
#include <Misra/Std/Log.h>

GenericVec *init_vec (
    GenericVec       *vec,
    size_t            item_size,
    GenericCopyInit   copy_init,
    GenericCopyDeinit copy_deinit
) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    deinit_vec (vec, item_size);
    vec->copy_init   = copy_init;
    vec->copy_deinit = copy_deinit;

    return vec;
}


void deinit_vec (GenericVec *vec, size_t item_size) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments");
        return;
    }

    if (vec->data) {
        if (vec->copy_deinit) {
            for (size_t i = 0; i < vec->length; i++) {
                vec->copy_deinit (vec->data + i * item_size);
            }
        } else {
            memset (vec->data, 0, item_size * vec->capacity);
        }

        free (vec->data);
    }

    memset (vec, 0, sizeof (GenericVec));
}


GenericVec *clear_vec (GenericVec *vec, size_t item_size) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (vec->data) {
        if (vec->copy_deinit) {
            for (size_t i = 0; i < vec->length; i++) {
                vec->copy_deinit (vec->data + i * item_size);
            }
        } else {
            memset (vec->data, 0, item_size * vec->capacity);
        }
    }

    vec->length = 0;

    return vec;
}


// Increase size for one more item to be stored.
GenericVec *expand_vec (GenericVec *vec, size_t item_size) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (vec->length + 1 > vec->capacity) {
        void *ptr;
        int   n = (vec->capacity == 0) ? 1 : vec->capacity << 1;
        ptr     = realloc (vec->data, n * item_size);
        if (!ptr) {
            LOG_ERROR ("realloc() failed : %s.", strerror (errno));
            return NULL;
        }
        memset (ptr + vec->capacity * item_size, 0, item_size * (n - vec->capacity));
        vec->data     = ptr;
        vec->capacity = n;
    }

    return vec;
}


// Reserve new space if n > capacity
GenericVec *reserve_vec (GenericVec *vec, size_t item_size, size_t n) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (n > vec->capacity) {
        void *ptr = realloc (vec->data, n * item_size);
        if (!ptr) {
            LOG_ERROR ("realloc() failed : %s.", strerror (errno));
            return NULL;
        }
        memset (ptr + vec->capacity * item_size, 0, item_size * (n - vec->capacity));
        vec->data     = ptr;
        vec->capacity = n;
    }

    return vec;
}


GenericVec *reserve_pow2_vec (GenericVec *vec, size_t item_size, size_t n) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    size_t n2 = 1;
    if (n == 0) {
        return vec;
    }

    while (n2 < n) {
        n2 <<= 1;
    }

    return reserve_vec (vec, item_size, n2);
}


GenericVec *reduce_space_vec (GenericVec *vec, size_t item_size) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (vec->length == 0) {
        free (vec->data);
        vec->data     = NULL;
        vec->capacity = 0;
        vec->length   = 0;
        return 0;
    } else {
        void *ptr;
        ptr = realloc (vec->data, vec->length * item_size);
        if (!ptr) {
            LOG_ERROR ("realloc() failed : %s.", strerror (errno));
            return NULL;
        }
        vec->capacity = vec->length;
        vec->data     = ptr;
    }

    return vec;
}


GenericVec *insert_into_vec (GenericVec *vec, void *item_data, size_t item_size, size_t idx) {
    if (!vec || !item_size || !item_data) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (!expand_vec (vec, item_size)) {
        LOG_ERROR ("failed to expand vec memory.");
        return NULL;
    }

    if (idx < vec->length) {
        memmove (
            vec->data + (idx + 1) * item_size,
            vec->data + idx * item_size,
            (vec->length - idx) * item_size
        );

        if (vec->copy_init) {
            vec->copy_init (vec->data + idx * item_size, item_data);
        } else {
            memcpy (vec->data + idx * item_size, item_data, item_size);
        }
    } else if (idx == vec->length) {
        if (vec->copy_init) {
            vec->copy_init (vec->data + vec->length * item_size, item_data);
        } else {
            memcpy (vec->data + vec->length * item_size, item_data, item_size);
        }
    } else {
        LOG_ERROR ("vector index out of bounds.");
        return NULL;
    }

    vec->length += 1;

    return vec;
}

GenericVec *insert_fast_into_vec (GenericVec *vec, void *item_data, size_t item_size, size_t idx) {
    if (!vec || !item_size || !item_data) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (!expand_vec (vec, item_size)) {
        LOG_ERROR ("failed to expand vec memory.");
        return NULL;
    }

    if (idx < vec->length) {
        // move item at index to last and insert the new item directly at index
        memcpy (vec->data + vec->length * item_size, vec->data + idx * item_size, item_size);

        if (vec->copy_init) {
            vec->copy_init (vec->data + idx * item_size, item_data);
        } else {
            memcpy (vec->data + idx * item_size, item_data, item_size);
        }
    } else if (idx == vec->length) {
        if (vec->copy_init) {
            vec->copy_init (vec->data + vec->length * item_size, item_data);
        } else {
            memcpy (vec->data + vec->length * item_size, item_data, item_size);
        }
    } else {
        LOG_ERROR ("vector index out of bounds.");
        return NULL;
    }

    vec->length += 1;

    return vec;
}


GenericVec *remove_range_vec (
    GenericVec *vec,
    void       *removed_data,
    size_t      item_size,
    size_t      start,
    size_t      count
) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (start + count > vec->length) {
        LOG_ERROR ("vector range out of bounds.");
        return NULL;
    }

    if (removed_data) {
        memcpy (removed_data, vec->data + start * item_size, count * item_size);
    } else {
        if (vec->copy_deinit) {
            void *vec_data = vec->data + start * item_size;
            for (size_t s = 0; s < count; s++) {
                vec->copy_deinit (vec_data);
                vec_data += item_size;
            }
        } else {
            memset (vec->data + start * item_size, 0, count * item_size);
        }
    }

    memmove (
        vec->data + start * item_size,
        vec->data + (start + count) * item_size,
        (vec->length - start - count) * item_size
    );
    memset (vec->data + (vec->length - start - count) * item_size, 0, count * item_size);

    vec->length -= count;

    return vec;
}


GenericVec *fast_remove_range_vec (
    GenericVec *vec,
    void       *removed_data,
    size_t      item_size,
    size_t      start,
    size_t      count
) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (start + count > vec->length) {
        LOG_ERROR ("vector range out of bounds.");
        return NULL;
    }

    if (removed_data) {
        memcpy (removed_data, vec->data + start * item_size, count * item_size);
    } else {
        if (vec->copy_deinit) {
            void *vec_data = vec->data + start * item_size;
            for (size_t s = 0; s < count; s++) {
                vec->copy_deinit (vec_data);
                vec_data += item_size;
            }
        } else {
            memset (vec->data + start * item_size, 0, count * item_size);
        }
    }

    memmove (
        vec->data + start * item_size,
        vec->data + (vec->length - count) * item_size,
        count * item_size
    );
    memset (vec->data + (vec->length - start - count) * item_size, 0, count * item_size);

    vec->length -= count;

    return vec;
}


GenericVec *qsort_vec (GenericVec *vec, size_t item_size, GenericCompare comp) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    qsort (vec->data, vec->length, item_size, comp);

    return vec;
}


GenericVec *swap_vec (GenericVec *vec, size_t item_size, size_t idx1, size_t idx2) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (idx1 >= vec->length || idx2 >= vec->length) {
        LOG_ERROR ("vector index out of bounds.");
        return NULL;
    }

    if (idx1 == idx2) {
        return vec;
    }

    unsigned char *a, *b, tmp;
    a = (unsigned char *)vec->data + idx1 * item_size;
    b = (unsigned char *)vec->data + idx2 * item_size;
    while (item_size--) {
        tmp = *a;
        *a  = *b;
        *b  = tmp;
        a++, b++;
    }

    return vec;
}


GenericVec *reverse_vec (GenericVec *vec, size_t item_size) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    size_t i = vec->length / 2;
    while (i--) {
        swap_vec (vec, item_size, i, vec->length - (i + 1));
    }

    return vec;
}


GenericVec *push_arr_vec (GenericVec *vec, size_t item_size, void *arr, size_t count, size_t pos) {
    if (!vec || !arr || !count || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (pos > vec->length) {
        LOG_ERROR ("vector index out of range.");
        return NULL;
    }

    if (!reserve_pow2_vec (vec, item_size, vec->length + count)) {
        LOG_ERROR ("failed to reserve memory for vector.");
        return NULL;
    }

    // shift data if being inserted in the middle
    if (pos < vec->length) {
        memmove (
            vec->data + (pos + count) * item_size,
            vec->data + pos * item_size,
            count * item_size
        );

        memset (vec->data + pos * item_size, 0, count * item_size);
    }

    // insert data
    if (vec->copy_init) {
        void *data = vec->data + pos * item_size;
        while (count--) {
            vec->copy_init (data, arr);
            arr  += item_size;
            data += item_size;
        }
    } else {
        memcpy (vec->data + pos * item_size, arr, count * item_size);
    }

    vec->length += count;

    return vec;
}


GenericVec *resize_vec (GenericVec *vec, size_t item_size, size_t new_size) {
    if (!vec || !item_size) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    if (new_size <= vec->capacity) {
        // if we're shrinking then we need to remove some part of the data
        if (new_size < vec->length) {
            remove_range_vec (vec, NULL, item_size, new_size, vec->length - new_size);
        }
        vec->length = new_size;
    } else {
        if (!reserve_pow2_vec (vec, item_size, new_size)) {
            LOG_ERROR ("vec reserve failed.");
            return NULL;
        }
        vec->length = new_size;
    }

    return vec;
}
