/*
----------------------------------------------------------------------------------------------------
Convolution test
Multithreaded;
Accelerator (DSP+spm) is OFF/never used;
Replication of spm's and DSP Unit is OFF;
In this test the convolution is used only through memory + regfile. I have 3 different functions
working with the same REGIONS as splitted in test_2, plus another function that does the convolution on 
the whole matrix
----------------------------------------------------------------------------------------------------
*/
#ifndef A_ORDER
#define A_ORDER 32//Matrix size, don't do 2x2 case, for that i have another test
#endif

#define NO_ACCEL 1

#ifndef PRINT_NUM_CYCLES
//#define PRINT_NUM_CYCLES 1 // to print the cycle count
#endif

//-------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsp_functions.h"
#include "functions.h"
#include "klessydra_defs.h"
#define SPM_MAX 64
#define SIZE_OF_INT 4
#define B_ORDER 3
#define K_COL (B_ORDER+1)

int loop_index=1;
int matA0[A_ORDER*A_ORDER];
int matA1[A_ORDER*A_ORDER];
int matA2[A_ORDER*A_ORDER];
int dimension_A=A_ORDER*A_ORDER*sizeof(int);

int matB[B_ORDER*B_ORDER] = {0};
int dimension_B=B_ORDER*B_ORDER*sizeof(int);

int output_compare0[A_ORDER*A_ORDER]={0};
int output_compare1[A_ORDER*A_ORDER]={0};
int output_compare2[A_ORDER*A_ORDER]={0};
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

int setting_cycles=0;

