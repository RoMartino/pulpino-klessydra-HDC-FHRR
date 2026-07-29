#include "fhrr_test_common.hpp"

#ifndef FHRR_ENCODE_TEST_ELEMENTS
#if FHRR_TEST_VECTOR_ELEMENTS > 0
#define FHRR_ENCODE_TEST_ELEMENTS FHRR_TEST_VECTOR_ELEMENTS
#elif FHRR_TEST_DEBUG_MODE
#define FHRR_ENCODE_TEST_ELEMENTS 8
#else
#define FHRR_ENCODE_TEST_ELEMENTS 16
#endif
#endif

#ifndef FHRR_ENCODE_TEST_ROWS
#if FHRR_TEST_ENCODE_ROWS > 0
#define FHRR_ENCODE_TEST_ROWS FHRR_TEST_ENCODE_ROWS
#else
#define FHRR_ENCODE_TEST_ROWS 2
#endif
#endif

#ifndef FHRR_ENCODE_TEST_SEED
#if FHRR_TEST_DEBUG_MODE
#define FHRR_ENCODE_TEST_SEED 0x7F4A7C15u
#else
#define FHRR_ENCODE_TEST_SEED 0
#endif
#endif

namespace {

volatile int g_test_status = 0;
constexpr std::size_t kEncodeElements = static_cast<std::size_t>(FHRR_ENCODE_TEST_ELEMENTS);
constexpr std::size_t kEncodeRows = static_cast<std::size_t>(FHRR_ENCODE_TEST_ROWS);

static_assert(kEncodeElements > 0, "FHRR_ENCODE_TEST_ELEMENTS must be greater than zero.");
static_assert(kEncodeRows > 0, "FHRR_ENCODE_TEST_ROWS must be greater than zero.");

std::uint32_t scalar_words[kEncodeRows] = {};
std::uint32_t hv_rows[kEncodeRows * kEncodeElements] = {};
std::uint32_t sw_out[kEncodeElements] = {};
std::uint32_t hw_out[kEncodeElements] = {};

void fill_direct_encode_inputs(std::uint32_t* scalar_words, std::uint32_t* hv_rows) {
    static const std::uint8_t kRowPatternA[] = {0x00u, 0x01u, 0x7Fu, 0x80u, 0xFFu, 0x20u, 0x40u, 0xC0u};
    static const std::uint8_t kRowPatternB[] = {0x00u, 0xFFu, 0x80u, 0x7Fu, 0x01u, 0xE0u, 0xC0u, 0x40u};
    const std::size_t pattern_size = sizeof(kRowPatternA) / sizeof(kRowPatternA[0]);

    for (std::size_t row = 0; row < kEncodeRows; ++row) {
        if ((row & 1u) == 0u) {
            scalar_words[row] = 0x40u;
        } else {
            scalar_words[row] = 0xC0u;
        }

        for (std::size_t element = 0; element < kEncodeElements; ++element) {
            const std::uint8_t value = ((row & 1u) == 0u)
                                           ? kRowPatternA[element % pattern_size]
                                           : kRowPatternB[element % pattern_size];
            hv_rows[row * kEncodeElements + element] = value;
        }
    }
}

int run_case() {
    using namespace klessydra::hdc;

    const std::uint32_t seed =
        tests::make_test_seed(static_cast<std::uint32_t>(FHRR_ENCODE_TEST_SEED), 0xDAA66D2Bu);
    tests::print_generation_mode("fhrr_encode", seed);

#if FHRR_TEST_DIRECT_MODE
    fill_direct_encode_inputs(scalar_words, hv_rows);
    std::printf("[fhrr_encode] case=direct_2x8_reference\n");
#else
    tests::fill_low_byte_words_for_mode(scalar_words, kEncodeRows, seed, 2);
    tests::fill_low_byte_words_for_mode(hv_rows, kEncodeRows * kEncodeElements, seed ^ 0xC2B2AE35u, 5);
#endif

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    FhrrPhaseMatrixConstView matrix {hv_rows, kEncodeRows, kEncodeElements, kEncodeElements};

    const Status sw_status = tests::measure_status(
        [&]() {
            return encode(sw_ctx,
                          make_fhrr_encode_accumulator(sw_out),
                          make_fhrr_scalars(scalar_words),
                          matrix);
        },
        timing.sw_cycles);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return encode(hw_ctx,
                          make_fhrr_encode_accumulator(hw_out),
                          make_fhrr_scalars(scalar_words),
                          matrix);
        },
        detail::get_perf_hdcu_encode,
        timing);

    if (sw_status != Status::Ok || hw_status != Status::Ok) {
        tests::print_status_line("sw_encode", sw_status);
        tests::print_status_line("hw_encode", hw_status);
        tests::print_result_line("encode", kEncodeElements, false, timing);
        std::printf("[fhrr_encode] FAIL\n");
        return 1;
    }

    const int compare_status =
        tests::compare_low_byte_vectors("fhrr_encode", sw_out, hw_out, kEncodeElements);

    if (compare_status == 0) {
        std::printf("[fhrr_encode] PASS\n");
    } else {
        std::printf("[fhrr_encode] FAIL\n");
    }

    tests::print_result_line("encode", kEncodeElements, compare_status == 0, timing);
    return compare_status;
}

}  // namespace

int main() {
    Klessydra_En_Int();

    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_encode_test] start (elements=%u, rows=%u)\n",
                    static_cast<unsigned>(kEncodeElements),
                    static_cast<unsigned>(kEncodeRows));
        g_test_status = run_case();
        if (g_test_status == 0) {
            std::printf("[fhrr_encode_test] PASS\n");
        } else {
            std::printf("[fhrr_encode_test] FAIL\n");
        }
    }

    sync_barrier();

    return g_test_status;
}
