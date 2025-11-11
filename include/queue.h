#pragma once

#include <stdbool.h>
#include <threads.h>

#define LockableQueuePrototypes(T)                                             \
    typedef struct LockableQueue_##T {                                         \
        T* items;                                                              \
        size_t size;                                                           \
        size_t capacity;                                                       \
        size_t at;                                                             \
        mtx_t lock;                                                            \
    } LockableQueue_##T;                                                       \
    typedef struct LockableQueue_##T##_Result {                                \
        T item;                                                                \
        bool success;                                                          \
    } LockableQueue_##T##_Result;                                              \
    [[nodiscard]] LockableQueue_##T lockable_queue_##T##_init(                 \
        size_t capacity);                                                      \
    /* Will block until available to add, but drops the input if q is full */  \
    void lockable_queue_##T##_add(LockableQueue_##T* q, T item);               \
    [[nodiscard]] bool lockable_queue_##T##_tryadd(LockableQueue_##T q,        \
                                                   T item);                    \
    T lockable_queue_##T##_get(LockableQueue_##T q, T item);                   \
    LockableQueue_##T##_Result lockable_queue_##T##_tryget(                    \
        LockableQueue_##T q, T item)

#define LockableQueueImpl(T)                                                   \
    [[nodiscard]] LockableQueue_##T lockable_queue_##T##_init(                 \
        size_t capacity) {                                                     \
        LockableQueue_##T lq =                                                 \
            (LockableQueue_##T){.items = calloc(sizeof(T) * capacity, 1),      \
                                .size = 0,                                     \
                                .capacity = capacity,                          \
                                .at = 0};                                      \
        int result = mtx_init(&lq.lock, mtx_plain);                            \
        if (result != thrd_success) {                                          \
            perror("mtx_init");                                                \
            exit(1);                                                           \
        }                                                                      \
                                                                               \
        return lq;                                                             \
    }                                                                          \
    void lockable_queue_##T##_add(LockableQueue_##T* q, T item) {              \
        mtx_lock(&q->lock);                                                    \
        if (q->size != q->capacity) {                                          \
            q->items[(q->at + q->size++) % q->capacity] = item;                \
        }                                                                      \
        mtx_unlock(&q->lock);                                                  \
    }                                                                          \
    [[nodiscard]] bool lockable_queue_##T##_tryadd(LockableQueue_##T* q,       \
                                                   T item) {                   \
        int result = mtx_trylock(&q->lock);                                    \
        if (result != thrd_success) {                                          \
            return false;                                                      \
        }                                                                      \
                                                                               \
        if (q->size != q->capacity) {                                          \
            q->items[(q->at + q->size++) % q->capacity] = item;                \
        }                                                                      \
        mtx_unlock(&q->lock);                                                  \
                                                                               \
        return true;                                                           \
    }
