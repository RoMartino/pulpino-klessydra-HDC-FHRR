//██████╗░██╗░██████╗░░░░░██╗░█████╗░██╗███╗░░██╗████████╗
//██╔══██╗██║██╔════╝░░░░░██║██╔══██╗██║████╗░██║╚══██╔══╝
//██║░░██║██║╚█████╗░░░░░░██║██║░░██║██║██╔██╗██║░░░██║░░░
//██║░░██║██║░╚═══██╗██╗░░██║██║░░██║██║██║╚████║░░░██║░░░
//██████╔╝██║██████╔╝╚█████╔╝╚█████╔╝██║██║░╚███║░░░██║░░░
//╚═════╝░╚═╝╚═════╝░░╚════╝░░╚════╝░╚═╝╚═╝░░╚══╝░░░╚═╝░░░
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
// Disjoint script:
// Script for executing disjoint algorithm algorithm. 
// - in part 1, the action choice is performed.
// - in part 2, the model update as a function of received reward is done.
// This script loads model parameters saved in values.cpp and use it to 
// update model as a function of received reward.
// The problem parameters (D and K) are defined in cb_libs/params.hpp.
// -----------------------------------------------------------------------

#include "values.hpp"

fixed_point computed_arm_ucb[K];                            // fixed_point array containing the UCB computed for every action
Linear_UCB_Disjoint linucb_disjoint_policy_object(K,D,0.5); // Disjoint model, the last parameter is alpha

int main()
{
  sync_barrier_reset();               // Thread initialization
  sync_barrier_thread_registration();
  __asm__("csrw 0x300, 0x8;" );       // each thread enables it's own interrupt

#if SINGLE_THREAD==1                  // Single Thread mode: SINGLE_THREAD macro defined in params.hpp
  if (Klessydra_get_coreID()==0)      // in this mode only thread zero works.
  {
  #endif
   #if PERFORMANCE_COUNTER==1         // Cycle count print: PERFORMANCE_COUNTER macro defined in params.hpp
      start_count();                  
   #endif

   init();                            // Model parameters are initialized in values.cpp

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
   //update(chosen_action,reward);       // Update model parameters starting from chosen action and obtained reward
   // Update 
   linucb_disjoint_policy_object.linucb_arms[chosen_action].reward_update(reward, ct);

   // Initial Address print:
   if (Klessydra_get_coreID() == 0)
     printf("i_addr: 0x%x\n",((unsigned)&linucb_disjoint_policy_object.linucb_arms[0] - 0x100000)/4);
  
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
  return 0;
  }
  else
    __asm__("wfi;");                   // In single thread execution mode, thread 1 and 2 are in WFI state
  #endif

  return 0;
}
