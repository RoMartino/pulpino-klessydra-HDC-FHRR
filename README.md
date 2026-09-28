<p align="center">
  <img src="pulpino-klessydra/pics/Klessydra_Logo.png" width="360">
</p>

# Klessydra FHRR — Open-Source HDC Hardware Accelerator on RISC-V

Artifact of the paper:

> **A Configurable Open-Source FHRR Hardware Accelerator on a RISC-V Core
> for Embedded Hyperdimensional Computing** — APCCAS 2026.

The repository contains the RTL of the Klessydra-Morph RISC-V core with the
**FHRR-HDCU** accelerator (Fourier Holographic Reduced Representation), the
C++ HDC library, the test suite, and the kit to build the RISC-V toolchain
with the custom HDC instructions. It is self-contained: nothing else needs to
be downloaded except the upstream toolchain sources, which the build script
fetches for you.

The accelerator implements six functional units, each with a software
reference and a hardware back-end that are checked against each other
(bit-exact) in RTL simulation:

| Operation   | Instruction | Test app               |
|-------------|-------------|------------------------|
| Encoding    | `hvenc`     | `fhrr_encode_test`     |
| Binding     | `hvbind`    | `fhrr_bind_test`       |
| Bundling    | `hvbundle`  | `fhrr_bundle_test`     |
| Similarity  | `hvsim`     | `fhrr_similarity_test` |
| Clipping    | `hvclip`    | `fhrr_clip_test`       |
| Permutation | `hvperm`    | `fhrr_permute_test`    |

---

## Repository layout

```
.
├── pulpino-klessydra/                PULPino SoC + Klessydra-Morph core
│   ├── ips/Morph/klessydra-m/        Core + accelerator RTL (VHDL)
│   │   └── RTL-FHRR_Unit.vhd         The FHRR-HDCU accelerator
│   └── sw/
│       ├── cmake_configure.klessydra-m.gcc.sh   Build configuration script
│       ├── apps/klessydra_tests/klessydra_hdc_tests/   FHRR test apps
│       ├── libs/klessydra_lib/hdc_libs/                C++ HDC library (SW + HW back-ends)
│       └── utils/                    Regression / sweep scripts
├── toolchain-hdc/                    RISC-V toolchain build kit (custom HDC opcodes)
└── README.md
```

---

## Prerequisites

Tested on Ubuntu 20.04 / 22.04.

* **ModelSim / QuestaSim** (tested with ModelSim SE-64 2020.4), with `vsim`
  in your `PATH`.
* Build dependencies:

```bash
sudo apt update
sudo apt install -y git cmake tcsh autoconf automake autotools-dev curl \
    libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex \
    texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev

# The simulation-script generator still uses Python 2.7 + PyYAML
sudo apt install -y python2.7
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py
sudo python2 get-pip.py
pip2 install pyyaml==5.4.1
sudo ln -sf "$(which python2.7)" /usr/local/bin/python
```

---

## Step 1 — Build the RISC-V toolchain (once)

```bash
cd toolchain-hdc
./build-toolchain.sh          # installs to $HOME/riscv-gnu-toolchain-hdc-build
export PATH="$HOME/riscv-gnu-toolchain-hdc-build/bin:$PATH"   # add to ~/.bashrc
klessydra-unknown-elf-gcc --version                          # check
```

The script clones the upstream `riscv-gnu-toolchain` (binutils 2.39, gcc
12.2.0, newlib), applies the HDC opcode tables in `toolchain-hdc/` and builds
an `rv32ima/ilp32` cross-compiler. Use `JOBS=<n>` to set the build
parallelism; the first argument optionally overrides the install prefix.

---

## Step 2 — Generate the simulation scripts (once)

```bash
cd pulpino-klessydra
./generate-scripts.py
```

---

## Step 3 — Run the FHRR tests

Create a build directory **inside `pulpino-klessydra/sw/`** and run the
configuration script from there (always call it as `../cmake_configure...`,
do not copy it into the build directory):

```bash
cd pulpino-klessydra/sw
mkdir build_fhrr && cd build_fhrr

KLESS_accl_sel=1 KLESS_SIMD=8 KLESS_Addr_Width=16 FHRR_TEST_VECTOR_ELEMENTS=256 \
    ../cmake_configure.klessydra-m.gcc.sh
make vcompile                      # compile the RTL for ModelSim

make fhrr_encode_test.vsimc
make fhrr_bind_test.vsimc
make fhrr_bundle_test.vsimc
make fhrr_similarity_test.vsimc
make fhrr_clip_test.vsimc
make fhrr_permute_test.vsimc
```

Each test runs the operation with the software back-end and with the
hardware accelerator, compares the two results bit by bit and prints, e.g.:

```
[fhrr_permute] shift=   0 PASS  (sw_cy=4681 hw_cy=938)
FHRR_RESULT op=permute simd=8 hv_elements=256 status=PASS sw_cycles=4681 hw_cycles=938 hw_accel_cycles=34 speedup=4.990 fu_speedup=137.676
...
[fhrr_permute_test] PASS
```

