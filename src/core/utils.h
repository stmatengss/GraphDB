#ifndef UTILS_H
#define UTILS_H

#include <ctime>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstdarg>
#include <unistd.h>
#include <cerrno>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define forceinline     inline __attribute__((always_inline))
#define packed          __attribute__((packed))

static long long
get_usec() {
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return (long long)now.tv_sec * 1000000 + now.tv_nsec / 1000;
}

static void
nano_sleep(int nsec) {
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = nsec;
    if (nanosleep(&tim, &tim2) < 0) {
        printf("SLEEP ERROR!\n");
        exit(1);
    }
}

static inline unsigned long long asm_rdtsc(void){
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((unsigned long long)lo)|(((unsigned long long)hi) << 32);
}

static inline uint32_t fastrand(uint64_t* seed) {
    *seed = *seed * 1103515245 + 12345;
    return (uint32_t)(*seed >> 32);
}

#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */

void REDLOG(const char *format, ...)
{   
    va_list args;

    char buf1[1000], buf2[1000];
    memset(buf1, 0, 1000);
    memset(buf2, 0, 1000);

    va_start(args, format);
    vsnprintf(buf1, 1000, format, args);

    snprintf(buf2, 1000, "\033[31m%s\033[0m", buf1);
    printf("%s", buf2);
    //write(2, buf2, 1000);
    va_end( args );
}

void GREENLOG(const char *format, ...)
{   
    va_list args;

    char buf1[1000], buf2[1000];
    memset(buf1, 0, 1000);
    memset(buf2, 0, 1000);

    va_start(args, format);
    vsnprintf(buf1, 1000, format, args);

    snprintf(buf2, 1000, "\033[32m%s\033[0m", buf1);
    printf("%s", buf2);
    //write(2, buf2, 1000);
    va_end( args );
}

static inline uint32_t hrd_fastrand() {
  uint64_t seed = rand() * 1103515245 + 12345;
  return (uint32_t)(seed >> 32);
}

forceinline void PRINT_BINARY_CHAR(char num) {
	for (int i = 0; i < 8; i ++ ) {
		if(num & (1<<(7-i)))
			printf("1");
		else 
			printf("0");
	}
	printf("\n");
}

#define ALIGN_MEM(x, y) ((x + (y - 1)) / y * y)
#define MAX(x, y)       (x > y ? x : y)
#define MIN(x, y)       (x < y ? x : y)
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#define cacheline       __attribute__((aligned(64)))
#define _unused(x)      ((void)(x))
#define IS_ALIGNED(x, A)        (((uint64_t) x) % A == 0)
#define LOG(...)        fprintf(stdout, __VA_ARGS__)
// #define LOG(...) do{}while(0);
#define ERR(...)        fprintf(stderr, __VA_ARGS__)
#define range_assert(x, a, b)   (assert(x >= a && x <= b))
#define make_nbit_1(x, n)       (x | (1 << n))
#define make_nbit_0(x, n)       (x & ~(1 << n))
#define CASLOCK(lock)   while(!__sync_bool_compare_and_swap(&lock, 0, 1))
#define CASUNLOCK(lock) assert(lock == 1);  lock = 0
#if (defined NDEBUG)
#define assert_range(x, a, b, msg) nullptr;
#else
#define assert_range(x, a, b, msg) do { \
    if (x < a || x >= b) { \
        GREENLOG("[ERR] %ld <=> %ld (%ld)\n", a, b, x); \
        GREENLOG("[ADD] %p\n", (void *)(r_nvm_mem + x)); \
        GREENLOG("[MSG] %p\n", msg); \
        assert(0); \
    } \
} while (0);
#endif
#define assert_range_inv(x, a, b, msg) do { \
    if (x < a || x >= b) { \
        GREENLOG("[NUM] x\n", x); \
        GREENLOG("[MSG] %p\n", msg); \
        assert(0); \
    } \
} while (0);
#define mem_dump(x, n) do { \
    LOG("[MEM_DUMP]%p\n", x); \
    for (int i = 0; i < n; i ++ ) { \
        printf("<%d>\t", i); \
        PRINT_BINARY_CHAR(*(x + i)); \
    } \
} while(0);
#define BENCH_BEGIN {long long pre_time; int counter = 0; double sum = 0;}

#define BENCH_COUNTER(print_dis) { \
    long long time_gap = m_get_usec() - pre_time; \
    counter ++; \
    double print_val = (double)print_dis / ((double)time_gap / 1000000); \
    printf("[W_Tput] %.6lf MOPS\n", print_val); \
    sum += print_val; \
    pre_time = m_get_usec(); \
}

#define BENCH_END() { \
    REDLOG("[Tput_AVE] %.6lf MOPS\n", sum / counter); \
}

#define POLL_EQ(ptr, value) { \
    volatile char *poll_ptr = (volatile char *)ptr; \
    while (char(poll_ptr[0]) == value) {} \
}

#define POLL_NEQ(ptr, value) { \
    volatile char *poll_ptr = (volatile char *)ptr; \
    while (! (char(poll_ptr[0]) == value) ) {} \
}

#define SET_FLAG(ptr, value) { \
    ((char *)ptr)[0] = value; \
}

#endif
