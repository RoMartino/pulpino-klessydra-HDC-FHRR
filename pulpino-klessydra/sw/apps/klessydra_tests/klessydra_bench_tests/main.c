// Copyright 2017 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.


#include <stdio.h>
#include "timer.h"
#include "utils.h"

#include "bench.h"

#define NUM_ITER 1
static unsigned samples[NUM_ITER];

extern void test_setup();
extern void test_clear();
extern void test_run(int);
extern int test_check();

//#ifndef PRINT_NUM_CYCLES
//#define PRINT_NUM_CYCLES 1 // to print the cycle count
//#endif

#define KLESSYDRA_PERF 0
#define RISCY_PERF 0

int perf0[3] = {0, 0, 0};
int final_perf0[3] = {777, 777, 777};
int *ptr_perf0[3];

int perf1[3] = {0, 0, 0};
int final_perf1[3] = {777, 777, 777};
int  *ptr_perf1[3];

int perf2[3] = {0, 0, 0};
int final_perf2[3] = {777, 777, 777};
int *ptr_perf2[3];

int perf3[3] = {0, 0, 0};
int final_perf3[3] = {777, 777, 777};
int *ptr_perf3[3];

int perf4[3] = {0, 0, 0};
int final_perf4[3] = {777, 777, 777};
int *ptr_perf4[3];

int perf5[3] = {0, 0, 0};
int final_perf5[3] = {777, 777, 777};
int *ptr_perf5[3];

int perf6[3] = {0, 0, 0};
int final_perf6[3] = {777, 777, 777};
int *ptr_perf6[3];

int perf7[3] = {0, 0, 0};
int final_perf7[3] = {777, 777, 777};
int *ptr_perf7[3];

