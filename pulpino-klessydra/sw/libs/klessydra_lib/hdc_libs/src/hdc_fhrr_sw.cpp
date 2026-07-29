#include "hdc_internal.hpp"

#include <array>

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

std::uint32_t to_low_byte_word(std::uint8_t value) {
    return static_cast<std::uint32_t>(value);
}

static const std::array<std::int32_t, 256> kSineLutTable = {{
    0, 1608, 3216, 4821, 6424, 8022, 9616, 11204,
    12785, 14359, 15924, 17479, 19024, 20557, 22078, 23586,
    25080, 26558, 28020, 29466, 30893, 32303, 33692, 35062,
    36410, 37736, 39040, 40320, 41576, 42806, 44011, 45190,
    46341, 47464, 48559, 49624, 50660, 51665, 52639, 53581,
    54491, 55368, 56212, 57022, 57798, 58538, 59244, 59914,
    60547, 61145, 61705, 62228, 62714, 63162, 63572, 63944,
    64277, 64571, 64827, 65043, 65220, 65358, 65457, 65516,
    65536, 65516, 65457, 65358, 65220, 65043, 64827, 64571,
    64277, 63944, 63572, 63162, 62714, 62228, 61705, 61145,
    60547, 59914, 59244, 58538, 57798, 57022, 56212, 55368,
    54491, 53581, 52639, 51665, 50660, 49624, 48559, 47464,
    46341, 45190, 44011, 42806, 41576, 40320, 39040, 37736,
    36410, 35062, 33692, 32303, 30893, 29466, 28020, 26558,
    25080, 23586, 22078, 20557, 19024, 17479, 15924, 14359,
    12785, 11204, 9616, 8022, 6424, 4821, 3216, 1608,
    0, -1608, -3216, -4821, -6424, -8022, -9616, -11204,
    -12785, -14359, -15924, -17479, -19024, -20557, -22078, -23586,
    -25080, -26558, -28020, -29466, -30893, -32303, -33692, -35062,
    -36410, -37736, -39040, -40320, -41576, -42806, -44011, -45190,
    -46341, -47464, -48559, -49624, -50660, -51665, -52639, -53581,
    -54491, -55368, -56212, -57022, -57798, -58538, -59244, -59914,
    -60547, -61145, -61705, -62228, -62714, -63162, -63572, -63944,
    -64277, -64571, -64827, -65043, -65220, -65358, -65457, -65516,
    -65536, -65516, -65457, -65358, -65220, -65043, -64827, -64571,
    -64277, -63944, -63572, -63162, -62714, -62228, -61705, -61145,
    -60547, -59914, -59244, -58538, -57798, -57022, -56212, -55368,
    -54491, -53581, -52639, -51665, -50660, -49624, -48559, -47464,
    -46341, -45190, -44011, -42806, -41576, -40320, -39040, -37736,
    -36410, -35062, -33692, -32303, -30893, -29466, -28020, -26558,
    -25080, -23586, -22078, -20557, -19024, -17479, -15924, -14359,
    -12785, -11204, -9616, -8022, -6424, -4821, -3216, -1608
}};

