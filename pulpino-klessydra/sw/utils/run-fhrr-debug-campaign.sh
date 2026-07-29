#!/usr/bin/env bash

set -u
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SW_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

BUILD_DIR="${SW_DIR}/build_fhrr"
TIMEOUT_SEC=60
LOG_ROOT=""
RUN_SMOKE=1

BUNDLE_DEBUG_WAIT_ITERS=10000
BUNDLE_DEBUG_WAIT_STEP_NS=100
BUNDLE_DEBUG_ITERATIONS=200
BUNDLE_DEBUG_STEP_NS=100
BUNDLE_DEBUG_STALL_LIMIT=40

HDC_TESTS=(
  "fhrr_bind_test"
  "fhrr_bundle_test"
  "fhrr_clip_test"
  "fhrr_encode_test"
  "fhrr_similarity_test"
)

usage() {
  cat <<'EOF'
Usage: run-fhrr-debug-campaign.sh [options]

Options:
  --build-dir PATH              Build directory to use. Default: sw/build_fhrr
  --timeout SEC                Timeout for each functional simulation. Default: 60
  --log-dir PATH               Directory where campaign logs are stored.
  --skip-smoke                 Skip the non-FHRR helloworld smoke test.
  --bundle-wait-iters N        Bundle probe wait iterations. Default: 10000
  --bundle-wait-step-ns N      Bundle probe wait step in ns. Default: 100
  --bundle-iterations N        Bundle probe progress iterations. Default: 200
  --bundle-step-ns N           Bundle probe progress step in ns. Default: 100
  --bundle-stall-limit N       Bundle probe stall limit. Default: 40
  --help                       Show this help.

Examples:
  ./sw/utils/run-fhrr-debug-campaign.sh
  ./sw/utils/run-fhrr-debug-campaign.sh --timeout 90 --bundle-iterations 1000
EOF
}

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
    --log-dir)
      LOG_ROOT="$2"
      shift 2
      ;;
    --skip-smoke)
      RUN_SMOKE=0
      shift
      ;;
    --bundle-wait-iters)
      BUNDLE_DEBUG_WAIT_ITERS="$2"
      shift 2
      ;;
    --bundle-wait-step-ns)
      BUNDLE_DEBUG_WAIT_STEP_NS="$2"
      shift 2
      ;;
    --bundle-iterations)
      BUNDLE_DEBUG_ITERATIONS="$2"
      shift 2
      ;;
    --bundle-step-ns)
      BUNDLE_DEBUG_STEP_NS="$2"
      shift 2
      ;;
    --bundle-stall-limit)
      BUNDLE_DEBUG_STALL_LIMIT="$2"
      shift 2
      ;;
    --help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
  esac
done

if [[ -z "${LOG_ROOT}" ]]; then
  LOG_ROOT="${BUILD_DIR}/fhrr_debug_campaign"
fi

RUN_STAMP="$(date +%Y%m%d_%H%M%S)"
LOG_DIR="${LOG_ROOT}/run_${RUN_STAMP}"
mkdir -p "${LOG_DIR}"

