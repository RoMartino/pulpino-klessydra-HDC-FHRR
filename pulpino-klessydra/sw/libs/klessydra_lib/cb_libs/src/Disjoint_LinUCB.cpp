// ██████╗░██╗░██████╗░░░░░██╗░█████╗░██╗███╗░░██╗████████╗
// ██╔══██╗██║██╔════╝░░░░░██║██╔══██╗██║████╗░██║╚══██╔══╝
// ██║░░██║██║╚█████╗░░░░░░██║██║░░██║██║██╔██╗██║░░░██║░░░
// ██║░░██║██║░╚═══██╗██╗░░██║██║░░██║██║██║╚████║░░░██║░░░
// ██████╔╝██║██████╔╝╚█████╔╝╚█████╔╝██║██║░╚███║░░░██║░░░
// ╚═════╝░╚═╝╚═════╝░░╚════╝░░╚════╝░╚═╝╚═╝░░╚══╝░░░╚═╝░░░
// 
// ██╗░░░░░██╗███╗░░██╗███████╗░█████╗░██████╗░  ██╗░░░██╗░█████╗░██████╗░
// ██║░░░░░██║████╗░██║██╔════╝██╔══██╗██╔══██╗  ██║░░░██║██╔══██╗██╔══██╗
// ██║░░░░░██║██╔██╗██║█████╗░░███████║██████╔╝  ██║░░░██║██║░░╚═╝██████╦╝
// ██║░░░░░██║██║╚████║██╔══╝░░██╔══██║██╔══██╗  ██║░░░██║██║░░██╗██╔══██╗
// ███████╗██║██║░╚███║███████╗██║░░██║██║░░██║  ╚██████╔╝╚█████╔╝██████╦╝
// ╚══════╝╚═╝╚═╝░░╚══╝╚══════╝╚═╝░░╚═╝╚═╝░░╚═╝  ░╚═════╝░░╚════╝░╚═════╝░
/*-------------------------------------------------------------------------------------------------------
Script:      Disjoint_LinUCB.cpp
Description: Definition of LinUCB_disjoint_element & Linear_UCB_Disjoint classes. These classes
			 			 fully define the Disjoint Linear UCB algorithm. All the methods used are described.
	     	 		 For each action (or configuration) there is a LinUCB_disjoint_element.
Author:      Marco Angioli
Data:        07/2022
Version:     3.0
-------------------------------------------------------------------------------------------------------*/

#include "Disjoint_LinUCB.hpp"

// ------------------------------------- LinUCB_disjoint_element METHODS-------------------------------------
// Method for computing the element UCB
fixed_point LinUCB_disjoint_element::calc_UCB(matrix<D,1> xt,int core)
{
	matrix<D,D> A_inv = LU_decomposition<D,D>(A,core);							                            	// Computation of A inverse matrix
	theta = mult<D,D,D,1>(A_inv,b,core);														                            	// Calculation of Theta starting from A_inv and b
	matrix<1,D>xt_transpose = transpose<1,D>(xt); 										                          	// Transpose matrix of xt
	matrix<1,D>theta_transpose = transpose<1,D>(theta);									                        	// Transpose matrix of theta
	fixed_point s = mult<1,D,D,1>(xt_transpose,mult<D,D,D,1>(A_inv,xt,core),core).value[0][0]; 		// Standard deviation s
	fixed_point s_sqrt = sqrt(s.to_float()); 	
	fixed_point p = mult<1,D,D,1>(theta_transpose,xt,core).value[0][0] + alpha*s_sqrt; 					  // UCB value	

	return p;
}

// Item update as a function of the received reward
void LinUCB_disjoint_element::reward_update(float reward, matrix<D,1> xt)
{			
	
  if (Klessydra_get_coreID()==0)
  {
    matrix<1,D>xt_transpose = transpose<1,D>(xt);
	  A = sum<D,D>(A,mult<D,1,1,D>(xt,xt_transpose));	      
	  b = sum<D,1>(b, mult_coeff<D,1>(xt,reward));
  }
  else
    __asm__("wfi;");
}



// ------------------------------------- Linear_UCB_Disjoint METHODS-------------------------------------
// 1) Best configuration selection by UCB computing for each element
int Linear_UCB_Disjoint::select_arm(matrix<D,1> xt)
{
	srand(140);					        	// Random seed based on Time variable (CMAKE)
	int highest_ucb = 0.0;
	int candidate_arms[K_arms];		// Inizialization 12 elements vector
	int chosen_arm_index=0;				// Configuration index
	int equal_numb = 1;					  // Configuration number with the same ucb
	int arm_index=0;
	int core=0;
	
	// Multi-Threaded execution/*
	for (arm_index=0; arm_index<K_arms; arm_index++)
	{
    #if SINGLE_THREAD==1
    	computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,core);
    #else
			if (Klessydra_get_coreID() == 0 && arm_index < K/3)
			{
				core=0;
				computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,core);		
			}

			else if (Klessydra_get_coreID() == 1 && arm_index>=K/3 && arm_index < 2*K/3)
			{
				core=1;
				computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,core);
			}

			else if (Klessydra_get_coreID() == 2 && arm_index >= 2*K/3)
			{
				core=2;
				computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,core);
			}
		#endif
	}
	
  if (Klessydra_get_coreID()==0)
  {
  		for (int arm_index=0; arm_index<K_arms; arm_index++)
  		{
       //printf("Computed UCB[%d]: %d\n",arm_index, computed_arm_ucb[arm_index].num);
  			if (computed_arm_ucb[arm_index].num > highest_ucb) 	    // Check if it's the best action
  			{
  				highest_ucb = computed_arm_ucb[arm_index].num;	      // Saving highest ucb value
  				equal_numb=0;								                          // Reset configuration number
  				candidate_arms[equal_numb++] = arm_index; 	          // save index
  			}
  			else if (computed_arm_ucb[arm_index].num == highest_ucb)
  				candidate_arms[equal_numb++] = arm_index; 	          // save index with ucb >= Highest_ucb
  		}
	
  	// Randomly choose between equal configurations
		int RandIndex = rand() % equal_numb;
		chosen_arm_index = candidate_arms[RandIndex];
  }
    else
    __asm__("wfi;");
	return chosen_arm_index;
}