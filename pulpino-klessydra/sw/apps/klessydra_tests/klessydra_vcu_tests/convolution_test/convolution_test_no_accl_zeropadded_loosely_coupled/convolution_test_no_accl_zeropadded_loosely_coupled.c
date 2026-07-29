/*
Test
-Multithread
-Decoupled
-Uses the ZEROPADDING convolutional algorithm
-Single accelerator
-can work with three SPM banks instead of four
-SPM size must be > 4KB
*/
// ----------------------------------------------------------------------------------------------------
// ----------------------------------ZEROPADDED--------------------------------------------------------
// ----------------------------------------------------------------------------------------------------

#ifndef A_ORDER
#define A_ORDER 32//Matrix size, don't do 2x2 case, for that i have another test
#endif

#define REPLICATION 1

#ifndef PRINT_NUM_CYCLES
#define PRINT_NUM_CYCLES 1 // to print the cycle count
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsp_functions.h"
#include "functions.h"
#include "klessydra_defs.h"

#define Z_ORDER (A_ORDER+2)

// #define PRINT_DEBUG 1 //to check if matrix is correct
#define MATRIX_CHECK 0 


int loop_index=1;

#define MULTI_TH 1

//don't change these :P
// // //#define MATRIX_CHECK_THREAD 0

#define SPM_MAX 64
#define SIZE_OF_INT 4
#define B_ORDER 3
#define K_COL (B_ORDER+1)

int matA0[A_ORDER*A_ORDER];
int matA1[A_ORDER*A_ORDER];
int matA2[A_ORDER*A_ORDER];
int dimension_A=A_ORDER*A_ORDER*sizeof(int);
int dimension_Zero=(A_ORDER+2)*(A_ORDER+2)*sizeof(int);

int matB[B_ORDER*B_ORDER] = {0};
int dimension_B=B_ORDER*B_ORDER*sizeof(int);

int output_compare0[A_ORDER*A_ORDER]={0};
int output_compare1[A_ORDER*A_ORDER]={0};
int output_compare2[A_ORDER*A_ORDER]={0};


int output_compare_zero[A_ORDER*A_ORDER]={0};

int output_compare_s0[A_ORDER*A_ORDER]={0};
int output_compare_s1[A_ORDER*A_ORDER]={0};
int output_compare_s2[A_ORDER*A_ORDER]={0};
int mat_second_A[3][A_ORDER][A_ORDER];

// #define Z_ORDER A_ORDER
int conv2D_out_scal=5;
int shift_pre=8;
int shift_out=5;
int warn[3];

int print_global_k , print_global_id	,	print_global_dbg;
int azzero=0;
int sign;
int memory_offset[3]={0};

int final_perf[3][8] = {{777}};

void convolution2D_Zeropadding(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_region_0(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_region_1(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Zeropadding_region_2(int size, int (*matrix)[size], int *kernel_tmp, int *out);

//------------------------------------------------------------------------------------------------------------
// 													MAIN
//------------------------------------------------------------------------------------------------------------
int main(){
	int off_idx=0;
	int squares[A_ORDER*A_ORDER]={0};
	int padded[Z_ORDER][Z_ORDER]={0};
	int junk=0;
	print_global_k =0;
	print_global_id=0;
	print_global_dbg=0;;
	warn[0]=2;
	warn[1]=2;
	warn[2]=2;
	sign=1;
	for(int i=0; i<B_ORDER*B_ORDER; i++){
    matB[i]=(i+1)<<8;
  }
	for(int i =0; i<A_ORDER*A_ORDER; i++){
    matA0[i]=(1000*sign*(i+1))<<8;
		matA1[i]=(20*sign*(i+1))<<8;
		matA2[i]=(3*sign*(i+1))<<8;
		sign=sign*(-1);
		output_compare_s0[i]=777;
		output_compare_s1[i]=777;
		output_compare_s2[i]=777;
		output_compare_zero[i]=777;
  }
	sign = 1;
  for(int i =0;i<A_ORDER; i++)
  {
    for(int j=0; j<A_ORDER; j++)
		{
      mat_second_A[0][i][j]=(1000*sign*(i*A_ORDER+j+1))<<8;
			mat_second_A[1][i][j]=(20*sign*(i*A_ORDER+j+1))<<8;
			mat_second_A[2][i][j]=(3*sign*(i*A_ORDER+j+1))<<8;
			sign=sign*(-1);
    }
  }
	memory_offset[0]=0;
	memory_offset[1]=1*32*32;
	memory_offset[2]=2*32*32;
	__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt
	
	#ifdef MULTI_TH
		//--------------------------------------Loosely Coupled No Accl--------------------------------------------------
		//--------------------------------------Loosely Coupled No Accl--------------------------------------------------
		//--------------------------------------Loosely Coupled No Accl--------------------------------------------------
		sync_barrier_reset();
		sync_barrier_thread_registration();
		if (Klessydra_get_coreID() == 0) {
			for(int i = 0 ; i < A_ORDER ; i++ ) {
				for (int j = 0 ; j < A_ORDER ; j++ ) {
					// output_compare0[i*A_ORDER+j]=0;
					output_compare1[i*A_ORDER+j]=0;
				}
			}
		}
		sync_barrier();

		int enable_perf_cnt = 0;
		int *ptr_perf;
		if (Klessydra_get_coreID() == 0) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_region_0(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1);
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			//------------------------------------------------------------------------------------------
		}
		if (Klessydra_get_coreID() == 1) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_region_1(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1);
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			//------------------------------------------------------------------------------------------
		}
		if (Klessydra_get_coreID() == 2) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) convolution2D_Zeropadding_region_2(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare1);
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			//------------------------------------------------------------------------------------------
		}
		sync_barrier();

		for (int i=0; i<3; i++) {
			if (Klessydra_get_coreID() == i) {
				for (int j=0; j<8; j++) {
					final_perf[i][j] = ptr_perf[j];
				}
			}
		}

		sync_barrier_thread_registration();
		if (Klessydra_get_coreID() == 0) {
			#ifdef PRINT_NUM_CYCLES
				printf("--------Test: NO ACCELERATION ZEROPADDED LOOSELY COUPLED[%dx%d]--------\n", A_ORDER,A_ORDER);
				printf("N of loops:%d\n\n",loop_index);
				for (int i=0; i<3; i++) {
					printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Taken Count = %d \n \n", 
							 final_perf[i][0], final_perf[i][1], final_perf[i][2], final_perf[i][3], final_perf[i][4], final_perf[i][5], final_perf[i][6], final_perf[i][7]);
				}
				printf("\n");
			#endif
		}
		sync_barrier();
		//--------------------------------------Loosely Coupled No Accl--------------------------------------------------
		//--------------------------------------Loosely Coupled No Accl--------------------------------------------------
		//--------------------------------------Loosely Coupled No Accl--------------------------------------------------
	#endif
	return 0;
}
//------------------------------------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------------------------------------

