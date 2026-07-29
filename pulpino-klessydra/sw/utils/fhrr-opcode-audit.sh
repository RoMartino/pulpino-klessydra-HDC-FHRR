#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SW_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

REPO_ROOT="$(cd "${SW_DIR}/.." && pwd)"

BUILD_DIR="${SW_DIR}/build_fhrr"
TOOLCHAIN_ROOT_DEFAULT="${REPO_ROOT}/../riscv-gnu-toolchain"
TOOLCHAIN_ROOT="${TOOLCHAIN_ROOT:-${TOOLCHAIN_ROOT_DEFAULT}}"
TOOLCHAIN_HEADER="${TOOLCHAIN_ROOT}/binutils/include/opcode/riscv-opc.h"
OBJDUMP="${OBJDUMP:-$(command -v klessydra-unknown-elf-objdump || echo "${REPO_ROOT}/../riscv-gnu-toolchain_build/bin/klessydra-unknown-elf-objdump")}"

DEFAULT_TESTS=(
  "fhrr_bind_test"
  "fhrr_bundle_test"
  "fhrr_clip_test"
  "fhrr_encode_test"
  "fhrr_similarity_test"
)

declare -A TEST_EXPECTED_OPS=(
  [fhrr_bind_test]="hvbind"
  [fhrr_bundle_test]="hvbundle"
  [fhrr_clip_test]="hvclip"
  [fhrr_encode_test]="hvenc"
  [fhrr_similarity_test]="hvsim"
)

declare -A MATCH_SYMBOLS=(
  [hvbundle]="MATCH_HV_BUNDLE"
  [hvclip]="MATCH_HV_CLIP"
  [hvbind]="MATCH_HV_BIND"
  [hvsim]="MATCH_HV_SIM"
  [hvenc]="MATCH_HV_ENC"
)

usage() {
  cat <<'EOF'
Usage: fhrr-opcode-audit.sh [options] [test_name ...]

Options:
  --build-dir PATH    Build directory to inspect. Default: sw/build_fhrr
  --toolchain PATH    Toolchain source root containing riscv-opc.h
  --objdump PATH      objdump binary to use
  --help              Show this help

Examples:
  ./sw/utils/fhrr-opcode-audit.sh
  ./sw/utils/fhrr-opcode-audit.sh fhrr_bind_test fhrr_bundle_test
EOF
}

POSITIONAL_TESTS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --toolchain)
      TOOLCHAIN_ROOT="$2"
      TOOLCHAIN_HEADER="${TOOLCHAIN_ROOT}/binutils/include/opcode/riscv-opc.h"
      shift 2
      ;;
    --objdump)
      OBJDUMP="$2"
      shift 2
      ;;
    --help)
      usage
      exit 0
      ;;
    *)
      POSITIONAL_TESTS+=("$1")
      shift
      ;;
  esac
done

if [[ ! -x "${OBJDUMP}" ]]; then
  echo "objdump not found or not executable: ${OBJDUMP}" >&2
  exit 1
fi

if [[ ! -f "${TOOLCHAIN_HEADER}" ]]; then
  echo "toolchain opcode header not found: ${TOOLCHAIN_HEADER}" >&2
  exit 1
fi

if [[ ${#POSITIONAL_TESTS[@]} -eq 0 ]]; then
  TESTS=("${DEFAULT_TESTS[@]}")
else
  TESTS=("${POSITIONAL_TESTS[@]}")
fi

extract_define_hex() {
  local symbol="$1"
  awk -v symbol="${symbol}" '$1 == "#define" && $2 == symbol { print $3; exit }' "${TOOLCHAIN_HEADER}"
}

MASK_K_ARITH="$(extract_define_hex MASK_K_ARITH)"
if [[ -z "${MASK_K_ARITH}" ]]; then
  echo "MASK_K_ARITH not found in ${TOOLCHAIN_HEADER}" >&2
  exit 1
fi

normalize_hex() {
  local value="${1#0x}"
  printf "0x%08X" "$((16#${value}))"
}

mask_word() {
  local word="${1#0x}"
  local mask="${2#0x}"
  printf "0x%08X" "$(( (16#${word}) & (16#${mask}) ))"
}

echo "Build directory: ${BUILD_DIR}"
echo "Toolchain header: ${TOOLCHAIN_HEADER}"
echo "Objdump: ${OBJDUMP}"
echo "MASK_K_ARITH: $(normalize_hex "${MASK_K_ARITH}")"
echo

overall_status=0

for test_name in "${TESTS[@]}"; do
  elf_path="${BUILD_DIR}/apps/klessydra_tests/klessydra_hdc_tests/${test_name}/${test_name}.elf"
  expected_ops="${TEST_EXPECTED_OPS[${test_name}]:-}"

  if [[ ! -f "${elf_path}" ]]; then
    echo "[${test_name}] missing ELF: ${elf_path}"
    overall_status=1
    echo
    continue
  fi

  if [[ -z "${expected_ops}" ]]; then
    echo "[${test_name}] no expected opcode map configured"
    overall_status=1
    echo
    continue
  fi

  echo "[${test_name}]"
  disassembly="$("${OBJDUMP}" -d "${elf_path}")"
  test_status=0

  for mnemonic in ${expected_ops}; do
    match_symbol="${MATCH_SYMBOLS[${mnemonic}]}"
    match_hex="$(extract_define_hex "${match_symbol}")"
    if [[ -z "${match_hex}" ]]; then
      echo "  ${mnemonic}: missing ${match_symbol} in toolchain header"
      test_status=1
      continue
    fi

    first_hit="$(awk -v mnemonic="${mnemonic}" '
      $0 ~ ("[[:space:]]" mnemonic "[[:space:]]") {
        print $0
        exit
      }' <<<"${disassembly}")"

    if [[ -z "${first_hit}" ]]; then
      echo "  ${mnemonic}: NOT EMITTED"
      test_status=1
      continue
    fi

    encoded_word="$(printf '%s\n' "${first_hit}" | awk '{print $2}')"
    masked_word="$(mask_word "${encoded_word}" "${MASK_K_ARITH}")"
    expected_masked="$(normalize_hex "${match_hex}")"
    result="OK"
    if [[ "${masked_word}" != "${expected_masked}" ]]; then
      result="MISMATCH"
      test_status=1
    fi

    echo "  ${mnemonic}: ${result}"
    echo "    objdump : ${first_hit}"
    echo "    masked  : ${masked_word}"
    echo "    expect  : ${expected_masked} (${match_symbol})"
  done

  if [[ "${test_status}" -ne 0 ]]; then
    overall_status=1
  fi
  echo
done

exit "${overall_status}"
