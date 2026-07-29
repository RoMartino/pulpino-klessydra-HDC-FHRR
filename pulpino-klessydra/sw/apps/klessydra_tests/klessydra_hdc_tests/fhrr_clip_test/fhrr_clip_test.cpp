#include "fhrr_test_common.hpp"

#ifndef FHRR_CLIP_TEST_ELEMENTS
#if FHRR_TEST_VECTOR_ELEMENTS > 0
#define FHRR_CLIP_TEST_ELEMENTS FHRR_TEST_VECTOR_ELEMENTS
#elif FHRR_TEST_DEBUG_MODE
#define FHRR_CLIP_TEST_ELEMENTS 8
#else
#define FHRR_CLIP_TEST_ELEMENTS 16
#endif
#endif

#ifndef FHRR_CLIP_TEST_SEED
#if FHRR_TEST_DEBUG_MODE
#define FHRR_CLIP_TEST_SEED 0x3C6EF372u
#else
#define FHRR_CLIP_TEST_SEED 0
#endif
#endif

namespace {

volatile int g_test_status = 0;
constexpr std::size_t kClipElements = static_cast<std::size_t>(FHRR_CLIP_TEST_ELEMENTS);

static_assert(kClipElements > 0, "FHRR_CLIP_TEST_ELEMENTS must be greater than zero.");

std::uint32_t bundle_hv_a[kClipElements] = {};
std::uint32_t bundle_hv_b[kClipElements] = {};
std::uint32_t imag_words[kClipElements]  = {};
std::uint32_t real_words[kClipElements]  = {};
std::uint32_t sw_clip[kClipElements]     = {};
std::uint32_t hw_clip[kClipElements]     = {};
std::int32_t imag_signed[kClipElements]  = {};
std::int32_t real_signed[kClipElements]  = {};

void pack_signed_words(std::uint32_t* dst, const std::int32_t* src, std::size_t elements) {
    for (std::size_t index = 0; index < elements; ++index) {
        dst[index] = static_cast<std::uint32_t>(src[index]);
    }
}

void make_clip_denominators_valid(std::uint32_t* real_words, std::size_t elements) {
    constexpr std::uint32_t kUnityReal = 0x00010000u;
    for (std::size_t index = 0; index < elements; ++index) {
        if (real_words[index] == 0u) {
            real_words[index] = kUnityReal;
        }
    }
}

int run_clip_compare_case(const char* case_name,
                          const std::uint32_t* imag_words,
                          const std::uint32_t* real_words) {
    using namespace klessydra::hdc;

    std::printf("[fhrr_clip] case=%s\n", case_name);

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    const auto acc_view = FhrrBundleAccumulatorConstView {imag_words, real_words, kClipElements};
    const Status sw_status = tests::measure_status(
        [&]() {
            return clip(sw_ctx, make_fhrr_phase_vector(sw_clip), acc_view);
        },
        timing.sw_cycles);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return clip(hw_ctx, make_fhrr_phase_vector(hw_clip), acc_view);
        },
        detail::get_perf_hdcu_clip,
        timing);

    if (sw_status != Status::Ok || hw_status != Status::Ok) {
        tests::print_status_line("sw_clip", sw_status);
        tests::print_status_line("hw_clip", hw_status);
        tests::print_result_line("clip", kClipElements, false, timing);
        return 1;
    }

    const int compare_status = tests::compare_word_vectors("fhrr_clip", sw_clip, hw_clip, kClipElements);
    tests::print_result_line("clip", kClipElements, compare_status == 0, timing);
    return compare_status;
}

int run_clip_status_case(const char* case_name,
                         const std::uint32_t* imag_words,
                         const std::uint32_t* real_words,
                         klessydra::hdc::Status expected_status) {
    using namespace klessydra::hdc;

    std::printf("[fhrr_clip] case=%s\n", case_name);

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    const auto acc_view = FhrrBundleAccumulatorConstView {imag_words, real_words, kClipElements};
    const Status sw_status = tests::measure_status(
        [&]() {
            return clip(sw_ctx, make_fhrr_phase_vector(hw_clip), acc_view);
        },
        timing.sw_cycles);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return clip(hw_ctx, make_fhrr_phase_vector(hw_clip), acc_view);
        },
        detail::get_perf_hdcu_clip,
        timing);

    if (sw_status != expected_status || hw_status != expected_status) {
        tests::print_status_line("sw_clip", sw_status);
        tests::print_status_line("hw_clip", hw_status);
        tests::print_result_line("clip", kClipElements, false, timing);
        return 1;
    }

    tests::print_result_line("clip", kClipElements, true, timing);
    return 0;
}

