#include<stdio_ext.h>
#include<stdint.h>


int main (){


typedef struct thread thread;

struct thread{
int32_t id;
char * name;
int*_t cursor;
int64_t max_len;

}


thread Thread;


// Read current cycle counter (x86 or ARM) – inline assembly
static inline uint64_t read_cycles(void) {
    uint32_t lo, hi;
    asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

// Called by the core scheduler when switching out 'old' and in 'new'
void switch_threads(thread_t *old, thread_t *new) {
    uint64_t now = read_cycles();

    // 1. Account for 'old' thread’s usage
    if (old) {
        old->accum_cycles += now - old->last_start_cycles;  // :contentReference[oaicite:7]{index=7}
    }

    // 2. Record start for 'new' thread
    new->last_start_cycles = now;

    // 3. Perform the low‑level context switch (stack, registers,…)
    arch_context_switch(&old->context, &new->context);
}




// Return total cycles so far, without disturbing accounting
uint64_t thread_get_cycles(thread_t *t) {
    uint64_t now = read_cycles();
    // If it's currently running, include up‑to‑now delta
    if (t == current_thread) {
        return t->accum_cycles + (now - t->last_start_cycles);
    } else {
        return t->accum_cycles;
    }
}

// Convert cycles to nanoseconds given CPU frequency in Hz
uint64_t cycles_to_ns(uint64_t cycles, uint64_t cpu_hz) {
    // (cycles * 1e9) / cpu_hz
    return (cycles * 1000000000ULL) / cpu_hz;  // :contentReference[oaicite:9]{index=9}
}
return 0;
}
