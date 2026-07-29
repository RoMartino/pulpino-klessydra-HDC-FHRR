#ifndef KLESSYDRA_HDC_CSR_ACCESS_HPP
#define KLESSYDRA_HDC_CSR_ACCESS_HPP

#include <cstdint>

namespace klessydra {
namespace hdc {
namespace detail {

inline std::int32_t read_csr(std::uint32_t csr_address) {
    std::int32_t value = 0;

    switch (csr_address) {
        case 0xBD0:
            __asm__ volatile("csrr %0, 0xBD0" : "=r"(value));
            break;
        case 0xBD1:
            __asm__ volatile("csrr %0, 0xBD1" : "=r"(value));
            break;
        case 0xBD2:
            __asm__ volatile("csrr %0, 0xBD2" : "=r"(value));
            break;
        case 0xBD3:
            __asm__ volatile("csrr %0, 0xBD3" : "=r"(value));
            break;
        case 0xBD4:
            __asm__ volatile("csrr %0, 0xBD4" : "=r"(value));
            break;
        case 0xBD5:
            __asm__ volatile("csrr %0, 0xBD5" : "=r"(value));
            break;
        case 0xBD6:
            __asm__ volatile("csrr %0, 0xBD6" : "=r"(value));
            break;
        case 0xBD7:
            __asm__ volatile("csrr %0, 0xBD7" : "=r"(value));
            break;
        case 0xBD8:
            __asm__ volatile("csrr %0, 0xBD8" : "=r"(value));
            break;
        default:
            value = 0;
            break;
    }

    return value;
}

inline std::int32_t get_perf_hdcu_total() {
    return read_csr(0xBD0);
}

inline std::int32_t get_perf_hdcu_bind() {
    return read_csr(0xBD1);
}

inline std::int32_t get_perf_hdcu_bundle() {
    return read_csr(0xBD2);
}

inline std::int32_t get_perf_hdcu_similarity() {
    return read_csr(0xBD3);
}

inline std::int32_t get_perf_hdcu_clip() {
    return read_csr(0xBD4);
}

inline std::int32_t get_perf_hdcu_perm() {
    return read_csr(0xBD5);
}

inline std::int32_t get_perf_hdcu_search() {
    return read_csr(0xBD6);
}

inline std::int32_t get_perf_hdcu_mcycle() {
    return read_csr(0xBD7);
}

inline std::int32_t get_perf_hdcu_encode() {
    return read_csr(0xBD8);
}

}  // namespace detail
}  // namespace hdc
}  // namespace klessydra

#endif  // KLESSYDRA_HDC_CSR_ACCESS_HPP
