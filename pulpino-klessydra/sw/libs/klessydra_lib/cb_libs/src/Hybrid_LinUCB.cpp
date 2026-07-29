//██╗░░██╗██╗░░░██╗██████╗░██████╗░██╗██████╗░
//██║░░██║╚██╗░██╔╝██╔══██╗██╔══██╗██║██╔══██╗
//███████║░╚████╔╝░██████╦╝██████╔╝██║██║░░██║
//██╔══██║░░╚██╔╝░░██╔══██╗██╔══██╗██║██║░░██║
//██║░░██║░░░██║░░░██████╦╝██║░░██║██║██████╔╝
//╚═╝░░╚═╝░░░╚═╝░░░╚═════╝░╚═╝░░╚═╝╚═╝╚═════╝░
//
// ██╗░░░░░██╗███╗░░██╗███████╗░█████╗░██████╗░  ██╗░░░██╗░█████╗░██████╗░
// ██║░░░░░██║████╗░██║██╔════╝██╔══██╗██╔══██╗  ██║░░░██║██╔══██╗██╔══██╗
// ██║░░░░░██║██╔██╗██║█████╗░░███████║██████╔╝  ██║░░░██║██║░░╚═╝██████╦╝
// ██║░░░░░██║██║╚████║██╔══╝░░██╔══██║██╔══██╗  ██║░░░██║██║░░██╗██╔══██╗
// ███████╗██║██║░╚███║███████╗██║░░██║██║░░██║  ╚██████╔╝╚█████╔╝██████╦╝
// ╚══════╝╚═╝╚═╝░░╚══╝╚══════╝╚═╝░░╚═╝╚═╝░░╚═╝  ░╚═════╝░░╚════╝░╚═════╝░
/*-------------------------------------------------------------------------------------------------------
Script:      Hybrid_LinUCB.cpp
Description: Definition of LinUCB_hybrid_element & Linear_UCB_Hybrid classes. These classes
			 fully define the Disjoint Linear UCB algorithm. All the methods used are described.
	     	 For each action (or configuration) there is a LinUCB_hybrid_element.
Author:      Marco Angioli
Data:        07/2022
Version:     3.0
-------------------------------------------------------------------------------------------------------*/
#include "Hybrid_LinUCB.hpp"

// ------------------------------------- LinUCB_hybrid_element METHODS-------------------------------------
// Method for computing the element UCB
fixed_point LinUCB_hybrid_element::calc_UCB(matrix<D,1> xt, matrix<K,K>A_node_inv, matrix<K,1>beta_hat, int core)
{
	matrix<D,D> A_inv = LU_decomposition<D,D>(A,core);									// Computation of A inverse matrix
	matrix<1,D>xt_transpose = transpose<1,D>(xt); 										// Transpose matrix of xt
	matrix<AF,1>arm_features_transpose = transpose<AF,1>(arm_features);					// Transpose vector of arm_features
	
	matrix<AF,D>z=mult<AF,1,1,D>(arm_features_transpose,xt_transpose,core);				// Z vector: a x d
	matrix<K,1>z_reshaped=reshape<K,AF,D>(z);											// Reshape -1,1 of z vector
	matrix<1,K>z_reshaped_transpose=transpose<1,K>(z_reshaped);							// Z_reshape transpose
	matrix<K,D>B_transpose=transpose<K,D>(B);											// B transpose

	theta = mult<D,D,D,1>(A_inv,sub<D,1>(b,mult<D,K,K,1>(B,beta_hat,core)),core);		// Calculation of Theta starting from A_inv, b, B and beta hat

	// Standard deviation s														
	fixed_point due=(float)2.0;
	fixed_point s1 =   mult<1,K,K,1>(z_reshaped_transpose,mult<K,K,K,1>(A_node_inv,z_reshaped,core),core).value[0][0];
	fixed_point s2 = due*mult<1,K,K,1>(z_reshaped_transpose,mult<K,K,K,1>(A_node_inv,mult<K,D,D,1>(B_transpose,mult<D,D,D,1>(A_inv,xt,core),core),core),core).value[0][0];
	fixed_point s3 =   mult<1,D,D,1>(xt_transpose,mult<D,D,D,1>(A_inv,xt,core),core).value[0][0];
	fixed_point s4 =   mult<1,D,D,1>(xt_transpose,mult<D,D,D,1>(A_inv, mult<D,K,K,1>(B,mult<K,K,K,1>(A_node_inv,mult<K,D,D,1>(B_transpose,mult<D,D,D,1>(A_inv,xt,core),core),core),core),core),core).value[0][0];
	fixed_point s = s1 - s2 + s3 + s4;

	// UCB value:
	fixed_point p = mult<1,K,K,1>(z_reshaped_transpose,beta_hat,core).value[0][0]+mult<1,D,D,1>(xt_transpose,theta,core).value[0][0]+alpha*sqrt(s.to_float());
	return p;
}

