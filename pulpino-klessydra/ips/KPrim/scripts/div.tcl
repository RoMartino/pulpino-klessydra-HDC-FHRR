#rm -rf prim_libs
mkdir -p prim_libs
vlib prim_libs/div_lib
vlog -sv tb/div_tb.sv -work prim_libs/div_lib
vcom -2008 modules/Dynamic\ Shifter/Dynamic_Shifter.vhd	-work prim_libs/div_lib
vcom -2008 modules/Divider/divider_LS.vhd				-work prim_libs/div_lib
vcom -2008 modules/Divider/divider_HF.vhd				-work prim_libs/div_lib
vcom -2008 modules/Divider/divider_HP.vhd				-work prim_libs/div_lib
vcom -2008 modules/Divider/divider_std.vhd				-work prim_libs/div_lib
vcom -2008 modules/Divider/divider.vhd					-work prim_libs/div_lib
vcom -2008 modules/Binary\ Counter/clz_decomposing.vhd	-work prim_libs/div_lib
vsim -voptargs=+acc prim_libs/div_lib.div_tb
add wave -r sim:/div_tb/*