static const std::array<std::int32_t, 256> kCosineLutTable = {{
    65536, 65516, 65457, 65358, 65220, 65043, 64827, 64571,
    64277, 63944, 63572, 63162, 62714, 62228, 61705, 61145,
    60547, 59914, 59244, 58538, 57798, 57022, 56212, 55368,
    54491, 53581, 52639, 51665, 50660, 49624, 48559, 47464,
    46341, 45190, 44011, 42806, 41576, 40320, 39040, 37736,
    36410, 35062, 33692, 32303, 30893, 29466, 28020, 26558,
    25080, 23586, 22078, 20557, 19024, 17479, 15924, 14359,
    12785, 11204, 9616, 8022, 6424, 4821, 3216, 1608,
    0, -1608, -3216, -4821, -6424, -8022, -9616, -11204,
    -12785, -14359, -15924, -17479, -19024, -20557, -22078, -23586,
    -25080, -26558, -28020, -29466, -30893, -32303, -33692, -35062,
    -36410, -37736, -39040, -40320, -41576, -42806, -44011, -45190,
    -46341, -47464, -48559, -49624, -50660, -51665, -52639, -53581,
    -54491, -55368, -56212, -57022, -57798, -58538, -59244, -59914,
    -60547, -61145, -61705, -62228, -62714, -63162, -63572, -63944,
    -64277, -64571, -64827, -65043, -65220, -65358, -65457, -65516,
    -65536, -65516, -65457, -65358, -65220, -65043, -64827, -64571,
    -64277, -63944, -63572, -63162, -62714, -62228, -61705, -61145,
    -60547, -59914, -59244, -58538, -57798, -57022, -56212, -55368,
    -54491, -53581, -52639, -51665, -50660, -49624, -48559, -47464,
    -46341, -45190, -44011, -42806, -41576, -40320, -39040, -37736,
    -36410, -35062, -33692, -32303, -30893, -29466, -28020, -26558,
    -25080, -23586, -22078, -20557, -19024, -17479, -15924, -14359,
    -12785, -11204, -9616, -8022, -6424, -4821, -3216, -1608,
    0, 1608, 3216, 4821, 6424, 8022, 9616, 11204,
    12785, 14359, 15924, 17479, 19024, 20557, 22078, 23586,
    25080, 26558, 28020, 29466, 30893, 32303, 33692, 35062,
    36410, 37736, 39040, 40320, 41576, 42806, 44011, 45190,
    46341, 47464, 48559, 49624, 50660, 51665, 52639, 53581,
    54491, 55368, 56212, 57022, 57798, 58538, 59244, 59914,
    60547, 61145, 61705, 62228, 62714, 63162, 63572, 63944,
    64277, 64571, 64827, 65043, 65220, 65358, 65457, 65516
}};

static const std::array<std::int32_t, 256> kAtanLutTable = {{
    0, 10197, 19923, 28826, 36724, 43588, 49487, 54532,
    58849, 62554, 65750, 68521, 70940, 73064, 74940, 76607,
    78095, 79431, 80636, 81728, 82721, 83628, 84459, 85224,
    85929, 86581, 87186, 87748, 88273, 88763, 89221, 89652,
    90056, 90437, 90797, 91136, 91457, 91762, 92051, 92325,
    92586, 92835, 93072, 93298, 93514, 93721, 93919, 94108,
    94290, 94465, 94633, 94794, 94949, 95099, 95242, 95381,
    95515, 95644, 95769, 95890, 96007, 96119, 96229, 96335,
    96437, 96537, 96633, 96727, 96818, 96906, 96992, 97075,
    97156, 97235, 97312, 97387, 97459, 97530, 97599, 97667,
    97732, 97796, 97859, 97920, 97980, 98038, 98095, 98150,
    98204, 98257, 98309, 98360, 98410, 98458, 98506, 98552,
    98598, 98643, 98687, 98729, 98771, 98813, 98853, 98893,
    98931, 98970, 99007, 99044, 99080, 99115, 99150, 99184,
    99217, 99250, 99283, 99314, 99346, 99376, 99407, 99436,
    99465, 99494, 99522, 99550, 99577, 99604, 99631, 99657,
    99682, 99708, 99732, 99757, 99781, 99805, 99828, 99851,
    99874, 99896, 99918, 99940, 99962, 99983, 100003, 100024,
    100044, 100064, 100084, 100103, 100123, 100141, 100160, 100179,
    100197, 100215, 100232, 100250, 100267, 100284, 100301, 100317,
    100334, 100350, 100366, 100382, 100397, 100413, 100428, 100443,
    100458, 100473, 100487, 100502, 100516, 100530, 100544, 100557,
    100571, 100584, 100598, 100611, 100624, 100636, 100649, 100662,
    100674, 100686, 100698, 100710, 100722, 100734, 100746, 100757,
    100769, 100780, 100791, 100802, 100813, 100824, 100834, 100845,
    100855, 100866, 100876, 100886, 100896, 100906, 100916, 100926,
    100936, 100945, 100955, 100964, 100974, 100983, 100992, 101001,
    101010, 101019, 101028, 101037, 101045, 101054, 101062, 101071,
    101079, 101087, 101096, 101104, 101112, 101120, 101128, 101136,
    101143, 101151, 101159, 101166, 101174, 101181, 101189, 101196,
    101203, 101211, 101218, 101225, 101232, 101239, 101246, 101253,
    101259, 101266, 101273, 101280, 101286, 101293, 101299, 101306
}};

}  // namespace

