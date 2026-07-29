// Copyright 2017 ETH Zurich and University of Bologna.
// Copyright and related rights are licensed under the Solderpad Hardware
// License, Version 0.51 (the “License”); you may not use this file except in
// compliance with the License.  You may obtain a copy of the License at
// http://solderpad.org/licenses/SHL-0.51. Unless required by applicable law
// or agreed to in writing, software, hardware and materials distributed under
// this License is distributed on an “AS IS” BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.
#include "common.h"

#define NINPUTS 256

int buf[2*NINPUTS] __sram;

//short dataR1[NINPUTS] = {
int dataR1[NINPUTS] = {
  /* inputs for test 1 */
  2, -4, -3, -8, -10, -11, -23, 11, 32, 10, 11, 8, 3, 3, -7, -5, 1, -4, -4, -4,
  -9, -5, -4, -8, -5, -2, 0, 0, -6, -7, -2, 3, 3, 8, 15, 10, 6, 6, 1, 4, -1, 
  -10, -4, -2, -9, -5, -7, -8, -2, -5, -6, -2, -3, 1, -3, -8, -6, 0, 5, 4, 15,
  17, 6, 5, 2, 0, 2, -3, -5, 0, -5, -5, -4, -9, -6, -2, -4, -4, -3, -1, 1, -5,
  -7, -4, 3, 5, 6, 20, 16, 8, 7, 3, 7, 4, -5, -4, -3, -8, -6, -7, -7, -1, -2, 
  -2, -2, -3, 1, 1, -5, -4, 2, 6, 7, 13, 17, 8, 7, 6, 2, 7, 4, 0, -3, -6, -2, 
  -3, -7, -7, -4, -5, -4, -2, 1, 4, -2, -4, -1, 3, 5, 5, 18, 19, 9, 7, 2, 4, 2,
  -6, -5, 0, -1, -2, -5, -8, -2, -4, -7, -5, -4, 1, 0, -5, -4, 1, 3, 3, 7, 15,
  11, 6, 5, 2, 6, 3, -5, -4, -4, -7, -6, -9, -8, -3, -4, -5, -5, -4, 1, -3, -7,
  -5, 0, 4, 3, 12, 15, 7, 5, 4, 1, 1, -5, -7, -1, -2, -5, -4, -8, -7, -3, -6, 
  -6, -5, -3, 0, -5, -6, -3, 1, 2, 3, 13, 14, 9, 6, 3, 4, 3, -4, -6, -3, -5,
  -5, -6, -9, -5, -3, -5, -5, -3, 0, 0, -5, -3, 1, 3, 3, 9, 16, 10, 6, 6, 6, 8,
  2, -2, -2,
};

//short dataI1[NINPUTS] = {
int dataI1[NINPUTS] = {
  /* inputs for test 1 */
  1, -1, -1, -2, -2, -2, -3, -1, 0, 0, 1, 2, 2, 3, 3, 2, 2, 2, 2, 1, 0, 0, -1,
  -1, -2, -2, -2, -3, -3, -3, -3, -2, -2, -1, 0, 1, 1, 2, 3, 3, 3, 3, 2, 2, 1,
  1, 0, 0, -1, -1, -2, -2, -2, -2, -3, -3, -3, -2, -2, -1, 0, 1, 1, 2, 3, 3, 4,
  3, 3, 3, 2, 2, 1, 1, 0, 0, -1, -1, -2, -2, -3, -3, -3, -3, -2, -2, -1, 0, 1,
  2, 3, 3, 4, 4, 4, 3, 3, 2, 2, 1, 0, 0, -1, -1, -2, -2, -2, -3, -3, -3, -3,
  -2, -2, -1, 0, 1, 2, 2, 3, 3, 4, 3, 3, 3, 2, 2, 1, 0, -1, -1, -2, -2, -3, -3,
  -4, -3, -4, -3, -2, -2, 0, 1, 1, 2, 2, 3, 3, 3, 2, 2, 2, 2, 1, 0, 0, -1, -2,
  -2, -3, -3, -3, -4, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 3, 3, 3, 2, 2, 1,
  1, 0, 0, -1, -1, -2, -2, -2, -3, -3, -3, -3, -2, -2, -1, 0, 1, 2, 2, 3, 3, 3,
  2, 2, 2, 2, 1, 0, 0, 0, -1, -1, -2, -2, -3, -3, -3, -3, -3, -2, -2, 0, 1, 1,
  2, 3, 3, 4, 4, 3, 3, 3, 2, 1, 1, 0, 0, -1, -2, -2, -2, -3, -3, -3, -3, -2, 
  -2, -1, 0, 1, 2, 2, 3, 4, 4, 4, 3,
};

