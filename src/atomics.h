/*
 * Copyright (c) 2015 Andrew Kelley
 *
 * This file is part of libsoundio, which is MIT licensed.
 * See http://opensource.org/licenses/MIT
 */

#ifndef SOUNDIO_ATOMICS_H
#define SOUNDIO_ATOMICS_H

// Simple wrappers around atomic values so that the compiler will catch it if
// I accidentally use operators such as +, -, += on them.

#ifdef __cplusplus

#include <atomic>

struct SoundIoAtomicLong {
    std::atomic<long> x;
};

struct SoundIoAtomicInt {
    std::atomic<int> x;
};

struct SoundIoAtomicBool {
    std::atomic<bool> x;
};

struct SoundIoAtomicFlag {
    std::atomic_flag x;
};

struct SoundIoAtomicULong {
    std::atomic<unsigned long> x;
};

#define SOUNDIO_ATOMIC_LOAD(a) (a.x.load())
#define SOUNDIO_ATOMIC_FETCH_ADD(a, delta) (a.x.fetch_add(delta))
#define SOUNDIO_ATOMIC_STORE(a, value) (a.x.store(value))
#define SOUNDIO_ATOMIC_EXCHANGE(a, value) (a.x.exchange(value))
#define SOUNDIO_ATOMIC_FLAG_TEST_AND_SET(a) (a.x.test_and_set())
#define SOUNDIO_ATOMIC_FLAG_CLEAR(a) (a.x.clear())
#define SOUNDIO_ATOMIC_FLAG_INIT ATOMIC_FLAG_INIT

#else

//#include <stdatomic.h>
#include <stdbool.h>
#include "tinycthread.h"

struct SoundIoAtomicLong {
    long x;
};

struct SoundIoAtomicInt {
    int x;
};

struct SoundIoAtomicBool {
    bool x;
};

struct SoundIoAtomicFlag {
    bool x;
};

struct SoundIoAtomicULong {
    unsigned long x;
};



#define SOUNDIO_ATOMIC_LOAD(a) ({ int _val; mtx_lock(&(a).mutex); _val = (a).x; mtx_unlock(&(a).mutex); _val; })
#define SOUNDIO_ATOMIC_FETCH_ADD(a, delta) ({ int _old; mtx_lock(&(a).mutex); _old = (a).x; (a).x += (delta); mtx_unlock(&(a).mutex); _old; })
#define SOUNDIO_ATOMIC_STORE(a, value) do { mtx_lock(&(a).mutex); (a).x = (value); mtx_unlock(&(a).mutex); } while (0)
#define SOUNDIO_ATOMIC_EXCHANGE(a, value) ({ int _old; mtx_lock(&(a).mutex); _old = (a).x; (a).x = (value); mtx_unlock(&(a).mutex); _old; })
#define SOUNDIO_ATOMIC_FLAG_TEST_AND_SET(f) ({ int _old; mtx_lock(&(f).mutex); _old = (f).flag; (f).flag = 1; mtx_unlock(&(f).mutex); _old; })
#define SOUNDIO_ATOMIC_FLAG_CLEAR(f) do { mtx_lock(&(f).mutex); (f).flag = 0; mtx_unlock(&(f).mutex); } while (0)
#define SOUNDIO_ATOMIC_FLAG_INIT false


#endif

#endif
