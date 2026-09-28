#include "hdc_internal.hpp"

#include <array>

extern "C" {
#include "dsp_functions.h"
#include "functions.h"
}

namespace klessydra {
namespace hdc {
namespace detail {

namespace {

template <typename ViewT>
bool valid_phase_view(ViewT view) {
    return view.words != nullptr && view.elements != 0;
}

bool valid_bundle_view(FhrrBundleAccumulatorConstView view) {
    return view.imag_words != nullptr && view.real_words != nullptr && view.elements != 0;
}

bool valid_bundle_view(FhrrBundleAccumulatorView view) {
    return view.imag_words != nullptr && view.real_words != nullptr && view.elements != 0;
}

bool valid_encode_args(FhrrEncodeAccumulatorView acc,
                       FhrrScalarVectorConstView scalars,
                       FhrrPhaseMatrixConstView hvs) {
    return acc.words != nullptr &&
           acc.elements != 0 &&
           scalars.words != nullptr &&
           scalars.count != 0 &&
           hvs.words != nullptr &&
           hvs.rows == scalars.count &&
           hvs.elements == acc.elements &&
           hvs.row_stride >= hvs.elements;
}

constexpr std::size_t kSimdLanes = kHardwareSimdLanes == 0 ? 1 : kHardwareSimdLanes;
constexpr std::size_t kMaxEncodeRows = 31;

inline void* spm_a() { return reinterpret_cast<void*>(spmaddrA); }
inline void* spm_b() { return reinterpret_cast<void*>(spmaddrB); }
inline void* spm_c() { return reinterpret_cast<void*>(spmaddrC); }
inline void* spm_d() { return reinterpret_cast<void*>(spmaddrD); }

// All FHRR hardware back-ends issue ONE custom instruction over the whole
// hypervector (CSR_MVSIZE = D * 4 bytes): the operands are placed in the
// scratchpads, the functional unit streams the D elements P lanes per step,
// and the result is read back. The hypervector size must be a multiple of
// the SIMD lanes P.
inline bool whole_hv_supported(std::size_t elements) {
    return elements != 0 && (elements % kSimdLanes) == 0;
}

// kmemld/kmemstr run in the load/store unit in the background. A scalar load
// waits until the LSU is free, i.e. until the previous scratchpad transfers
// have completed. Issuing the FHRR instruction only after this barrier means
// that the functional unit starts with idle scratchpads (as in a deployed
// system, where the model is already resident in the SPMs) and its cycle
// counter measures only the operation itself.
volatile std::uint32_t g_lsu_sync_word = 0;
inline void wait_spm_transfers() {
    (void)g_lsu_sync_word;
}

inline std::uint8_t* spm_offset(void* spm, std::size_t bytes) {
    return reinterpret_cast<std::uint8_t*>(spm) + bytes;
}

}  // namespace

Status bind_hw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_phase_view(dst) || !valid_phase_view(a) || !valid_phase_view(b)) {
        return Status::InvalidArgument;
    }
    if (dst.elements != a.elements || dst.elements != b.elements) {
        return Status::SizeMismatch;
    }
    if (!whole_hv_supported(dst.elements)) {
        return Status::UnsupportedOperation;
    }

    const int bytes = static_cast<int>(dst.elements * kFhrrLaneBytes);
    CSR_MVSIZE(bytes);
    kmemld(spm_a(), const_cast<std::uint32_t*>(a.words), bytes);
    kmemld(spm_b(), const_cast<std::uint32_t*>(b.words), bytes);
    wait_spm_transfers();
    hvbind(spm_c(), spm_a(), spm_b());
    kmemstr(dst.words, spm_c(), bytes);

