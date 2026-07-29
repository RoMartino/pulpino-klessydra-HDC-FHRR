#!/usr/bin/env bash
#
# build-toolchain.sh — Build the RISC-V GNU toolchain with the custom HDC/FHRR
# instruction opcodes required by this project.
#
# This script is fully self-contained and independent: it clones the *upstream*
# riscv-gnu-toolchain and its binutils/gcc/newlib submodules, overlays the two
# modified opcode files shipped in this directory (riscv-opc.c / riscv-opc.h),
# and builds a newlib cross-toolchain for the rv32ima / ilp32 target used by
# the Klessydra-Morph core.
#
# Usage:
#   ./build-toolchain.sh [INSTALL_PREFIX]
#
#   INSTALL_PREFIX   where the toolchain is installed
#                    (default: $HOME/riscv-gnu-toolchain-hdc-build)
#
# After it finishes, add the toolchain to your PATH:
#   export PATH="$INSTALL_PREFIX/bin:$PATH"
#
set -euo pipefail

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

INSTALL_PREFIX="${1:-$HOME/riscv-gnu-toolchain-hdc-build}"
WORK_DIR="${TOOLCHAIN_WORK_DIR:-$HOME/riscv-gnu-toolchain-hdc-src}"
JOBS="${JOBS:-$(nproc)}"

# Upstream sources (NOT klessydra — this repository is independent).
RISCV_GNU_URL="https://github.com/riscv-collab/riscv-gnu-toolchain.git"

# Submodule refs that provide binutils 2.39 (the version carrying our opcode
# edits) and the matching gcc 12.1 / newlib used to build this project.
BINUTILS_REF="binutils-2_39-branch"
GCC_REF="releases/gcc-12.2.0"
NEWLIB_REF="master"

TARGET_ARCH="rv32ima"
TARGET_ABI="ilp32"

echo "==> Install prefix : ${INSTALL_PREFIX}"
echo "==> Work directory : ${WORK_DIR}"
echo "==> Parallel jobs  : ${JOBS}"

# ---------------------------------------------------------------------------
# 1. Clone the upstream toolchain superproject
# ---------------------------------------------------------------------------
if [ ! -d "${WORK_DIR}/.git" ]; then
	echo "==> Cloning upstream riscv-gnu-toolchain ..."
	git clone "${RISCV_GNU_URL}" "${WORK_DIR}"
fi
cd "${WORK_DIR}"

# ---------------------------------------------------------------------------
# 2. Fetch the binutils / gcc / newlib submodules at the pinned refs
# ---------------------------------------------------------------------------
echo "==> Initialising submodules (binutils, gcc, newlib) ..."
git submodule update --init --depth 1 binutils gcc newlib

echo "==> Pinning binutils -> ${BINUTILS_REF}"
( cd binutils && git fetch --depth 1 origin "${BINUTILS_REF}" && git checkout FETCH_HEAD )

echo "==> Pinning gcc      -> ${GCC_REF}"
( cd gcc && git fetch --depth 1 origin "${GCC_REF}" && git checkout FETCH_HEAD )

echo "==> Pinning newlib   -> ${NEWLIB_REF}"
( cd newlib && git fetch --depth 1 origin "${NEWLIB_REF}" && git checkout FETCH_HEAD )

# ---------------------------------------------------------------------------
# 3. Overlay the custom HDC/FHRR opcode definitions
# ---------------------------------------------------------------------------
echo "==> Applying custom HDC/FHRR opcodes ..."
cp -v "${SCRIPT_DIR}/riscv-opc.c" binutils/opcodes/riscv-opc.c
cp -v "${SCRIPT_DIR}/riscv-opc.h" binutils/include/opcode/riscv-opc.h

# ---------------------------------------------------------------------------
# 4. Configure and build
# ---------------------------------------------------------------------------
echo "==> Configuring (${TARGET_ARCH} / ${TARGET_ABI}) ..."
./configure --prefix="${INSTALL_PREFIX}" \
            --with-arch="${TARGET_ARCH}" \
            --with-abi="${TARGET_ABI}"

echo "==> Building (make -j${JOBS}) ... this can take a while."
make -j"${JOBS}"

# ---------------------------------------------------------------------------
# 5. Create the klessydra-unknown-elf-* symlinks expected by the build system
# ---------------------------------------------------------------------------
if [ -f "${SCRIPT_DIR}/make_links.sh" ]; then
	echo "==> Creating klessydra-unknown-elf-* tool symlinks ..."
	cp "${SCRIPT_DIR}/make_links.sh" "${INSTALL_PREFIX}/bin/"
	( cd "${INSTALL_PREFIX}/bin" && chmod +x make_links.sh && ./make_links.sh ) || \
		echo "   (some symlinks may already exist — this is harmless)"
fi

echo
echo "==> Done. Toolchain installed in: ${INSTALL_PREFIX}"
echo "    Add it to your PATH:"
echo "        export PATH=\"${INSTALL_PREFIX}/bin:\$PATH\""
