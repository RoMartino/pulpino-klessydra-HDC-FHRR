#
#███╗░░░███╗███████╗███╗░░░███╗░█████╗░██████╗░██╗░░░██╗
#████╗░████║██╔════╝████╗░████║██╔══██╗██╔══██╗╚██╗░██╔╝
#██╔████╔██║█████╗░░██╔████╔██║██║░░██║██████╔╝░╚████╔╝░
#██║╚██╔╝██║██╔══╝░░██║╚██╔╝██║██║░░██║██╔══██╗░░╚██╔╝░░
#██║░╚═╝░██║███████╗██║░╚═╝░██║╚█████╔╝██║░░██║░░░██║░░░
#╚═╝░░░░░╚═╝╚══════╝╚═╝░░░░░╚═╝░╚════╝░╚═╝░░╚═╝░░░╚═╝░░░
#
#███████╗██╗░░░░░░█████╗░██████╗░░█████╗░██████╗░░█████╗░████████╗██╗░█████╗░███╗░░██╗
#██╔════╝██║░░░░░██╔══██╗██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚══██╔══╝██║██╔══██╗████╗░██║
#█████╗░░██║░░░░░███████║██████╦╝██║░░██║██████╔╝███████║░░░██║░░░██║██║░░██║██╔██╗██║
#██╔══╝░░██║░░░░░██╔══██║██╔══██╗██║░░██║██╔══██╗██╔══██║░░░██║░░░██║██║░░██║██║╚████║
#███████╗███████╗██║░░██║██████╦╝╚█████╔╝██║░░██║██║░░██║░░░██║░░░██║╚█████╔╝██║░╚███║
#╚══════╝╚══════╝╚═╝░░╚═╝╚═════╝░░╚════╝░╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝
#
#░██████╗░█████╗░██████╗░██╗██████╗░████████╗
#██╔════╝██╔══██╗██╔══██╗██║██╔══██╗╚══██╔══╝
#╚█████╗░██║░░╚═╝██████╔╝██║██████╔╝░░░██║░░░
#░╚═══██╗██║░░██╗██╔══██╗██║██╔═══╝░░░░██║░░░
#██████╔╝╚█████╔╝██║░░██║██║██║░░░░░░░░██║░░░
#╚═════╝░░╚════╝░╚═╝░░╚═╝╚═╝╚═╝░░░░░░░░╚═╝░░░
# --------------------------------------------------------------------------------------------------------
# ***************************************Mem_elaboration.sh***********************************************
# This script takes as argument: 
# - Context feature size (D)
# - Initial Address in memory
# and elaborates the ModelSim memory dump contained in file "mem.txt"
# It reads file contents and extracts from it the algorithm useful informations.
# This script returns as output the info.txt file. It is then used from scripts update.sh and update1.sh
# to change the content of values.cpp and correctly initialize the algorithm.
# --------------------------------------------------------------------------------------------------------

