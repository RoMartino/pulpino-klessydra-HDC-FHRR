// FHRR UCIHAR inference benchmark.
//
// Pipeline (per test sample):
//   1) Load pre-encoded Q0.8 phase HV (length D = 1024) into a working buffer.
//   2) For each of 6 class prototypes, call klessydra::hdc::similarity() with
//      Backend::Software. Each call runs the SW Q1.16 cosine LUT pipeline.
//   3) argmax over the 6 scores -> predicted class index in {1..6}.
//   4) Compare against the embedded ground-truth label.
//
// Outputs (printf to UART / sim console):
//   - Per-sample line: "[ucihar] sample=N pred=X label=Y scores=[...]"
//   - Summary line:    "UCIHAR_RESULT n_test=200 correct=K accuracy_q1q15=A_FXP"
//   - Cycle counts for the first inference (binding+similarity), so the
//     per-inference cycle count can be cross-checked against the analytical
//     M2 estimate produced from the campaign logs.
//
// Memory layout: byte tables (~210 KB) live in .rodata -> dataram.
//                One uint32_t[1024] working buffer (~4 KB) in .bss.

#include "fhrr_test_common.hpp"
#include "dataset.hpp"

#include <cstdint>
#include <cstdio>

namespace {

using namespace klessydra::hdc;

constexpr std::size_t kHvD = static_cast<std::size_t>(UCIHAR_HV_D);
constexpr std::size_t kNClasses = static_cast<std::size_t>(UCIHAR_N_CLASSES);
constexpr std::size_t kNTest = static_cast<std::size_t>(UCIHAR_N_TEST);

// Phase vectors require uint32_t storage (low byte = phase). Expand the
// Q0.8 byte tables on-the-fly into these working buffers.
std::uint32_t hv_words[kHvD];
std::uint32_t proto_words[kNClasses][kHvD];

void expand_bytes_to_words(const std::uint8_t* src, std::uint32_t* dst, std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        dst[i] = static_cast<std::uint32_t>(src[i]);
    }
}

int run_inference() {
    // Pre-expand the 6 class prototypes (one-time cost).
    for (std::size_t c = 0; c < kNClasses; ++c) {
        expand_bytes_to_words(&ucihar_class_proto_q08[c][0], &proto_words[c][0], kHvD);
    }

    Context sw_ctx {{Representation::FHRR, Backend::Software}};

    std::size_t correct = 0;
    std::uint32_t first_sample_total_cycles = 0;
    std::uint32_t first_sample_similarity_cycles = 0;

    for (std::size_t n = 0; n < kNTest; ++n) {
        const std::uint32_t t_start = tests::read_mcycle();

        expand_bytes_to_words(&ucihar_test_hv_q08[n][0], hv_words, kHvD);

        FhrrPhaseVectorConstView a {hv_words, kHvD};

        std::int32_t scores[kNClasses] = {0};
        std::uint32_t sim_cycles_acc = 0;

        for (std::size_t c = 0; c < kNClasses; ++c) {
            FhrrPhaseVectorConstView b {&proto_words[c][0], kHvD};
            std::int32_t result = 0;
            std::uint32_t cycles = 0;
            const Status st = tests::measure_status(
                [&]() { return similarity(sw_ctx, result, a, b); }, cycles);
            sim_cycles_acc += cycles;
            if (st != Status::Ok) {
                std::printf("[ucihar] ERROR similarity status=%s sample=%u class=%u\n",
                            to_string(st),
                            static_cast<unsigned>(n),
                            static_cast<unsigned>(c));
                return 1;
            }
            scores[c] = result;
        }

        // argmax (signed; higher = more similar)
        std::size_t best = 0;
        for (std::size_t c = 1; c < kNClasses; ++c) {
            if (scores[c] > scores[best]) {
                best = c;
            }
        }
        const unsigned predicted = static_cast<unsigned>(best + 1);
        const unsigned label = static_cast<unsigned>(ucihar_test_label[n]);

        const std::uint32_t t_end = tests::read_mcycle();
        const std::uint32_t total = t_end - t_start;
        if (n == 0) {
            first_sample_total_cycles = total;
            first_sample_similarity_cycles = sim_cycles_acc;
        }

        if (predicted == label) {
            ++correct;
        }

        // Compact per-sample log; full scores omitted to save sim console time.
        // UCIHAR_SAMPLE_OFFSET allows batched runs: global_sample = n + offset.
#ifndef UCIHAR_SAMPLE_OFFSET
#define UCIHAR_SAMPLE_OFFSET 0
#endif
        std::printf("[ucihar] sample=%u pred=%u label=%u %s sim_cycles=%u\n",
                    static_cast<unsigned>(n) + static_cast<unsigned>(UCIHAR_SAMPLE_OFFSET),
                    predicted,
                    label,
                    predicted == label ? "OK" : "MISS",
                    static_cast<unsigned>(sim_cycles_acc));
    }

    // Summary
    // accuracy as fixed-point Q1.15 (integer = correct*32768/n_test) so the
    // floating point printf is not required on RISC-V with reduced libc.
    const std::uint32_t acc_q15 =
        static_cast<std::uint32_t>((static_cast<std::uint64_t>(correct) << 15) /
                                   static_cast<std::uint64_t>(kNTest));
    std::printf("\nUCIHAR_RESULT n_test=%u correct=%u accuracy_q15=%u/32768 "
                "first_total_cycles=%u first_similarity_cycles=%u\n",
                static_cast<unsigned>(kNTest),
                static_cast<unsigned>(correct),
                static_cast<unsigned>(acc_q15),
                static_cast<unsigned>(first_sample_total_cycles),
                static_cast<unsigned>(first_sample_similarity_cycles));

    // Print also as integer percentage (rounded).
    const unsigned pct_x100 =
        static_cast<unsigned>((static_cast<std::uint64_t>(correct) * 10000ULL) /
                              static_cast<std::uint64_t>(kNTest));
    std::printf("UCIHAR_ACCURACY_PCT_X100=%u (i.e. %u.%02u%%)\n",
                pct_x100, pct_x100 / 100u, pct_x100 % 100u);
    return 0;
}

}  // namespace

int main() {
    Klessydra_En_Int();
    sync_barrier_thread_registration();

    if (Klessydra_get_coreID() == 0) {
        std::printf("\n[fhrr_ucihar_inference] start (n_test=%u, D=%u, n_classes=%u)\n",
                    static_cast<unsigned>(kNTest),
                    static_cast<unsigned>(kHvD),
                    static_cast<unsigned>(kNClasses));
        const int status = run_inference();
        if (status == 0) {
            std::printf("[fhrr_ucihar_inference] PASS\n");
        } else {
            std::printf("[fhrr_ucihar_inference] FAIL\n");
        }
    }

    sync_barrier();
    return 0;
}
