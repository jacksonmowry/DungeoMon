#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

typedef enum Result { SUCCESS, BLOCKED, EMPTY, FULL } Result;

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
        Result status;                                                         \
    } LockableQueue_##T##_Result;                                              \
    [[nodiscard]] LockableQueue_##T lockable_queue_##T##_init(                 \
        size_t capacity);                                                      \
    void lockable_queue_##T##_deinit(LockableQueue_##T q);                     \
    /* Will block until available to add, but drops the input if q is full */  \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_add(         \
        LockableQueue_##T* q, T item);                                         \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_tryadd(      \
        LockableQueue_##T* q, T item);                                         \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_get(         \
        LockableQueue_##T* q);                                                 \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_tryget(      \
        LockableQueue_##T* q)

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
    void lockable_queue_##T##_deinit(LockableQueue_##T q) {                    \
        if (q.items) {                                                         \
            free(q.items);                                                     \
        }                                                                      \
    }                                                                          \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_add(         \
        LockableQueue_##T* q, T item) {                                        \
        LockableQueue_##T##_Result result = {0};                               \
        mtx_lock(&q->lock);                                                    \
        if (q->size != q->capacity) {                                          \
            q->items[(q->at + q->size++) % q->capacity] = item;                \
            result.status = SUCCESS;                                           \
        } else {                                                               \
            result.status = FULL;                                              \
        }                                                                      \
        mtx_unlock(&q->lock);                                                  \
                                                                               \
        return result;                                                         \
    }                                                                          \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_tryadd(      \
        LockableQueue_##T* q, T item) {                                        \
        LockableQueue_##T##_Result result = {0};                               \
        int mtx_result = mtx_trylock(&q->lock);                                \
        if (mtx_result != thrd_success) {                                      \
            result.status = BLOCKED;                                           \
            return result;                                                     \
        }                                                                      \
                                                                               \
        if (q->size != q->capacity) {                                          \
            q->items[(q->at + q->size++) % q->capacity] = item;                \
            result.status = SUCCESS;                                           \
        } else {                                                               \
            result.status = FULL;                                              \
        }                                                                      \
                                                                               \
        mtx_unlock(&q->lock);                                                  \
        return result;                                                         \
    }                                                                          \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_get(         \
        LockableQueue_##T* q) {                                                \
        LockableQueue_##T##_Result result = {0};                               \
        mtx_lock(&q->lock);                                                    \
                                                                               \
        if (q->size != 0) {                                                    \
            result.item = q->items[q->at];                                     \
            q->at++;                                                           \
            q->at %= q->capacity;                                              \
            result.status = SUCCESS;                                           \
        } else {                                                               \
            result.status = EMPTY;                                             \
        }                                                                      \
                                                                               \
        mtx_unlock(&q->lock);                                                  \
        return result;                                                         \
    }                                                                          \
    [[nodiscard]] LockableQueue_##T##_Result lockable_queue_##T##_tryget(      \
        LockableQueue_##T* q) {                                                \
        LockableQueue_##T##_Result result = {0};                               \
        int mtx_result = mtx_trylock(&q->lock);                                \
        if (mtx_result != thrd_success) {                                      \
            result.status = BLOCKED;                                           \
            return result;                                                     \
        }                                                                      \
                                                                               \
        if (q->size != 0) {                                                    \
            result.item = q->items[q->at];                                     \
            q->at++;                                                           \
            q->at %= q->capacity;                                              \
            q->size--;                                                         \
            result.status = SUCCESS;                                           \
        } else {                                                               \
            result.status = EMPTY;                                             \
        }                                                                      \
                                                                               \
        mtx_unlock(&q->lock);                                                  \
        return result;                                                         \
    }