# Checking that the argument's number is correct:
if (($# != 2))
then
	echo -e "\e[91mWarning:\e[39m mem_elaboration.sh requires two arguments as input. Context feature size and initial memory address"
	exit
fi

# If arguments' number is correct, then D is the first one and add is the second one:
AF=3										  # AF VALUE !!
D=$1
K=$((D*AF))
addr=$2
SIZE=$((12*(D*D+D*K+D+D+6)+K*K+K+1+100))
											

# The initial memory address is used to compute the starting file line
initial_line=$((($addr/5)+1)) 								# The initial file line is computed dividing the starting address by the number of elements in each row
final_line=$((($SIZE+$addr)/5))								# The final line is computed adding the SIZE variable to the initial line


# File elaboration:
cat memory2.mem|grep -v '//' > mem.txt 						# The first three rows are removed (heading)
sed -n ${initial_line},${final_line}p mem.txt > output.txt  # Only the lines included between $initial_line and $final_line are maintained
cat output.txt |cut -d ':' -f 2 > output2.txt      			# The first column (line number) is now removed 
resto=$((addr % 5))											# It's important to note that the first element could be in any position of initial_line. 
															# This position is identified by the remainder of the division (addr/5)

first_line=($(cat output2.txt | head -1 ))
echo "${first_line[@]: -((5-resto)):((5-resto))}"> output3.txt
#| sed -r 's/          /,/g'|cut -d ',' -f $(($resto+2))- >output3.txt
cat output2.txt | tail -n +2 >> output3.txt



# Starting from file output3.txt the content is formatted to be processed by scripts update.sh and update1.sh
A=0; B=1; theta=1; b=1; count=0; element=0; first=1; count_node=0;offset=0
A_size=$((D*D))
B_size=$((D*K))
A_node_size=$((K*K))
A_node=0;B_node=1
echo -n " ^"
while read -r line
do
	array=($line)
	for ((i=0; i < ${#array[*]}; i++))
	do
		if [ "${array[$i]}" != ' ' ]
		then

			if ((element!=12))
			then
				# A matrix:
				if ((A==0))
				then
					if ((count==4+$offset)) && ((first==1))
					then
						echo  -n "&${array[$i]} "
						count=1
						first=0
					elif ((count<$A_size)) && ((first==0))
					then
						if ((count%D==0))
						then
							echo -n "${array[$i]} ;"
						else
							echo -n "${array[$i]} "
						fi
					elif ((count==$A_size))
					then
						A=1;B=0;
						echo -n "${array[$i]}"					
						#echo -e "\e[91m\nA\e[39m"
					fi

				# B matrix:
				elif ((B==0))
				then
					if ((count==$A_size+1)) 
					then
						echo -n " ?${array[$i]} "
					elif ((count<(($A_size+$B_size))))
					then
						if (((count-A_size)%K==0))
						then
							echo -n "${array[$i]} ;"
						else
							echo -n "${array[$i]} "
						fi
					elif ((count==$A_size+$B_size))
					then
						B=1;b=0;
						echo -n "${array[$i]}"
						#echo -e "\e[91m\nB\e[39m"
					fi
		
				# b vector
				elif ((b==0))
				then
					if ((count==$A_size+$B_size+1)) 
					then
						echo -n " _${array[$i]} ;"
					elif ((count<(($A_size+$B_size+D))))
					then
						echo -n "${array[$i]} ;"
					elif ((count==(($A_size+$B_size+D))))
					then
						b=1;theta=0;
						echo -n "${array[$i]}"
						#echo -e "\e[91m\nb\e[39m"
					fi
				# Theta vector
				elif ((theta==0))
				then
					if ((count==(($A_size+$B_size+D+1)))) 
					then
						echo -n " @${array[$i]} ;"
					elif ((count<(($A_size+$B_size+2*D))))
					then
						echo -n "${array[$i]} ;"
					elif ((count==(($A_size+$B_size+2*D))))
					then
						echo -n "${array[$i]}"
						#echo -e "\e[91m\ntheta\e[39m"
						#Termine elemento:
						A=0; b=1; theta=1; B=1
						echo -n " !"
						count=-1
						offset=$AF
						first=1
						(( element++ ))
					fi
				fi
		 		((count++))
		 	else
				# Read A_node and b_node
				# A matrix:
				if ((count_node>=offset))
				then

					if ((A_node==0))
					then
						if ((count_node==offset)) 
						then
							echo  -n "#${array[$i]} "
							
		
						elif ((count_node<$A_node_size+(offset-1)))
						then
							if (((count_node-(offset-1))%K==0))
							then
								echo -n "${array[$i]} ;"
							else
								echo -n "${array[$i]} "
							fi
						elif ((count_node==$A_node_size+((offset-1))))
						then
							A_node=1;B_node=0;
							echo -n "${array[$i]}"
						fi
					# B matrix:
					elif ((B_node==0))
					then
						if ((count_node==$A_node_size+(offset))) 
						then
							echo -n " =${array[$i]} ;"
						elif ((count_node<(($A_node_size+(offset-1)+$K))))
						then
							echo -n "${array[$i]} ;"
						elif ((count_node==$A_node_size+(offset-1)+$K))
						then
							B=1;b=0;
							echo -n "${array[$i]} !"
						fi
					fi
				fi
				((count_node++))
			fi
		fi
	done
	if ((element==12 && all_done==1))
	then
		break
	fi
done<output3.txt
echo

# Delete temporary files:
 #rm mem.txt output.txt output2.txt output3.txt