void convolution2D_Scaling_region_1(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Scaling_region_2(int size, int (*matrix)[size], int *kernel_tmp, int *out);
void convolution2D_Scaling_region_3(int size, int (*matrix)[size], int *kernel_tmp, int *out);

void matrix_check_v2( int* mat1, int* mat2, int size);

//------------------------------------------------------------------------------------------------------------
// 													MAIN
//------------------------------------------------------------------------------------------------------------
int main(){

	print_global_k =0;
	print_global_id=0;
	print_global_dbg=0;;
	int squares[A_ORDER*A_ORDER]={0};
	warn[0]=2;
	warn[1]=2;
	warn[2]=2;

	for(int i=0; i<B_ORDER*B_ORDER; i++){
    matB[i]=(i+1)<<8;
  }
	sign=1;
	for(int i =0; i<A_ORDER*A_ORDER; i++){
    matA0[i]=(1000*sign*(i+1))<<8;
		matA1[i]=(20*sign*(i+1))<<8;
		matA2[i]=(3*sign*(i+1))<<8;
		sign=sign*(-1);
		output_compare_s0[i]=777;
		output_compare_s1[i]=777;
		output_compare_s2[i]=777;
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

	int off_idx=0;
	int array_boundaries[3]={0};
	array_boundaries[0] = A_ORDER/3;
	array_boundaries[1] =	array_boundaries[0]*2;
	array_boundaries[2] = A_ORDER;

	#ifdef NO_ACCEL
		sync_barrier_reset();
		//----------------------------------------------------------------------------------------------------
		// for (int main_size = 32 ; main_size > 2; main_size/2){
		int main_size=A_ORDER;
		int enable_oerf_cnt = 0;
		int *ptr_perf;
		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			// ------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) convolution2D_Scaling_region_1(main_size, mat_second_A[0],(int*)matB, (int*)output_compare1);
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// ------------------------------------------------------------------------------------------
		}
		if (Klessydra_get_coreID()==1){
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			// ------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) convolution2D_Scaling_region_2(main_size, mat_second_A[0],(int*)matB, (int*)output_compare1);
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			//------------------------------------------------------------------------------------------
		}
		if (Klessydra_get_coreID()==2){
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			for (int l=0; l<loop_index; l++) convolution2D_Scaling_region_3(main_size, mat_second_A[0],(int*)matB, (int*)output_compare1);
			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// printf("Convolution with DSP 0 Speed is:\n\t%d Cycles\n", final_perf0);
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
		
		if (Klessydra_get_coreID()==0){
			#ifdef PRINT_NUM_CYCLES
				printf("--------Test: NO ACCELERATION SUBKERNELS LOOSELY COUPLED[%dx%d]--------\n", main_size, main_size);
				printf("N of loops:%d\n\n",loop_index);
				for (int i=0; i<3; i++) {
					printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Taken Count = %d \n \n",
									 final_perf[i][0], final_perf[i][1], final_perf[i][2], final_perf[i][3], final_perf[i][4], final_perf[i][5], final_perf[i][6], final_perf[i][7]);
				}
				printf("\n");
			#endif

		}
		sync_barrier();

	#endif

	return 0;
}

//------------------------------------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
void convolution2D_Scaling_region_1(int size, int (*matrix)[size], int *kernel_tmp, int *out)
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
	
	int reg_0=size/3;
	int reg_1=reg_0+reg_0;
	///////////////////////////////////
	//scandisci tutta l'ultima						colonna	F
	j=(size-1);
	for(int i=1; i< reg_0;i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)     +
					(((matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)     +
					(((matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)     +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)     +
					(((matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)     +
					(((matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    ;
	}
	if(print){
		printf("dopo kernel F\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
		//printf("out[%d]=%d\n",pt,(int)out[pt]);
	///////////////////////////////////
													//alto sinistra A
	i=0;
	j=0;
	pt=i*size+j;
		out[pt] +=	(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>			conv2D_out_scal) 	 +
					(((matrix[i][j+1]>>conv2D_scaling_factor)		* kernel[5])>>			conv2D_out_scal)	 +
					(((matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>			conv2D_out_scal)	 +
					(((matrix[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>			conv2D_out_scal)	;
		//printf("out[%d]=%d\n",pt,(int)out[pt]);

	///////////////////////////////////
	//vertice alto a destra 						C
	i=0;
	j=(size-1);
	pt=i*size+j;
		out[pt] +=	(((matrix[i][j-1]>>conv2D_scaling_factor)		* kernel[3])>>			conv2D_out_scal) 	  +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>			conv2D_out_scal) 	  +
					(((matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>			conv2D_out_scal) 	  +
					(((matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>			conv2D_out_scal) 	 ;
		//printf("out[%d]=%d\n",pt,(int)out[pt]);

	///////////////////////////////////
	//scandisci tutta la prima colonna 				D
	j=0;
	for(int i=1; i< reg_0;i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j]>>conv2D_scaling_factor)	* kernel[1])>>			conv2D_out_scal) 	   +
					(((matrix[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>			conv2D_out_scal) 	   +
					(((matrix[i][j]	>>conv2D_scaling_factor)	* kernel[4])>>			conv2D_out_scal) 	   +
					(((matrix[i][j+1]>>conv2D_scaling_factor)	* kernel[5])>>			conv2D_out_scal) 	   +
					(((matrix[i+1][j] >>conv2D_scaling_factor)	* kernel[7])>>			conv2D_out_scal) 	   +
					(((matrix[i+1][j+1]>>conv2D_scaling_factor) * kernel[8])>>			conv2D_out_scal) 	  ;
	}
		//printf("out[%d]=%d\n",pt,(int)out[pt]);
	if(print){
		printf("dopo kernel F-ACGI-D\n");
	for (int rig=0;rig<size;rig++){
		for (int col=0;col<size;col++){
			printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	// kernel 										E centrale
	for(int i=1; i< reg_0;i++)
	{
		for (j = 1; j < size-1; j++)
		{
			pt=i*size+j;
			out[pt] +=	(	(	(matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
						(	(	(matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
						(	(	(matrix[i-1][j+1]>>conv2D_scaling_factor)		* kernel[2])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
	if(print){
		printf("dopo kernel F-ACGI-D-E\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	//scandisci tutta la prima riga tra i due		 vertici alti 	B
	i=0;
	for (j = 1; j < size-1; j++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i][j-1]>>conv2D_scaling_factor)	* kernel[3])>>			conv2D_out_scal) 	  +
					(((matrix[i][j]	>>conv2D_scaling_factor)	* kernel[4])>>			conv2D_out_scal) 	  +
					(((matrix[i][j+1]>>conv2D_scaling_factor)	* kernel[5])>>			conv2D_out_scal) 	  +
					(((matrix[i+1][j-1]>>conv2D_scaling_factor) * kernel[6])>>			conv2D_out_scal) 	  +
					(((matrix[i+1][j] >>conv2D_scaling_factor)	* kernel[7])>>			conv2D_out_scal) 	  +
					(((matrix[i+1][j+1]>>conv2D_scaling_factor) * kernel[8])>>			conv2D_out_scal) 	 ;
	}
	//printf("out[%d]=%d\n",pt,(int)out[pt]);
	if(print){
		printf("dopo kernel F-ACGI-D-E-B\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
}
void convolution2D_Scaling_region_2(int size, int (*matrix)[size], int *kernel_tmp, int *out)
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
	
	int reg_0=size/3;
	int reg_1=reg_0+reg_0;

	///////////////////////////////////
	//scandisci tutta l'ultima						colonna	F
	j=(size-1);
	for(int i=reg_0; i< reg_1;i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)     +
					(((matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)     +
					(((matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)     +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)     +
					(((matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)     +
					(((matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    ;
	}
	if(print){
		printf("dopo kernel F\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	//scandisci tutta la prima colonna 				D
	j=0;
	for(int i=reg_0; i< reg_1;i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j]>>conv2D_scaling_factor)	* kernel[1])>>			conv2D_out_scal) 	   +
					(((matrix[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>			conv2D_out_scal) 	   +
					(((matrix[i][j]	>>conv2D_scaling_factor)	* kernel[4])>>			conv2D_out_scal) 	   +
					(((matrix[i][j+1]>>conv2D_scaling_factor)	* kernel[5])>>			conv2D_out_scal) 	   +
					(((matrix[i+1][j] >>conv2D_scaling_factor)	* kernel[7])>>			conv2D_out_scal) 	   +
					(((matrix[i+1][j+1]>>conv2D_scaling_factor) * kernel[8])>>			conv2D_out_scal) 	  ;
	}
		//printf("out[%d]=%d\n",pt,(int)out[pt]);
	if(print){
		printf("dopo kernel F-ACGI-D\n");
	for (int rig=0;rig<size;rig++){
		for (int col=0;col<size;col++){
			printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	// kernel 										E centrale
	for(int i=reg_0; i< reg_1;i++)
	{
		for (j = 1; j < size-1; j++)
		{
			pt=i*size+j;
			out[pt] +=	(	(	(matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
						(	(	(matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
						(	(	(matrix[i-1][j+1]>>conv2D_scaling_factor)		* kernel[2])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
	if(print){
		printf("dopo kernel F-ACGI-D-E\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}

}
void convolution2D_Scaling_region_3(int size, int (*matrix)[size], int *kernel_tmp, int *out)
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
	

	int reg_0=size/3;
	int reg_1=reg_0+reg_0;

	///////////////////////////////////
	//scandisci tutta l'ultima						colonna	F
	j=(size-1);
	for(int i=reg_1; i< size-1;i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)     +
					(((matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)     +
					(((matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)     +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)     +
					(((matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)     +
					(((matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    ;
	}
	if(print){
		printf("dopo kernel F\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
		//printf("out[%d]=%d\n",pt,(int)out[pt]);
	///////////////////////////////////
	//in basso a 									sinistra G
	j=0;
	i=size-1;
	pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)     +
					(((matrix[i-1][j+1]>>conv2D_scaling_factor)		* kernel[2])>>		conv2D_out_scal)     +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)     +
					(((matrix[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    ;
		//printf("out[%d]=%d\n",pt,(int)out[pt]);

	///////////////////////////////////
	//in basso a 									destra	I
	i=(size-1);
	j=size-1;
	pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)     +
					(((matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)     +
					(((matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)     +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    ;
		//printf("out[%d]=%d\n",pt,(int)out[pt]);
	if(print){
		printf("dopo kernel F-ACGI\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	//scandisci tutta la prima colonna 				D
	j=0;
	for(int i=reg_1; i< size-1;i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j]>>conv2D_scaling_factor)	* kernel[1])>>			conv2D_out_scal) 	   +
					(((matrix[i-1][j+1]>>conv2D_scaling_factor)	* kernel[2])>>			conv2D_out_scal) 	   +
					(((matrix[i][j]	>>conv2D_scaling_factor)	* kernel[4])>>			conv2D_out_scal) 	   +
					(((matrix[i][j+1]>>conv2D_scaling_factor)	* kernel[5])>>			conv2D_out_scal) 	   +
					(((matrix[i+1][j] >>conv2D_scaling_factor)	* kernel[7])>>			conv2D_out_scal) 	   +
					(((matrix[i+1][j+1]>>conv2D_scaling_factor) * kernel[8])>>			conv2D_out_scal) 	  ;
	}
		//printf("out[%d]=%d\n",pt,(int)out[pt]);
	if(print){
		printf("dopo kernel F-ACGI-D\n");
	for (int rig=0;rig<size;rig++){
		for (int col=0;col<size;col++){
			printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	// kernel 										E centrale
	for(int i=reg_1; i< size-1;i++)
	{
		for (j = 1; j < size-1; j++)
		{
			pt=i*size+j;
			out[pt] +=	(	(	(matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)    +
						(	(	(matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)    +
						(	(	(matrix[i-1][j+1]>>conv2D_scaling_factor)		* kernel[2])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)    +
						(	(	(matrix[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j-1]>>conv2D_scaling_factor) 	* kernel[6])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j] >>conv2D_scaling_factor)		* kernel[7])>>		conv2D_out_scal)    +
						(	(	(matrix[i+1][j+1]>>conv2D_scaling_factor) 	* kernel[8])>>		conv2D_out_scal)   ;
		}
	}
	if(print){
		printf("dopo kernel F-ACGI-D-E\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
	///////////////////////////////////
	//scandisci tutta l'ultima riga tra i due vertici bassi	 H
	i=size-1;
	for (j = 1; j < size-1; j++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j-1]>>conv2D_scaling_factor) 	* kernel[0])>>		conv2D_out_scal)  +
					(((matrix[i-1][j]	>>conv2D_scaling_factor)	* kernel[1])>>		conv2D_out_scal)  +
					(((matrix[i-1][j+1]>>conv2D_scaling_factor)		* kernel[2])>>		conv2D_out_scal)  +
					(((matrix[i][j-1]	>>conv2D_scaling_factor)	* kernel[3])>>		conv2D_out_scal)  +
					(((matrix[i][j]	>>conv2D_scaling_factor)		* kernel[4])>>		conv2D_out_scal)  +
					(((matrix[i][j+1]	>>conv2D_scaling_factor)	* kernel[5])>>		conv2D_out_scal) ;
	}
	//printf("out[%d]=%d\n",pt,(int)out[pt]);
	if(print){
		printf("dopo kernel F-ACGI-D-E-B-H\n");
		for (int rig=0;rig<size;rig++){
			for (int col=0;col<size;col++){
				printf("%d\t",out[rig*size+col]);
			}printf("\n");
		}
	}
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