int main() {

for (int i=0; i<3; i++){
  ptr_perf0[i] = &perf0[i];
  ptr_perf1[i] = &perf1[i];
  ptr_perf2[i] = &perf2[i];
  ptr_perf3[i] = &perf3[i];
  ptr_perf4[i] = &perf4[i];
  ptr_perf5[i] = &perf5[i];
  ptr_perf6[i] = &perf6[i];
  ptr_perf7[i] = &perf7[i];
}


__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt

int enable_perf_cnt = 0;
for (int j=0; j<3; j++) {
//if (Klessydra_get_coreID() == j) {
  //test_setup();
  //perf_start();

  for (int i = 0; i < NUM_ITER; ++i) {
    test_clear();

    //reset_timer();
    //start_timer();

    // ENABLE COUNTING -------------------------------------------------------------------------
    #if KLESSYDRA_PERF == 1
      final_perf0[j]=0;
      final_perf1[j]=0;
      final_perf2[j]=0;
      final_perf3[j]=0;
      final_perf4[j]=0;
      final_perf5[j]=0;
      final_perf6[j]=0;
      final_perf7[j]=0;
      __asm__("csrrw zero, mcycle, zero;"    // reset cycle count
          "csrrw zero, minstret, zero;"  // reset instruction count
          "csrrw zero, 0xB03, zero;"     // reset load store access stall count
          "csrrw zero, 0xB06, zero;"     // reset load count
          "csrrw zero, 0xB07, zero;"     // reset store count
          "csrrw zero, 0xB08, zero;"     // reset unconditional count
          "csrrw zero, 0xB09, zero;"     // reset branch count
          "csrrw zero, 0xB0A, zero;"     // reset taken branch count
          "li %[enable], 0x000003E7;"    // enable performance counters
          "csrrw zero, 0x7A0, %[enable]" // enable performance counters
          :
          :[enable] "r" (enable_perf_cnt)
      );
    #endif
    #if RISCY_PERF == 1
      // ENABLE COUNTING -------------------------------------------------------------------------
      int enable_perf_cnt = 0;
      __asm__("csrrw zero, 0x780, zero;"  // reset cycle count
          "csrrw zero, 0x781, zero;"  // reset instruction count
          "csrrw zero, 0x784, zero;"  // reset instruction wait count
          "csrrw zero, 0x785, zero;"  // reset memory load count
          "csrrw zero, 0x786, zero;"  // reset memory store count
          "csrrw zero, 0x787, zero;"  // reset uncoditional jump count
          "csrrw zero, 0x788, zero;"  // reset branch count
          "csrrw zero, 0x789, zero;"  // reset taken branch count
          "li %[enable], 0x000003F3;"  // 
          "csrrw zero, 0x7A0, %[enable]" // enable performance counters
          :
          :[enable] "r" (enable_perf_cnt)
      );
    #endif
    //------------------------------------------------------------------------------------------

    test_run(i);

    // DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
    #if KLESSYDRA_PERF == 1
      __asm__("csrrw zero, 0x7A0, 0x00000000;" // disable performance counters
          "csrrw %[perf0], mcycle, zero;"
          "sw %[perf0], 0(%[ptr_perf0]);"
          "csrrw %[perf1], minstret, zero;"
          "sw %[perf1], 0(%[ptr_perf1]);"
          "csrrw %[perf2], 0xB03, zero;"
          "sw %[perf2], 0(%[ptr_perf2]);"
          "csrrw %[perf3], 0xB06, zero;"
          "sw %[perf3], 0(%[ptr_perf3]);"
          "csrrw %[perf4], 0xB07, zero;"
          "sw %[perf4], 0(%[ptr_perf4]);"
          "csrrw %[perf5], 0xB08, zero;"
          "sw %[perf5], 0(%[ptr_perf5]);"
          "csrrw %[perf6], 0xB09, zero;"
          "sw %[perf6], 0(%[ptr_perf6]);"
          "csrrw %[perf7], 0xB0A, zero;"
          "sw %[perf7], 0(%[ptr_perf7]);"
          :
          :[perf0] "r" (perf0[j]), [ptr_perf0] "r" (ptr_perf0[j]),
           [perf1] "r" (perf1[j]), [ptr_perf1] "r" (ptr_perf1[j]),
           [perf2] "r" (perf2[j]), [ptr_perf2] "r" (ptr_perf2[j]),
           [perf3] "r" (perf3[j]), [ptr_perf3] "r" (ptr_perf3[j]),
           [perf4] "r" (perf4[j]), [ptr_perf4] "r" (ptr_perf4[j]),
           [perf5] "r" (perf5[j]), [ptr_perf5] "r" (ptr_perf5[j]),
           [perf6] "r" (perf6[j]), [ptr_perf6] "r" (ptr_perf6[j]),
           [perf7] "r" (perf7[j]), [ptr_perf7] "r" (ptr_perf7[j])
      );
    #endif
    //------------------------------------------------------------------------------------------
    #if RISCY_PERF == 1
    // -------------------------------------------------------------------------------------------------------------------
    // DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
      __asm__("csrrw zero, 0x7A0, 0x00000000;" // disable performance counters
          "csrrw %[perf0], 0x780, zero;"
          "sw %[perf0], 0(%[ptr_perf0]);"
          "csrrw %[perf1], 0x781, zero;"
          "sw %[perf1], 0(%[ptr_perf1]);"
          "csrrw %[perf2], 0x784, zero;"
          "sw %[perf2], 0(%[ptr_perf2]);"
          "csrrw %[perf3], 0x785, zero;"
          "sw %[perf3], 0(%[ptr_perf3]);"
          "csrrw %[perf4], 0x786, zero;"
          "sw %[perf4], 0(%[ptr_perf4]);"
          "csrrw %[perf5], 0x787, zero;"
          "sw %[perf5], 0(%[ptr_perf5]);"
          :
          :[perf0] "r" (perf0[j]), [ptr_perf0] "r" (ptr_perf0[j]),
           [perf1] "r" (perf1[j]), [ptr_perf1] "r" (ptr_perf1[j]),
           [perf2] "r" (perf2[j]), [ptr_perf2] "r" (ptr_perf2[j]),
           [perf3] "r" (perf3[j]), [ptr_perf3] "r" (ptr_perf3[j]),
           [perf4] "r" (perf4[j]), [ptr_perf4] "r" (ptr_perf4[j]),
           [perf5] "r" (perf5[j]), [ptr_perf5] "r" (ptr_perf5[j])
      );
      __asm__("csrrw %[perf6], 0x788, zero;"
          "sw %[perf6], 0(%[ptr_perf6]);"
          "csrrw %[perf7], 0x789, zero;"
          "sw %[perf7], 0(%[ptr_perf7]);"
          :
          :[perf6] "r" (perf6[j]), [ptr_perf6] "r" (ptr_perf6[j]),
           [perf7] "r" (perf7[j]), [ptr_perf7] "r" (ptr_perf7[j])
      );
    #endif
    final_perf0[j]=*(ptr_perf0[j]);
    final_perf1[j]=*(ptr_perf1[j]);
    final_perf2[j]=*(ptr_perf2[j]);
    final_perf3[j]=*(ptr_perf3[j]);
    final_perf4[j]=*(ptr_perf4[j]);
    final_perf5[j]=*(ptr_perf5[j]);
    final_perf6[j]=*(ptr_perf6[j]);
    final_perf7[j]=*(ptr_perf7[j]);

  if (Klessydra_get_coreID() == 0) {
    #if KLESSYDRA_PERF == 1 || RISCY_PERF == 1
    for (int i=0; i<1; i++) {
      printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Taken Count = %d \n \n", 
           final_perf0[i], final_perf1[i], final_perf2[i], final_perf3[i], final_perf4[i], final_perf5[i], final_perf6[i], final_perf7[i]);
    }
    printf("\n");
    #endif
  }
  else {
    __asm__("wfi;");
  }

    //stop_timer();

    //samples[i] = get_time();
  }

  //perf_stop();
  //int check = test_check();

  //printf("Correct: %d\n", check);
  //for (int i = 0; i < NUM_ITER; ++i)
  //  printf("TS[%d]: %d\n", i, samples[i]);

  //perf_print_all();

  return 0;
//}
//else {__asm__("wfi;");}
//}
}
}