int run_case() {
    using namespace klessydra::hdc;

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    const std::uint32_t base_seed =
        tests::make_test_seed(static_cast<std::uint32_t>(FHRR_CLIP_TEST_SEED), 0x3C6EF372u);
    tests::print_generation_mode("fhrr_clip", base_seed);

#if FHRR_TEST_DIRECT_MODE
    const std::int32_t kValidImagPattern[] = {0, 256, -256, 1024, -1024, 2048, -2048, 4096};
    const std::int32_t kValidRealPattern[] = {65536, 65536, 65536, 65536, 65536, -65536, -65536, -65536};
    const std::int32_t kInvalidImagPattern[] = {0, 256, -256, 1024, -1024, 2048, -2048, 4096};
    const std::int32_t kInvalidRealPattern[] = {0, 65536, 65536, 65536, 65536, 65536, 65536, 65536};

    for (std::size_t index = 0; index < kClipElements; ++index) {
        imag_signed[index] = kValidImagPattern[index % (sizeof(kValidImagPattern) / sizeof(kValidImagPattern[0]))];
        real_signed[index] = kValidRealPattern[index % (sizeof(kValidRealPattern) / sizeof(kValidRealPattern[0]))];
    }
    pack_signed_words(imag_words, imag_signed, kClipElements);
    pack_signed_words(real_words, real_signed, kClipElements);

    if (run_clip_compare_case("near_axes", imag_words, real_words) != 0) {
        std::printf("[fhrr_clip] FAIL\n");
        return 1;
    }

    for (std::size_t index = 0; index < kClipElements; ++index) {
        imag_signed[index] = kInvalidImagPattern[index % (sizeof(kInvalidImagPattern) / sizeof(kInvalidImagPattern[0]))];
        real_signed[index] = kInvalidRealPattern[index % (sizeof(kInvalidRealPattern) / sizeof(kInvalidRealPattern[0]))];
    }
    pack_signed_words(imag_words, imag_signed, kClipElements);
    pack_signed_words(real_words, real_signed, kClipElements);

    if (run_clip_status_case("invalid_real_zero", imag_words, real_words, Status::UnsupportedOperation) != 0) {
        std::printf("[fhrr_clip] FAIL\n");
        return 1;
    }

    std::printf("[fhrr_clip] PASS\n");
    return 0;
#endif

    Status clip_status = Status::UnsupportedOperation;

    for (std::uint32_t attempt = 0; attempt < 16 && clip_status == Status::UnsupportedOperation; ++attempt) {
        const std::uint32_t seed = base_seed ^ (attempt * 0x9E3779B9u);
        tests::fill_low_byte_words_for_mode(bundle_hv_a, kClipElements, seed, 0);
        tests::fill_low_byte_words_for_mode(bundle_hv_b, kClipElements, seed ^ 0xA54FF53Au, 4);

        for (std::size_t index = 0; index < kClipElements; ++index) {
            imag_words[index] = 0;
            real_words[index] = 0;
            sw_clip[index] = 0;
            hw_clip[index] = 0;
        }

        Status status = bundle(sw_ctx,
                               make_fhrr_bundle_accumulator(imag_words, real_words),
                               make_fhrr_phase_vector(bundle_hv_a));
        if (status != Status::Ok) {
            tests::print_status_line("sw_bundle_a", status);
            std::printf("[fhrr_clip] FAIL\n");
            return 1;
        }

        status = bundle(sw_ctx,
                        make_fhrr_bundle_accumulator(imag_words, real_words),
                        make_fhrr_phase_vector(bundle_hv_b));
        if (status != Status::Ok) {
            tests::print_status_line("sw_bundle_b", status);
            std::printf("[fhrr_clip] FAIL\n");
            return 1;
        }

        make_clip_denominators_valid(real_words, kClipElements);
        clip_status = clip(sw_ctx,
                           make_fhrr_phase_vector(sw_clip),
                           make_fhrr_bundle_accumulator(imag_words, real_words));
    }

    if (clip_status != Status::Ok) {
        tests::print_status_line("sw_clip", clip_status);
        std::printf("[fhrr_clip] FAIL\n");
        return 1;
    }

    const auto acc_view = make_fhrr_bundle_accumulator(imag_words, real_words);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return clip(hw_ctx, make_fhrr_phase_vector(hw_clip), acc_view);
        },
        detail::get_perf_hdcu_clip,
        timing);

    if (hw_status != Status::Ok) {
        tests::print_status_line("hw_clip", hw_status);
        tests::print_result_line("clip", kClipElements, false, timing);
        std::printf("[fhrr_clip] FAIL\n");
        return 1;
    }

    timing.sw_cycles = 0;
    const Status sw_status = tests::measure_status(
        [&]() {
            return clip(sw_ctx, make_fhrr_phase_vector(sw_clip), acc_view);
        },
        timing.sw_cycles);
    if (sw_status != Status::Ok) {
        tests::print_status_line("sw_clip", sw_status);
        tests::print_result_line("clip", kClipElements, false, timing);
        std::printf("[fhrr_clip] FAIL\n");
        return 1;
    }

    const int compare_status =
        tests::compare_word_vectors("fhrr_clip", sw_clip, hw_clip, kClipElements);

    if (compare_status == 0) {
        std::printf("[fhrr_clip] PASS\n");
    } else {
        std::printf("[fhrr_clip] FAIL\n");
    }

    tests::print_result_line("clip", kClipElements, compare_status == 0, timing);
    return compare_status;
}

}  // namespace

int main() {
    Klessydra_En_Int();

    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_clip_test] start (elements=%u)\n",
                    static_cast<unsigned>(kClipElements));
        g_test_status = run_case();
        if (g_test_status == 0) {
            std::printf("[fhrr_clip_test] PASS\n");
        } else {
            std::printf("[fhrr_clip_test] FAIL\n");
        }
    }

    sync_barrier();

    return g_test_status;
}
