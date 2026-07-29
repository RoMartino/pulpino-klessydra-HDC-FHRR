#ifndef KLESSYDRA_HDC_DEFINES_HPP
#define KLESSYDRA_HDC_DEFINES_HPP

#include <cstddef>
#include <cstdint>

namespace klessydra {
namespace hdc {

enum class Representation : std::uint8_t {
    FHRR = 0,
    BSC  = 1,
    MCR  = 2,
};

enum class Backend : std::uint8_t {
    Auto     = 0,
    Software = 1,
    Hardware = 2,
};

enum class Status : std::uint8_t {
    Ok = 0,
    InvalidArgument,
    UnsupportedRepresentation,
    UnsupportedOperation,
    HardwareUnavailable,
    SizeMismatch,
    BufferTooSmall,
};

struct PrimitiveSupport {
    bool bind       = false;
    bool bundle     = false;
    bool similarity = false;
    bool clip       = false;
    bool encode     = false;
    bool permute    = false;   // [Op-N3] cyclic-shift FHRR primitive (Plate 1995 §VIII-G)
};

struct Capabilities {
    Representation representation = Representation::FHRR;
    PrimitiveSupport software {};
    PrimitiveSupport hardware {};
};

struct ContextConfig {
    Representation representation = Representation::FHRR;
    Backend backend               = Backend::Auto;
};

constexpr std::size_t kFhrrLaneBytes        = sizeof(std::uint32_t);
constexpr std::size_t kFhrrPhaseMask        = 0xFFu;
constexpr std::size_t kFhrrFractionalBits   = 16;
constexpr std::size_t kFhrrTangentShift     = 13;
constexpr std::int32_t kFhrrMaxTangentValue = 40 << kFhrrFractionalBits;

#ifdef KLESS_HDC_SIMD
constexpr std::size_t kHardwareSimdLanes = static_cast<std::size_t>(KLESS_HDC_SIMD);
#else
constexpr std::size_t kHardwareSimdLanes = 2;
#endif

#ifdef KLESS_ACCL_SEL
constexpr std::uint32_t kCompiledAcceleratorSelection = static_cast<std::uint32_t>(KLESS_ACCL_SEL);
#else
constexpr std::uint32_t kCompiledAcceleratorSelection = 0;
#endif

constexpr std::uint32_t kAcceleratorSelectionDSP  = 0;
constexpr std::uint32_t kAcceleratorSelectionFHRR = 1;

}  // namespace hdc
}  // namespace klessydra

#endif  // KLESSYDRA_HDC_DEFINES_HPP