// Item update as a function of the received reward
void LinUCB_hybrid_element::reward_update(float reward, matrix<D,1> xt, matrix<K,1>z_array)
{			
	if (Klessydra_get_coreID() == 0)
	{
		matrix<1,D>xt_transpose = transpose<1,D>(xt);
		matrix<1,K>z_array_transpose=transpose<1,K>(z_array);					// Z_reshape transpose
		A = sum<D,D>(A,mult<D,1,1,D>(xt,xt_transpose));
		B = sum<D,K>(B,mult<D,1,1,K>(xt,z_array_transpose));
		b = sum<D,1>(b, mult_coeff<D,1>(xt,reward));
  }
}





// ------------------------------------- Linear_UCB_Hybrid METHODS-------------------------------------
// 1) Best configuration selection by UCB computing for each element
int Linear_UCB_Hybrid::select_arm(matrix<D,1> xt, matrix<K,K> A_node_inv, matrix<K,1>beta_hat)
{
	srand(TIME);						// Random seed based on Time variable (CMAKE)
	int highest_ucb = 0;
	int candidate_arms[K_arms];			// Inizialization 12 elements vector
	int chosen_arm_index = 0;				// Configuration index
	int equal_numb = 1;					// Configuration number with the same ucb
	int arm_index = 0;
	int core = 0;

	// Operation done in an interleaved manner. In function declarations, the "core" argument is set to zero by default
	// So, not setting this argument explicitly no offset will be used
	// Multi-Threaded execution: the work is divided between the three thread

	for (arm_index=0; arm_index<K_arms; arm_index++)
	{	
    	#if SINGLE_THREAD==1
      		computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,A_node_inv,beta_hat,core);
    	#else
	  		if (Klessydra_get_coreID() == 0 && arm_index < 4)
	  		{
	  			core = 0;
	  			computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,A_node_inv,beta_hat,core);		
	  		}
	  
	  		else if (Klessydra_get_coreID() == 1 && arm_index>=4 && arm_index < 8)
	  		{
	  			core = 1;
	  			computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,A_node_inv,beta_hat,core);
	  		}
	  
	  		else if (Klessydra_get_coreID() == 2 && arm_index >= 8)
	  		{
	  			core = 2;
	  			computed_arm_ucb[arm_index] = linucb_arms[arm_index].calc_UCB(xt,A_node_inv,beta_hat,core);
	  		}
   		#endif
	}
	

	if (Klessydra_get_coreID() == 0)
	{
		for (int arm_index=0; arm_index<K_arms; arm_index++)
		{
      printf("Computed UCB[%d]: %d\n",arm_index, computed_arm_ucb[arm_index].num);
			if (computed_arm_ucb[arm_index].num > highest_ucb) 	// Check if it's the best action
			{
				highest_ucb = computed_arm_ucb[arm_index].num;	  // Saving highest ucb value
				equal_numb=0;								                      // Reset configuration number
				candidate_arms[equal_numb++] = arm_index; 	      // save index
			}
			else if (computed_arm_ucb[arm_index].num == highest_ucb)
				candidate_arms[equal_numb++] = arm_index; 	      // save index with ucb >= Highest_ucb
		}
		
    // Randomly choose between equal configurations
	int RandIndex = rand() % equal_numb;
	chosen_arm_index = candidate_arms[RandIndex];

	}
  else
    __asm__("wfi;");

  return chosen_arm_index;
}

// Update Phases
// Phase 1: Local rewards update
void Linear_UCB_Hybrid::update_shared_features_matrices_phase1(matrix<D,K> chosen_arm_B, matrix<D,D> chosen_arm_A, matrix<D,1> chosen_arm_b)
{
	if (Klessydra_get_coreID() == 0)
	{
		matrix<D,D> chosen_arm_A_inv = LU_decomposition<D,D>(chosen_arm_A);

		matrix<K,D> chosen_arm_B_transpose = transpose<K,D>(chosen_arm_B);
		A_node = sum<K,K>(A_node, mult<K,D,D,K>(chosen_arm_B_transpose,mult<D,D,D,K>(chosen_arm_A_inv,chosen_arm_B))); // (k x k) + (k x d)*(d x d)*(d x k)
		b_node = sum<K,1>(b_node, mult<K,D,D,1>(chosen_arm_B_transpose,mult<D,D,D,1>(chosen_arm_A_inv,chosen_arm_b)));
  }
}

// Phase 2: update using reward
void Linear_UCB_Hybrid::update_shared_features_matrices_phase2(matrix<K,1> z_array, float reward, matrix<D,K> chosen_arm_B, matrix<D,D> chosen_arm_A, matrix<D,1> chosen_arm_b)        
{
  if (Klessydra_get_coreID() == 0)
  {
    matrix<D,D> chosen_arm_A_inv = LU_decomposition<D,D>(chosen_arm_A);
		matrix<K,D> chosen_arm_B_transpose = transpose<K,D>(chosen_arm_B);
		matrix<1,K> z_array_transpose      = transpose<1,K>(z_array);

		A_node = sum<K,K>(A_node, sub<K,K>(mult<K,1,1,K>(z_array, z_array_transpose),mult<K,D,D,K>(chosen_arm_B_transpose, mult<D,D,D,K>(chosen_arm_A_inv,chosen_arm_B))));
		b_node = sum<K,1>(b_node, sub<K,1>(mult_coeff<K,1>(z_array,reward), 		 mult<K,D,D,1>(chosen_arm_B_transpose, mult<D,D,D,1>(chosen_arm_A_inv,chosen_arm_b))));
  }
}