    return Status::Ok;
}

// Bundling: the functional unit reads the accumulator interleaved per SIMD
// chunk ([imag chunk k | real chunk k], 2*P words per chunk) from spm_a and the
// phase hypervector from spm_b, and writes the updated accumulator back to
// spm_c with the same interleaved layout.
Status bundle_hw(FhrrBundleAccumulatorView acc, FhrrPhaseVectorConstView hv) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_bundle_view(acc) || !valid_phase_view(hv)) {
        return Status::InvalidArgument;
    }
    if (acc.elements != hv.elements) {
        return Status::SizeMismatch;
    }
    if (!whole_hv_supported(acc.elements)) {
        return Status::UnsupportedOperation;
    }

    const std::size_t chunk_bytes = kSimdLanes * kFhrrLaneBytes;
    const int bytes = static_cast<int>(acc.elements * kFhrrLaneBytes);
    for (std::size_t offset = 0, k = 0; offset < acc.elements; offset += kSimdLanes, ++k) {
        kmemld(spm_offset(spm_a(), 2 * k * chunk_bytes), acc.imag_words + offset, static_cast<int>(chunk_bytes));
        kmemld(spm_offset(spm_a(), (2 * k + 1) * chunk_bytes), acc.real_words + offset, static_cast<int>(chunk_bytes));
    }
    CSR_MVSIZE(bytes);
    kmemld(spm_b(), const_cast<std::uint32_t*>(hv.words), bytes);
    wait_spm_transfers();
    hvbundle(spm_c(), spm_a(), spm_b());
    for (std::size_t offset = 0, k = 0; offset < acc.elements; offset += kSimdLanes, ++k) {
        kmemstr(acc.imag_words + offset, spm_offset(spm_c(), 2 * k * chunk_bytes), static_cast<int>(chunk_bytes));
        kmemstr(acc.real_words + offset, spm_offset(spm_c(), (2 * k + 1) * chunk_bytes), static_cast<int>(chunk_bytes));
    }

    return Status::Ok;
}

Status similarity_hw(std::int32_t& out, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_phase_view(a) || !valid_phase_view(b)) {
        return Status::InvalidArgument;
    }
    if (a.elements != b.elements) {
        return Status::SizeMismatch;
    }
    if (!is_power_of_two(a.elements) || a.elements < kSimdLanes) {
        return Status::UnsupportedOperation;
    }

    std::int32_t hw_result = 0;
    CSR_MVSIZE(static_cast<int>(a.elements * kFhrrLaneBytes));
    kmemld(spm_a(), const_cast<std::uint32_t*>(a.words), static_cast<int>(a.elements * kFhrrLaneBytes));
    kmemld(spm_b(), const_cast<std::uint32_t*>(b.words), static_cast<int>(b.elements * kFhrrLaneBytes));
    wait_spm_transfers();
    hvsim(spm_c(), spm_a(), spm_b());
    kmemstr(&hw_result, spm_c(), static_cast<int>(sizeof(hw_result)));
    out = hw_result;

    return Status::Ok;
}

Status clip_hw(FhrrPhaseVectorView dst, FhrrBundleAccumulatorConstView acc) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_phase_view(dst) || !valid_bundle_view(acc)) {
        return Status::InvalidArgument;
    }
    if (dst.elements != acc.elements) {
        return Status::SizeMismatch;
    }
    if (!whole_hv_supported(dst.elements)) {
        return Status::UnsupportedOperation;
    }
    for (std::size_t index = 0; index < acc.elements; ++index) {
        if (static_cast<std::int32_t>(acc.real_words[index]) == 0) {
            return Status::UnsupportedOperation;
        }
    }

    const int bytes = static_cast<int>(dst.elements * kFhrrLaneBytes);
    CSR_MVSIZE(bytes);
    kmemld(spm_a(), const_cast<std::uint32_t*>(acc.imag_words), bytes);
    kmemld(spm_b(), const_cast<std::uint32_t*>(acc.real_words), bytes);
    wait_spm_transfers();
    hvclip(spm_c(), spm_a(), spm_b());
    kmemstr(dst.words, spm_c(), bytes);

    return Status::Ok;
}

// Encoding (FPE): spm_a holds the F scalar features, spm_b the F item
// hypervectors stored row after row (row r at offset r * D * 4 bytes).
Status encode_hw(FhrrEncodeAccumulatorView acc,
                 FhrrScalarVectorConstView scalars,
                 FhrrPhaseMatrixConstView hvs) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_encode_args(acc, scalars, hvs)) {
        return Status::InvalidArgument;
    }
    if (scalars.count > kMaxEncodeRows || !whole_hv_supported(acc.elements)) {
        return Status::UnsupportedOperation;
    }

    const std::size_t row_bytes = acc.elements * kFhrrLaneBytes;
    CSR_MPSCLFAC(static_cast<int>(scalars.count));
    CSR_MVSIZE(static_cast<int>(row_bytes));
    kmemld(spm_a(), const_cast<std::uint32_t*>(scalars.words), static_cast<int>(scalars.count * kFhrrLaneBytes));
    for (std::size_t row = 0; row < scalars.count; ++row) {
        kmemld(spm_offset(spm_b(), row * row_bytes),
               const_cast<std::uint32_t*>(hvs.words + row * hvs.row_stride),
               static_cast<int>(row_bytes));
    }
    wait_spm_transfers();
    hvenc(spm_c(), spm_a(), spm_b());
    kmemstr(acc.words, spm_c(), static_cast<int>(row_bytes));

    return Status::Ok;
}

