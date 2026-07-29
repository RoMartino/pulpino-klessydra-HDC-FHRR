#ifndef KLESSYDRA_HDC_INTERNAL_HPP
#define KLESSYDRA_HDC_INTERNAL_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "hdc_class.hpp"

namespace klessydra {
namespace hdc {
namespace detail {

inline constexpr bool is_hw_fhrr_selected() {
    return kCompiledAcceleratorSelection == kAcceleratorSelectionFHRR;
}

inline constexpr bool is_power_of_two(std::size_t value) {
    return value != 0 && (value & (value - 1)) == 0;
}

std::int32_t to_sfixed32(double value);
std::uint8_t phase_byte(std::uint32_t word);
std::int8_t signed_phase(std::uint32_t word);
std::uint32_t pack_phase(std::uint8_t value);
std::uint32_t pack_signed_sfixed(std::int32_t value);

const std::array<std::int32_t, 256>& sine_lut_bundle();
const std::array<std::int32_t, 256>& cosine_lut();
const std::array<std::int32_t, 256>& atan_lut();

Backend resolve_backend(const Context& context, bool hw_supported);

Status bind_sw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b);
Status bundle_sw(FhrrBundleAccumulatorView acc, FhrrPhaseVectorConstView hv);
Status similarity_sw(std::int32_t& out, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b);
Status clip_sw(FhrrPhaseVectorView dst, FhrrBundleAccumulatorConstView acc);
Status encode_sw(FhrrEncodeAccumulatorView acc,
                 FhrrScalarVectorConstView scalars,
                 FhrrPhaseMatrixConstView hvs);
Status permute_sw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView src, std::int32_t shift);

Status bind_hw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b);
Status bundle_hw(FhrrBundleAccumulatorView acc, FhrrPhaseVectorConstView hv);
Status similarity_hw(std::int32_t& out, FhrrPhaseVectorConstView a, FhrrPhaseVectorConstView b);
Status clip_hw(FhrrPhaseVectorView dst, FhrrBundleAccumulatorConstView acc);
Status encode_hw(FhrrEncodeAccumulatorView acc,
                 FhrrScalarVectorConstView scalars,
                 FhrrPhaseMatrixConstView hvs);
Status permute_hw(FhrrPhaseVectorView dst, FhrrPhaseVectorConstView src, std::int32_t shift);

}  // namespace detail
}  // namespace hdc
}  // namespace klessydra

#endif  // KLESSYDRA_HDC_INTERNAL_HPP