* `sw_cycles` / `hw_cycles`: cycles of the whole software / hardware library
  call (the hardware call includes moving the vectors to and from the
  accelerator scratchpads with `kmemld`/`kmemstr`);
* `hw_accel_cycles`: cycles of the FHRR instruction, from the accelerator's
  per-FU counter. The hardware library issues **one instruction over the whole
  hypervector** and dispatches it only after the scratchpad transfers have
  completed, so this is the execution time of the operation on operands already
  resident in the scratchpads;
* `speedup` = `sw_cycles / hw_cycles` (end-to-end library call);
* `fu_speedup` = `sw_cycles / hw_accel_cycles` (functional unit only).

Measured FU latency (cycles, *D* elements, *P* lanes, *F* encoded features):

| Operation   | Cycles                                   |
|-------------|------------------------------------------|
| Encoding    | F·D/P + 4                                |
| Binding     | D/P + 2                                  |
| Bundling    | 2·D/P + 3                                |
| Similarity  | D/P + log2(P) + 4 (+1 for P ≥ 4)         |
| Clipping    | 52·D/P + 6 (radix-2 divider)             |
| Permutation | D/P + 2 (any shift)                      |

### Configuration

All variables are read from the environment by `cmake_configure`. After
changing any of them, re-run `../cmake_configure.klessydra-m.gcc.sh` in the
build directory (use one build directory per configuration).

| Variable                    | Meaning                                                  | Default |
|-----------------------------|----------------------------------------------------------|---------|
| `KLESS_accl_sel`            | Accelerator: `0` = DSP, `1` = **FHRR** (needed for the tests above) | `0` |
| `KLESS_SIMD`                | Parallelism *P* (lanes per cycle): 1, 2, 4, 8, 16, 32    | `2`     |
| `KLESS_Addr_Width`          | Scratchpad size = `2^Addr_Width` bytes                   | `14`    |
| `FHRR_TEST_VECTOR_ELEMENTS` | Hypervector size *D* used by the tests                   | `16`    |
| `FHRR_TEST_ENCODE_ROWS`     | Number of features encoded by `fhrr_encode_test` (≤ 31)  | `2`     |
| `FHRR_TEST_FIXED_SEED`      | Fixed seed for the pseudo-random test vectors            | from cycle counter |
| `FHRR_TEST_PERMUTE_CASE_BEGIN` / `_END` | Run only shift cases [BEGIN, END) of the 12 in `fhrr_permute_test` (split very long runs) | all |
| `KLESS_tracer_en`           | Instruction trace files (`execution_*.txt`); set `0` for long runs (*D* ≥ 4096), it does not change the cycle counts | `1` |

Example: *P* = 32, *D* = 1024:

```bash
cd pulpino-klessydra/sw
mkdir build_p32_d1024 && cd build_p32_d1024
KLESS_accl_sel=1 KLESS_SIMD=32 KLESS_Addr_Width=16 FHRR_TEST_VECTOR_ELEMENTS=1024 \
    ../cmake_configure.klessydra-m.gcc.sh
make vcompile
make fhrr_permute_test.vsimc
```

Constraints: *D* must be a multiple of *P* (and *D* ≥ *P*; Similarity also
needs *D* to be a power of two). Each scratchpad must hold the largest operand:
one hypervector is `4·D` bytes, the bundling accumulator `8·D` bytes and the
encoding item matrix `4·F·D` bytes. `KLESS_Addr_Width=16` covers *D* ≤ 4096
with *F* = 2; use `KLESS_Addr_Width=17` for *D* = 8192 (all six tests were
validated bit-exact for *P* = 1…32 and *D* = 64…8192 with `Addr_Width=17`).

### Regression and sweeps

```bash
cd pulpino-klessydra/sw
# all six FHRR tests on an already configured build directory
utils/run-fhrr-regression.sh --build-dir build_fhrr

# sweep of parallelism P and hypervector size D (one build per point)
utils/run-fhrr-simd-hv-campaign.sh --simd 1,8,32 --hv 256,512,1024 --fixed-seed 305419896
```

Logs and a markdown summary table are written under the build directories.

---

## FPGA synthesis

The design targets the Xilinx Zynq UltraScale+ **ZCU106**
(`xczu7ev-ffvc1156-2-e`). All synthesizable sources are in
`pulpino-klessydra/ips/Morph/klessydra-m/`; create a standalone Vivado project
from those VHDL files (top: `klessydra_top`) for area / power / timing.

---

## Citation

```bibtex
@inproceedings{klessydra_fhrr_apccas2026,
  title     = {A Configurable Open-Source FHRR Hardware Accelerator on a
               RISC-V Core for Embedded Hyperdimensional Computing},
  booktitle = {IEEE Asia Pacific Conference on Circuits and Systems (APCCAS)},
  year      = {2026}
}
```

## License

The Klessydra and PULPino sources keep their original licenses (see
`pulpino-klessydra/LICENSE`). The opcode modifications in `toolchain-hdc/`
follow the license of upstream GNU binutils (GPL).
