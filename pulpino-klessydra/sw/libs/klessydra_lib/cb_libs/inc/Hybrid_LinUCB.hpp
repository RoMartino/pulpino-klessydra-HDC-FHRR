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

#ifndef _Hybrid_hpp_
#define _Hybrid_hpp_
#include <math.h>
#include "params_hybrid.hpp"
#include "matrix.hpp"
#ifndef TIME
	#define TIME 10
#endif

extern fixed_point computed_arm_ucb[KA];	// UCB computed for every configuration
extern int cycle_inv, cycle_other;

// Lin UCB CLASS DEFINITION:
class LinUCB_hybrid_element
{
   		
    public:
    		//---------- Attributes: ----------
	   		int index;			   // element index: in the final LinUCB_Hybrid class there will be one element for every possible configuration (or action)
			int d;			 	   // d = context feature dimension
			int k;				   // k = context_feature dimension * action_feature dimension
			fixed_point alpha;		   // Exploration parameter
			matrix<D,D> A;  	   // A Matrix,  size dxd 
			matrix<D,K> B;  	   // B Matrix,  size dxk 
			matrix<D,1> b;  	   // Matrice b, size dx1
			matrix<D,1> theta; 	   // Theta matrix
			matrix<1,AF> arm_features;	// Configuration features: 3x1 (where 3 is: M,F,D parameters)

    public:
			// Default constructor
    		LinUCB_hybrid_element()
    		{
    			d=1;
    			alpha=0.2;
				A = identity<D,D>();
				B = zero<D,K>();
				b = zero<D,1>();
				theta= zero<D,1>();
    		}
    		
    		
    		// Custom constructor
    		LinUCB_hybrid_element(int d_value, int k_value, float alpha_value, int index_value)
    		{
    			d=d_value;
    			k=k_value;
    			alpha=alpha_value;
    			index=index_value;
				A = identity<D,D>();
				B = zero<D,K>();
		  		b = zero<D,1>();
				theta= zero<D,1>();
    		}
    		
  
    		//---------- Methods: -------------- 
			fixed_point calc_UCB(matrix<D,1>, matrix<K,K>, matrix<K,1>, int); 	// UCB computation function
	    	void reward_update(float, matrix<D,1>, matrix<K,1>);	// Update function given the reward		
};



class Linear_UCB_Hybrid : public LinUCB_hybrid_element
{
	public:
			//---------- Attributes: ----------
			int K_arms;						      	// Possible configurations number 
			LinUCB_hybrid_element linucb_arms[KA]; 	// Allocate KA (12) elments linucb_hybrid_element
			matrix<K,K> A_node;						// Shared matrix, A_0: size (k x k)
			matrix<K,1> b_node;						// Shared matrix, b_0: size (k x 1)
				
	public:			
			// Default constructor
			Linear_UCB_Hybrid()
			{
				K_arms = 12;
				int d_value=4;
				int k_value=12;
				float alpha_value=0.1;			
				for (int i=0;i<K_arms;i++)
					linucb_arms[i] = LinUCB_hybrid_element(d_value, k_value, alpha_value,i);
				A_node = identity<K,K>();
				b_node = zero<K,1>();
			}	
			
			
			// Custom constructor:
			Linear_UCB_Hybrid(int action_number, int d_value, int k_value, float alpha_value)
			{
				K_arms = action_number;		
				for (int i=0;i<K_arms;i++)
					linucb_arms[i] = LinUCB_hybrid_element(d_value, k_value, alpha_value,i);
				A_node = identity<K,K>();
				b_node = zero<K,1>();
			}		

			
		 	//---------- Methods: -------------- 
			int select_arm(matrix<D,1>,matrix<K,K>, matrix<K,1>);										// Best configuration selection by UCB computing for each element
			void update_shared_features_matrices_phase1(matrix<D,K> chosen_arm_B, matrix<D,D> chosen_arm_A, matrix<D,1> chosen_arm_b); // Phase 1: Local rewards update
			void update_shared_features_matrices_phase2(matrix<K,1> z_array, float reward, matrix<D,K> chosen_arm_B, matrix<D,D> chosen_arm_A, matrix<D,1> chosen_arm_b);        // Phase 2: update using reward
			void init_arm();
};
#endif