std::int32_t to_sfixed32(double value) {
    const double scale = static_cast<double>(1u << kFhrrFractionalBits);
    double scaled      = value * scale;
    if (scaled >= 0.0) {
        scaled += 0.5;
    } else {
        scaled -= 0.5;
    }
    return static_cast<std::int32_t>(scaled);
}

std::uint8_t phase_byte(std::uint32_t word) {
    return static_cast<std::uint8_t>(word & kFhrrPhaseMask);
}

std::int8_t signed_phase(std::uint32_t word) {
    return static_cast<std::int8_t>(phase_byte(word));
}

std::uint32_t pack_phase(std::uint8_t value) {
    return static_cast<std::uint32_t>(value);
}

std::uint32_t pack_signed_sfixed(std::int32_t value) {
    return static_cast<std::uint32_t>(value);
}

const std::array<std::int32_t, 256>& sine_lut_bundle() {
    return kSineLutTable;
}

const std::array<std::int32_t, 256>& cosine_lut() {
    return kCosineLutTable;
}

const std::array<std::int32_t, 256>& atan_lut() {
    return kAtanLutTable;
}

Status bind_sw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b) {
    if (!valid_phase_view(dst) || !valid_phase_view(a) || !valid_phase_view(b)) {
        return Status::InvalidArgument;
    }
    if (dst.elements != a.elements || dst.elements != b.elements) {
        return Status::SizeMismatch;
    }

    for (std::size_t index = 0; index < dst.elements; ++index) {
        const std::uint16_t sum = static_cast<std::uint16_t>(phase_byte(a.words[index])) +
                                  static_cast<std::uint16_t>(phase_byte(b.words[index]));
        dst.words[index] = to_low_byte_word(static_cast<std::uint8_t>(sum & 0xFFu));
    }

    return Status::Ok;
}

Status bundle_sw(FhrrBundleAccumulatorView acc, FhrrPhaseVectorConstView hv) {
    if (!valid_bundle_view(acc) || !valid_phase_view(hv)) {
        return Status::InvalidArgument;
    }
    if (acc.elements != hv.elements) {
        return Status::SizeMismatch;
    }

    for (std::size_t index = 0; index < acc.elements; ++index) {
        const std::int8_t phase = signed_phase(hv.words[index]);
        const std::uint8_t lut_index = static_cast<std::uint8_t>(phase < 0 ? -phase : phase);
        const std::int32_t sine_value = sine_lut_bundle()[lut_index];
        const std::int32_t imag       = phase < 0 ? -sine_value : sine_value;
        const std::int32_t real       = cosine_lut()[lut_index];

        const std::int32_t current_imag = static_cast<std::int32_t>(acc.imag_words[index]);
        const std::int32_t current_real = static_cast<std::int32_t>(acc.real_words[index]);

        acc.imag_words[index] = pack_signed_sfixed(current_imag + imag);
        acc.real_words[index] = pack_signed_sfixed(current_real + real);
    }

    return Status::Ok;
}

Status similarity_sw(std::int32_t& out, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b) {
    if (!valid_phase_view(a) || !valid_phase_view(b)) {
        return Status::InvalidArgument;
    }
    if (a.elements != b.elements) {
        return Status::SizeMismatch;
    }
    if (!is_power_of_two(a.elements)) {
        return Status::UnsupportedOperation;
    }

    std::int32_t accumulator  = 0;

    for (std::size_t index = 0; index < a.elements; ++index) {
        const std::uint8_t delta = static_cast<std::uint8_t>(phase_byte(a.words[index]) - phase_byte(b.words[index]));
        const std::int8_t signed_delta = static_cast<std::int8_t>(delta);
        const std::uint8_t lut_index = static_cast<std::uint8_t>(signed_delta < 0 ? -signed_delta : signed_delta);
        accumulator += cosine_lut()[lut_index];
    }

    std::size_t shift = 0;
    for (std::size_t value = a.elements; value > 1; value >>= 1) {
        ++shift;
    }

    out = accumulator >> shift;
    return Status::Ok;
}

