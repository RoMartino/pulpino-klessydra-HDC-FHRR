
░█████╗░░█████╗░███╗░░██╗████████╗███████╗██╗░░██╗████████╗██╗░░░██╗░█████╗░██╗░░░░░
██╔══██╗██╔══██╗████╗░██║╚══██╔══╝██╔════╝╚██╗██╔╝╚══██╔══╝██║░░░██║██╔══██╗██║░░░░░
██║░░╚═╝██║░░██║██╔██╗██║░░░██║░░░█████╗░░░╚███╔╝░░░░██║░░░██║░░░██║███████║██║░░░░░
██║░░██╗██║░░██║██║╚████║░░░██║░░░██╔══╝░░░██╔██╗░░░░██║░░░██║░░░██║██╔══██║██║░░░░░
╚█████╔╝╚█████╔╝██║░╚███║░░░██║░░░███████╗██╔╝╚██╗░░░██║░░░╚██████╔╝██║░░██║███████╗
░╚════╝░░╚════╝░╚═╝░░╚══╝░░░╚═╝░░░╚══════╝╚═╝░░╚═╝░░░╚═╝░░░░╚═════╝░╚═╝░░╚═╝╚══════╝

██████╗░░█████╗░███╗░░██╗██████╗░██╗████████╗░██████╗
██╔══██╗██╔══██╗████╗░██║██╔══██╗██║╚══██╔══╝██╔════╝
██████╦╝███████║██╔██╗██║██║░░██║██║░░░██║░░░╚█████╗░
██╔══██╗██╔══██║██║╚████║██║░░██║██║░░░██║░░░░╚═══██╗
██████╦╝██║░░██║██║░╚███║██████╔╝██║░░░██║░░░██████╔╝
╚═════╝░╚═╝░░╚═╝╚═╝░░╚══╝╚═════╝░╚═╝░░░╚═╝░░░╚═════╝░

██╗░░░██╗░██████╗███████╗██████╗░     ░██████╗░██╗░░░██╗██╗██████╗░███████╗
██║░░░██║██╔════╝██╔════╝██╔══██╗     ██╔════╝░██║░░░██║██║██╔══██╗██╔════╝
██║░░░██║╚█████╗░█████╗░░██████╔╝     ██║░░██╗░██║░░░██║██║██║░░██║█████╗░░
██║░░░██║░╚═══██╗██╔══╝░░██╔══██╗     ██║░░╚██╗██║░░░██║██║██║░░██║██╔══╝░░
╚██████╔╝██████╔╝███████╗██║░░██║     ╚██████╔╝╚██████╔╝██║██████╔╝███████╗
░╚═════╝░╚═════╝░╚══════╝╚═╝░░╚═╝     ░╚═════╝░░╚═════╝░╚═╝╚═════╝░╚══════╝

Contextual multi-armed Bandits (CB) are sequential decision-making problems
in which the algorithm must choose the best action based on the context. These
algorithms work online using a learning procedure based on a rewarding mechanism.

At every iteration, algorithm observes a context, chooses an action and receives
a reward related to the chosen option. The reward is then used to update the 
model parameters so the algorithm can learn in an online way as the number 
of iterations increases, collecting enough information to predict the relation 
between the observed context and the reward associated with every action.

A branch of CB algorithms is the Linear UCB that use the Upper Confidence Bound (UCB)
strategy assuming a linear relationship between the context and the reward. 
These algorithms are presented in two version, namely Disjoint and Hybrid.

The Disjoint algorithm is the simplest version, in which all actions are considered independent. 
Each action is analytically modeled through three matrices without shared elements.

The Hybrid Linear UCB is the second algorithm in which the actions are modelled 
through feature vectors. Two shared matrices are presents, while for each
action there are four matrices. 

In both algorithms, the context is modeled by a feature vector of size D,
while the action is modeled with a feature vector of size AF. 
These parameters depend on the problem and can be set in files
params.hpp and params_hybrid.hpp (respectively for Disjoint and Hybrid)\*.
If you want to model a new problem as a CB algorithm, you have to modify these
parameters, the action encoding and the context encoding.

Both algorithms are composed by two phases:
- action selection: the UCB is computed for each action and 
					the one with the lowest value is selected;

- model update: the obtained reward is used to update the model parameters.

For algorithm Disjoint you can run:
- ```make disjoint_algorithm ->``` to select the action;
- ```make update_disjoint_algorithm ->``` to update the model as a function of reward and chosen_action written on "info.hpp"

For alogirhtm Hybrid you can run:
- ```make hybrid_algorithm ->``` to select the action;
- ```make update_hybrid_algorithm ->``` to update the model as a function of reward and chosen_action written on "info.hpp"


---------------------------------------------------------------------------------
To simulate the online functioning of CB algorithms, it's possible to run some
bash script contained in sw/apps/klessydra_tests/klessydra_contextual_bandits/simulation_scripts
These scripts simulate real online operation and have to be copied in sw/build.
Remember to give the execution permission to bash files (chmod +x file_name).
Then you can run ```./exec.sh``` and ```./exec_hybrid.sh``` to simulate Disjoint and Hybrid algorithms
respectively. At each iteration:
- an operation is requested,
- the algorithm chooses the accelerator configuration
- the requested operation is performed
- a reward is received starting from the obtained performance data
- the reward is used to update the model

To do this, the model parameters are transferred between different iterations thanks
to the following process:
- the memory is saved on ModelSim\*\*
- some script are executed to elaborate memory and produce a file values.cpp
  that is included in the main script and initializes every model element.
---------------------------------------------------------------------------------

\* NOTE 1: increasing the model parameters could require a bigger memory space.
This is necessary for executing hybrid algorithm that is more complex.
In order to increase program memory and/or datamemory you have to modify the following files:
 - pulpino-klessydra/sw/refs/link.boot.ld   --> For change in rom or stack
 - pulpino-klessydra/sw/refs/link.common.ld --> For change in instrram, dataram or stack
 - pulpino-klessydra/rtl/core_region.sv     --> change DATA_RAM_SIZE & INSTR_RAM_SIZE rows
 - pulpino-klessydra/rtl/instr_ram_wrap.sv  --> change INSTR_RAM size row
 - pulpino-klessydra/rtl/sp_ram_wrap.sv     --> change RAM_SIZE row
 - pulpino-klessydra/sw/utils/s19toboot.py  --> Only for change in ROM size and/or starting address
 - pulpino-klessydra/sw/utils/s19toslm.py   --> Only for change in ROM size and/or starting address

\*\* NOTE 2: to save the Data memory on ModelSim, you have to modify the file 
pulpino-klessydra/sw/apps/CMakeLists.txt searching for the "vsimc" options and
change them as follows:

	OLD ->
	     COMMAND tcsh -c "${SETENV} ${VSIM}  -c -64 -do 'source tcl_files/$<TARGET_PROPERTY:${NAME}.elf,TB>\\; run -a\\;' >vsim.log" 
	
	NEW ->
	 	 COMMAND tcsh -c "${SETENV} ${VSIM}  -c -64 -do 'source tcl_files/$<TARGET_PROPERTY:${NAME}.elf,TB>\\; run -a\\; mem save -o /home/USERNAME/pulpino-klessydra/sw/build/memory2.mem -f mti -data decimal -addr hex /tb/top_i/core_region_i/mem_gen/data_mem/sp_ram_i/mem \\; exit\\;' >vsim.log"