short ref[2*NINPUTS] = {
 /* outputs for test 1 */
  6, 13, -47, -11, 91, 38, 44, 30, 48, 21, 48, 13,
  71, 26, 92, 41, 162, 76, 598, 139, -1002, -60, -284, 59,
  -210, 40, -154, 65, -100, 67, -23, 35, -98, 92, -18, 96,
  -125, 53, -414, 121, 135, 80, 78, 37, 62, 57, 29, 19,
  40, -16, 84, 115, 23, -6, 52, 8, -3, -52, 283, 126,
  123, -38, 50, -63, 28, -16, 31, -78, 31, -36, -2, -62,
  -12, -54, -33, -43, -1, 60, -12, -141, -44, -49, -21, -68,
  -62, -59, -32, -61, -39, -23, -48, -13, -70, -14, -28, -7,
  5, -98, -28, 10, -50, -10, -32, 2, -42, 11, -4, 36,
  -65, 37, -9, 19, -52, 28, -94, 3, 228, 74, 53, 73,
  69, 22, 52, 52, 56, 8, 21, 80, 55, 6, 41, 5,
  54, 21, 95, 83, 8, -75, -6, -27, 23, -32, 14, -27,
  20, -34, -2, -57, -2, -28, -7, -32, -11, -21, 19, -70,
  -20, -7, -16, -32, -25, -15, -27, -17, -21, -13, -25, 0,
  -10, -7, 17, -20, 48, -41, -154, 48, -59, 75, -44, 45,
  -19, 42, 1, 39, -18, 33, -2, 43, -1, 36, 30, 33,
  48, 150, 46, -33, 10, -11, 17, 5, 23, 9, 36, 2,
  29, 2, 22, -9, -1, -16, 11, -8, 47, -38, -1, -14,
  2, -20, 7, 4, 9, -25, -2, 7, 5, -30, -5, -1,
  8, 0, 14, -18, -7, 0, -6, 2, 10, -10, -4, 8,
  7, -3, 8, -3, 9, 7, -8, 10, 2, -3, 12, 8,
  19, -7, -1, -4, -2, -9, -3, 9, -3, 6, 18, -2,
  10, -1, 2, -1, -1, -6, 0, 5, -4, 10, 4, 1,
  0, -10, -6, 7, -4, 4, 8, 21, 8, -9, 3, 19,
  4, 32, 14, -6, -1, 29, 0, 13, 11, 22, 16, 9,
  56, 55, 5, -11, 2, 28, 22, 9, 25, 8, 22, 12,
  17, -2, 13, -6, 7, 10, 40, 31, 72, -156, 39, -36,
  5, -32, 12, -46, -16, -44, 17, -55, -21, -48, -22, -35,
  -50, -65, -141, -52, 40, 37, 12, 17, -21, -15, -16, -15,
  -28, 6, -10, 3, -21, 24, -14, 18, -20, 4, 8, 62,
  -12, 11, -6, 38, -9, 31, 1, 75, 24, 38, 12, 37,
  26, 38, -11, 31, 5, 80, 98, -86, 64, -30, 31, -18,
  61, -10, 21, -63, 50, -9, 55, -55, 66, -13, 48, -53,
  219, -56, -86, -15, -58, -36, -12, -19, -56, -42, 5, -34,
  -27, -27, -17, 4, -48, -3, -20, -6, -1, 83, -29, 3,
  -65, 7, -45, 10, -44, 29, -23, 62, -58, 49, -28, 52,
  -39, 49, -14, 147, 27, -68, -7, 45, -8, 41, 10, 65,
  31, 40, 32, 63, 36, 6, 56, 71, 122, 39, 289, -121,
  1, 45, 56, 4, 33, 6, 96, -99, 47, 10, 43, -16,
  67, -40, 81, -38, 126, -58, -303, -140, -63, -55, 8, -75,
  -45, -88, 3, -31, -38, -54, -66, -54, -79, -34, -112, -52,
  -396, -18, 216, -26, 69, -15, 42, -17, 25, -15, 34, -26,
  40, 1, 38, -10, 141, -27, -83, 30,
};

extern void fft(int *, int);

void test_setup() {
}

void test_clear() {
  for (int i = 0; i < NINPUTS; ++i) {
    buf[2 * i] = dataR1[i];
    buf[2 * i + 1] = dataI1[i];
  }
}

void test_run(int n) {
  fft(buf, NINPUTS);
}

int test_check() {
  for (int i = 0; i != 2 * NINPUTS; ++i)
    if (buf[i] != ref[i])
      return 0;

  return 1;
}


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsp_functions.h"
#include "functions.h"
#include "klessydra_defs.h"

#include "common.h"

#define NINPUTS 256

#define Debug_en 1

#ifdef Debug_en

