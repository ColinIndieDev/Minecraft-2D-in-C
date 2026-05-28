#pragma once

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define VEC_DEF(type, name) VEC_DECL(type, name); VEC_IMPL(type, name);

#define VEC_DECL(type, name)                                                   \
    typedef struct {                                                           \
        type *data;                                                            \
        u32 size;                                                              \
        u32 capacity;                                                          \
    } name;                                                                    \
    void name##_init(name *v, u32 size, type val);                             \
    void name##_reserve(name *v, u32 size);                                    \
    type *name##_at(name *v, u32 i);                                           \
    type name##_get(name *v, u32 i);                                           \
    void name##_push_back(name *v, type val);                                  \
    void name##_push_front(name *v, type val);                                 \
    void name##_pop_back(name *v);                                             \
    void name##_pop_front(name *v);                                            \
    void name##_delete(name *v, u32 i);                                        \
    void name##_destroy(name *v);                                              \
    void name##_set(name *v, u32 i, type val);                                 \
    void name##_copy(name *v, name *dest);                                     \
    void name##_clear(name *v);                                                \
    type *name##_begin(name *v);                                               \
    type *name##_end(name *v);                                                 \
    type *name##_front(name *v);                                               \
    type *name##_back(name *v);                                                \
    b8 name##_empty(name *v);

#define VEC_IMPL(type, name)                                                   \
    void name##_init(name *v, u32 size, type val) {                            \
        assert(v != NULLPTR);                                                  \
        v->capacity = size > 10 ? size * 2 : 10;                               \
        v->data = malloc(v->capacity * sizeof(type));                          \
        v->size = size;                                                        \
        for (u32 i = 0; i < v->size; i++) {                                    \
            v->data[i] = val;                                                  \
        }                                                                      \
    }                                                                          \
    void name##_reserve(name *v, u32 size) {                                   \
        assert(v != NULLPTR);                                                  \
        assert(size > 0);                                                      \
        v->capacity = size;                                                    \
        v->data = malloc(v->capacity * sizeof(type));                          \
        v->size = 0;                                                           \
    }                                                                          \
    type *name##_at(name *v, u32 i) {                                          \
        assert(v != NULLPTR);                                                  \
        assert(0 <= i && i < v->size && v->size > 0);                          \
        return &v->data[i];                                                    \
    }                                                                          \
    type name##_get(name *v, u32 i) {                                          \
        assert(v != NULLPTR);                                                  \
        assert(0 <= i && i < v->size && v->size > 0);                          \
        return v->data[i];                                                     \
    }                                                                          \
    void name##_push_back(name *v, type val) {                                 \
        assert(v != NULLPTR);                                                  \
        if (v->size >= v->capacity) {                                          \
            v->capacity *= 2;                                                  \
            type *temp = realloc(v->data, v->capacity * sizeof(type));         \
            assert(temp != NULLPTR);                                           \
            v->data = temp;                                                    \
        }                                                                      \
        v->data[v->size] = val;                                                \
        v->size++;                                                             \
    }                                                                          \
    void name##_push_front(name *v, type val) {                                \
        assert(v != NULLPTR);                                                  \
        if (v->size >= v->capacity) {                                          \
            type *new_data =                                                   \
                realloc(v->data, (u64)v->capacity * 2 * sizeof(type));         \
            if (new_data != NULLPTR) {                                         \
                v->capacity *= 2;                                              \
                v->data = new_data;                                            \
            }                                                                  \
        }                                                                      \
        memmove(&v->data[1], &v->data[0], v->size * sizeof(type));             \
        v->data[0] = val;                                                      \
        v->size++;                                                             \
    }                                                                          \
    void name##_pop_back(name *v) {                                            \
        assert(v != NULLPTR);                                                  \
        assert(v->size > 0);                                                   \
        v->size--;                                                             \
    }                                                                          \
    void name##_pop_front(name *v) {                                           \
        assert(v != NULLPTR);                                                  \
        assert(v->size > 0);                                                   \
        memmove(&v->data[0], &v->data[1], (v->size - 1) * sizeof(type));       \
        v->size--;                                                             \
    }                                                                          \
    void name##_delete(name *v, u32 i) {                                       \
        assert(v != NULLPTR);                                                  \
        assert(0 <= i && i < v->size && v->size > 0);                          \
        memmove(&v->data[i], &v->data[i + 1],                                  \
                (v->size - i - 1) * sizeof(type));                             \
        v->size--;                                                             \
    }                                                                          \
    void name##_destroy(name *v) {                                             \
        assert(v != NULLPTR);                                                  \
        free(v->data);                                                         \
        v->size = 0;                                                           \
        v->capacity = 0;                                                       \
    }                                                                          \
    void name##_set(name *v, u32 i, type val) {                                \
        assert(v != NULLPTR);                                                  \
        assert(0 <= i && i < v->size && v->size > 0);                          \
        v->data[i] = val;                                                      \
    }                                                                          \
    void name##_copy(name *v, name *dest) {                                    \
        assert(v != NULLPTR && dest != NULLPTR);                               \
        dest->size = v->size;                                                  \
        dest->capacity = v->capacity;                                          \
        if (dest->capacity < v->size) {                                        \
            if (dest->data != NULLPTR) {                                       \
                free(dest->data);                                              \
            }                                                                  \
            dest->capacity = v->size * 2;                                      \
            dest->data = malloc((u64)v->size * 2 * sizeof(type));              \
        }                                                                      \
        for (int i = 0; i < v->size; i++) {                                    \
            dest->data[i] = v->data[i];                                        \
        }                                                                      \
    }                                                                          \
    void name##_clear(name *v) {                                               \
        assert(v != NULLPTR);                                                  \
        v->size = 0;                                                           \
    }                                                                          \
    type *name##_begin(name *v) {                                              \
        assert(v != NULLPTR);                                                  \
        return v->data;                                                        \
    }                                                                          \
    type *name##_end(name *v) {                                                \
        assert(v != NULLPTR);                                                  \
        return v->data + v->size;                                              \
    }                                                                          \
    type *name##_front(name *v) {                                              \
        assert(v != NULLPTR);                                                  \
        return &v->data[0];                                                    \
    }                                                                          \
    type *name##_back(name *v) {                                               \
        assert(v != NULLPTR);                                                  \
        return &v->data[v->size - 1];                                          \
    }                                                                          \
    b8 name##_empty(name *v) {                                                 \
        assert(v != NULLPTR);                                                  \
        return v->size <= 0;                                                   \
    }

#define FOREACH_VEC(type, vtype, it, vptr)                                     \
    for (type *it = vtype##_begin(vptr); it != vtype##_end(vptr); it++)

#define VEC_ERASE_IF(v, cond)                                                  \
    do {                                                                       \
        u32 w = 0;                                                             \
        for (u32 _i = 0; _i < (v)->size; _i++) {                               \
            __auto_type it = (v)->data[_i];                                    \
            if (!(cond)) {                                                     \
                (v)->data[w++] = (v)->data[_i];                                \
            }                                                                  \
        }                                                                      \
        (v)->size = w;                                                         \
    } while (0)