void convolution2D_Zeropadding(int size, int (*matrix)[size], int *kernel_tmp, int *out)
{
	int print=0;
	int kernel[9];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<9;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	for (int i=1; i< size+2-1; i++)
		for (int j=1; j< size+2-1; j++)
			zeropad[i][j]=matrix[i-1][j-1];

	for (i = 1; i < (size+2)-1; i++)
	{
		for (j = 1; j < (size+2)-1; j++)
		{
			pt=(i-1)*size+(j-1);
			out[pt] +=	(	(	(zeropad[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
}
void convolution2D_Zeropadding_region_0(int size, int (*matrix)[size], int *kernel_tmp, int *out)
{
	int print=0;
	int kernel[9];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<9;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	for (int i=1; i< size+2-1; i++)
		for (int j=1; j< size+2-1; j++)
			zeropad[i][j]=matrix[i-1][j-1];

	int region[2];
	region[0]=(size+2)/3;
	region[1]=region[0]*2;
	
	for (i = 1; i < region[0]; i++)
	// for (i = region[0]; i < region[1]; i++)
	// for (i = region[1]; i < (size+2)-1; i++)
	{
		for (j = 1; j < (size+2)-1; j++)
		{
			pt=(i-1)*size+(j-1);
			out[pt] +=	(	(	(zeropad[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
}
void convolution2D_Zeropadding_region_1(int size, int (*matrix)[size], int *kernel_tmp, int *out)
{
	int print=0;
	int kernel[9];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<9;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	for (int i=1; i< size+2-1; i++)
		for (int j=1; j< size+2-1; j++)
			zeropad[i][j]=matrix[i-1][j-1];

	int region[2];
	region[0]=(size+2)/3;
	region[1]=region[0]*2;
	
	// for (i = 1; i < region[0]; i++)
	for (i = region[0]; i < region[1]; i++)
	// for (i = region[1]; i < (size+2)-1; i++)
	{
		for (j = 1; j < (size+2)-1; j++)
		{
			pt=(i-1)*size+(j-1);
			out[pt] +=	(	(	(zeropad[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
}
void convolution2D_Zeropadding_region_2(int size, int (*matrix)[size], int *kernel_tmp, int *out)
{
	int print=0;
	int kernel[9];
	int conv2D_scaling_factor=8;
	int conv2D_out_scal=5;
	for(int i=0;i<9;i++){
    	kernel[i]=(kernel_tmp[i]>>conv2D_scaling_factor);
	}
	int i, j;
	int pt=0;
	int zeropad[Z_ORDER][Z_ORDER]={0};
	for (int i=1; i< size+2-1; i++)
		for (int j=1; j< size+2-1; j++)
			zeropad[i][j]=matrix[i-1][j-1];

	int region[2];
	region[0]=(size+2)/3;
	region[1]=region[0]*2;
	
	// for (i = 1; i < region[0]; i++)
	// for (i = region[0]; i < region[1]; i++)
	for (i = region[1]; i < (size+2)-1; i++)
	{
		for (j = 1; j < (size+2)-1; j++)
		{
			pt=(i-1)*size+(j-1);
			out[pt] +=	(	(	(zeropad[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
									(	(	(zeropad[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
									(	(	(zeropad[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
									(	(	(zeropad[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
}