Status clip_sw(FhrrPhaseVectorView dst, FhrrBundleAccumulatorConstView acc) {
    if (!valid_phase_view(dst) || !valid_bundle_view(acc)) {
        return Status::InvalidArgument;
    }
    if (dst.elements != acc.elements) {
        return Status::SizeMismatch;
    }

    for (std::size_t index = 0; index < dst.elements; ++index) {
        const std::int32_t imag = static_cast<std::int32_t>(acc.imag_words[index]);
        const std::int32_t real = static_cast<std::int32_t>(acc.real_words[index]);

        if (real == 0) {
            return Status::UnsupportedOperation;
        }

        const std::int64_t dividend = static_cast<std::int64_t>(imag) << kFhrrFractionalBits;
        const std::int32_t tangent = static_cast<std::int32_t>(dividend / static_cast<std::int64_t>(real));
        const std::int32_t magnitude = tangent < 0 ? -tangent : tangent;

        const std::size_t lut_index = magnitude < kFhrrMaxTangentValue
                                          ? static_cast<std::size_t>(magnitude >> kFhrrTangentShift)
                                          : 255u;

        const std::int32_t atan_value = atan_lut()[static_cast<std::uint8_t>(lut_index)];
        const std::int32_t clipped    = tangent < 0 ? -atan_value : atan_value;
        dst.words[index] = pack_signed_sfixed(clipped);
    }

    return Status::Ok;
}

Status encode_sw(FhrrEncodeAccumulatorView acc,
                 FhrrScalarVectorConstView scalars,
                 FhrrPhaseMatrixConstView hvs) {
    if (!valid_encode_args(acc, scalars, hvs)) {
        return Status::InvalidArgument;
    }

    for (std::size_t element = 0; element < acc.elements; ++element) {
        acc.words[element] = 0;
    }

    for (std::size_t row = 0; row < scalars.count; ++row) {
        const std::int8_t scalar = static_cast<std::int8_t>(phase_byte(scalars.words[row]));

        for (std::size_t element = 0; element < acc.elements; ++element) {
            const std::int8_t value = static_cast<std::int8_t>(
                phase_byte(hvs.words[row * hvs.row_stride + element]));
            const std::int16_t product = static_cast<std::int16_t>(scalar) * static_cast<std::int16_t>(value);
            const std::int8_t truncated = static_cast<std::int8_t>(product >> 8);
            const std::uint8_t updated =
                static_cast<std::uint8_t>(phase_byte(acc.words[element]) + static_cast<std::uint8_t>(truncated));

            acc.words[element] = to_low_byte_word(updated);
        }
    }

    return Status::Ok;
}

// [Op-N3] Cyclic-shift permutation (FHRR ρ_k):  dst[i] = src[(i - shift) mod D].
// Plate 1995 §VIII-G; Kleyko 2022 §2.2.3. Shift normalized to [0, D) regardless of sign or magnitude.
// Bit-exact (0 ULP) reference: pure index remapping, no arithmetic on the phase values.
Status permute_sw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView src, std::int32_t shift) {
    if (!valid_phase_view(dst) || !valid_phase_view(src)) {
        return Status::InvalidArgument;
    }
    if (dst.elements != src.elements) {
        return Status::SizeMismatch;
    }
    if (dst.words == src.words) {
        // In-place not supported by this reference; caller must use a separate destination buffer.
        return Status::InvalidArgument;
    }

    const std::int64_t elements   = static_cast<std::int64_t>(src.elements);
    std::int64_t       normalized = static_cast<std::int64_t>(shift) % elements;
    if (normalized < 0) {
        normalized += elements;
    }

    for (std::size_t index = 0; index < dst.elements; ++index) {
        std::int64_t source_index = static_cast<std::int64_t>(index) - normalized;
        if (source_index < 0) {
            source_index += elements;
        }
        dst.words[index] = src.words[static_cast<std::size_t>(source_index)];
    }

    return Status::Ok;
}

}  // namespace detail
}  // namespace hdc
}  // namespace klessydra
