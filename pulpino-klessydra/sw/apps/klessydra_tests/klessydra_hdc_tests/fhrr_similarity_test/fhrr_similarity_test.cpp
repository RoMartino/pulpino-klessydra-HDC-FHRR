#include "fhrr_test_common.hpp"

#ifndef FHRR_SIMILARITY_TEST_ELEMENTS
#if FHRR_TEST_VECTOR_ELEMENTS > 0
#define FHRR_SIMILARITY_TEST_ELEMENTS FHRR_TEST_VECTOR_ELEMENTS
#elif FHRR_TEST_DEBUG_MODE
#define FHRR_SIMILARITY_TEST_ELEMENTS 8
#else
#define FHRR_SIMILARITY_TEST_ELEMENTS 16
#endif
#endif

#ifndef FHRR_SIMILARITY_TEST_SEED
#if FHRR_TEST_DEBUG_MODE
#define FHRR_SIMILARITY_TEST_SEED 0x94D049BBu
#else
#define FHRR_SIMILARITY_TEST_SEED 0
#endif
#endif

namespace {

volatile int g_test_status = 0;
constexpr std::size_t kSimilarityElements = static_cast<std::size_t>(FHRR_SIMILARITY_TEST_ELEMENTS);

static_assert(kSimilarityElements > 0, "FHRR_SIMILARITY_TEST_ELEMENTS must be greater than zero.");
static_assert(klessydra::hdc::tests::is_power_of_two(kSimilarityElements),
              "FHRR_SIMILARITY_TEST_ELEMENTS must be a power of two.");

std::uint32_t lhs_words[kSimilarityElements] = {};
std::uint32_t rhs_words[kSimilarityElements] = {};

int run_similarity_subcase(const char* case_name,
                           const std::uint32_t* lhs_words,
                           const std::uint32_t* rhs_words) {
    using namespace klessydra::hdc;

    std::int32_t sw_result = 0;
    std::int32_t hw_result = 0;

    std::printf("[fhrr_similarity] case=%s\n", case_name);

    Context sw_ctx {{Representation::FHRR, Backend::Software}};
    Context hw_ctx {{Representation::FHRR, Backend::Hardware}};
    tests::OperationTiming timing {};

    const Status sw_status = tests::measure_status(
        [&]() {
            return similarity(sw_ctx,
                              sw_result,
                              FhrrPhaseVectorConstView {lhs_words, kSimilarityElements},
                              FhrrPhaseVectorConstView {rhs_words, kSimilarityElements});
        },
        timing.sw_cycles);
    const Status hw_status = tests::measure_hw_status(
        [&]() {
            return similarity(hw_ctx,
                              hw_result,
                              FhrrPhaseVectorConstView {lhs_words, kSimilarityElements},
                              FhrrPhaseVectorConstView {rhs_words, kSimilarityElements});
        },
        detail::get_perf_hdcu_similarity,
        timing);

    if (sw_status != Status::Ok || hw_status != Status::Ok) {
        tests::print_status_line("sw_similarity", sw_status);
        tests::print_status_line("hw_similarity", hw_status);
        tests::print_result_line("similarity", kSimilarityElements, false, timing);
        return 1;
    }

    const int compare_status = tests::compare_scalar("fhrr_similarity", sw_result, hw_result);
    tests::print_result_line("similarity", kSimilarityElements, compare_status == 0, timing);
    return compare_status;
}

int run_case() {
    using namespace klessydra::hdc;

    const std::uint32_t seed =
        tests::make_test_seed(static_cast<std::uint32_t>(FHRR_SIMILARITY_TEST_SEED), 0x94D049BBu);
    tests::print_generation_mode("fhrr_similarity", seed);

#if FHRR_TEST_DIRECT_MODE
    tests::fill_direct_low_byte_words(lhs_words, kSimilarityElements, 0);

    for (std::size_t index = 0; index < kSimilarityElements; ++index) {
        rhs_words[index] = lhs_words[index];
    }
    if (run_similarity_subcase("identical", lhs_words, rhs_words) != 0) {
        std::printf("[fhrr_similarity] FAIL\n");
        return 1;
    }

    for (std::size_t index = 0; index < kSimilarityElements; ++index) {
        rhs_words[index] = (lhs_words[index] + 0x80u) & 0xFFu;
    }
    if (run_similarity_subcase("opposite_phase", lhs_words, rhs_words) != 0) {
        std::printf("[fhrr_similarity] FAIL\n");
        return 1;
    }

    for (std::size_t index = 0; index < kSimilarityElements; ++index) {
        rhs_words[index] = lhs_words[index];
    }
    rhs_words[0] = (rhs_words[0] + 0x01u) & 0xFFu;
    if (run_similarity_subcase("single_lane_delta", lhs_words, rhs_words) != 0) {
        std::printf("[fhrr_similarity] FAIL\n");
        return 1;
    }
#else
    tests::fill_low_byte_words_for_mode(lhs_words, kSimilarityElements, seed, 0);
    tests::fill_low_byte_words_for_mode(rhs_words, kSimilarityElements, seed ^ 0x2545F491u, 6);

    if (run_similarity_subcase("random", lhs_words, rhs_words) != 0) {
        std::printf("[fhrr_similarity] FAIL\n");
        return 1;
    }
#endif

    std::printf("[fhrr_similarity] PASS\n");
    return 0;
}

}  // namespace

int main() {
    Klessydra_En_Int();

    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_similarity_test] start (elements=%u)\n",
                    static_cast<unsigned>(kSimilarityElements));
        g_test_status = run_case();
        if (g_test_status == 0) {
            std::printf("[fhrr_similarity_test] PASS\n");
        } else {
            std::printf("[fhrr_similarity_test] FAIL\n");
        }
    }

    sync_barrier();

    return g_test_status;
}