// Arm initialization routine: 
// in this method you have to specify arm feature vectors (size AF)
void Linear_UCB_Hybrid::init_arm()
{  
  #if AF==8
  linucb_arms[0].arm_features   = {.value={{1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,0}}};
  linucb_arms[1].arm_features   = {.value={{1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,1<<SHIFT_AMOUNT,0,0}}};
  linucb_arms[2].arm_features   = {.value={{1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,0,1<<SHIFT_AMOUNT,0}}};
  linucb_arms[3].arm_features   = {.value={{1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,0,0,1<<SHIFT_AMOUNT}}};
  linucb_arms[4].arm_features   = {.value={{0,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,0}}};
  linucb_arms[5].arm_features   = {.value={{0,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0}}};
  linucb_arms[6].arm_features   = {.value={{0,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,1<<SHIFT_AMOUNT,0}}};
  linucb_arms[7].arm_features   = {.value={{0,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,0,1<<SHIFT_AMOUNT}}};
  linucb_arms[8].arm_features   = {.value={{0,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,0}}};
  linucb_arms[9].arm_features   = {.value={{0,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,1<<SHIFT_AMOUNT,0,0}}};
  linucb_arms[10].arm_features  = {.value={{0,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,0,1<<SHIFT_AMOUNT,0}}};
  linucb_arms[11].arm_features  = {.value={{0,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,0,0,1<<SHIFT_AMOUNT}}};
  
  #elif AF==6
  linucb_arms[0].arm_features   = {.value={{0,0,1<<SHIFT_AMOUNT,0,0,0}}};
  linucb_arms[1].arm_features   = {.value={{0,0,0,1<<SHIFT_AMOUNT,0,0}}};
  linucb_arms[2].arm_features   = {.value={{0,0,0,0,1<<SHIFT_AMOUNT,0}}};
  linucb_arms[3].arm_features   = {.value={{0,0,0,0,0,1<<SHIFT_AMOUNT}}};
  linucb_arms[4].arm_features   = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,0}}};
  linucb_arms[5].arm_features   = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0}}};
  linucb_arms[6].arm_features   = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,1<<SHIFT_AMOUNT,0}}};
  linucb_arms[7].arm_features   = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,0,0,0,1<<SHIFT_AMOUNT}}};
  linucb_arms[8].arm_features   = {.value={{1<<SHIFT_AMOUNT,0,1<<SHIFT_AMOUNT,0,0,0}}};
  linucb_arms[9].arm_features   = {.value={{1<<SHIFT_AMOUNT,0,0,1<<SHIFT_AMOUNT,0,0}}};
  linucb_arms[10].arm_features  = {.value={{1<<SHIFT_AMOUNT,0,0,0,1<<SHIFT_AMOUNT,0}}};
  linucb_arms[11].arm_features  = {.value={{1<<SHIFT_AMOUNT,0,0,0,0,1<<SHIFT_AMOUNT}}};
  
  #else 
	linucb_arms[0].arm_features  = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT}}};
	linucb_arms[1].arm_features  = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,2<<SHIFT_AMOUNT}}};
	linucb_arms[2].arm_features  = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,4<<SHIFT_AMOUNT}}};
	linucb_arms[3].arm_features  = {.value={{1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,8<<SHIFT_AMOUNT}}};
	linucb_arms[4].arm_features  = {.value={{3<<SHIFT_AMOUNT,3<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT}}};
	linucb_arms[5].arm_features  = {.value={{3<<SHIFT_AMOUNT,3<<SHIFT_AMOUNT,2<<SHIFT_AMOUNT}}};
	linucb_arms[6].arm_features  = {.value={{3<<SHIFT_AMOUNT,3<<SHIFT_AMOUNT,4<<SHIFT_AMOUNT}}};
	linucb_arms[7].arm_features  = {.value={{3<<SHIFT_AMOUNT,3<<SHIFT_AMOUNT,8<<SHIFT_AMOUNT}}};
	linucb_arms[8].arm_features  = {.value={{3<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT}}};
	linucb_arms[9].arm_features  = {.value={{3<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,2<<SHIFT_AMOUNT}}};
	linucb_arms[10].arm_features = {.value={{3<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,4<<SHIFT_AMOUNT}}};
	linucb_arms[11].arm_features = {.value={{3<<SHIFT_AMOUNT,1<<SHIFT_AMOUNT,8<<SHIFT_AMOUNT}}};
 #endif
}