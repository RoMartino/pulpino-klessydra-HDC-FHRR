#include "hdc_internal.hpp"

namespace klessydra {
namespace hdc {

namespace {

Status dispatch_bind(Context& context,
                     FhrrPhaseVectorView dst,
                     FhrrPhaseVectorConstView a,
                     FhrrPhaseVectorConstView b) {
    const bool hw_supported = detail::is_hw_fhrr_selected();
    const Backend backend   = detail::resolve_backend(context, hw_supported);

    if (backend == Backend::Hardware) {
        return detail::bind_hw(dst, a, b);
    }
    return detail::bind_sw(dst, a, b);
}

Status dispatch_bundle(Context& context,
                       FhrrBundleAccumulatorView acc,
                       FhrrPhaseVectorConstView hv) {
    const bool hw_supported = detail::is_hw_fhrr_selected();
    const Backend backend   = detail::resolve_backend(context, hw_supported);

    if (backend == Backend::Hardware) {
        const Status hw_status = detail::bundle_hw(acc, hv);
        if (hw_status == Status::UnsupportedOperation && context.config.backend == Backend::Auto) {
            return detail::bundle_sw(acc, hv);
        }
        return hw_status;
    }
    return detail::bundle_sw(acc, hv);
}

Status dispatch_similarity(Context& context,
                           std::int32_t& out,
                           FhrrPhaseVectorConstView a,
                           FhrrPhaseVectorConstView b) {
    const bool hw_supported = detail::is_hw_fhrr_selected();
    const Backend backend   = detail::resolve_backend(context, hw_supported);

    if (backend == Backend::Hardware) {
        const Status hw_status = detail::similarity_hw(out, a, b);
        if (hw_status == Status::UnsupportedOperation && context.config.backend == Backend::Auto) {
            return detail::similarity_sw(out, a, b);
        }
        return hw_status;
    }
    return detail::similarity_sw(out, a, b);
}

Status dispatch_clip(Context& context,
                     FhrrPhaseVectorView dst,
                     FhrrBundleAccumulatorConstView acc) {
    const bool hw_supported = detail::is_hw_fhrr_selected();
    const Backend backend   = detail::resolve_backend(context, hw_supported);

    if (backend == Backend::Hardware) {
        const Status hw_status = detail::clip_hw(dst, acc);
        if (hw_status == Status::UnsupportedOperation && context.config.backend == Backend::Auto) {
            return detail::clip_sw(dst, acc);
        }
        return hw_status;
    }
    return detail::clip_sw(dst, acc);
}

Status dispatch_encode(Context& context,
                       FhrrEncodeAccumulatorView acc,
                       FhrrScalarVectorConstView scalars,
                       FhrrPhaseMatrixConstView hvs) {
    const bool hw_supported = detail::is_hw_fhrr_selected();
    const Backend backend   = detail::resolve_backend(context, hw_supported);

    if (backend == Backend::Hardware) {
        const Status hw_status = detail::encode_hw(acc, scalars, hvs);
        if (hw_status == Status::UnsupportedOperation && context.config.backend == Backend::Auto) {
            return detail::encode_sw(acc, scalars, hvs);
        }
        return hw_status;
    }
    return detail::encode_sw(acc, scalars, hvs);
}

// [Op-N3] permutation dispatch — same fallback pattern as the other primitives.
// Until Op-N2 lands, permute_hw returns UnsupportedOperation and Auto mode falls back to SW.
Status dispatch_permute(Context& context,
                        FhrrPhaseVectorView dst,
                        FhrrPhaseVectorConstView src,
                        std::int32_t shift) {
    const bool hw_supported = detail::is_hw_fhrr_selected();
    const Backend backend   = detail::resolve_backend(context, hw_supported);

    if (backend == Backend::Hardware) {
        const Status hw_status = detail::permute_hw(dst, src, shift);
        if (hw_status == Status::UnsupportedOperation && context.config.backend == Backend::Auto) {
            return detail::permute_sw(dst, src, shift);
        }
        return hw_status;
    }
    return detail::permute_sw(dst, src, shift);
}

}  // namespace

Capabilities capabilities(const Context& context) {
    Capabilities caps {};
    caps.representation = context.config.representation;

    if (context.config.representation != Representation::FHRR) {
        return caps;
    }

    // [Op-N2 Phase A] permute SW always supported; HW now landed (RTL FU + ID_STAGE
    // decode + permute_hw glue). HW path returns UnsupportedOperation for D > kSimdLanes
    // (cross-chunk path is Phase B); Auto mode falls back to SW automatically there.
    caps.software = {true, true, true, true, true, true};
    if (detail::is_hw_fhrr_selected()) {
        caps.hardware = {true, true, true, true, true, true};
    }

    return caps;
}

Status bind(Context& context,
            FhrrPhaseVectorView dst,
            FhrrPhaseVectorConstView a,
            FhrrPhaseVectorConstView b) {
    if (context.config.representation != Representation::FHRR) {
        return Status::UnsupportedRepresentation;
    }
    return dispatch_bind(context, dst, a, b);
}

Status bundle(Context& context,
              FhrrBundleAccumulatorView acc,
              FhrrPhaseVectorConstView hv) {
    if (context.config.representation != Representation::FHRR) {
        return Status::UnsupportedRepresentation;
    }
    return dispatch_bundle(context, acc, hv);
}

Status similarity(Context& context,
                  std::int32_t& out,
                  FhrrPhaseVectorConstView a,
                  FhrrPhaseVectorConstView b) {
    if (context.config.representation != Representation::FHRR) {
        return Status::UnsupportedRepresentation;
    }
    return dispatch_similarity(context, out, a, b);
}

Status clip(Context& context,
            FhrrPhaseVectorView dst,
            FhrrBundleAccumulatorConstView acc) {
    if (context.config.representation != Representation::FHRR) {
        return Status::UnsupportedRepresentation;
    }
    return dispatch_clip(context, dst, acc);
}

Status encode(Context& context,
              FhrrEncodeAccumulatorView acc,
              FhrrScalarVectorConstView scalars,
              FhrrPhaseMatrixConstView hvs) {
    if (context.config.representation != Representation::FHRR) {
        return Status::UnsupportedRepresentation;
    }
    return dispatch_encode(context, acc, scalars, hvs);
}

Status permute(Context& context,
               FhrrPhaseVectorView dst,
               FhrrPhaseVectorConstView src,
               std::int32_t shift) {
    if (context.config.representation != Representation::FHRR) {
        return Status::UnsupportedRepresentation;
    }
    return dispatch_permute(context, dst, src, shift);
}

const char* to_string(Status status) {
    switch (status) {
        case Status::Ok:
            return "ok";
        case Status::InvalidArgument:
            return "invalid_argument";
        case Status::UnsupportedRepresentation:
            return "unsupported_representation";
        case Status::UnsupportedOperation:
            return "unsupported_operation";
        case Status::HardwareUnavailable:
            return "hardware_unavailable";
        case Status::SizeMismatch:
            return "size_mismatch";
        case Status::BufferTooSmall:
            return "buffer_too_small";
        default:
            return "unknown_status";
    }
}

const char* to_string(Backend backend) {
    switch (backend) {
        case Backend::Auto:
            return "auto";
        case Backend::Software:
            return "software";
        case Backend::Hardware:
            return "hardware";
        default:
            return "unknown_backend";
    }
}

const char* to_string(Representation representation) {
    switch (representation) {
        case Representation::FHRR:
            return "fhrr";
        case Representation::BSC:
            return "bsc";
        case Representation::MCR:
            return "mcr";
        default:
            return "unknown_representation";
    }
}

namespace detail {

Backend resolve_backend(const Context& context, bool hw_supported) {
    if (context.config.backend == Backend::Hardware) {
        return hw_supported ? Backend::Hardware : Backend::Hardware;
    }
    if (context.config.backend == Backend::Software) {
        return Backend::Software;
    }
    return hw_supported ? Backend::Hardware : Backend::Software;
}

}  // namespace detail

}  // namespace hdc
}  // namespace klessydra
