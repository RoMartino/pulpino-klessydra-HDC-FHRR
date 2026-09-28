#!/usr/bin/env bash

set -u
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SW_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${SW_DIR}/.." && pwd)"
WORKSPACE_ROOT="$(cd "${REPO_ROOT}/.." && pwd)"

BUILD_ROOT="${SW_DIR}/build_fhrr_campaign"
LOG_ROOT=""
TRACKER_FILE=""
TIMEOUT_SEC=300
JOBS=1
FIXED_SEED=305419896
ENCODE_ROWS=2
DIRECT_MODE=OFF

SIMD_VALUES=(1 2 4 8 16 32)
HV_VALUES=(256 512 1024 2048 4096 8192)
TESTS=(
  "fhrr_bind_test"
  "fhrr_bundle_test"
  "fhrr_similarity_test"
  "fhrr_clip_test"
  "fhrr_encode_test"
  "fhrr_permute_test"
)

usage() {
  cat <<'EOF'
Usage: run-fhrr-simd-hv-campaign.sh [options]

Options:
  --build-root PATH     Root for per-configuration builds. Default: sw/build_fhrr_campaign
  --log-root PATH       Root for campaign logs. Default: BUILD_ROOT/logs
  --tracker PATH        Optional log file to which the summary is also appended.
  --timeout SEC         Timeout for each simulation. Default: 300
  --jobs N              Make parallelism for ELF builds. Default: 1
  --fixed-seed N        FHRR_TEST_FIXED_SEED. Default: 305419896
  --direct-mode ON|OFF  FHRR_TEST_DIRECT_MODE. Default: OFF
  --encode-rows N       FHRR_TEST_ENCODE_ROWS. Default: 2
  --simd LIST           Comma-separated SIMD values. Default: 1,2,4,8,16,32
  --hv LIST             Comma-separated HV element values. Default: 256,512,1024,2048,4096,8192
  --help                Show this help.
EOF
}

split_csv() {
  local csv="$1"
  local -n out_ref="$2"
  IFS=',' read -r -a out_ref <<<"${csv}"
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-root)
      BUILD_ROOT="$2"
      shift 2
      ;;
    --log-root)
      LOG_ROOT="$2"
      shift 2
      ;;
    --tracker)
      TRACKER_FILE="$2"
      shift 2
      ;;
    --timeout)
      TIMEOUT_SEC="$2"
      shift 2
      ;;
    --jobs)
      JOBS="$2"
      shift 2
      ;;
    --fixed-seed)
      FIXED_SEED="$2"
      shift 2
      ;;
    --direct-mode)
      DIRECT_MODE="$2"
      shift 2
      ;;
    --encode-rows)
      ENCODE_ROWS="$2"
      shift 2
      ;;
    --simd)
      split_csv "$2" SIMD_VALUES
      shift 2
      ;;
    --hv)
      split_csv "$2" HV_VALUES
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
  LOG_ROOT="${BUILD_ROOT}/logs"
fi

RUN_STAMP="$(date +%Y%m%d_%H%M%S)"
CAMPAIGN_LOG_DIR="${LOG_ROOT}/run_${RUN_STAMP}"
SUMMARY_FILE="${CAMPAIGN_LOG_DIR}/summary.md"
mkdir -p "${CAMPAIGN_LOG_DIR}"

declare -A OP_BY_TEST=(
  ["fhrr_bind_test"]="bind"
  ["fhrr_bundle_test"]="bundle"
  ["fhrr_similarity_test"]="similarity"
  ["fhrr_clip_test"]="clip"
  ["fhrr_encode_test"]="encode"
  ["fhrr_permute_test"]="permute"
)

declare -a SUMMARY_ROWS=()
RTL_COMPILE_LOG=""

