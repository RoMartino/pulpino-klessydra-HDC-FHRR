#!/usr/bin/env bash

set -u
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SW_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${SW_DIR}/build_fhrr"
TIMEOUT_SEC=60
MODE="vsimc"
LOG_ROOT=""

DEFAULT_TESTS=(
  "fhrr_bind_test"
  "fhrr_bundle_test"
  "fhrr_clip_test"
  "fhrr_encode_test"
  "fhrr_similarity_test"
)

print_usage() {
  cat <<'EOF'
Usage: run-fhrr-regression.sh [options] [test_name ...]

Options:
  --build-dir PATH   Build directory to use. Default: sw/build_fhrr
  --timeout SEC      Timeout for each test. Default: 60
  --mode SUFFIX      Make target suffix. Default: vsimc
  --log-dir PATH     Directory where regression logs are stored.
  --help             Show this help message.

Examples:
  ./sw/utils/run-fhrr-regression.sh
  ./sw/utils/run-fhrr-regression.sh --timeout 20 fhrr_bind_test fhrr_bundle_test
EOF
}

POSITIONAL_TESTS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --timeout)
      TIMEOUT_SEC="$2"
      shift 2
      ;;
    --mode)
      MODE="$2"
      shift 2
      ;;
    --log-dir)
      LOG_ROOT="$2"
      shift 2
      ;;
    --help)
      print_usage
      exit 0
      ;;
    *)
      POSITIONAL_TESTS+=("$1")
      shift
      ;;
  esac
done

if [[ ${#POSITIONAL_TESTS[@]} -eq 0 ]]; then
  TESTS=("${DEFAULT_TESTS[@]}")
else
  TESTS=("${POSITIONAL_TESTS[@]}")
fi

if [[ -z "${LOG_ROOT}" ]]; then
  LOG_ROOT="${BUILD_DIR}/fhrr_regression_logs"
fi

RUN_STAMP="$(date +%Y%m%d_%H%M%S)"
LOG_DIR="${LOG_ROOT}/run_${RUN_STAMP}"
mkdir -p "${LOG_DIR}"

declare -A RESULTS
declare -A TEST_LOGS

artifact_path() {
  local test_name="$1"
  echo "${BUILD_DIR}/apps/klessydra_tests/klessydra_hdc_tests/${test_name}"
}

copy_artifacts() {
  local test_name="$1"
  local source_dir
  local dest_dir

  source_dir="$(artifact_path "${test_name}")"
  dest_dir="${LOG_DIR}/${test_name}"
  mkdir -p "${dest_dir}"

  for file_name in transcript vsim.log execution_0.txt execution_1.txt execution_2.txt; do
    if [[ -f "${source_dir}/${file_name}" ]]; then
      cp "${source_dir}/${file_name}" "${dest_dir}/"
    fi
  done

  if [[ -d "${source_dir}/stdout" ]]; then
    mkdir -p "${dest_dir}/stdout"
    cp -r "${source_dir}/stdout/." "${dest_dir}/stdout/"
  fi
}

grep_any() {
  local pattern="$1"
  shift
  local candidate
  for candidate in "$@"; do
    if [[ -f "${candidate}" ]] && grep -q "${pattern}" "${candidate}"; then
      return 0
    fi
  done
  return 1
}

classify_result() {
  local test_name="$1"
  local command_status="$2"
  local source_dir
  local transcript
  local vsim_log
  local uart_log
  local run_log

  source_dir="$(artifact_path "${test_name}")"
  transcript="${source_dir}/transcript"
  vsim_log="${source_dir}/vsim.log"
  uart_log="${source_dir}/stdout/uart"
  run_log="${TEST_LOGS[${test_name}]}"

  if [[ "${command_status}" -eq 124 || "${command_status}" -eq 137 ]]; then
    echo "TIMEOUT"
    return
  fi

  if [[ "${command_status}" -ne 0 ]]; then
    echo "ERROR"
    return
  fi

  if grep_any "\\[SPI\\] Test OK" "${transcript}" "${vsim_log}" "${run_log}"; then
    echo "PASS"
    return
  fi

  if grep_any "\\[SPI\\] Test FAILED" "${transcript}" "${vsim_log}" "${run_log}"; then
    echo "FAIL"
    return
  fi

  if grep_any "\\[${test_name}\\] PASS" "${uart_log}" "${transcript}" "${vsim_log}" "${run_log}"; then
    echo "PASS"
    return
  fi

  if grep_any "\\[${test_name}\\] FAIL" "${uart_log}" "${transcript}" "${vsim_log}" "${run_log}"; then
    echo "FAIL"
    return
  fi

  echo "UNKNOWN"
}

printf "FHRR regression logs: %s\n" "${LOG_DIR}"
printf "Build directory: %s\n" "${BUILD_DIR}"
printf "Per-test timeout: %ss\n" "${TIMEOUT_SEC}"
printf "Mode: %s\n\n" "${MODE}"

overall_status=0

for test_name in "${TESTS[@]}"; do
  target="${test_name}.${MODE}"
  run_log="${LOG_DIR}/${test_name}.${MODE}.log"
  TEST_LOGS["${test_name}"]="${run_log}"

  printf "[%s] running %s with timeout %ss\n" "${test_name}" "${target}" "${TIMEOUT_SEC}"

  (
    cd "${BUILD_DIR}" &&
    timeout --kill-after=10s "${TIMEOUT_SEC}s" make "${target}"
  ) >"${run_log}" 2>&1
  command_status=$?

  copy_artifacts "${test_name}"

  result="$(classify_result "${test_name}" "${command_status}")"
  RESULTS["${test_name}"]="${result}"

  printf "[%s] %s\n\n" "${test_name}" "${result}"

  if [[ "${result}" != "PASS" ]]; then
    overall_status=1
  fi
done

printf "Summary\n"
printf "%s\n" "-------"
for test_name in "${TESTS[@]}"; do
  printf "%-24s %s\n" "${test_name}" "${RESULTS[${test_name}]}"
done

printf "\nSaved logs under %s\n" "${LOG_DIR}"

exit "${overall_status}"