// [Op-N2 Phase B] Hardware permutation via the HVPERM custom instruction.
// rs1 = scalar shift amount (int, normalized mod D in SW glue), rs2 = SPM
// address of src (HV base), rd = SPM address of dst.
// Phase B adds cross-chunk cyclic shift in the FU: it streams chunks
// sequentially on a single SPM read channel (the SPM banks are single-port
// at the SCI layer, see RTL-Scratchpad_Memory_Interface.vhd) and splices
// the previous-cycle chunk (latched as chunk_b) with the current cycle's
// chunk (chunk_a) via a 2*SIMD-byte barrel. The FU produces one output
// chunk per cycle after a one-cycle pipeline warm-up. Works for any HV
// size that is a multiple of SIMD lanes (the canonical FHRR layout);
// non-multiple sizes fall back to the SW reference automatically via
// dispatch_permute()'s Auto/Software branch.
// rs1 is taken mod D in this glue so the FU only sees a value in [0, D)
// (the FU reads the shift from rs1[15:0], so any D < 65536 is supported).
Status permute_hw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView src, std::int32_t shift) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_phase_view(dst) || !valid_phase_view(src)) {
        return Status::InvalidArgument;
    }
    if (dst.elements != src.elements) {
        return Status::SizeMismatch;
    }
    if (dst.words == src.words) {
        return Status::InvalidArgument;
    }
    if (src.elements == 0) {
        return Status::Ok;
    }
    // Phase B: HV element count must be an integer multiple of the SIMD lane
    // count so the chunk grid is well-defined. permute_sw handles arbitrary
    // tail sizes; multi-chunk HW currently requires the canonical layout.
    if ((src.elements % kSimdLanes) != 0) {
        return Status::UnsupportedOperation;
    }

    const std::size_t bytes = src.elements * kFhrrLaneBytes;

    // Normalize the shift mod D = src.elements (signed handled), so the FU
    // only sees a non-negative shift in [0, D). The FU then derives
    // chunk_shift = shift / SIMD and intra_shift = shift mod SIMD.
    //
    // FHRR semantics (Plate 1995, Kleyko 2022): rho_k(a)[i] = a[(i - k) mod D].
    // The FU's splice mux currently implements out[i] = src[(i + shift_arg) mod D]
    // (chunk_b at (i + chunk_shift) mod N, chunk_a at (i + chunk_shift + 1) mod N,
    // src_idx = j + intra_shift). To match permute_sw bit-exactly we invert the
    // sign: shift_arg = (D - shift_norm) mod D so out[i] = src[(i - shift_norm) mod D].
    // Phase A passed only because for D=2 (single chunk) +shift ≡ −shift (mod 2)
    // for the single non-zero shift value; in Phase B the asymmetry shows up.
    const std::int32_t d_signed = static_cast<std::int32_t>(src.elements);
    std::int32_t shift_norm = shift % d_signed;
    if (shift_norm < 0) {
        shift_norm += d_signed;
    }
    const std::int32_t shift_arg_int = (d_signed - shift_norm) % d_signed;

    CSR_MVSIZE(static_cast<int>(bytes));
    kmemld(spm_a(),
           const_cast<std::uint32_t*>(src.words),
           static_cast<int>(bytes));

    void* shift_arg = reinterpret_cast<void*>(static_cast<std::intptr_t>(shift_arg_int));
    wait_spm_transfers();
    hvperm(spm_c(), shift_arg, spm_a());

    kmemstr(dst.words, spm_c(), static_cast<int>(bytes));

    return Status::Ok;
}

}  // namespace detail
}  // namespace hdc
}  // namespace klessydra
