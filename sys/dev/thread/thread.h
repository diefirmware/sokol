#ifndef THREAD_RUNTIME_H
#define THREAD_RUNTIME_H

#include <stdint.h>
#include <stddef.h>

/*
 * thread_runtime.h
 *
 * Per-thread CPU-cycle accounting for a non-Linux kernel.
 *
 * Usage:
 *  - Include this header in your scheduler and thread-management code.
 *  - Call switch_thread_runtime(old, new) in your context-switch path.
 *  - Query runtimes with thread_runtime_get_cycles() and
 *    thread_runtime_cycles_to_ns().
 */

/* Opaque thread handle; your kernel’s thread struct must embed this. */
typedef struct thread_runtime {
    uint64_t last_start_cycles;  /* Cycle count when the thread was last scheduled in */
    uint64_t accum_cycles;       /* Total cycles consumed by this thread so far */
} thread_runtime_t;

/*
 * Read the processor’s cycle counter.
 * 
 * On x86: uses RDTSC (requires invariant TSC). On other architectures,
 * you must provide your own implementation.
 */
static inline uint64_t thread_runtime_read_cycles(void) {
#if defined(__i386__) || defined(__x86_64__)
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#else
    /* Fallback stub; replace with your platform’s cycle‑counter read */
    return 0;
#endif
}

/*
 * Instrumented context-switch hook.
 *
 * Must be called by the scheduler when switching out 'old_rt' and
 * switching in 'new_rt'. If 'old_rt' is NULL (e.g. on first-ever switch)
 * or 'new_rt' is NULL (e.g. shutting down), those arguments may be NULL.
 */
static inline void switch_thread_runtime(thread_runtime_t *old_rt,
                                         thread_runtime_t *new_rt) {
    uint64_t now = thread_runtime_read_cycles();

    if (old_rt != NULL) {
        old_rt->accum_cycles += (now - old_rt->last_start_cycles);
    }

    if (new_rt != NULL) {
        new_rt->last_start_cycles = now;
    }
}

/*
 * Get total cycles consumed by a thread so far.
 *
 * If you pass a pointer to the currently running thread’s runtime struct,
 * this will include the cycles consumed up to the moment of the call.
 */
static inline uint64_t thread_runtime_get_cycles(const thread_runtime_t *rt,
                                                 int is_current_thread) {
    uint64_t total = rt->accum_cycles;

    if (is_current_thread) {
        uint64_t now = thread_runtime_read_cycles();
        total += (now - rt->last_start_cycles);
    }

    return total;
}

/*
 * Convert cycle counts to nanoseconds given the CPU frequency (Hz).
 * Beware of overflow for very large cycle counts; use 128-bit if needed.
 */
static inline uint64_t thread_runtime_cycles_to_ns(uint64_t cycles,
                                                   uint64_t cpu_hz) {
    return (cycles * 1000000000ULL) / cpu_hz;
}

/*
 * Initialize a thread_runtime_t structure.
 * Must be called when creating a new thread.
 */
static inline void thread_runtime_init(thread_runtime_t *rt) {
    rt->last_start_cycles = 0;
    rt->accum_cycles      = 0;
}

#endif /* THREAD_RUNTIME_H */