cache_get() {
  local key="$1"
  local cache_file="${BUILD_DIR}/CMakeCache.txt"
  awk -F= -v key="${key}" '$1 ~ ("^" key ":") { print $2; exit }' "${cache_file}"
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

copy_dir_artifacts() {
  local source_dir="$1"
  local dest_dir="$2"

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

hdc_artifact_dir() {
  local test_name="$1"
  echo "${BUILD_DIR}/apps/klessydra_tests/klessydra_hdc_tests/${test_name}"
}

general_artifact_dir() {
  local test_name="$1"
  echo "${BUILD_DIR}/apps/${test_name}"
}

run_make_target() {
  local run_log="$1"
  shift
  (
    cd "${BUILD_DIR}" &&
    timeout --kill-after=10s "${TIMEOUT_SEC}s" "$@"
  ) >"${run_log}" 2>&1
  return $?
}

classify_hdc_result() {
  local test_name="$1"
  local command_status="$2"
  local run_log="$3"
  local source_dir
  local transcript
  local vsim_log
  local uart_log

  source_dir="$(hdc_artifact_dir "${test_name}")"
  transcript="${source_dir}/transcript"
  vsim_log="${source_dir}/vsim.log"
  uart_log="${source_dir}/stdout/uart"

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

classify_smoke_result() {
  local test_name="$1"
  local command_status="$2"
  local run_log="$3"
  local source_dir
  local transcript
  local vsim_log
  local uart_log

  source_dir="$(general_artifact_dir "${test_name}")"
  transcript="${source_dir}/transcript"
  vsim_log="${source_dir}/vsim.log"
  uart_log="${source_dir}/stdout/uart"

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

  if grep_any "Hello World!!!!!" "${uart_log}" "${transcript}" "${vsim_log}" "${run_log}"; then
    echo "PASS"
    return
  fi

  echo "FAIL"
}

classify_bundle_probe() {
  local run_log="$1"
  local source_dir
  local vsim_log

  source_dir="$(hdc_artifact_dir "fhrr_bundle_test")"
  vsim_log="${source_dir}/vsim.log"

  if grep_any "FHRR activity detected" "${vsim_log}" "${run_log}"; then
    echo "ACTIVITY"
    return
  fi

  if grep_any "FHRR activity not detected" "${vsim_log}" "${run_log}"; then
    echo "NO_ACTIVITY"
    return
  fi

  if grep_any "stage1_seen=1" "${vsim_log}" "${run_log}"; then
    echo "STAGE1_SEEN"
    return
  fi

  echo "UNKNOWN"
}

echo "FHRR debug campaign logs: ${LOG_DIR}"
echo "Build directory: ${BUILD_DIR}"
echo "Functional timeout: ${TIMEOUT_SEC}s"
echo

if [[ ! -f "${BUILD_DIR}/CMakeCache.txt" ]]; then
  echo "Missing CMakeCache.txt in ${BUILD_DIR}" >&2
  exit 1
fi

KLESS_ACCL_SEL="$(cache_get "KLESS_accl_sel")"
FHRR_DEBUG_MODE="$(cache_get "FHRR_TEST_DEBUG_MODE")"
FHRR_DIRECT_MODE="$(cache_get "FHRR_TEST_DIRECT_MODE")"
FHRR_VECTOR_ELEMENTS="$(cache_get "FHRR_TEST_VECTOR_ELEMENTS")"
FHRR_ENCODE_ROWS="$(cache_get "FHRR_TEST_ENCODE_ROWS")"
FHRR_FIXED_SEED="$(cache_get "FHRR_TEST_FIXED_SEED")"

if [[ "${KLESS_ACCL_SEL}" != "1" ]]; then
  echo "This campaign requires KLESS_accl_sel=1, found '${KLESS_ACCL_SEL}'." >&2
  exit 1
fi

{
  echo "KLESS_accl_sel=${KLESS_ACCL_SEL}"
  echo "FHRR_TEST_DEBUG_MODE=${FHRR_DEBUG_MODE}"
  echo "FHRR_TEST_DIRECT_MODE=${FHRR_DIRECT_MODE}"
  echo "FHRR_TEST_VECTOR_ELEMENTS=${FHRR_VECTOR_ELEMENTS}"
  echo "FHRR_TEST_ENCODE_ROWS=${FHRR_ENCODE_ROWS}"
  echo "FHRR_TEST_FIXED_SEED=${FHRR_FIXED_SEED}"
} | tee "${LOG_DIR}/campaign_config.txt"

BUILD_LOG="${LOG_DIR}/build_elfs.log"
echo
echo "[build] compiling deterministic FHRR ELFs"
(
  cd "${BUILD_DIR}" &&
  make -j1 \
    fhrr_bind_test.elf \
    fhrr_bundle_test.elf \
    fhrr_clip_test.elf \
    fhrr_encode_test.elf \
    fhrr_similarity_test.elf \
    helloworld.elf
) >"${BUILD_LOG}" 2>&1
BUILD_STATUS=$?
if [[ "${BUILD_STATUS}" -ne 0 ]]; then
  echo "[build] ERROR"
  echo "Build failed. See ${BUILD_LOG}" >&2
  exit 1
fi
echo "[build] PASS"

OPCODE_LOG="${LOG_DIR}/opcode_audit.log"
echo
echo "[opcode] auditing emitted FHRR instructions"
"${SW_DIR}/utils/fhrr-opcode-audit.sh" --build-dir "${BUILD_DIR}" >"${OPCODE_LOG}" 2>&1
OPCODE_STATUS=$?
if [[ "${OPCODE_STATUS}" -eq 0 ]]; then
  OPCODE_RESULT="PASS"
else
  OPCODE_RESULT="FAIL"
fi
echo "[opcode] ${OPCODE_RESULT}"

declare -A RESULTS

for test_name in "${HDC_TESTS[@]}"; do
  target="${test_name}.vsimc"
  run_log="${LOG_DIR}/${target}.log"
  echo
  echo "[${test_name}] running ${target}"
  run_make_target "${run_log}" make -j1 "${target}"
  status=$?
  copy_dir_artifacts "$(hdc_artifact_dir "${test_name}")" "${LOG_DIR}/${test_name}"
  RESULTS["${test_name}"]="$(classify_hdc_result "${test_name}" "${status}" "${run_log}")"
  echo "[${test_name}] ${RESULTS[${test_name}]}"
done

BUNDLE_PROBE_LOG="${LOG_DIR}/fhrr_bundle_test.vsimcdbg.log"
echo
echo "[fhrr_bundle_test] running focused bundle probe"
(
  cd "${BUILD_DIR}" &&
  timeout --kill-after=10s "${TIMEOUT_SEC}s" env \
    FHRR_DEBUG_WAIT_ITERS="${BUNDLE_DEBUG_WAIT_ITERS}" \
    FHRR_DEBUG_WAIT_STEP_NS="${BUNDLE_DEBUG_WAIT_STEP_NS}" \
    FHRR_DEBUG_ITERATIONS="${BUNDLE_DEBUG_ITERATIONS}" \
    FHRR_DEBUG_STEP_NS="${BUNDLE_DEBUG_STEP_NS}" \
    FHRR_DEBUG_STALL_LIMIT="${BUNDLE_DEBUG_STALL_LIMIT}" \
    make -j1 fhrr_bundle_test.vsimcdbg
) >"${BUNDLE_PROBE_LOG}" 2>&1
BUNDLE_PROBE_STATUS=$?
copy_dir_artifacts "$(hdc_artifact_dir "fhrr_bundle_test")" "${LOG_DIR}/fhrr_bundle_test_probe"
if [[ "${BUNDLE_PROBE_STATUS}" -eq 124 || "${BUNDLE_PROBE_STATUS}" -eq 137 ]]; then
  RESULTS["fhrr_bundle_probe"]="TIMEOUT"
else
  RESULTS["fhrr_bundle_probe"]="$(classify_bundle_probe "${BUNDLE_PROBE_LOG}")"
fi
echo "[fhrr_bundle_probe] ${RESULTS[fhrr_bundle_probe]}"

if [[ "${RUN_SMOKE}" -eq 1 ]]; then
  SMOKE_LOG="${LOG_DIR}/helloworld.vsimc.log"
  echo
  echo "[helloworld] running smoke test"
  run_make_target "${SMOKE_LOG}" make -j1 helloworld.vsimc
  SMOKE_STATUS=$?
  copy_dir_artifacts "$(general_artifact_dir "helloworld")" "${LOG_DIR}/helloworld"
  RESULTS["helloworld"]="$(classify_smoke_result "helloworld" "${SMOKE_STATUS}" "${SMOKE_LOG}")"
  echo "[helloworld] ${RESULTS[helloworld]}"
fi

SUMMARY_FILE="${LOG_DIR}/summary.txt"
{
  echo "FHRR Debug Campaign Summary"
  echo "==========================="
  echo "Build directory: ${BUILD_DIR}"
  echo "KLESS_accl_sel: ${KLESS_ACCL_SEL}"
  echo "FHRR_TEST_DEBUG_MODE: ${FHRR_DEBUG_MODE}"
  echo "FHRR_TEST_DIRECT_MODE: ${FHRR_DIRECT_MODE}"
  echo "FHRR_TEST_VECTOR_ELEMENTS: ${FHRR_VECTOR_ELEMENTS}"
  echo "FHRR_TEST_ENCODE_ROWS: ${FHRR_ENCODE_ROWS}"
  echo "FHRR_TEST_FIXED_SEED: ${FHRR_FIXED_SEED}"
  echo
  echo "opcode_audit: ${OPCODE_RESULT}"
  echo "fhrr_bind_test: ${RESULTS[fhrr_bind_test]:-NOT_RUN}"
  echo "fhrr_bundle_test: ${RESULTS[fhrr_bundle_test]:-NOT_RUN}"
  echo "fhrr_bundle_probe: ${RESULTS[fhrr_bundle_probe]:-NOT_RUN}"
  echo "fhrr_clip_test: ${RESULTS[fhrr_clip_test]:-NOT_RUN}"
  echo "fhrr_encode_test: ${RESULTS[fhrr_encode_test]:-NOT_RUN}"
  echo "fhrr_similarity_test: ${RESULTS[fhrr_similarity_test]:-NOT_RUN}"
  if [[ "${RUN_SMOKE}" -eq 1 ]]; then
    echo "helloworld: ${RESULTS[helloworld]:-NOT_RUN}"
  fi
  echo
  echo "Logs: ${LOG_DIR}"
} | tee "${SUMMARY_FILE}"

exit 0
