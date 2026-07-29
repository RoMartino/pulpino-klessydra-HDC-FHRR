<p align="center">
  <img src="pulpino-klessydra/pics/Klessydra_Logo.png" width="360">
</p>

# Klessydra FHRR — Open-Source HDC Hardware Accelerator on RISC-V

This repository is the **self-contained artifact** accompanying the paper:

> **A Configurable Open-Source FHRR Hardware Accelerator on a RISC-V Core
> for Embedded Hyperdimensional Computing** — APCCAS 2026.

It contains everything needed to build the custom RISC-V toolchain, simulate
the Klessydra-Morph RISC-V core with the **FHRR (Fourier Holographic Reduced
Representation) hyperdimensional-computing accelerator**, and reproduce the
software/hardware HDC experiments reported in the paper.

The repository is **fully independent**: it does not depend on any external
project checkout. The RISC-V toolchain is rebuilt from upstream sources with a
small set of custom-opcode patches shipped here.

---

## Repository layout

```
.
├── pulpino-klessydra/     Full PULPino + Klessydra SoC (RTL, testbench, sw)
│   └── ips/Morph/klessydra-m/
│       ├── RTL-FHRR_Unit.vhd          The FHRR-HDCU accelerator
│       ├── RTL-HDC_BSC_Unit.vhd       Binary spatter-code unit
│       ├── RTL-MCR_Unit.vhd           Multiply-clip-round unit
│       └── ...                        Morph core pipeline, VCU, SPMs, ...
│   └── sw/
│       ├── apps/klessydra_tests/klessydra_hdc_tests/   FHRR test suite
│       └── libs/klessydra_lib/hdc_libs/                C++ HDC library
├── toolchain-hdc/         Custom RISC-V toolchain build kit
│   ├── build-toolchain.sh  Clones upstream, applies HDC opcodes, builds
│   ├── riscv-opc.c         Modified binutils opcode table (HDC/FHRR insns)
│   ├── riscv-opc.h         Modified binutils opcode macros
│   └── make_links.sh       Creates the klessydra-unknown-elf-* symlinks
└── README.md
```

---

## Prerequisites

Tested on Ubuntu 20.04 / 22.04.

```bash
# Toolchain + simulation build dependencies
sudo apt update
sudo apt install -y git cmake tcsh autoconf automake autotools-dev curl \
    libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex \
    texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev

# The IP/build scripts still use Python 2.7
sudo apt install -y python2.7
curl https://bootstrap.pypa.io/pip/2.7/get-pip.py --output get-pip.py
sudo python2 get-pip.py
pip2 install pyyaml==5.4.1
sudo ln -sf "$(which python2.7)" /usr/local/bin/python
```

A **ModelSim/QuestaSim** installation is required to run the RTL simulations
(`make *.vsimc` targets).

---

## Step 1 — Build the custom RISC-V toolchain

The FHRR/HDC custom instructions require a `binutils` with the modified opcode
tables shipped in `toolchain-hdc/`. Build the toolchain once:

```bash
cd toolchain-hdc
./build-toolchain.sh                 # installs to $HOME/riscv-gnu-toolchain-hdc-build
```

Then add it to your `PATH` (append to `~/.bashrc` to make it permanent):

```bash
export PATH="$HOME/riscv-gnu-toolchain-hdc-build/bin:$PATH"
```

Verify the custom target is visible:

```bash
klessydra-unknown-elf-gcc --version
```

> The script clones the **upstream** `riscv-gnu-toolchain`, pins `binutils`
> (2.39), `gcc` (12.2.0) and `newlib`, overlays `riscv-opc.c` / `riscv-opc.h`,
> and builds an `rv32ima / ilp32` newlib cross-toolchain. Use
> `JOBS=<n>` and an optional install-prefix argument to customise the build.

---

## Step 2 — Build and simulate the SoC ("Hello World")

All IP cores are already vendored in this repository, so there is **no need to
fetch anything** — just generate the simulation scripts from the local sources:

```bash
cd pulpino-klessydra
./generate-scripts.py                # builds the vsim compile scripts from ips/

cd sw
mkdir -p build
cp cmake_configure.klessydra-m.gcc.sh build/
cd build
./cmake_configure.klessydra-m.gcc.sh
make vcompile                        # compile the Klessydra-Morph RTL
make helloworld_kless.vsimc          # build + run Hello World in simulation
```

The default Morph configuration instantiates the **DSP** accelerator
(`KLESS_accl_sel=0`).

---

## Step 3 — Run the FHRR / HDC experiments

The FHRR accelerator and its test suite are selected through the
`cmake_configure` variables (all overridable from the environment):

| Variable            | Meaning                                             | Default |
|---------------------|-----------------------------------------------------|---------|
| `KLESS_accl_sel`    | Accelerator in Morph: `0` = DSP, `1` = **FHRR**     | `0`     |
| `KLESS_SIMD`        | SIMD width (functional units / SPM banks, pow. of 2)| `2`     |
| `KLESS_Addr_Width`  | Scratchpad address width (SPM size = `2^Addr_Width`)| `14`    |

Configure a build with the FHRR accelerator enabled and run a test:

```bash
cd pulpino-klessydra/sw
mkdir -p build_fhrr
cp cmake_configure.klessydra-m.gcc.sh build_fhrr/
cd build_fhrr

# Select the FHRR accelerator and (optionally) the SIMD width
KLESS_accl_sel=1 KLESS_SIMD=8 ./cmake_configure.klessydra-m.gcc.sh
make vcompile

# Individual FHRR kernels (test name + ".vsimc" suffix):
make fhrr_bind_test.vsimc
make fhrr_bundle_test.vsimc
make fhrr_clip_test.vsimc
make fhrr_encode_test.vsimc
make fhrr_similarity_test.vsimc
make fhrr_permute_test.vsimc

# End-to-end UCIHAR classification inference:
make fhrr_ucihar_inference.vsimc
```

The full FHRR test sources live in
`sw/apps/klessydra_tests/klessydra_hdc_tests/`, and the C++ HDC library
(software and hardware back-ends, `hdc_fhrr_sw.cpp` / `hdc_fhrr_hw.cpp`) in
`sw/libs/klessydra_lib/hdc_libs/`.

### Batch regression / SIMD sweeps

Helper scripts to run the whole FHRR suite or sweep configurations are provided
in `sw/utils/`:

```bash
sw/utils/run-fhrr-regression.sh        # run the full FHRR test suite
sw/utils/run-fhrr-simd-hv-campaign.sh  # sweep SIMD widths / HV dimensionalities
```

---

## FPGA synthesis

The Morph core with the FHRR accelerator targets the Xilinx Zynq UltraScale+
**ZCU106** (`xczu7ev-ffvc1156-2-e`). The synthesizable RTL is entirely under
`pulpino-klessydra/ips/Morph/klessydra-m/`; use those VHDL sources to create a
standalone Vivado project for area/power/timing characterisation.

---

## Citation

If you use this work, please cite the accompanying paper:

```bibtex
@inproceedings{klessydra_fhrr_apccas2026,
  title     = {A Configurable Open-Source FHRR Hardware Accelerator on a
               RISC-V Core for Embedded Hyperdimensional Computing},
  booktitle = {IEEE Asia Pacific Conference on Circuits and Systems (APCCAS)},
  year      = {2026}
}
```

## License

The Klessydra and PULPino sources retain their original licenses (see
`pulpino-klessydra/LICENSE`). The custom opcode modifications in `toolchain-hdc/`
follow the license of upstream GNU binutils (GPL).
