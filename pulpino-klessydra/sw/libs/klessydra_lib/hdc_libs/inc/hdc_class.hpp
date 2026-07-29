#ifndef KLESSYDRA_HDC_CLASS_HPP
#define KLESSYDRA_HDC_CLASS_HPP

#include <cstdint>

#include "hdc_defines.hpp"
#include "hv_struct.hpp"

namespace klessydra {
namespace hdc {

struct Context {
    explicit constexpr Context(ContextConfig cfg = {}) : config(cfg) {}
    ContextConfig config;
};

Capabilities capabilities(const Context& context);

Status bind(Context& context,
            FhrrPhaseVectorView dst,
            FhrrPhaseVectorConstView a,
            FhrrPhaseVectorConstView b);

Status bundle(Context& context,
              FhrrBundleAccumulatorView acc,
              FhrrPhaseVectorConstView hv);

Status similarity(Context& context,
                  std::int32_t& out,
                  FhrrPhaseVectorConstView a,
                  FhrrPhaseVectorConstView b);

Status clip(Context& context,
            FhrrPhaseVectorView dst,
            FhrrBundleAccumulatorConstView acc);

Status encode(Context& context,
              FhrrEncodeAccumulatorView acc,
              FhrrScalarVectorConstView scalars,
              FhrrPhaseMatrixConstView hvs);

// [Op-N3] Cyclic-shift permutation (FHRR ρ_k):  dst[i] = src[(i - shift) mod D].
// Plate 1995 §VIII-G, Kleyko 2022 §2.2.3. shift may be negative or |shift| > elements.
// Composition is closed: ρ_{k1}(ρ_{k2}(a)) = ρ_{k1+k2}(a) → ρ^t encoded by single
// call with shift = t * base_shift. Inverse exact: ρ_{-k}(ρ_k(a)) = a.
Status permute(Context& context,
               FhrrPhaseVectorView dst,
               FhrrPhaseVectorConstView src,
               std::int32_t shift);

const char* to_string(Status status);
const char* to_string(Backend backend);
const char* to_string(Representation representation);

}  // namespace hdc
}  // namespace klessydra

#endif  // KLESSYDRA_HDC_CLASS_HPP
