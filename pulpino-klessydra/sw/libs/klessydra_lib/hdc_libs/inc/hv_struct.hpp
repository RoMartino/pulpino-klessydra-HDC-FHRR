#ifndef KLESSYDRA_HDC_HV_STRUCT_HPP
#define KLESSYDRA_HDC_HV_STRUCT_HPP

#include <cstddef>
#include <cstdint>

namespace klessydra {
namespace hdc {

struct FhrrPhaseVectorView;
struct FhrrBundleAccumulatorView;
struct FhrrEncodeAccumulatorView;

struct FhrrPhaseVectorConstView {
    const std::uint32_t* words = nullptr;
    std::size_t elements       = 0;

    constexpr FhrrPhaseVectorConstView() = default;
    constexpr FhrrPhaseVectorConstView(const std::uint32_t* in_words, std::size_t in_elements)
        : words(in_words), elements(in_elements) {}
    constexpr FhrrPhaseVectorConstView(FhrrPhaseVectorView view);
};

struct FhrrPhaseVectorView {
    std::uint32_t* words = nullptr;
    std::size_t elements = 0;
};

struct FhrrScalarVectorConstView {
    const std::uint32_t* words = nullptr;
    std::size_t count          = 0;
};

struct FhrrBundleAccumulatorConstView {
    const std::uint32_t* imag_words = nullptr;
    const std::uint32_t* real_words = nullptr;
    std::size_t elements            = 0;

    constexpr FhrrBundleAccumulatorConstView() = default;
    constexpr FhrrBundleAccumulatorConstView(const std::uint32_t* in_imag_words,
                                            const std::uint32_t* in_real_words,
                                            std::size_t in_elements)
        : imag_words(in_imag_words), real_words(in_real_words), elements(in_elements) {}
    constexpr FhrrBundleAccumulatorConstView(FhrrBundleAccumulatorView view);
};

struct FhrrBundleAccumulatorView {
    std::uint32_t* imag_words = nullptr;
    std::uint32_t* real_words = nullptr;
    std::size_t elements      = 0;
};

struct FhrrEncodeAccumulatorView {
    std::uint32_t* words = nullptr;
    std::size_t elements = 0;
};

struct FhrrEncodeAccumulatorConstView {
    const std::uint32_t* words = nullptr;
    std::size_t elements       = 0;

    constexpr FhrrEncodeAccumulatorConstView() = default;
    constexpr FhrrEncodeAccumulatorConstView(const std::uint32_t* in_words, std::size_t in_elements)
        : words(in_words), elements(in_elements) {}
    constexpr FhrrEncodeAccumulatorConstView(FhrrEncodeAccumulatorView view);
};

struct FhrrPhaseMatrixConstView {
    const std::uint32_t* words = nullptr;
    std::size_t rows           = 0;
    std::size_t elements       = 0;
    std::size_t row_stride     = 0;
};

constexpr inline FhrrPhaseVectorConstView::FhrrPhaseVectorConstView(FhrrPhaseVectorView view)
    : words(view.words), elements(view.elements) {}

constexpr inline FhrrBundleAccumulatorConstView::FhrrBundleAccumulatorConstView(FhrrBundleAccumulatorView view)
    : imag_words(view.imag_words), real_words(view.real_words), elements(view.elements) {}

constexpr inline FhrrEncodeAccumulatorConstView::FhrrEncodeAccumulatorConstView(FhrrEncodeAccumulatorView view)
    : words(view.words), elements(view.elements) {}

template <std::size_t N>
inline FhrrPhaseVectorView make_fhrr_phase_vector(std::uint32_t (&words)[N]) {
    return {words, N};
}

template <std::size_t N>
inline FhrrPhaseVectorConstView make_fhrr_phase_vector(const std::uint32_t (&words)[N]) {
    return {words, N};
}

template <std::size_t N>
inline FhrrScalarVectorConstView make_fhrr_scalars(const std::uint32_t (&words)[N]) {
    return {words, N};
}

template <std::size_t N>
inline FhrrEncodeAccumulatorView make_fhrr_encode_accumulator(std::uint32_t (&words)[N]) {
    return {words, N};
}

template <std::size_t N>
inline FhrrBundleAccumulatorView make_fhrr_bundle_accumulator(std::uint32_t (&imag_words)[N],
                                                              std::uint32_t (&real_words)[N]) {
    return {imag_words, real_words, N};
}

template <std::size_t N>
inline FhrrBundleAccumulatorConstView make_fhrr_bundle_accumulator(const std::uint32_t (&imag_words)[N],
                                                                   const std::uint32_t (&real_words)[N]) {
    return {imag_words, real_words, N};
}

}  // namespace hdc
}  // namespace klessydra

#endif  // KLESSYDRA_HDC_HV_STRUCT_HPP
