#ifndef KLESSYDRA_HDC_TEST_COMMON_HPP
#define KLESSYDRA_HDC_TEST_COMMON_HPP

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "csr_access.hpp"
#include "hdc_class.hpp"

extern "C" {
#include "functions.h"
#include "utils.h"
}

namespace klessydra {
namespace hdc {
namespace tests {

using TestBody = int (*)();

#ifndef FHRR_TEST_DEBUG_MODE
#define FHRR_TEST_DEBUG_MODE 0
#endif

#ifndef FHRR_TEST_DIRECT_MODE
#define FHRR_TEST_DIRECT_MODE FHRR_TEST_DEBUG_MODE
#endif

#ifndef FHRR_TEST_VECTOR_ELEMENTS
#define FHRR_TEST_VECTOR_ELEMENTS 0
#endif

#ifndef FHRR_TEST_ENCODE_ROWS
#define FHRR_TEST_ENCODE_ROWS 0
#endif

#ifndef FHRR_TEST_FIXED_SEED
#define FHRR_TEST_FIXED_SEED 0
#endif

inline void print_status_line(const char* name, Status status) {
    std::printf("[%s] status=%s\n", name, to_string(status));
}

inline std::uint32_t read_mcycle() {
    std::uint32_t value = 0;
    __asm__ volatile("csrr %0, mcycle" : "=r"(value));
    return value;
}

inline void start_cycle_measurement() {
    const int enable_perf_cnt = 0x002003E7;
    __asm__ volatile("csrrw zero, mcycle, zero;"
                     "csrrw zero, mcycleh, zero;"
                     "csrrw zero, 0x7A0, %[enable]"
                     :
                     : [enable] "r"(enable_perf_cnt));
}

inline std::uint32_t finish_cycle_measurement() {
    std::uint32_t value = 0;
    __asm__ volatile("csrrw zero, 0x7A0, zero;"
                     "csrrw %[value], mcycle, zero"
                     : [value] "=r"(value));
    return value;
}

inline std::uint32_t elapsed_cycles(std::uint32_t start, std::uint32_t end) {
    return end - start;
}

struct OperationTiming {
    std::uint32_t sw_cycles = 0;
    std::uint32_t hw_cycles = 0;
    std::uint32_t hw_accel_cycles = 0;
};

using PerfCounterReader = std::int32_t (*)();

template <typename Operation>
Status measure_status(Operation operation, std::uint32_t& cycles) {
    start_cycle_measurement();
    const Status status = operation();
    cycles = finish_cycle_measurement();
    return status;
}

// hw_accel_cycles: the per-FU counter of the accelerator is cleared when an
// instruction is dispatched and counts the cycles in which that functional
// unit is active, so the value read after the call is the execution time of
// the (single) FHRR instruction issued by the library.
template <typename Operation>
Status measure_hw_status(Operation operation, PerfCounterReader perf_counter, OperationTiming& timing) {
    const Status status = measure_status(operation, timing.hw_cycles);
    timing.hw_accel_cycles =
        perf_counter == nullptr ? 0u : static_cast<std::uint32_t>(perf_counter());
    return status;
}

inline void print_speedup(std::uint32_t sw_cycles, std::uint32_t hw_cycles) {
    if (hw_cycles == 0) {
        std::printf("nan");
        return;
    }

    const std::uint32_t integer = sw_cycles / hw_cycles;
    const std::uint32_t fractional = ((sw_cycles % hw_cycles) * 1000u) / hw_cycles;
    std::printf("%u.%03u", static_cast<unsigned>(integer), static_cast<unsigned>(fractional));
}

inline void print_result_line(const char* op,
                              std::size_t elements,
                              bool passed,
                              const OperationTiming& timing) {
    std::printf("FHRR_RESULT op=%s simd=%u hv_elements=%u status=%s sw_cycles=%u hw_cycles=%u "
                "hw_accel_cycles=%u speedup=",
                op,
                static_cast<unsigned>(kHardwareSimdLanes),
                static_cast<unsigned>(elements),
                passed ? "PASS" : "FAIL",
                static_cast<unsigned>(timing.sw_cycles),
                static_cast<unsigned>(timing.hw_cycles),
                static_cast<unsigned>(timing.hw_accel_cycles));
    print_speedup(timing.sw_cycles, timing.hw_cycles);
    std::printf(" fu_speedup=");
    print_speedup(timing.sw_cycles, timing.hw_accel_cycles);
    std::printf("\n");
}

inline int compare_low_byte_vectors(const char* name,
                                    const std::uint32_t* expected,
                                    const std::uint32_t* actual,
                                    std::size_t elements) {
    for (std::size_t index = 0; index < elements; ++index) {
        const std::uint32_t lhs = expected[index] & 0xFFu;
        const std::uint32_t rhs = actual[index] & 0xFFu;
        if (lhs != rhs) {
            std::printf("[%s] mismatch at element %u: expected=0x%02X actual=0x%02X\n",
                        name,
                        static_cast<unsigned>(index),
                        static_cast<unsigned>(lhs),
                        static_cast<unsigned>(rhs));
            return 1;
        }
    }
    return 0;
}

inline int compare_word_vectors(const char* name,
                                const std::uint32_t* expected,
                                const std::uint32_t* actual,
                                std::size_t elements) {
    for (std::size_t index = 0; index < elements; ++index) {
        if (expected[index] != actual[index]) {
            std::printf("[%s] mismatch at element %u: expected=0x%08X actual=0x%08X\n",
                        name,
                        static_cast<unsigned>(index),
                        static_cast<unsigned>(expected[index]),
                        static_cast<unsigned>(actual[index]));
            return 1;
        }
    }
    return 0;
}

inline int compare_scalar(const char* name, std::int32_t expected, std::int32_t actual) {
    if (expected != actual) {
        std::printf("[%s] mismatch: expected=0x%08X (%d) actual=0x%08X (%d)\n",
                    name,
                    static_cast<unsigned>(static_cast<std::uint32_t>(expected)),
                    static_cast<int>(expected),
                    static_cast<unsigned>(static_cast<std::uint32_t>(actual)),
                    static_cast<int>(actual));
        return 1;
    }
    return 0;
}

inline std::uint32_t make_test_seed(std::uint32_t fixed_seed, std::uint32_t salt) {
    if (fixed_seed != 0) {
        return fixed_seed;
    }

    if (FHRR_TEST_FIXED_SEED != 0) {
        return static_cast<std::uint32_t>(FHRR_TEST_FIXED_SEED) ^ salt;
    }

    std::uint32_t cycle_seed = 0;
    csrr(mcycle, cycle_seed);
    return cycle_seed ^ salt;
}

inline std::uint32_t next_random_word(std::uint32_t& state) {
    if (state == 0) {
        state = 0xA341316Cu;
    }
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

inline void fill_random_low_byte_words(std::uint32_t* words,
                                       std::size_t elements,
                                       std::uint32_t seed) {
    std::uint32_t state = seed;
    for (std::size_t index = 0; index < elements; ++index) {
        words[index] = next_random_word(state) & 0xFFu;
    }
}

inline void fill_direct_low_byte_words(std::uint32_t* words,
                                       std::size_t elements,
                                       std::size_t pattern_offset = 0) {
    static const std::uint8_t kPattern[] = {0x00u, 0x01u, 0x7Fu, 0x80u, 0xFFu, 0x20u, 0x40u, 0xC0u};
    const std::size_t pattern_size = sizeof(kPattern) / sizeof(kPattern[0]);

    for (std::size_t index = 0; index < elements; ++index) {
        words[index] = static_cast<std::uint32_t>(kPattern[(index + pattern_offset) % pattern_size]);
    }
}

inline void fill_low_byte_words_for_mode(std::uint32_t* words,
                                         std::size_t elements,
                                         std::uint32_t seed,
                                         std::size_t pattern_offset = 0) {
#if FHRR_TEST_DIRECT_MODE
    (void)seed;
    fill_direct_low_byte_words(words, elements, pattern_offset);
#else
    fill_random_low_byte_words(words, elements, seed);
#endif
}

inline void print_generation_mode(const char* name, std::uint32_t seed) {
#if FHRR_TEST_DIRECT_MODE
    std::printf("[%s] input_mode=direct\n", name);
#else
    std::printf("[%s] input_mode=random seed=0x%08X\n", name, static_cast<unsigned>(seed));
#endif
}

constexpr inline bool is_power_of_two(std::size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

inline int run_named_test(const char* name, TestBody body) {
    __asm__ volatile("csrw 0x300, 0x8;");
    sync_barrier_reset();
    sync_barrier_thread_registration();

    int status = 0;

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[%s] start\n", name);
        status = body();
        if (status == 0) {
            std::printf("[%s] PASS\n", name);
        } else {
            std::printf("[%s] FAIL\n", name);
        }
    }

    sync_barrier();

    if (Klessydra_get_coreID() == 0) {
        exit(status);
    }

    while (1) {
        __asm__ volatile("wfi");
    }
}

}  // namespace tests
}  // namespace hdc
}  // namespace klessydra

#endif  // KLESSYDRA_HDC_TEST_COMMON_HPP
