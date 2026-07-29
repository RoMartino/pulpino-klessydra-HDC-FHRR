//██╗░░██╗██╗░░░██╗██████╗░██████╗░██╗██████╗░
//██║░░██║╚██╗░██╔╝██╔══██╗██╔══██╗██║██╔══██╗
//███████║░╚████╔╝░██████╦╝██████╔╝██║██║░░██║
//██╔══██║░░╚██╔╝░░██╔══██╗██╔══██╗██║██║░░██║
//██║░░██║░░░██║░░░██████╦╝██║░░██║██║██████╔╝
//╚═╝░░╚═╝░░░╚═╝░░░╚═════╝░╚═╝░░╚═╝╚═╝╚═════╝░
//
//░██████╗░█████╗░██████╗░██╗██████╗░████████╗
//██╔════╝██╔══██╗██╔══██╗██║██╔══██╗╚══██╔══╝
//╚█████╗░██║░░╚═╝██████╔╝██║██████╔╝░░░██║░░░
//░╚═══██╗██║░░██╗██╔══██╗██║██╔═══╝░░░░██║░░░
//██████╔╝╚█████╔╝██║░░██║██║██║░░░░░░░░██║░░░
//╚═════╝░░╚════╝░╚═╝░░╚═╝╚═╝╚═╝░░░░░░░░╚═╝░░░
//
//██████╗░████████╗░░░██████╗░
//██╔══██╗╚══██╔══╝░░░╚════██╗
//██████╔╝░░░██║░░░░░░░░███╔═╝
//██╔═══╝░░░░██║░░░░░░██╔══╝░░
//██║░░░░░░░░██║░░░██╗███████╗
//╚═╝░░░░░░░░╚═╝░░░╚═╝╚══════╝
// -----------------------------------------------------------------------
// Hybrid script:
// Script for executing disjoint algorithm algorithm. 
// - in part 1, the action choice is performed.
// - in part 2, the model update as a function of received reward is done.
// This script loads model parameters saved in values.cpp and use it to 
// update model as a function of received reward.
// The problem parameters (D,AF and K) are defined in cb_libs/params_hybrid.hpp.
// -----------------------------------------------------------------------
#include "values_h.hpp"
Linear_UCB_Hybrid linucb_hybrid_policy_object(KA,D,K,0.2);
matrix<K,K> A_node_inv;
matrix<K,1> beta_hat;
fixed_point computed_arm_ucb[KA];            // UCB computed for every configuration

int main()
{
  // Threads initializations:   
  sync_barrier_reset();               // Barrier reset
  sync_barrier_thread_registration(); // Thread registration
  __asm__("csrw 0x300, 0x8;" );       // Each thread enables it's own interrupt

  #if SINGLE_THREAD==1                // Single Thread mode: SINGLE_THREAD macro defined in params.hpp
  if (Klessydra_get_coreID()==0)      // in this mode only thread zero works.
  {
  #endif
   #if PERFORMANCE_COUNTER==1         // Cycle count print: PERFORMANCE_COUNTER macro defined in params.hpp
      start_count();                  // Note that if you want to compare multithreading effect, for disjoint algorithm you can use count_all=0 in 
   #endif                             // cmake_configure file, while for hybrid you need count_all=1 (because there are code sections in which sequential and parallel execution)
                                      // are both presents.
  
   init_h();                             // Model parameters are initialized in values.cpp

   // Context feature vector: 
   // -  operation = 1 -> FFT 256 samples
   // -  operation = 2 -> MatMul 64x64
   // Note that the cases for D!=14 are useful only for speed-testing purpose.
   // if you want to use this algorithm in a different problem change this encoding
   #if operation==1
      #if D == 14
         matrix<D,1> ct={.value={{0},{1<<SHIFT_AMOUNT},{0},{0},{1<<SHIFT_AMOUNT},{0},{1<<SHIFT_AMOUNT},{0},{0},{0},{0},{0},{0},{1<<SHIFT_AMOUNT}}};  // FFT context with feature size = 14
      #else 
         matrix<D,1> ct;
      #endif
   #else
      #if D == 14
         matrix<D,1> ct={.value={{0},{0},{1<<SHIFT_AMOUNT},{0},{1<<SHIFT_AMOUNT},{0},{0},{0},{0},{0},{0},{1<<SHIFT_AMOUNT},{1<<SHIFT_AMOUNT},{0}}};  // FFT with 14 feature size
      #else
         matrix<D,1> ct; 
      #endif
   #endif
  
  linucb_hybrid_policy_object.init_arm();

  // Model Update: 

  // 1) Update Phase 1:
  linucb_hybrid_policy_object.update_shared_features_matrices_phase1(linucb_hybrid_policy_object.linucb_arms[chosen_action].B,\
                                                                     linucb_hybrid_policy_object.linucb_arms[chosen_action].A,\
                                                                     linucb_hybrid_policy_object.linucb_arms[chosen_action].b);

  // 2) Z computation   
  matrix<K,1>data_z_array = reshape<K,AF,D>(mult<AF,1,1,D>(transpose<AF,1>(linucb_hybrid_policy_object.linucb_arms[chosen_action].arm_features),transpose<1,D>(ct)));

  // 3) Update of the expected rewards for each action:
  linucb_hybrid_policy_object.linucb_arms[chosen_action].reward_update(reward, ct, data_z_array);

  // 4) Update Phase 2:
  linucb_hybrid_policy_object.update_shared_features_matrices_phase2(data_z_array,reward,linucb_hybrid_policy_object.linucb_arms[chosen_action].B,\
                                                                                         linucb_hybrid_policy_object.linucb_arms[chosen_action].A,\
                                                                                         linucb_hybrid_policy_object.linucb_arms[chosen_action].b);

  // Optimized_print:
  if (Klessydra_get_coreID() == 0)
    printf("i_addr: 0x%x\n",((unsigned)&linucb_hybrid_policy_object.linucb_arms[0] - 0x100000)/4);


   #if PERFORMANCE_COUNTER
      int cycle= finish_count(); 
      if (Klessydra_get_coreID()==0)
      {
      #if ACCL==1                      // Different print for accelerated version 
        printf("ACCL: %d\n", cycle);
      #else
        printf("Not-ACCL: %d\n", cycle);
      #endif
      }
   #endif
   
  #if SINGLE_THREAD==1
  }
  else
    __asm__("wfi;");                   // In single thread execution mode, thread 1 and 2 are in WFI state
  #endif

 return 0;
}

