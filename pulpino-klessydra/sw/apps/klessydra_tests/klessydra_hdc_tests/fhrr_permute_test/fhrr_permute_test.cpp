// [Op-N4] fhrr_permute_test — SW=HW gate for the FHRR HVPERM (cyclic shift).
//
// FHRR cyclic shift (Plate 1995 §VIII-G; Kleyko 2022 §2.2.3):
//     ρ_k(a)[i] = a[(i − k) mod D]
//
// This test mirrors fhrr_bind_test.cpp but runs a sweep of shift amounts to
// cover edge cases (zero, ±1, ±D/2, ±D, > D, < −D). Each case is checked:
//   1. Round-trip property: ρ_{−k}(ρ_k(a)) ≡ a   (exact-inverse).
//   2. SW=HW byte-exact match (0 ULP).
//
// Until Op-N2 lands the FU permutation in RTL-FHRR_Unit.vhd, permute_hw stubs
// Status::UnsupportedOperation and the dispatcher in Backend::Auto mode falls
// back to permute_sw. With Backend::Hardware forced (as the bind test does),
// the HW path returns UnsupportedOperation and this test detects the absence
// of the FU. To allow Wave -1 SW-only validation, this test forces both paths
// to Backend::Auto: when Op-N2 is live, both will resolve to Hardware and the
// SW=HW comparison becomes meaningful end-to-end.

#include "fhrr_test_common.hpp"

#ifndef FHRR_PERMUTE_TEST_ELEMENTS
// [Op-N2 Phase B] HV size = 4 phases. With SIMD=2 (kSimdLanes=2) this is
// 2 SIMD chunks, exercising the cross-chunk splice introduced in Phase B
// (intra_shift > 0 alone tested intra-chunk; chunk_shift > 0 now too).
// Element count must be a multiple of kSimdLanes for the HW path; SW path
// covers arbitrary tail sizes.
#define FHRR_PERMUTE_TEST_ELEMENTS 4
#endif

#ifndef FHRR_PERMUTE_TEST_SEED
#if FHRR_TEST_DEBUG_MODE
#define FHRR_PERMUTE_TEST_SEED 0x4D2C8B1Fu
#else
#define FHRR_PERMUTE_TEST_SEED 0
#endif
#endif

