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

template <std::size_t N>
void zero_words(std::array<std::uint32_t, N>& words) {
    for (std::size_t index = 0; index < N; ++index) {
        words[index] = 0;
    }
}

void copy_phase_chunk(std::array<std::uint32_t, kSimdLanes>& dst,
                      FhrrPhaseVectorConstView src,
                      std::size_t offset) {
    zero_words(dst);
    for (std::size_t lane = 0; lane < kSimdLanes && offset + lane < src.elements; ++lane) {
        dst[lane] = src.words[offset + lane];
    }
}

void copy_phase_chunk_back(FhrrPhaseVectorView dst,
                           std::size_t offset,
                           const std::array<std::uint32_t, kSimdLanes>& src) {
    for (std::size_t lane = 0; lane < kSimdLanes && offset + lane < dst.elements; ++lane) {
        dst.words[offset + lane] = src[lane];
    }
}

template <typename AccViewT>
void copy_acc_chunk(std::array<std::uint32_t, kSimdLanes>& imag_chunk,
                    std::array<std::uint32_t, kSimdLanes>& real_chunk,
                    AccViewT src,
                    std::size_t offset) {
    zero_words(imag_chunk);
    zero_words(real_chunk);
    for (std::size_t lane = 0; lane < kSimdLanes && offset + lane < src.elements; ++lane) {
        imag_chunk[lane] = src.imag_words[offset + lane];
        real_chunk[lane] = src.real_words[offset + lane];
    }
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

    std::array<std::uint32_t, kSimdLanes> lhs {};
    std::array<std::uint32_t, kSimdLanes> rhs {};
    std::array<std::uint32_t, kSimdLanes> out {};

    for (std::size_t offset = 0; offset < dst.elements; offset += kSimdLanes) {
        copy_phase_chunk(lhs, a, offset);
        copy_phase_chunk(rhs, b, offset);
        zero_words(out);

        CSR_MVSIZE(static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        kmemld(spm_a(), lhs.data(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        kmemld(spm_b(), rhs.data(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        hvbind(spm_c(), spm_a(), spm_b());
        kmemstr(out.data(), spm_c(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));

        copy_phase_chunk_back(dst, offset, out);
    }

    return Status::Ok;
}

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

    std::array<std::uint32_t, kSimdLanes> imag_chunk {};
    std::array<std::uint32_t, kSimdLanes> real_chunk {};
    std::array<std::uint32_t, kSimdLanes> phase_chunk {};
    std::array<std::uint32_t, 2 * kSimdLanes> phase_cycle_aligned {};
    std::array<std::uint32_t, 2 * kSimdLanes> interleaved_in {};
    std::array<std::uint32_t, 2 * kSimdLanes> interleaved_out {};

    for (std::size_t offset = 0; offset < acc.elements; offset += kSimdLanes) {
        copy_acc_chunk(imag_chunk, real_chunk, acc, offset);
        copy_phase_chunk(phase_chunk, hv, offset);

        for (std::size_t lane = 0; lane < kSimdLanes; ++lane) {
            interleaved_in[lane] = imag_chunk[lane];
            interleaved_in[kSimdLanes + lane] = real_chunk[lane];
        }
        for (std::size_t word = 0; word < interleaved_out.size(); ++word) {
            interleaved_out[word] = 0;
            phase_cycle_aligned[word] = phase_chunk[word % kSimdLanes];
        }

        CSR_MVSIZE(static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        kmemld(spm_a(), interleaved_in.data(), static_cast<int>(sizeof(interleaved_in)));
        kmemld(spm_b(), phase_cycle_aligned.data(), static_cast<int>(sizeof(phase_cycle_aligned)));
        hvbundle(spm_c(), spm_a(), spm_b());
        kmemstr(interleaved_out.data(), spm_c(), static_cast<int>(sizeof(interleaved_out)));

        for (std::size_t lane = 0; lane < kSimdLanes && offset + lane < acc.elements; ++lane) {
            acc.imag_words[offset + lane] = interleaved_out[lane];
            acc.real_words[offset + lane] = interleaved_out[kSimdLanes + lane];
        }
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

    for (std::size_t index = 0; index < acc.elements; ++index) {
        if (static_cast<std::int32_t>(acc.real_words[index]) == 0) {
            return Status::UnsupportedOperation;
        }
    }

    std::array<std::uint32_t, kSimdLanes> imag_chunk {};
    std::array<std::uint32_t, kSimdLanes> real_chunk {};
    std::array<std::uint32_t, kSimdLanes> out_chunk {};

    for (std::size_t offset = 0; offset < dst.elements; offset += kSimdLanes) {
        copy_acc_chunk(imag_chunk, real_chunk, acc, offset);
        zero_words(out_chunk);

        CSR_MVSIZE(static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        kmemld(spm_a(), imag_chunk.data(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        kmemld(spm_b(), real_chunk.data(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        hvclip(spm_c(), spm_a(), spm_b());
        kmemstr(out_chunk.data(), spm_c(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));

        copy_phase_chunk_back(dst, offset, out_chunk);
    }

    return Status::Ok;
}

Status encode_hw(FhrrEncodeAccumulatorView acc,
                 FhrrScalarVectorConstView scalars,
                 FhrrPhaseMatrixConstView hvs) {
    if (!is_hw_fhrr_selected()) {
        return Status::HardwareUnavailable;
    }
    if (!valid_encode_args(acc, scalars, hvs)) {
        return Status::InvalidArgument;
    }
    std::array<std::uint32_t, kSimdLanes> out_chunk {};
    std::array<std::uint32_t, kSimdLanes> row_buffer {};
    std::array<std::uint32_t, kMaxEncodeRows * kSimdLanes> hv_matrix_buffer {};

    if (scalars.count > kMaxEncodeRows) {
        return Status::UnsupportedOperation;
    }

    CSR_MPSCLFAC(static_cast<int>(scalars.count));

    for (std::size_t offset = 0; offset < acc.elements; offset += kSimdLanes) {
        zero_words(out_chunk);
        for (std::size_t word = 0; word < hv_matrix_buffer.size(); ++word) {
            hv_matrix_buffer[word] = 0;
        }

        for (std::size_t row = 0; row < scalars.count; ++row) {
            for (std::size_t lane = 0; lane < kSimdLanes; ++lane) {
                row_buffer[lane] = 0;
                if (offset + lane < acc.elements) {
                    row_buffer[lane] = hvs.words[row * hvs.row_stride + offset + lane];
                }
            }
            for (std::size_t lane = 0; lane < kSimdLanes; ++lane) {
                hv_matrix_buffer[row * kSimdLanes + lane] = row_buffer[lane];
            }
        }

        CSR_MVSIZE(static_cast<int>(kSimdLanes * kFhrrLaneBytes));
        kmemld(spm_a(), const_cast<std::uint32_t*>(scalars.words), static_cast<int>(scalars.count * kFhrrLaneBytes));
        kmemld(spm_b(), hv_matrix_buffer.data(), static_cast<int>(scalars.count * kSimdLanes * kFhrrLaneBytes));
        hvenc(spm_c(), spm_a(), spm_b());
        kmemstr(out_chunk.data(), spm_c(), static_cast<int>(kSimdLanes * kFhrrLaneBytes));

        for (std::size_t lane = 0; lane < kSimdLanes && offset + lane < acc.elements; ++lane) {
            acc.words[offset + lane] = out_chunk[lane];
        }
    }

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
// rs1 is taken mod D in this glue so the FU only sees a value in [0, D),
// keeping the FU's chunk_count/chunk_shift arithmetic compact (D <= 256
// covers every HDC dimension we use; for D > 256 the SW path takes over).
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
    hvperm(spm_c(), shift_arg, spm_a());

    kmemstr(dst.words, spm_c(), static_cast<int>(bytes));

    return Status::Ok;
}

}  // namespace detail
}  // namespace hdc
}  // namespace klessydra
