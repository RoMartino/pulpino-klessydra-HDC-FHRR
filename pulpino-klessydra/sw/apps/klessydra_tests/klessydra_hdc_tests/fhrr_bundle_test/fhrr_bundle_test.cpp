#include "fhrr_test_common.hpp"

#ifndef FHRR_BUNDLE_TEST_ELEMENTS
#if FHRR_TEST_VECTOR_ELEMENTS > 0
#define FHRR_BUNDLE_TEST_ELEMENTS FHRR_TEST_VECTOR_ELEMENTS
#elif FHRR_TEST_DEBUG_MODE
#define FHRR_BUNDLE_TEST_ELEMENTS 8
#else
#define FHRR_BUNDLE_TEST_ELEMENTS 16
#endif
#endif

#ifndef FHRR_BUNDLE_TEST_SEED
#if FHRR_TEST_DEBUG_MODE
#define FHRR_BUNDLE_TEST_SEED 0x517CC1B7u
#else
#define FHRR_BUNDLE_TEST_SEED 0
#endif
#endif

namespace {

volatile int g_test_status = 0;
constexpr std::size_t kBundleElements = static_cast<std::size_t>(FHRR_BUNDLE_TEST_ELEMENTS);

static_assert(kBundleElements > 0, "FHRR_BUNDLE_TEST_ELEMENTS must be greater than zero.");

std::uint32_t hv_words[kBundleElements] = {};
std::uint32_t sw_imag[kBundleElements]  = {};
std::uint32_t sw_real[kBundleElements]  = {};
std::uint32_t hw_imag[kBundleElements]  = {};
std::uint32_t hw_real[kBundleElements]  = {};

int run_case() {
    using namespace klessydra::hdc;

    const std::uint32_t seed =
        tests::make_test_seed(static_cast<std::uint32_t>(FHRR_BUNDLE_TEST_SEED), 0x517CC1B7u);
    tests::print_generation_mode("fhrr_bundle", seed);
    tests::fill_low_byte_words_for_mode(hv_words, kBundleElements, seed, 1);

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    const Status sw_status = tests::measure_status(
        [&]() {
            return bundle(sw_ctx,
                          make_fhrr_bundle_accumulator(sw_imag, sw_real),
                          make_fhrr_phase_vector(hv_words));
        },
        timing.sw_cycles);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return bundle(hw_ctx,
                          make_fhrr_bundle_accumulator(hw_imag, hw_real),
                          make_fhrr_phase_vector(hv_words));
        },
        detail::get_perf_hdcu_bundle,
        timing);

    if (sw_status != Status::Ok || hw_status != Status::Ok) {
        klessydra::hdc::tests::print_status_line("sw_bundle", sw_status);
        klessydra::hdc::tests::print_status_line("hw_bundle", hw_status);
        tests::print_result_line("bundle", kBundleElements, false, timing);
        std::printf("[fhrr_bundle] FAIL\n");
        return 1;
    }

    const int imag_status =
        klessydra::hdc::tests::compare_word_vectors("fhrr_bundle.imag", sw_imag, hw_imag, kBundleElements);
    const int real_status =
        imag_status == 0
            ? klessydra::hdc::tests::compare_word_vectors("fhrr_bundle.real", sw_real, hw_real, kBundleElements)
            : imag_status;

    if (real_status == 0) {
        std::printf("[fhrr_bundle] PASS\n");
    } else {
        std::printf("[fhrr_bundle] FAIL\n");
    }

    tests::print_result_line("bundle", kBundleElements, real_status == 0, timing);
    return real_status;
}

}  // namespace

int main() {
    Klessydra_En_Int();

    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_bundle_test] start (elements=%u)\n",
                    static_cast<unsigned>(kBundleElements));
        g_test_status = run_case();
        if (g_test_status == 0) {
            std::printf("[fhrr_bundle_test] PASS\n");
        } else {
            std::printf("[fhrr_bundle_test] FAIL\n");
        }
    }

    sync_barrier();

    return g_test_status;
}
