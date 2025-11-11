#pragma once

#include <stdbool.h>
#include <threads.h>

#define LockableQueueType(T)                                                   \
    typedef struct LockableQueue_##T {                                         \
        T* items;                                                              \
        size_t size;                                                           \
        size_t capacity;                                                       \
        mtx_t lock;                                                            \
    } LockableQueue_##T;                                                       \
    typedef struct LockableQueue_##T##_Result {                                \
        T item;                                                                \
        bool success;                                                          \
    } LockableQueue_##T##_Result;                                              \
    LockableQueue_##T lockable_queue_##T##_init(size_t capacity);              \
    void lockable_queue_##T##_add(T item);                                     \
    bool lockable_queue_##T_tryadd(T item);                                    \
    T lockable_queue_##T_get(T item);                                          \
    LockableQueue_##T##_Result lockable_queue_##T_tryget(T item)
