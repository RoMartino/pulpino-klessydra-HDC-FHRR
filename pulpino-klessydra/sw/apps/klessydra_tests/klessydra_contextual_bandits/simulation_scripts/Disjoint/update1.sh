#!/bin/bash

# Read matrix info from file
cat choice2.txt > matrices.txt														# The row -4 contains matrix info, formatted in a compact way

# Preprocessing instructions: 
sed -ir 's/ ;/},{/g' matrices.txt	 																	# The ; char represent a row change and it's substituted by },{
sed -iz 's/&/linucb_disjoint_policy_object.linucb_arms[CCC].A={.value={{/g' matrices.txt	   			# The & char is substituted by the assign instruction for matrix A
sed -iz 's/ -/}}};\nlinucb_disjoint_policy_object.linucb_arms[DDD].b={.value={{/g' matrices.txt			# The - char is sobstituted by ending the A definition and starting the assign instruction for b
sed -iz 's/ @/}}};\nlinucb_disjoint_policy_object.linucb_arms[FFF].theta={.value={{/g' matrices.txt	# The @ char is substituted by ending the b definition and starting the assign instruction for theta
sed -iz 's/ !/}}};\n\n/g' matrices.txt   																# The ! char represents the end of element's parameters
sed -ir 's/ /,/g' matrices.txt																			# Every space char is substituted by a comma  

# Header initialization for values.cpp file
echo "#include \"values.hpp\"">final_matrices.hpp												# Necessary include in values.cpp file
echo -e "void init()\n{\n">>final_matrices.hpp															# Function declaration

# Incrementing index
awk '{for(x=1;x<=NF;x++)if($x~/CCC/){sub(/CCC/,i++)}}1' matrices.txt>matrices2.txt						# Assigning the correct index to A definition
awk '{for(x=1;x<=NF;x++)if($x~/DDD/){sub(/DDD/,i++)}}1' matrices2.txt>>matrices3.txt					# Assigning the correct index to b definition
awk '{for(x=1;x<=NF;x++)if($x~/FFF/){sub(/FFF/,i++)}}1' matrices3.txt>>final_matrices.hpp				# Assigning the correct index to theta definition
echo -e "}">>final_matrices.hpp																			# Closing function
cp final_matrices.hpp ../libs/klessydra_lib/cb_libs/src/values.cpp										# The final file is copied in the cb_libs library directory as values.cpp

# Remove temporary files:
rm matrices.txt matrices2.txt matrices3.txt final_matrices.hpp											# Every file previously generated is deleted