short ref_res[2*NINPUTS] = {
   6, 13, -47, -11, 91, 38, 44, 30, 48, 21, 48, 13,
  71, 26, 92, 41, 162, 76, 598, 139, -1002, -60, -284, 59,
  -210, 40, -154, 65, -100, 67, -23, 35, -98, 92, -18, 96,
  -125, 53, -414, 121, 135, 80, 78, 37, 62, 57, 29, 19,
  40, -16, 84, 115, 23, -6, 52, 8, -3, -52, 283, 126,
  123, -38, 50, -63, 28, -16, 31, -78, 31, -36, -2, -62,
  -12, -54, -33, -43, -1, 60, -12, -141, -44, -49, -21, -68,
  -62, -59, -32, -61, -39, -23, -48, -13, -70, -14, -28, -7,
  5, -98, -28, 10, -50, -10, -32, 2, -42, 11, -4, 36,
  -65, 37, -9, 19, -52, 28, -94, 3, 228, 74, 53, 73,
  69, 22, 52, 52, 56, 8, 21, 80, 55, 6, 41, 5,
  54, 21, 95, 83, 8, -75, -6, -27, 23, -32, 14, -27,
  20, -34, -2, -57, -2, -28, -7, -32, -11, -21, 19, -70,
  -20, -7, -16, -32, -25, -15, -27, -17, -21, -13, -25, 0,
  -10, -7, 17, -20, 48, -41, -154, 48, -59, 75, -44, 45,
  -19, 42, 1, 39, -18, 33, -2, 43, -1, 36, 30, 33,
  48, 150, 46, -33, 10, -11, 17, 5, 23, 9, 36, 2,
  29, 2, 22, -9, -1, -16, 11, -8, 47, -38, -1, -14,
  2, -20, 7, 4, 9, -25, -2, 7, 5, -30, -5, -1,
  8, 0, 14, -18, -7, 0, -6, 2, 10, -10, -4, 8,
  7, -3, 8, -3, 9, 7, -8, 10, 2, -3, 12, 8,
  19, -7, -1, -4, -2, -9, -3, 9, -3, 6, 18, -2,
  10, -1, 2, -1, -1, -6, 0, 5, -4, 10, 4, 1,
  0, -10, -6, 7, -4, 4, 8, 21, 8, -9, 3, 19,
  4, 32, 14, -6, -1, 29, 0, 13, 11, 22, 16, 9,
  56, 55, 5, -11, 2, 28, 22, 9, 25, 8, 22, 12,
  17, -2, 13, -6, 7, 10, 40, 31, 72, -156, 39, -36,
  5, -32, 12, -46, -16, -44, 17, -55, -21, -48, -22, -35,
  -50, -65, -141, -52, 40, 37, 12, 17, -21, -15, -16, -15,
  -28, 6, -10, 3, -21, 24, -14, 18, -20, 4, 8, 62,
  -12, 11, -6, 38, -9, 31, 1, 75, 24, 38, 12, 37,
  26, 38, -11, 31, 5, 80, 98, -86, 64, -30, 31, -18,
  61, -10, 21, -63, 50, -9, 55, -55, 66, -13, 48, -53,
  219, -56, -86, -15, -58, -36, -12, -19, -56, -42, 5, -34,
  -27, -27, -17, 4, -48, -3, -20, -6, -1, 83, -29, 3,
  -65, 7, -45, 10, -44, 29, -23, 62, -58, 49, -28, 52,
  -39, 49, -14, 147, 27, -68, -7, 45, -8, 41, 10, 65,
  31, 40, 32, 63, 36, 6, 56, 71, 122, 39, 289, -121,
  1, 45, 56, 4, 33, 6, 96, -99, 47, 10, 43, -16,
  67, -40, 81, -38, 126, -58, -303, -140, -63, -55, 8, -75,
  -45, -88, 3, -31, -38, -54, -66, -54, -79, -34, -112, -52,
  -396, -18, 216, -26, 69, -15, 42, -17, 25, -15, 34, -26,
  40, 1, 38, -10, 141, -27, -83, 30,
};

#endif

#ifndef local_result
  #define local_result 1
#endif

//short dataR1[NINPUTS] = {
int dataR[NINPUTS] = {
  /* inputs for test 1 */
  2, -4, -3, -8, -10, -11, -23, 11, 32, 10, 11, 8, 3, 3, -7, -5, 1, -4, -4, -4,
  -9, -5, -4, -8, -5, -2, 0, 0, -6, -7, -2, 3, 3, 8, 15, 10, 6, 6, 1, 4, -1, 
  -10, -4, -2, -9, -5, -7, -8, -2, -5, -6, -2, -3, 1, -3, -8, -6, 0, 5, 4, 15,
  17, 6, 5, 2, 0, 2, -3, -5, 0, -5, -5, -4, -9, -6, -2, -4, -4, -3, -1, 1, -5,
  -7, -4, 3, 5, 6, 20, 16, 8, 7, 3, 7, 4, -5, -4, -3, -8, -6, -7, -7, -1, -2, 
  -2, -2, -3, 1, 1, -5, -4, 2, 6, 7, 13, 17, 8, 7, 6, 2, 7, 4, 0, -3, -6, -2, 
  -3, -7, -7, -4, -5, -4, -2, 1, 4, -2, -4, -1, 3, 5, 5, 18, 19, 9, 7, 2, 4, 2,
  -6, -5, 0, -1, -2, -5, -8, -2, -4, -7, -5, -4, 1, 0, -5, -4, 1, 3, 3, 7, 15,
  11, 6, 5, 2, 6, 3, -5, -4, -4, -7, -6, -9, -8, -3, -4, -5, -5, -4, 1, -3, -7,
  -5, 0, 4, 3, 12, 15, 7, 5, 4, 1, 1, -5, -7, -1, -2, -5, -4, -8, -7, -3, -6, 
  -6, -5, -3, 0, -5, -6, -3, 1, 2, 3, 13, 14, 9, 6, 3, 4, 3, -4, -6, -3, -5,
  -5, -6, -9, -5, -3, -5, -5, -3, 0, 0, -5, -3, 1, 3, 3, 9, 16, 10, 6, 6, 6, 8,
  2, -2, -2,
};