git_field() {
  local field="$1"
  if git -C "${REPO_ROOT}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    case "${field}" in
      commit)
        git -C "${REPO_ROOT}" rev-parse --short HEAD 2>/dev/null || true
        ;;
      status)
        if git -C "${REPO_ROOT}" diff --quiet --ignore-submodules -- && \
           git -C "${REPO_ROOT}" diff --cached --quiet --ignore-submodules -- && \
           [[ -z "$(git -C "${REPO_ROOT}" ls-files --others --exclude-standard)" ]]; then
          echo "clean"
        else
          echo "dirty"
        fi
        ;;
    esac
  else
    echo "n/a"
  fi
}

artifact_dir() {
  local build_dir="$1"
  local test_name="$2"
  echo "${build_dir}/apps/klessydra_tests/klessydra_hdc_tests/${test_name}"
}

result_sources() {
  local build_dir="$1"
  local test_name="$2"
  local run_log="$3"
  local source_dir
  source_dir="$(artifact_dir "${build_dir}" "${test_name}")"
  printf '%s\n' \
    "${run_log}" \
    "${source_dir}/stdout/uart" \
    "${source_dir}/vsim.log" \
    "${source_dir}/transcript"
}

grep_sources() {
  local pattern="$1"
  shift
  local file
  for file in "$@"; do
    if [[ -f "${file}" ]] && grep -q "${pattern}" "${file}"; then
      return 0
    fi
  done
  return 1
}

classify_test() {
  local command_status="$1"
  local test_name="$2"
  shift 2
  local sources=("$@")

  if [[ "${command_status}" -eq 124 || "${command_status}" -eq 137 ]]; then
    echo "TIMEOUT"
    return
  fi
  if [[ "${command_status}" -ne 0 ]]; then
    echo "ERROR"
    return
  fi
  if grep_sources "\\[${test_name}\\] PASS" "${sources[@]}" || \
     grep_sources "\\[SPI\\] Test OK" "${sources[@]}"; then
    echo "PASS"
    return
  fi
  if grep_sources "\\[${test_name}\\] FAIL" "${sources[@]}" || \
     grep_sources "\\[SPI\\] Test FAILED" "${sources[@]}"; then
    echo "FAIL"
    return
  fi
  echo "UNKNOWN"
}

extract_result_line() {
  local test_name="$1"
  shift
  local op="${OP_BY_TEST[${test_name}]}"
  local file
  for file in "$@"; do
    if [[ -f "${file}" ]]; then
      grep "FHRR_RESULT op=${op} " "${file}" | tail -n 1 && return 0
    fi
  done
  return 1
}

field_value() {
  local line="$1"
  local key="$2"
  awk -v key="${key}" '{
    for (i = 1; i <= NF; ++i) {
      split($i, kv, "=")
      if (kv[1] == key) {
        print kv[2]
        exit
      }
    }
  }' <<<"${line}"
}

add_row() {
  local simd="$1"
  local hv="$2"
  local test_name="$3"
  local status="$4"
  local result_line="$5"
  local log_path="$6"
  local op sw_cycles hw_cycles hw_accel_cycles speedup fu_speedup

  op="${OP_BY_TEST[${test_name}]}"
  sw_cycles="-"
  hw_cycles="-"
  hw_accel_cycles="-"
  speedup="-"
  fu_speedup="-"

  if [[ -n "${result_line}" ]]; then
    op="$(field_value "${result_line}" "op")"
    sw_cycles="$(field_value "${result_line}" "sw_cycles")"
    hw_cycles="$(field_value "${result_line}" "hw_cycles")"
    hw_accel_cycles="$(field_value "${result_line}" "hw_accel_cycles")"
    speedup="$(field_value "${result_line}" "speedup")"
    fu_speedup="$(field_value "${result_line}" "fu_speedup")"
  fi

  SUMMARY_ROWS+=("| ${simd} | ${hv} | ${test_name} | ${op} | ${status} | ${sw_cycles:-"-"} | ${hw_cycles:-"-"} | ${hw_accel_cycles:-"-"} | ${speedup:-"-"} | ${fu_speedup:-"-"} | ${log_path} |")
}