namespace {

volatile int g_test_status = 0;
constexpr std::size_t kPermuteElements = static_cast<std::size_t>(FHRR_PERMUTE_TEST_ELEMENTS);

static_assert(kPermuteElements > 0, "FHRR_PERMUTE_TEST_ELEMENTS must be greater than zero.");

std::uint32_t input_words[kPermuteElements]      = {};
std::uint32_t sw_words[kPermuteElements]         = {};
std::uint32_t hw_words[kPermuteElements]         = {};
std::uint32_t round_trip_words[kPermuteElements] = {};

// Shift values exercised by the test. Cover normalization edge cases of
// permute_sw: zero, ±1, ±D/2, ±D-1, ±D, > D, < -D.
constexpr std::int32_t kShiftCases[] = {
    0,
    1,
    -1,
    static_cast<std::int32_t>(kPermuteElements) / 2,
    -static_cast<std::int32_t>(kPermuteElements) / 2,
    static_cast<std::int32_t>(kPermuteElements) - 1,
    static_cast<std::int32_t>(kPermuteElements),
    -static_cast<std::int32_t>(kPermuteElements),
    static_cast<std::int32_t>(kPermuteElements) + 1,
    -static_cast<std::int32_t>(kPermuteElements) - 1,
    3 * static_cast<std::int32_t>(kPermuteElements) + 5,
    -3 * static_cast<std::int32_t>(kPermuteElements) - 5,
};
constexpr std::size_t kNumShiftCases = sizeof(kShiftCases) / sizeof(kShiftCases[0]);

int run_one_shift(std::int32_t shift, const std::uint32_t* input) {
    using namespace klessydra::hdc;

    Context auto_ctx_sw {{Representation::FHRR, Backend::Software}};
    Context auto_ctx_hw {{Representation::FHRR, Backend::Hardware}};   // Op-N2 Phase A: HW path is live; force Hardware to actually exercise hvperm.
    tests::OperationTiming timing {};

    const Status sw_status = tests::measure_status(
        [&]() {
            return permute(auto_ctx_sw,
                           make_fhrr_phase_vector(sw_words),
                           FhrrPhaseVectorConstView {input, kPermuteElements},
                           shift);
        },
        timing.sw_cycles);

    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return permute(auto_ctx_hw,
                           make_fhrr_phase_vector(hw_words),
                           FhrrPhaseVectorConstView {input, kPermuteElements},
                           shift);
        },
        detail::get_perf_hdcu_perm,
        timing);

    if (sw_status != Status::Ok || hw_status != Status::Ok) {
        std::printf("[fhrr_permute] shift=%d ", static_cast<int>(shift));
        klessydra::hdc::tests::print_status_line("sw", sw_status);
        std::printf("[fhrr_permute] shift=%d ", static_cast<int>(shift));
        klessydra::hdc::tests::print_status_line("hw", hw_status);
        tests::print_result_line("permute", kPermuteElements, false, timing);
        return 1;
    }

    // (1) SW=HW byte-exact gate (0 ULP).
    const int compare_status =
        klessydra::hdc::tests::compare_low_byte_vectors("fhrr_permute_sw_vs_hw",
                                                        sw_words, hw_words,
                                                        kPermuteElements);
    if (compare_status != 0) {
        std::printf("[fhrr_permute] SW=HW MISMATCH at shift=%d\n", static_cast<int>(shift));
        tests::print_result_line("permute", kPermuteElements, false, timing);
        return compare_status;
    }

    // (2) Round-trip property: ρ_{-k}(ρ_k(a)) = a.  Exact for any shift, since
    // cyclic shift is a pure index remapping (no value arithmetic).
    Context check_ctx {{Representation::FHRR, Backend::Software}};
    (void)permute(check_ctx,
                  make_fhrr_phase_vector(round_trip_words),
                  FhrrPhaseVectorConstView {sw_words, kPermuteElements},
                  -shift);

    const int round_trip_status =
        klessydra::hdc::tests::compare_low_byte_vectors("fhrr_permute_round_trip",
                                                        input, round_trip_words,
                                                        kPermuteElements);
    if (round_trip_status != 0) {
        std::printf("[fhrr_permute] ROUND-TRIP MISMATCH at shift=%d\n", static_cast<int>(shift));
        tests::print_result_line("permute", kPermuteElements, false, timing);
        return round_trip_status;
    }

    std::printf("[fhrr_permute] shift=%4d PASS  (sw_cy=%u hw_cy=%u)\n",
                static_cast<int>(shift),
                static_cast<unsigned>(timing.sw_cycles),
                static_cast<unsigned>(timing.hw_cycles));
    tests::print_result_line("permute", kPermuteElements, true, timing);
    return 0;
}

int run_case() {
    using namespace klessydra::hdc;

    const std::uint32_t seed = tests::make_test_seed(static_cast<std::uint32_t>(FHRR_PERMUTE_TEST_SEED),
                                                     0xC2B2AE3Du);
    tests::print_generation_mode("fhrr_permute", seed);
    tests::fill_low_byte_words_for_mode(input_words, kPermuteElements, seed, 0);

    int fails = 0;
    for (std::size_t i = 0; i < kNumShiftCases; ++i) {
        if (run_one_shift(kShiftCases[i], input_words) != 0) {
            ++fails;
        }
    }

    if (fails == 0) {
        std::printf("[fhrr_permute] all %u shift cases PASS\n", static_cast<unsigned>(kNumShiftCases));
    } else {
        std::printf("[fhrr_permute] %d / %u shift cases FAILED\n",
                    fails, static_cast<unsigned>(kNumShiftCases));
    }
    return fails == 0 ? 0 : 1;
}

}  // namespace

int main() {
    Klessydra_En_Int();

    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_permute_test] start (elements=%u, shift_cases=%u)\n",
                    static_cast<unsigned>(kPermuteElements),
                    static_cast<unsigned>(kNumShiftCases));
        g_test_status = run_case();
        if (g_test_status == 0) {
            std::printf("[fhrr_permute_test] PASS\n");
        } else {
            std::printf("[fhrr_permute_test] FAIL\n");
        }
    }

    sync_barrier();

    return g_test_status;
}