//short dataI1[NINPUTS] = {
int dataI[NINPUTS] = {
  /* inputs for test 1 */
  1, -1, -1, -2, -2, -2, -3, -1, 0, 0, 1, 2, 2, 3, 3, 2, 2, 2, 2, 1, 0, 0, -1,
  -1, -2, -2, -2, -3, -3, -3, -3, -2, -2, -1, 0, 1, 1, 2, 3, 3, 3, 3, 2, 2, 1,
  1, 0, 0, -1, -1, -2, -2, -2, -2, -3, -3, -3, -2, -2, -1, 0, 1, 1, 2, 3, 3, 4,
  3, 3, 3, 2, 2, 1, 1, 0, 0, -1, -1, -2, -2, -3, -3, -3, -3, -2, -2, -1, 0, 1,
  2, 3, 3, 4, 4, 4, 3, 3, 2, 2, 1, 0, 0, -1, -1, -2, -2, -2, -3, -3, -3, -3,
  -2, -2, -1, 0, 1, 2, 2, 3, 3, 4, 3, 3, 3, 2, 2, 1, 0, -1, -1, -2, -2, -3, -3,
  -4, -3, -4, -3, -2, -2, 0, 1, 1, 2, 2, 3, 3, 3, 2, 2, 2, 2, 1, 0, 0, -1, -2,
  -2, -3, -3, -3, -4, -4, -3, -3, -2, -1, 0, 0, 1, 2, 2, 3, 3, 3, 3, 2, 2, 1,
  1, 0, 0, -1, -1, -2, -2, -2, -3, -3, -3, -3, -2, -2, -1, 0, 1, 2, 2, 3, 3, 3,
  2, 2, 2, 2, 1, 0, 0, 0, -1, -1, -2, -2, -3, -3, -3, -3, -3, -2, -2, 0, 1, 1,
  2, 3, 3, 4, 4, 3, 3, 3, 2, 1, 1, 0, 0, -1, -2, -2, -2, -3, -3, -3, -3, -2, 
  -2, -1, 0, 1, 2, 2, 3, 4, 4, 4, 3,
};


static int wprBase128_mem[] __sram = {
  32767,  32758,  32729,  32679,  32610,  32522,  32413,  32286,
  32138,  31972,  31786,  31581,  31357,  31114,  30853,  30572,
  30274,  29957,  29622,  29269,  28899,  28511,  28106,  27684,
  27246,  26791,  26320,  25833,  25330,  24812,  24279,  23732,
  23170,  22595,  22006,  21403,  20788,  20160,  19520,  18868,
  18205,  17531,  16846,  16151,  15447,  14733,  14010,  13279,
  12540,  11793,  11039,  10279,   9512,   8740,   7962,   7180,
   6393,   5602,   4808,   4011,   3212,   2411,   1608,    804,
      0,   -804,  -1608,  -2411,  -3212,  -4011,  -4808,  -5602,
  -6393,  -7180,  -7962,  -8740,  -9512, -10279, -11039, -11793,
 -12540, -13279, -14010, -14733, -15447, -16151, -16846, -17531,
 -18205, -18868, -19520, -20160, -20788, -21403, -22006, -22595,
 -23170, -23732, -24279, -24812, -25330, -25833, -26320, -26791,
 -27246, -27684, -28106, -28511, -28899, -29269, -29622, -29957,
 -30274, -30572, -30853, -31114, -31357, -31581, -31786, -31972,
 -32138, -32286, -32413, -32522, -32610, -32679, -32729, -32758,
};

/*
int wprBase64_mem[64] __sram;
int wprBase32_mem[32] __sram;
int wprBase16_mem[16] __sram;
int wprBase8_mem[8]  __sram;
int wprBase4_mem[4]  __sram;
int wprBase2_mem[2]  __sram;
*/

///*
static int wprBase64_mem[] __sram = {
  32767,  32729,  32610,  32413,
  32138,  31786,  31357,  30853,
  30274,  29622,  28899,  28106,
  27246,  26320,  25330,  24279,
  23170,  22006,  20788,  19520,
  18205,  16846,  15447,  14010,
  12540,  11039,   9512,   7962,
   6393,   4808,   3212,   1608,
      0,  -1608,  -3212,  -4808,
  -6393,  -7962,  -9512, -11039,
 -12540, -14010, -15447, -16846,
 -18205, -19520, -20788, -22006,
 -23170, -24279, -25330, -26320,
 -27246, -28106, -28899, -29622,
 -30274, -30853, -31357, -31786,
 -32138, -32413, -32610, -32729,
};

static int wprBase32_mem[] __sram = {
  32767,  32610,
  32138,  31357,
  30274,  28899,
  27246,  25330,
  23170,  20788,
  18205,  15447,
  12540,   9512,
   6393,   3212,
      0,  -3212,
  -6393,  -9512,
 -12540, -15447,
 -18205, -20788,
 -23170, -25330,
 -27246, -28899,
 -30274, -31357,
 -32138, -32610,
};

static int wprBase16_mem[] __sram = {
  32767,32138,30274,27246,23170,18205,12540,6393,0,-6393,-12540,-18205,-23170,-27246,-30274,-32138,
};

static int wprBase8_mem[] __sram = {
  32767,30274,23170,12540,0,-12540,-23170,-30274,
};

static int wprBase4_mem[] __sram = {
  32767,23170,0,-23170,
};

static int wprBase2_mem[] __sram = {
  32767,0,
};
//*/

static int wpiBase128_mem[] __sram = {
      0,    804,   1608,   2411,   3212,   4011,   4808,   5602,
   6393,   7180,   7962,   8740,   9512,  10279,  11039,  11793,
  12540,  13279,  14010,  14733,  15447,  16151,  16846,  17531,
  18205,  18868,  19520,  20160,  20788,  21403,  22006,  22595,
  23170,  23732,  24279,  24812,  25330,  25833,  26320,  26791,
  27246,  27684,  28106,  28511,  28899,  29269,  29622,  29957,
  30274,  30572,  30853,  31114,  31357,  31581,  31786,  31972,
  32138,  32286,  32413,  32522,  32610,  32679,  32729,  32758,
  32767,  32758,  32729,  32679,  32610,  32522,  32413,  32286,
  32138,  31972,  31786,  31581,  31357,  31114,  30853,  30572,
  30274,  29957,  29622,  29269,  28899,  28511,  28106,  27684,
  27246,  26791,  26320,  25833,  25330,  24812,  24279,  23732,
  23170,  22595,  22006,  21403,  20788,  20160,  19520,  18868,
  18205,  17531,  16846,  16151,  15447,  14733,  14010,  13279,
  12540,  11793,  11039,  10279,   9512,   8740,   7962,   7180,
   6393,   5602,   4808,   4011,   3212,   2411,   1608,    804,
};

