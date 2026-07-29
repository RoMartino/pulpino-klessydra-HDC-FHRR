#rm -rf prim_libs
mkdir -p prim_libs
vlib prim_libs/clz_lib
vcom  tb/clz_tb.vhd 									-work prim_libs/clz_lib
vcom -2008 modules/Binary\ Counter/clz_for.vhd         	-work prim_libs/clz_lib
vcom -2008 modules/Binary\ Counter/clz_decomposing.vhd 	-work prim_libs/clz_lib
vsim -voptargs=+acc prim_libs/clz_lib.clz_tb
add wave -r sim:/clz_tb/*
