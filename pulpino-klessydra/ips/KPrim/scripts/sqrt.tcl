#rm -rf prim_libs
mkdir -p prim_libs
vlib prim_libs/sqrt_lib
vlog -sv tb/sqrt_tb.sv									-work prim_libs/sqrt_lib
vcom -2008 modules/Dynamic\ Shifter/Dynamic_Shifter.vhd	-work prim_libs/sqrt_lib
vcom -2008 modules/Divider/divider_LS.vhd				-work prim_libs/sqrt_lib
vcom -2008 modules/Divider/divider_HF.vhd				-work prim_libs/sqrt_lib
vcom -2008 modules/Divider/divider_HP.vhd				-work prim_libs/sqrt_lib
vcom -2008 modules/Divider/divider_std.vhd				-work prim_libs/sqrt_lib
vcom -2008 modules/Divider/divider.vhd					-work prim_libs/sqrt_lib
vcom -2008 modules/Binary\ Counter/clz_decomposing.vhd	-work prim_libs/sqrt_lib
vcom -2008 modules/Square\ Root/sqrt_NR.vhd				-work prim_libs/sqrt_lib
vcom -2008 modules/Square\ Root/sqrt.vhd				-work prim_libs/sqrt_lib
vsim -voptargs=+acc prim_libs/sqrt_lib.sqrt_tb
add wave -r sim:/sqrt_tb/*