/*
int wpiBase64_mem[64] __sram;
int wpiBase32_mem[32] __sram;
int wpiBase16_mem[16] __sram;
int wpiBase8_mem[8]  __sram;
int wpiBase4_mem[4]  __sram;
int wpiBase2_mem[2]  __sram;
*/

///*
static int wpiBase64_mem[] __sram = {
      0,   1608,   3212,   4808,
   6393,   7962,   9512,  11039,
  12540,  14010,  15447,  16846,
  18205,  19520,  20788,  22006,
  23170,  24279,  25330,  26320,
  27246,  28106,  28899,  29622,
  30274,  30853,  31357,  31786,
  32138,  32413,  32610,  32729,
  32767,  32729,  32610,  32413,
  32138,  31786,  31357,  30853,
  30274,  29622,  28899,  28106,
  27246,  26320,  25330,  24279,
  23170,  22006,  20788,  19520,
  18205,  16846,  15447,  14010,
  12540,  11039,   9512,   7962,
   6393,   4808,   3212,   1608,
};

static int wpiBase32_mem[] __sram = {
      0,   3212,
   6393,   9512,
  12540,  15447,
  18205,  20788,
  23170,  25330,
  27246,  28899,
  30274,  31357,
  32138,  32610,
  32767,  32610,
  32138,  31357,
  30274,  28899,
  27246,  25330,
  23170,  20788,
  18205,  15447,
  12540,   9512,
   6393,   3212,
};

static int wpiBase16_mem[] __sram = {
 0, 6393,12540,18205,23170,27246,30274,32138,32767,32138,30274,27246,23170,18205,12540, 6393,
};

static int wpiBase8_mem[] __sram = {
 0,12540,23170,30274,32767,30274,23170,12540,
};

static int wpiBase4_mem[] __sram = {
 0,23170,32767,23170,
};

static int wpiBase2_mem[] __sram = {
 0,32767,
};
//*/

#define SPM_A spmaddrA
#define SPM_B spmaddrB
#define SPM_C spmaddrC
#define SPM_D spmaddrD

// SPM_A offsets
int offset_i_e      = 0x0000;
int offset_i_o      = 0x0400;
int a1_offset       = 0x0800;
int b1_offset       = 0x0A00;
int a_offset        = 0x0C00;
int final_e1_offset = 0x3000;
int final_e2_offset = 0x3200;
int final_o1_offset = 0x3400;
int final_o2_offset = 0x3600;

// SPM_B offsets
int offset_j_e = 0x0000;
int offset_j_o = 0x0400;
int a2_offset  = 0x0800;
int b2_offset  = 0x0A00;
int b_offset   = 0x0C00;

// SPM_C offsets
int tr_offset = 0x0000;
int ti_offset = 0x0200;
int xr_offset = 0x0400;
int xi_offset = 0x0600;


// SPM_D offsets
int wpr128_offset = 0x0000;
int wpi128_offset = 0x0200;
int wpr64_offset  = 0x0400;
int wpi64_offset  = 0x0500;
int wpr32_offset  = 0x0600;
int wpi32_offset  = 0x0680;
int wpr16_offset  = 0x0700;
int wpi16_offset  = 0x0740;
int wpr8_offset   = 0x0780;
int wpi8_offset   = 0x07A0;
int wpr4_offset   = 0x07C0;
int wpi4_offset   = 0x07D0;
int wpr2_offset   = 0x07E0;
int wpi2_offset   = 0x07E8;

void debug_fft(int hart_ID, int* data_R_ptr, int* data_I_ptr);


//  ███████╗███████╗████████╗
//  ██╔════╝██╔════╝╚══██╔══╝
//  █████╗  █████╗     ██║   
//  ██╔══╝  ██╔══╝     ██║   
//  ██║     ██║        ██║   
//  ╚═╝     ╚═╝        ╚═╝   

#ifdef local_result == 0
  int data_Rf[256*3];
  int data_If[256*3];                              
#endif

