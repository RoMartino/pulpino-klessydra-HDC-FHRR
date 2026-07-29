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
Script:      Disjoint_LinUCB.hpp
Description: Definition of LinUCB_disjoint_element & Linear_UCB_Disjoint classes. These classes
			 fully define the Disjoint Linear UCB algorithm. All the methods used are described.
	     	 For each action (or configuration) there is a LinUCB_disjoint_element.
Author:      Marco Angioli
Data:        09/02/2022
Version:     1.1
-------------------------------------------------------------------------------------------------------*/
#ifndef _Disjoint_hpp_
#define _Disjoint_hpp_
#include "matrix.hpp"
#include "params.hpp"
#ifndef TIME
	#define TIME 10
#endif

extern fixed_point computed_arm_ucb[K];	// UCB computed for every configuration
extern int cycle_inv, cycle_other;

// Lin UCB CLASS DEFINITION:
class LinUCB_disjoint_element
{
   		
    public:
    		//---------- Attributes: ----------
	   		int index;			   // element index: in the final class there will be one element for every possible action
			int d;			 	   // d = context feature dimension
			fixed_point alpha;	   // Exploration parameter alpha
			matrix<D,D> A;  	   // A Matrix,  size dxd 
			matrix<D,1> b;  	   // Matrice b, size dx1
			matrix<D,1> theta; 	   // Theta matrix


      public:
			// Default constructor
    		LinUCB_disjoint_element()
    		{
    			d=1;
    			alpha=0.1;
				A = identity<D,D>();
				b = zero<D,1>();
				theta= zero<D,1>();
    		}
    		
    		
    		// Custom constructor
    		LinUCB_disjoint_element(int d_value, float alpha_value, int index_value)
    		{
    			d=d_value;
    			alpha=alpha_value;
    			index=index_value;
				A = identity<D,D>();
		  		b = zero<D,1>();
				theta= zero<D,1>();
    		}
    		
  
    		//---------- Methods: -------------- 
			fixed_point calc_UCB(matrix<D,1>,int); 			// UCB computation function
    	void reward_update(float, matrix<D,1>);				// Update function given the reward		

};



class Linear_UCB_Disjoint : public LinUCB_disjoint_element
{
	public:
			//---------- Attributes: ----------
			int K_arms;						      	// Possible configurations number 
			LinUCB_disjoint_element linucb_arms[K]; // LinUCB_disjoint_element array of size K
				
	public:			
			// Default constructor
			Linear_UCB_Disjoint()
			{
				K_arms = 12;
				int d_value=4;
				float alpha_value=0.1;			
				for (int i=0;i<K_arms;i++)
					linucb_arms[i] = LinUCB_disjoint_element(d_value, alpha_value,i);
			}	
			
			
			// Custom constructor:
			Linear_UCB_Disjoint(int action_number, int d_value, float alpha_value)
			{
				K_arms = action_number;		
				for (int i=0;i<K_arms;i++)
					linucb_arms[i] = LinUCB_disjoint_element(d_value, alpha_value,i);
			}		

			
		 	//---------- Methods: -------------- 
			int select_arm(matrix<D,1>);		// Best configuration selection by UCB computing for each element
};
void init();
#endif