configure_build() {
  local build_dir="$1"
  local simd="$2"
  local hv="$3"
  local log_file="$4"

  mkdir -p "${build_dir}"
  (
    cd "${build_dir}" &&
    env \
      KLESS_accl_sel=1 \
      KLESS_SIMD="${simd}" \
      KLESS_Addr_Width=16 \
      FHRR_TEST_VECTOR_ELEMENTS="${hv}" \
      FHRR_TEST_FIXED_SEED="${FIXED_SEED}" \
      FHRR_TEST_DIRECT_MODE="${DIRECT_MODE}" \
      FHRR_TEST_ENCODE_ROWS="${ENCODE_ROWS}" \
      "${SW_DIR}/cmake_configure.klessydra-m.gcc.sh"
  ) >"${log_file}" 2>&1
}

build_elfs() {
  local build_dir="$1"
  local log_file="$2"
  (
    cd "${build_dir}" &&
    make -j"${JOBS}" \
      fhrr_bind_test.elf \
      fhrr_bundle_test.elf \
      fhrr_similarity_test.elf \
      fhrr_clip_test.elf \
      fhrr_encode_test.elf \
      fhrr_permute_test.elf
  ) >"${log_file}" 2>&1
}

compile_morph_rtl() {
  local log_file="$1"
  (
    cd "${SW_DIR}" &&
    make -C "${BUILD_ROOT}/rtl_compile" vcompile_morph
  ) >"${log_file}" 2>&1
}

run_test() {
  local build_dir="$1"
  local test_name="$2"
  local log_file="$3"
  (
    cd "${build_dir}" &&
    timeout --kill-after=10s "${TIMEOUT_SEC}s" make -j1 "${test_name}.vsimc"
  ) >"${log_file}" 2>&1
}

echo "FHRR SIMD/HV campaign logs: ${CAMPAIGN_LOG_DIR}"
echo "SIMD values: ${SIMD_VALUES[*]}"
echo "HV elements: ${HV_VALUES[*]}"

RTL_COMPILE_DIR="${BUILD_ROOT}/rtl_compile"
RTL_COMPILE_LOG="${CAMPAIGN_LOG_DIR}/vcompile_morph.log"
mkdir -p "${RTL_COMPILE_DIR}"
echo
echo "[rtl] configure Morph RTL compile build"
(
  cd "${RTL_COMPILE_DIR}" &&
  env KLESS_accl_sel=1 KLESS_Addr_Width=16 "${SW_DIR}/cmake_configure.klessydra-m.gcc.sh"
) >"${CAMPAIGN_LOG_DIR}/vcompile_morph_configure.log" 2>&1
rtl_config_status=$?
if [[ "${rtl_config_status}" -ne 0 ]]; then
  echo "[rtl] configure ERROR"
  for simd in "${SIMD_VALUES[@]}"; do
    for hv in "${HV_VALUES[@]}"; do
      for test_name in "${TESTS[@]}"; do
        add_row "${simd}" "${hv}" "${test_name}" "ERROR" "" "${CAMPAIGN_LOG_DIR}/vcompile_morph_configure.log"
      done
    done
  done
else
  echo "[rtl] make vcompile_morph"
  compile_morph_rtl "${RTL_COMPILE_LOG}"
  rtl_compile_status=$?
  if [[ "${rtl_compile_status}" -ne 0 ]]; then
    echo "[rtl] vcompile_morph ERROR"
    for simd in "${SIMD_VALUES[@]}"; do
      for hv in "${HV_VALUES[@]}"; do
        for test_name in "${TESTS[@]}"; do
          add_row "${simd}" "${hv}" "${test_name}" "ERROR" "" "${RTL_COMPILE_LOG}"
        done
      done
    done
  fi
fi