void fft(int *data, int len) {

  #ifdef local_result == 1
    int data_Rf_local[256*3];
    int data_If_local[256*3];
  #endif

  int hart_ID = Klessydra_get_coreID();

  #if Debug_en == 1
    if (hart_ID == 0) {
      printf("\nAccelerated FFT\n");
    }
  #endif

  int vect_i_e = hart_ID*0x1000 + SPM_A+offset_i_e;
  int vect_i_o = hart_ID*0x1000 + SPM_A+offset_i_o;
  int vect_j_e = hart_ID*0x1000 + SPM_B+offset_j_e;
  int vect_j_o = hart_ID*0x1000 + SPM_B+offset_j_o;

  int wprBase128 = hart_ID*0x800 + SPM_D+wpr128_offset;
  int wpiBase128 = hart_ID*0x800 + SPM_D+wpi128_offset;
  int wprBase64  = hart_ID*0x800 + SPM_D+wpr64_offset;
  int wpiBase64  = hart_ID*0x800 + SPM_D+wpi64_offset;
  int wprBase32  = hart_ID*0x800 + SPM_D+wpr32_offset;
  int wpiBase32  = hart_ID*0x800 + SPM_D+wpi32_offset;
  int wprBase16  = hart_ID*0x800 + SPM_D+wpr16_offset;
  int wpiBase16  = hart_ID*0x800 + SPM_D+wpi16_offset;
  int wprBase8   = hart_ID*0x800 + SPM_D+wpr8_offset;
  int wpiBase8   = hart_ID*0x800 + SPM_D+wpi8_offset;
  int wprBase4   = hart_ID*0x800 + SPM_D+wpr4_offset;
  int wpiBase4   = hart_ID*0x800 + SPM_D+wpi4_offset;
  int wprBase2   = hart_ID*0x800 + SPM_D+wpr2_offset;
  int wpiBase2   = hart_ID*0x800 + SPM_D+wpi2_offset;

  // Base addresses
  int xr = SPM_C + xr_offset;
  int xi = SPM_C + xi_offset;
  int tr = SPM_C + tr_offset;
  int ti = SPM_C + ti_offset;
  int a1 = SPM_A + a1_offset;
  int a2 = SPM_B + a2_offset;
  int a  = SPM_A + a_offset;
  int b1 = SPM_A + b1_offset;
  int b2 = SPM_B + b2_offset;
  int b  = SPM_B + b_offset;

  /*
  int k=0;
  for (int i=0; i<128; i+=2) {
    wprBase64_mem[k] =  wprBase128_mem[i];
    wpiBase64_mem[k] =  wpiBase128_mem[i];
    k++;
  }
  k=0;
  for (int i=0; i<64; i+=2) {
    wprBase32_mem[k] =  wprBase64_mem[i];
    wpiBase32_mem[k] =  wpiBase64_mem[i];
    k++;
  }
  k=0;
  for (int i=0; i<32; i+=2) {
    wprBase16_mem[k] =  wprBase32_mem[i];
    wpiBase16_mem[k] =  wpiBase32_mem[i];
    k++;
  }
  k=0;
  for (int i=0; i<16; i+=2) {
    wprBase8_mem[k] =  wprBase16_mem[i];
    wpiBase8_mem[k] =  wpiBase16_mem[i];
    k++;
  }
  k=0;
  for (int i=0; i<8; i+=2) {
    wprBase4_mem[k] =  wprBase8_mem[i];
    wpiBase4_mem[k] =  wpiBase8_mem[i];
    k++;
  }
  k=0;
  for (int i=0; i<4; i+=2) {
    wprBase2_mem[k] =  wprBase4_mem[i];
    wpiBase2_mem[k] =  wpiBase4_mem[i];
    k++;
  }
  */

  int max = len;
  len <<= 1;

  kmemld((void*) vect_i_e, (void*) dataR,   4*NINPUTS/2);
  kmemld((void*) vect_j_e + (4*NINPUTS/2), (void*) dataR + (4*NINPUTS/2), 4*NINPUTS/2);
  kmemld((void*) vect_i_o, (void*) dataI,   4*NINPUTS/2);
  kmemld((void*) vect_j_o + (4*NINPUTS/2), (void*) dataI + (4*NINPUTS/2), 4*NINPUTS/2);

  kmemld((void*) wprBase128, (void*) wprBase128_mem, 4*128);
  kmemld((void*) wpiBase128, (void*) wpiBase128_mem, 4*128);
  kmemld((void*) wprBase64,  (void*) wprBase64_mem,  4*64);
  kmemld((void*) wpiBase64,  (void*) wpiBase64_mem,  4*64);
  kmemld((void*) wprBase32,  (void*) wprBase32_mem,  4*32);
  kmemld((void*) wpiBase32,  (void*) wpiBase32_mem,  4*32);
  kmemld((void*) wprBase16,  (void*) wprBase16_mem,  4*16);
  kmemld((void*) wpiBase16,  (void*) wpiBase16_mem,  4*16);
  kmemld((void*) wprBase8,   (void*) wprBase8_mem,   4*8);
  kmemld((void*) wpiBase8,   (void*) wpiBase8_mem,   4*8);
  kmemld((void*) wprBase4,   (void*) wprBase4_mem,   4*4);
  kmemld((void*) wpiBase4,   (void*) wpiBase4_mem,   4*4);
  kmemld((void*) wprBase2,   (void*) wprBase2_mem,   4*2);
  kmemld((void*) wpiBase2,   (void*) wpiBase2_mem,   4*2);


  #if Debug_en == 1
    int while_loop_cnt = 0;
  #endif

  while (max > 2) {

    int *wpr;
    int *wpi;

    if (max == 256) { 
      wpr = (int*) wprBase128;
      wpi = (int*) wpiBase128;
    }
    else if (max == 128) { 
      wpr = (int*) wprBase64;
      wpi = (int*) wpiBase64;
    }
    else if (max == 64) {
      wpr = (int*) wprBase32;
      wpi = (int*) wpiBase32;
    }
    else if (max == 32) {
      wpr = (int*) wprBase16;
      wpi = (int*) wpiBase16;
    }
    else if (max == 16) {
      wpr = (int*) wprBase8;
      wpi = (int*) wpiBase8;
    }
    else if (max == 8) {
      wpr = (int*) wprBase4;
      wpi = (int*) wpiBase4;
    }
    else if (max == 4) {
      wpr = (int*) wprBase2;
      wpi = (int*) wpiBase2;
    }

    CSR_MVSIZE(4*max/2); // max/2 since data[i] and data[i+1] are divided into two different vectors so m+=2 becomes m+=1

    int step = max/2 << 1;

    for (int i = 0; i < len/2; i += step) {

      // i increments by a value being a multiple of step

      // loop varying addresses
      int vect_i_e_pos = vect_i_e + 4*i;
      int vect_i_o_pos = vect_i_o + 4*i;
      int vect_j_e_pos = vect_j_e + 4*i + 4*(max/2); // Addresses have always byte percision hence 4*(max/2) instead of max/2
      int vect_j_o_pos = vect_j_o + 4*i + 4*(max/2); // Addresses have always byte percision hence 4*(max/2) instead of max/2

      int a1_pos = hart_ID*0x1000 + a1 + 4*(i/2);
      int a2_pos = hart_ID*0x1000 + a2 + 4*(i/2);
      int a_pos  = hart_ID*0x1000 + a  + 4*(i/2); 

      int b1_pos = hart_ID*0x1000 + b1 + 4*(i/2);
      int b2_pos = hart_ID*0x1000 + b2 + 4*(i/2);
      int b_pos  = hart_ID*0x1000 + b  + 4*(i/2);

      int tr_pos = hart_ID*0x800 + tr + 4*(i/2);
      int ti_pos = hart_ID*0x800 + ti + 4*(i/2);

      int xr_pos  = hart_ID*0x800 + xr + 4*(i/2);
      int xi_pos  = hart_ID*0x800 + xi + 4*(i/2);

      // short tr = data[i]   - data[j];
      // short ti = data[i+1] - data[j+1];

      ksubv((void*) tr_pos, (void*) vect_i_e_pos, (void*) vect_j_e_pos); // spm_c = spm_a + spm_b
      ksubv((void*) ti_pos, (void*) vect_i_o_pos, (void*) vect_j_o_pos); // spm_c = spm_a + spm_b

      // int xr = ((wr * tr + wi * ti) << 1) + 0x8000;
      // int xi = ((wr * ti - wi * tr) << 1) + 0x8000;

      kvmul((void*)    a1_pos, (void*) wpr,    (void*) tr_pos); // spm_a <= spm_d * spm_c
      kvmul((void*)    a2_pos, (void*) wpi,    (void*) ti_pos); // spm_b <= spm_d * spm_c
      kaddv((void*)    a_pos,  (void*) a1_pos, (void*) a2_pos); // spm_a <= spm_a + spm_b
      kvmul((void*)    b1_pos, (void*) wpr,    (void*) ti_pos); // spm_a <= spm_d * spm_c
      kvmul((void*)    b2_pos, (void*) wpi,    (void*) tr_pos); // spm_b <= spm_d * spm_c
      ksubv((void*)    b_pos,  (void*) b1_pos, (void*) b2_pos); // spm_b <= spm_a - spm_b
      ksvmulrf((void*) xr_pos, (void*) a_pos,  (void*) 2);      // spm_c <= spm_a * scal
      ksvaddrf((void*) xr_pos, (void*) xr_pos, (void*) 0x8000); // spm_c <= spm_c + scal
      ksvmulrf((void*) xi_pos, (void*) b_pos,  (void*) 2);      // spm_c <= spm_b + scal
      ksvaddrf((void*) xi_pos, (void*) xi_pos, (void*) 0x8000); // spm_c <= spm_c + scal

      //  data[i] += data[j];
      //  data[i+1] += data[j+1];
      //  data[j] = xr >> 16;
      //  data[j+1] = xi >> 16;

      CSR_MVSIZE(4*max/4);
      if (max == 4) {
        int vect_i_e_final = vect_i_e + 2*i;
        int vect_i_o_final = vect_i_o + 2*i;
        int vect_j_e_final = vect_j_e + 2*i;
        int vect_j_o_final = vect_j_o + 2*i;
        kaddv((void*) vect_i_e_final, (void*) vect_i_e_pos, (void*) vect_j_e_pos);
        kaddv((void*) vect_j_e_final, (void*) vect_i_e_pos+4, (void*) vect_j_e_pos+4); 
        ksrav((void*) vect_i_e_final+4, (void*) xr_pos, (void*) 16);
        ksrav((void*) vect_j_e_final+4, (void*) xr_pos+4, (void*) 16);
        kaddv((void*) vect_i_o_final, (void*) vect_i_o_pos, (void*) vect_j_o_pos);
        kaddv((void*) vect_j_o_final, (void*) vect_i_o_pos+4, (void*) vect_j_o_pos+4);
        ksrav((void*) vect_i_o_final+4, (void*) xi_pos, (void*) 16);
        ksrav((void*) vect_j_o_final+4, (void*) xi_pos+4, (void*) 16);
      }
      else {
        kaddv((void*) vect_i_e_pos, (void*) vect_i_e_pos, (void*) vect_j_e_pos);
        kaddv((void*) vect_j_e_pos-4*(max/4), (void*) vect_i_e_pos+4*(max/4), (void*) vect_j_e_pos+4*(max/4)); 
        ksrav((void*) vect_i_e_pos+4*(2*(max/4)), (void*) xr_pos, (void*) 16);
        ksrav((void*) vect_j_e_pos+4*(max/4), (void*) xr_pos+4*(max/4), (void*) 16);
        kaddv((void*) vect_i_o_pos, (void*) vect_i_o_pos, (void*) vect_j_o_pos);
        kaddv((void*) vect_j_o_pos-4*(max/4), (void*) vect_i_o_pos+4*(max/4), (void*) vect_j_o_pos+4*(max/4));
        ksrav((void*) vect_i_o_pos+4*(2*(max/4)), (void*) xi_pos, (void*) 16);
        ksrav((void*) vect_j_o_pos+4*(max/4), (void*) xi_pos+4*(max/4), (void*) 16);
      }
      CSR_MVSIZE(4*max/2);
    }
    max >>= 1;
  }

  {
    int step = max/2 << 1;

    CSR_MVSIZE(4*128);

    ksubv((void*) tr, (void*) vect_i_e, (void*) vect_j_e); // spm_c = spm_a + spm_b
    ksubv((void*) ti, (void*) vect_i_o, (void*) vect_j_o); // spm_c = spm_a + spm_b

    kaddv((void*) vect_i_e, (void*) vect_i_e, (void*) vect_j_e);
    kaddv((void*) vect_i_o, (void*) vect_i_o, (void*) vect_j_o);
  }
  
  vect_j_e = tr;
  vect_j_o = ti;

  #if local_result == 1
    int *data_R_ptr = data_Rf_local+256*Klessydra_get_coreID();
    int *data_I_ptr = data_If_local+256*Klessydra_get_coreID();
  #else
    int *data_R_ptr = data_Rf+256*Klessydra_get_coreID();
    int *data_I_ptr = data_If+256*Klessydra_get_coreID();
  #endif


  kmemstr((void*) data_R_ptr,        (void*) vect_i_e, 4*128);
  kmemstr((void*) data_I_ptr,        (void*) vect_i_o, 4*128);
  kmemstr((void*) data_R_ptr+4*128,  (void*) vect_j_e, 4*128);
  kmemstr((void*) data_I_ptr+4*128,  (void*) vect_j_o, 4*128);

  #define SWAP(a, b) tmp=(a); (a)=(b); (b)=tmp

  int j = 1;
  for (int i = 1; i < len/2; i += 2) {
    if(j > i) {
      int tmp;

      int k = (i+1) & 3; // if k = 2, then we swap with the j_even vector else if k = 0 we swap with the i_even vector
      int l = (j+1) & 3; // if l = 2, then we swap with the j_odd vector else if l = 0 we swap with the i_odd vector
      int i_new = (i-1)/2;
      int j_new = ((j+1) >> 2) + ((l>>1))-1;

      if(j_new > i_new) {
        SWAP(data_R_ptr[j_new], data_R_ptr[i_new]);
        SWAP(data_I_ptr[j_new], data_I_ptr[i_new]);
        SWAP(data_R_ptr[j_new+128], data_R_ptr[i_new+128]);
        SWAP(data_I_ptr[j_new+128], data_I_ptr[i_new+128]);
      }
      //SWAP(data[j], data[i]);
      //SWAP(data[j+1], data[i+1]);
    }
    int m = len>> 1;
    while (m >= 2 && j >m) {
      j -= m;
      m >>= 1;
    }
    j += m;
  }
  #if Debug_en == 1
    debug_fft(hart_ID, data_R_ptr, data_I_ptr);
  #endif
}


