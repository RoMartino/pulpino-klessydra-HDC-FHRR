#include "fhrr_test_common.hpp"

#ifndef FHRR_BIND_TEST_ELEMENTS
#if FHRR_TEST_VECTOR_ELEMENTS > 0
#define FHRR_BIND_TEST_ELEMENTS FHRR_TEST_VECTOR_ELEMENTS
#elif FHRR_TEST_DEBUG_MODE
#define FHRR_BIND_TEST_ELEMENTS 8
#else
#define FHRR_BIND_TEST_ELEMENTS 16
#endif
#endif

#ifndef FHRR_BIND_TEST_SEED
#if FHRR_TEST_DEBUG_MODE
#define FHRR_BIND_TEST_SEED 0x19C2A5D1u
#else
#define FHRR_BIND_TEST_SEED 0
#endif
#endif

namespace {

volatile int g_test_status = 0;
constexpr std::size_t kBindElements = static_cast<std::size_t>(FHRR_BIND_TEST_ELEMENTS);

static_assert(kBindElements > 0, "FHRR_BIND_TEST_ELEMENTS must be greater than zero.");

std::uint32_t lhs_words[kBindElements] = {};
std::uint32_t rhs_words[kBindElements] = {};
std::uint32_t sw_words[kBindElements]  = {};
std::uint32_t hw_words[kBindElements]  = {};

int run_case() {
    using namespace klessydra::hdc;

    const std::uint32_t seed = tests::make_test_seed(static_cast<std::uint32_t>(FHRR_BIND_TEST_SEED), 0x9E3779B9u);
    tests::print_generation_mode("fhrr_bind", seed);
    tests::fill_low_byte_words_for_mode(lhs_words, kBindElements, seed, 0);
    tests::fill_low_byte_words_for_mode(rhs_words, kBindElements, seed ^ 0x85EBCA6Bu, 3);

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    const Status sw_status = tests::measure_status(
        [&]() {
            return bind(sw_ctx,
                        make_fhrr_phase_vector(sw_words),
                        make_fhrr_phase_vector(lhs_words),
                        make_fhrr_phase_vector(rhs_words));
        },
        timing.sw_cycles);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return bind(hw_ctx,
                        make_fhrr_phase_vector(hw_words),
                        make_fhrr_phase_vector(lhs_words),
                        make_fhrr_phase_vector(rhs_words));
        },
        detail::get_perf_hdcu_bind,
        timing);

    if (sw_status != Status::Ok || hw_status != Status::Ok) {
        klessydra::hdc::tests::print_status_line("sw_bind", sw_status);
        klessydra::hdc::tests::print_status_line("hw_bind", hw_status);
        tests::print_result_line("bind", kBindElements, false, timing);
        std::printf("[fhrr_bind] FAIL\n");
        return 1;
    }

    const int compare_status =
        klessydra::hdc::tests::compare_low_byte_vectors("fhrr_bind", sw_words, hw_words, kBindElements);

    if (compare_status == 0) {
        std::printf("[fhrr_bind] PASS\n");
    } else {
        std::printf("[fhrr_bind] FAIL\n");
    }

    tests::print_result_line("bind", kBindElements, compare_status == 0, timing);
    return compare_status;
}

}  // namespace

int main() {
    Klessydra_En_Int();

    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_bind_test] start (elements=%u)\n",
                    static_cast<unsigned>(kBindElements));
        g_test_status = run_case();
        if (g_test_status == 0) {
            std::printf("[fhrr_bind_test] PASS\n");
        } else {
            std::printf("[fhrr_bind_test] FAIL\n");
        }
    }

    sync_barrier();

    return g_test_status;
}