if [[ "${#SUMMARY_ROWS[@]}" -eq 0 ]]; then

  for simd in "${SIMD_VALUES[@]}"; do
    for hv in "${HV_VALUES[@]}"; do
    build_dir="${BUILD_ROOT}/simd_${simd}/hv_${hv}"
    config_log="${CAMPAIGN_LOG_DIR}/simd_${simd}_hv_${hv}_configure.log"
    build_log="${CAMPAIGN_LOG_DIR}/simd_${simd}_hv_${hv}_build.log"

    echo
    echo "[simd=${simd} hv=${hv}] configure"
    configure_build "${build_dir}" "${simd}" "${hv}" "${config_log}"
    config_status=$?
    if [[ "${config_status}" -ne 0 ]]; then
      echo "[simd=${simd} hv=${hv}] configure ERROR"
      for test_name in "${TESTS[@]}"; do
        add_row "${simd}" "${hv}" "${test_name}" "ERROR" "" "${config_log}"
      done
      continue
    fi

    echo "[simd=${simd} hv=${hv}] build"
    build_elfs "${build_dir}" "${build_log}"
    build_status=$?
    if [[ "${build_status}" -ne 0 ]]; then
      echo "[simd=${simd} hv=${hv}] build ERROR"
      for test_name in "${TESTS[@]}"; do
        add_row "${simd}" "${hv}" "${test_name}" "ERROR" "" "${build_log}"
      done
      continue
    fi

    for test_name in "${TESTS[@]}"; do
      run_log="${CAMPAIGN_LOG_DIR}/simd_${simd}_hv_${hv}_${test_name}.vsimc.log"
      echo "[simd=${simd} hv=${hv}] ${test_name}.vsimc"
      run_test "${build_dir}" "${test_name}" "${run_log}"
      run_status=$?
      mapfile -t sources < <(result_sources "${build_dir}" "${test_name}" "${run_log}")
      status="$(classify_test "${run_status}" "${test_name}" "${sources[@]}")"
      result_line="$(extract_result_line "${test_name}" "${sources[@]}" || true)"
      add_row "${simd}" "${hv}" "${test_name}" "${status}" "${result_line}" "${run_log}"
      echo "[simd=${simd} hv=${hv}] ${test_name}: ${status}"
    done
    done
  done
fi

commit="$(git_field commit)"
repo_status="$(git_field status)"

{
  echo
  echo "[${RUN_STAMP}] - FHRR SIMD/HV campaign"
  echo
  echo "  Configuration"
  echo "    - SIMD: ${SIMD_VALUES[*]}"
  echo "    - HV elements: ${HV_VALUES[*]}"
  echo "    - KLESS_accl_sel=1, KLESS_Addr_Width=16"
  echo "    - FHRR_TEST_FIXED_SEED=${FIXED_SEED}, FHRR_TEST_DIRECT_MODE=${DIRECT_MODE}, FHRR_TEST_ENCODE_ROWS=${ENCODE_ROWS}"
  echo "    - Repo commit: ${commit}, workspace: ${repo_status}"
  echo "    - Log root: ${CAMPAIGN_LOG_DIR}"
  echo "    - RTL compile log: ${RTL_COMPILE_LOG:-n/a}"
  echo
  echo "  Results"
  echo
  echo "| SIMD | HV elements | Test | Op | Status | SW cycles | HW cycles | HW accel cycles | Speedup SW/HW | FU speedup | Log |"
  echo "|---:|---:|---|---|---|---:|---:|---:|---:|---:|---|"
  printf '%s\n' "${SUMMARY_ROWS[@]}"
} >"${SUMMARY_FILE}"

if [[ -n "${TRACKER_FILE}" ]]; then
  cat "${SUMMARY_FILE}" >>"${TRACKER_FILE}"
  echo "Summary also appended to ${TRACKER_FILE}"
fi

echo
echo "Summary written to ${SUMMARY_FILE}"