//  ██████╗ ███████╗██████╗ ██╗   ██╗ ██████╗     ███████╗███████╗████████╗
//  ██╔══██╗██╔════╝██╔══██╗██║   ██║██╔════╝     ██╔════╝██╔════╝╚══██╔══╝
//  ██║  ██║█████╗  ██████╔╝██║   ██║██║  ███╗    █████╗  █████╗     ██║   
//  ██║  ██║██╔══╝  ██╔══██╗██║   ██║██║   ██║    ██╔══╝  ██╔══╝     ██║   
//  ██████╔╝███████╗██████╔╝╚██████╔╝╚██████╔╝    ██║     ██║        ██║   
//  ╚═════╝ ╚══════╝╚═════╝  ╚═════╝  ╚═════╝     ╚═╝     ╚═╝        ╚═╝   
                                                                
#if Debug_en == 1
void debug_fft(int hart_ID, int* data_R_ptr, int* data_I_ptr) {

  if (hart_ID == 0){
    int errors = 0;
    for (int i=0; i<256; i++) {
      if (data_R_ptr[i] != ref_res[2*i]) {
        printf("error data_re(%d) \n", i);
        errors++;
      }

      if (data_I_ptr[i] != ref_res[2*i+1]) {
        printf("error data_im(%d) \n", i);
        errors++;
      }
      //printf("data_R(%d)=%d \n", i, (short) data_R_ptr[i]);
      //printf("data_I(%d)=%d \n", i, (short) *(data_I_ptr+i));
      //printf("\n");
    }
    if (errors == 0) {
      printf(" FFT PASS !!!\n");
    }
    else {
     printf("FFT FAIL \n"); 
     printf("Error count: %d \n", errors); 
    }
  }
}
#endif


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

#define KLESSYDRA_PERF 1
#define RISCY_PERF 0

int final_perf[8][3];

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

__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt

int enable_perf_cnt = 0;
for (int j=0; j<3; j++) {
if (Klessydra_get_coreID() == j) {
  //test_setup();
  //perf_start();

  for (int i = 0; i < NUM_ITER; ++i) {
    test_clear();

    //reset_timer();
    //start_timer();

    int* ptr_perf;

    // ENABLE COUNTING -------------------------------------------------------------------------
    #if KLESSYDRA_PERF == 1
      Klessydra_perf_cnt_start();
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
      ptr_perf = Klessydra_perf_cnt_finish();
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

  for(int k=0; k<8; k++) {
    final_perf[k][j]=ptr_perf[k];
  }

  if (Klessydra_get_coreID() == 0) {
    #if KLESSYDRA_PERF == 1 || RISCY_PERF == 1
    for (int i=0; i<3; i++) {
      printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Taken Count = %d \n \n", 
           final_perf[0][i], final_perf[1][i], final_perf[2][i], final_perf[3][i], final_perf[4][i], final_perf[5][i], final_perf[6][i], final_perf[7][i]);
    }
    #endif
    printf("\n");
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
}
}
}
