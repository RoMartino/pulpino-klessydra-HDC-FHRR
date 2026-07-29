/*
----------------------------------------------------------------------------------------------------
Convolution test
Multithreaded;
Accelerator (DSP+spm) is ON;
Replication of spm's and DSP Unit is ON;
the Multithread is used with different thread working on 3 batches of rows of the same output matrix, 
so in this approach each thread has a REGION of the output matrix to work on
----------------------------------------------------------------------------------------------------
*/
#ifndef A_ORDER
#define A_ORDER 32//Matrix size, don't do 2x2 case, for that i have another test
#endif

#define REPLICATION 1

#ifndef PRINT_NUM_CYCLES
//#define PRINT_NUM_CYCLES 1 // to print the cycle count
#endif

//#define PRINT_DEBUG 1 //to check if matrix is correct

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "dsp_functions.h"
#include "functions.h"
#include "klessydra_defs.h"

int loop_index=1;

#define SPM_MAX 32
#define SIZE_OF_INT 4
#define B_ORDER 3
#define K_COL (B_ORDER+1)

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

int perf0ld = 0;
int final_perf0ld = 777;
int *ptr_perf0ld = &perf0ld;

int final_perf_mem = 777;
int perf_mem = 0;
int *ptr_perf_mem = &perf_mem;
int kbcastld_cycles=0;

void display_spm_matrix(int size_r,int size_c,void* pt_spm_mat);

void matrix_check( int* mat1, int* mat2, int size );
void convolution2D_Scaling(int size, int (*matrix)[size], int *kernel_tmp, int *out);

void convolution2D_SPM_off(void* spm_dest, void* spm_fm, void* spm_krn, int size, int mem_off);
void convolution2D_SPM_off_NOB_print_region_1(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_print_region_2(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_print_region_3(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);

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

	CSR_MVSIZE(SPM_MAX*SPM_MAX*sizeof(int));
	kbcast((void*)spmaddrA,(void*)azzero);
	kbcast((void*)spmaddrB,(void*)azzero);
	kbcast((void*)spmaddrC,(void*)azzero);
	// kbcast((void*)spmaddrD,(void*)azzero);
	CSR_MVSIZE(dimension_A);
	int junk=0;
	//so i just use a quick function that do the trick
	CSR_MVSIZE(2*SIZE_OF_INT);
	kdotpps_v3((void*)spmaddrA,	(void*)spmaddrA, (void*)spmaddrB, (void*) conv2D_out_scal);
	CSR_MVSIZE(dimension_A);
	int off_idx=0;
	int array_boundaries[3]={0};
	array_boundaries[0] = A_ORDER/3;
	array_boundaries[1] =	array_boundaries[0]*2;
	array_boundaries[2] = A_ORDER;

		sync_barrier_reset();
		//------------------------------------------------------------------------------------------------------------
		sync_barrier_thread_registration();
				
		if (Klessydra_get_coreID() == 0) {

			// ENABLE COUNTING -------------------------------------------------------------------------
			kbcastld_cycles=0;
			__asm__("csrrw zero, mcycle, zero;"
					"csrrw zero, 0x7A0, 0x00000001");
			//------------------------------------------------------------------------------------------// CSR_MVSIZE(dimension_A);
			 kbcastld((void*)((int*)spmaddrB+memory_offset[0]), (void*)matB, dimension_B);

			 kbcastld((void*)((int*)spmaddrA+memory_offset[0]), (void*)matA0, dimension_A);

			// DISABLE COUNTING AND SAVE MCYCLE -----------------------------------------------------------
			__asm__("csrrw zero, 0x7A0, 0x00000000;"
					"csrrw %[perf0ld], mcycle, zero;"
					"sw %[perf0ld], 0(%[ptr_perf0ld]);"
					:
					:[perf0ld] "r" (perf0ld), [ptr_perf0ld] "r" (ptr_perf0ld)
					);
			kbcastld_cycles=*(ptr_perf0ld);
			// //------------------------------------------------------------------------------------------	
		}
		
		sync_barrier();
		//------------------------------------------------------------------------------------------------------------
		sync_barrier_thread_registration();
		int main_size=A_ORDER;
		int enable_perf_cnt = 0;
		int *ptr_perf;
		if (Klessydra_get_coreID() == 0) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			off_idx=0;
			ksrav((void*)((int*)spmaddrB+memory_offset[0]),(void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
			ksrav((void*)((int*)spmaddrA+memory_offset[0]),(void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
			for (int l=0; l<loop_index; l++) {
				convolution2D_SPM_off_NOB_print_region_1 ((void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	), (void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), (void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), (void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	A_ORDER);
			}
				kmemstr((void*)((int*)output_compare_s0 				+ 0),
			 			(void*)((int*)spmaddrC+memory_offset[0] + 0),
						 SIZE_OF_INT*(	A_ORDER*(array_boundaries[0] - 0))		);

			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// //------------------------------------------------------------------------------------------
		}

		if (Klessydra_get_coreID() == 1) {
			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			ksrav((void*)((int*)spmaddrB+memory_offset[0]),(void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
			ksrav((void*)((int*)spmaddrA+memory_offset[0]),(void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
			for (int l=0; l<loop_index; l++) {
				off_idx=0;
				convolution2D_SPM_off_NOB_print_region_2 ((void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	), (void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), (void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), (void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	A_ORDER);
			}
				kmemstr((void*)((int*)output_compare_s0 				+ A_ORDER*(array_boundaries[0])),
 			 			(void*)((int*)spmaddrC+memory_offset[0] + A_ORDER*(array_boundaries[0])),
 						 SIZE_OF_INT*(	A_ORDER*(array_boundaries[1] - array_boundaries[0]))		);

			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif			// printf("Convolution with DSP Speed 1 is:\n\t%d Cycles\n", final_perf1);
			//------------------------------------------------------------------------------------------
		}

		if (Klessydra_get_coreID() == 2) {

			// ENABLE COUNTING -------------------------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				Klessydra_perf_cnt_start();
			#endif
			//------------------------------------------------------------------------------------------
			ksrav((void*)((int*)spmaddrB+memory_offset[0]),(void*)((int*)spmaddrB+memory_offset[0]),	(int*)shift_pre);
			ksrav((void*)((int*)spmaddrA+memory_offset[0]),(void*)((int*)spmaddrA+memory_offset[0]),	(int*)shift_pre);
			for (int l=0; l<loop_index; l++) {
				off_idx=0;
				convolution2D_SPM_off_NOB_print_region_3 ((void*)(	(int*)spmaddrC+	 memory_offset[off_idx]	), (void*)(	(int*)spmaddrA+	 memory_offset[off_idx]	), (void*)(	(int*)spmaddrB+  memory_offset[off_idx]	), (void*)(	(int*)spmaddrD+	 memory_offset[off_idx]	),	A_ORDER);
			}

		 		kmemstr((void*)((int*)output_compare_s0 				+ A_ORDER*(array_boundaries[1])),
			 			(void*)((int*)spmaddrC+memory_offset[0] + A_ORDER*(array_boundaries[1])),
						 SIZE_OF_INT*(	A_ORDER*(array_boundaries[2] - array_boundaries[1]))		);

			// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
			#ifdef PRINT_NUM_CYCLES
				ptr_perf = Klessydra_perf_cnt_finish();
			#endif
			// //------------------------------------------------------------------------------------------
		}

		for (int i=0; i<3; i++) {
			if (Klessydra_get_coreID() == i) {
				for (int j=0; j<8; j++) {
					final_perf[i][j] = ptr_perf[j];
				}
			}
		}

		sync_barrier();

		//------------------------------------------------------------------------------------------------------------
		sync_barrier_thread_registration();

		if (Klessydra_get_coreID() == 0) {
			#ifdef PRINT_NUM_CYCLES
				printf("--------Test: MULTI SPMU SUBKERNEL LOOSELY COUPLED[%dx%d]--------\n", A_ORDER,A_ORDER);
				printf("N of loops:%d\n\n",loop_index);
				printf("Speed of kbcastld() of thread0 is:\t%d Cycles\n", kbcastld_cycles);
				printf("\n");
				for (int i=0; i<3; i++) {
					printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Taken Count = %d \n \n", 
							 final_perf[i][0], final_perf[i][1], final_perf[i][2], final_perf[i][3], final_perf[i][4], final_perf[i][5], final_perf[i][6], final_perf[i][7]);
				}
				printf("\n");
			#endif

			#ifdef PRINT_DEBUG
				// ENABLE COUNTING -------------------------------------------------------------------------
				final_perf_mem=0;
				__asm__("csrrw zero, mcycle, zero;"
						"csrrw zero, 0x7A0, 0x00000001");
				//------------------------------------------------------------------------------------------

				for (int l=0; l<loop_index; l++) convolution2D_Scaling(A_ORDER, mat_second_A[0],(int*)matB, (int*)output_compare0);
				// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------
				__asm__("csrrw zero, 0x7A0, 0x00000000;"
						"csrrw %[perf_mem], mcycle, zero;"
						"sw %[perf_mem], 0(%[ptr_perf_mem]);"
						:
						:[perf_mem] "r" (perf_mem), [ptr_perf_mem] "r" (ptr_perf_mem)
						);
				final_perf_mem=*(ptr_perf_mem);
				// //------------------------------------------------------------------------------------------
				printf("Convolution with Multiplier (single thread) Speed is:\t%d Cycles\n", final_perf_mem);

				matrix_check((int*)output_compare_s0,(int*)output_compare0, A_ORDER);
				printf("\n");
			#endif
		}

		sync_barrier();

	return 0;
}

//------------------------------------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------------------------------------

void convolution2D_Scaling(int size, int (*matrix)[size], int *kernel_tmp, int *out)
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
	///////////////////////////////////
	//scandisci tutta l'ultima						colonna	F
	j=(size-1);
	for(i = 1; i < size-1 ; i++)
	{
		pt=i*size+j;
		out[pt] +=	(((matrix[i-1][j-1]>>conv2D_scaling_factor)  * kernel[0])>> conv2D_out_scal) +
					(((matrix[i-1][j]	>>conv2D_scaling_factor) * kernel[1])>> conv2D_out_scal) +
					(((matrix[i][j-1]	>>conv2D_scaling_factor) * kernel[3])>> conv2D_out_scal) +
					(((matrix[i][j]	>>conv2D_scaling_factor)	 * kernel[4])>> conv2D_out_scal) +
					(((matrix[i+1][j-1]>>conv2D_scaling_factor)  * kernel[6])>> conv2D_out_scal) +
					(((matrix[i+1][j] >>conv2D_scaling_factor)	 * kernel[7])>> conv2D_out_scal);
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
		out[pt] +=	(((matrix[i][j]	>>conv2D_scaling_factor)	* kernel[4])>> conv2D_out_scal) +
					(((matrix[i][j+1]>>conv2D_scaling_factor)	* kernel[5])>> conv2D_out_scal) +
					(((matrix[i+1][j] >>conv2D_scaling_factor)	* kernel[7])>> conv2D_out_scal) +
					(((matrix[i+1][j+1]>>conv2D_scaling_factor) * kernel[8])>> conv2D_out_scal);
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
	for(i = 1; i < size-1 ; i++)
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
	for (i = 1; i < size-1; i++)
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
//base algorithm for check pourposes
void matrix_check( int* mat1, int* mat2, int size )
{
	printf("Checking if there is an error...");
  int err=0;
	for(int i=0; i<size; i++)
	{
		for(int j=0; j<size; j++)
		{
			if ( *((int*)mat1+i*size+j) != *((int*)mat2+i*size+j) ) {
				printf("\nERROR at elements [%d][%d] !\n",i,j);
        err++;

			}
		}
	}
  if (err==0){
    printf("No errors.\n");
  }
}

void display_spm_matrix(int size_r,int size_c,void* pt_spm_mat)
{

  printf("\n--------------------------------------\n");
  printf("\t\t-------Display_matrix 0x_@-----\n",pt_spm_mat);
  int pref_buff[32*32]={0};
  kmemstr((int*)pref_buff, (void*)(pt_spm_mat), (size_r*size_c)*sizeof(int));

  // int k=0,quad=0,temp=0;
  for(int i=0; i<size_r; i++)
  {
    for(int j=0; j<size_c; j++)
    {
      printf("\t%d",pref_buff[i*size_r+j]);
    }
    printf("\n");
  }
  printf("\t\t-------Display_matrix 0x_@-----\n");
  printf("--------------------------------------\n\n");

}

void convolution2D_SPM_off_NOB_print_region_1(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size)
{
	int print_F		=0;
	int print_ACGI=0;
	int print_D		=0;
	int print_E		=0;
	int print_B		=0;
	int print_H		=1;
	//Pointers to Spms and other index that i'll need for the convolution
	int backward_space=100;
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	// void* spmaddrDoff= (void*)(spm_temp);
	void* spmaddrDoff= (void*)((int*)spmaddrC-backward_space*1);

	void* dest_in_C;
	void* dest_in_B;
	void* dest_in_D;

	int k_element=0;
	int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
	int zero=0;

	int reg_0=size/3;
	int reg_1=reg_0+reg_0;

	//______________________________sub_kernel F
	CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	0;
	fm_offset= (size-1-1);
	// for(int i=1; i< size-1;i++){
	for(int i=1; i< reg_0;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff;
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ) );
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset )	);
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset )	);
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_F){
		printf("Primo ciclo spmC dopo kernel F\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

  //______________________________sub_kernel___________A-C
	CSR_MVSIZE(2*SIZE_OF_INT);
  //______________________________sub_kernel A
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(4);
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D + SIZE_OF_INT*(0),		(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D + SIZE_OF_INT*(1),		(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(4);
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D + SIZE_OF_INT*(2),		(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D + SIZE_OF_INT*(3),		(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));

	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
	CSR_MVSIZE(1*SIZE_OF_INT);

	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(0)	);
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(2)	);

	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	  	(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(1)	);
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),      		 	(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(3)	);

	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0&& print_ACGI){
		printf("Primo ciclo spmC dopo kernel F-ACGI\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	//______________________________sub_kernel D
	dest_in_D	=		(void*)spmaddrDoff;
	CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	1;
	fm_offset		=	0;
	// for(int i=1; i< size-1;i++){
	for(int i=1; i< reg_0;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff;
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0&& print_D){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	//______________________________sub_kernel E
	// CSR_MVSIZE(backward_space*SIZE_OF_INT);
	// ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
	CSR_MVSIZE((size-2)*SIZE_OF_INT);
	// for(int i=1; i< size-1;i++)
	for(int i=1; i< reg_0;i++)
	{
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf32
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	),
			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
			(void*)	( (int*)spmaddrBoff+k_element++) );

			ksrav((void*)(	(int*)	(spmaddrDoff)	) ,
			(void*)(	(int*)	(spmaddrDoff)	) ,
			(int*)conv2D_out_scal);

			kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
			(void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
			(void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) ,
			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
			(void*)	( (int*)spmaddrBoff+k_element++) );

			ksrav((void*)(	(int*)	(spmaddrDoff)	) ,
			(void*)(	(int*)	(spmaddrDoff)	) ,
			(int*)conv2D_out_scal);

			kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
			(void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
			(void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) ,
			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
			(void*)	( (int*)spmaddrBoff+k_element++) );

			ksrav((void*)(	(int*)	(spmaddrDoff)	) ,
			(void*)(	(int*)	(spmaddrDoff)	) ,
			(int*)conv2D_out_scal);

			kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
			(void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
			(void*)(	(int*)	(spmaddrDoff)	) );
		}
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_E){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	// //______________________________sub_kernel B
	// CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=0; i< 1;i++)
	{
		dest_in_C	= (void*)spmaddrCoff  + 1*SIZE_OF_INT;
		dest_in_D	= (void*)spmaddrDoff;
		k_element=3;
		for (int rw_pt=0; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc(dest_in_D,			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),	(void*)	( (int*)spmaddrBoff+k_element++) );
			ksrav(dest_in_D,	dest_in_D,	(int*)conv2D_out_scal);
	    	kaddv (dest_in_C, dest_in_C,  dest_in_D);
	    	ksvmulsc(dest_in_D,			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),	(void*)	( (int*)spmaddrBoff+k_element++) );
			ksrav(dest_in_D,	dest_in_D,	(int*)conv2D_out_scal);
			kaddv	(dest_in_C, dest_in_C,  dest_in_D);
			ksvmulsc(dest_in_D,			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),	(void*)	( (int*)spmaddrBoff+k_element++) );
			ksrav(dest_in_D,	dest_in_D,	(int*)conv2D_out_scal);
			kaddv (dest_in_C, dest_in_C,  dest_in_D);
    	}
	}

	CSR_MVSIZE(backward_space*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}


void convolution2D_SPM_off_NOB_print_region_2(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size)
{
	int print_F		=0;
	int print_ACGI=0;
	int print_D		=0;
	int print_E		=0;
	int print_B		=0;
	int print_H		=1;
  //Pointers to Spms and other index that i'll need for the convolution
	int backward_space=100;
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	// void* spmaddrDoff= (void*)(spm_temp);
	void* spmaddrDoff= (void*)((int*)spmaddrC-backward_space*2);

	void* dest_in_C;
	void* dest_in_B;
	void* dest_in_D;

	int k_element=0;
	int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
	int zero=0;

	int reg_0=size/3;
	int reg_1=reg_0+reg_0;

	//______________________________sub_kernel F
	CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	0;
	fm_offset= (size-1-1);
	// for(int i=1; i< size-1;i++){
	for(int i=reg_0; i< reg_1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff;
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ) );
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset )	);
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset )	);
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_F){
		printf("Primo ciclo spmC dopo kernel F\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}



	//______________________________sub_kernel D
  CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	1;
	fm_offset		=	0;
	// for(int i=1; i< size-1;i++){
	for(int i=reg_0; i< reg_1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff;
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0&& print_D){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }


  //______________________________sub_kernel E
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
	// for(int i=1; i< size-1;i++)
	for(int i=reg_0; i< reg_1;i++)
	{
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) ,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) ,
								(void*)(	(int*)	(spmaddrDoff)	) ,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) ,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) ,
								(void*)(	(int*)	(spmaddrDoff)	) ,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) ,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) ,
								(void*)(	(int*)	(spmaddrDoff)	) ,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) );
		}
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_E){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



	CSR_MVSIZE(backward_space*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
	// k_element=0;
	// CSR_MVSIZE(size*size*SIZE_OF_INT);
	// kbcast((void*)spmaddrDoff,(void*)k_element);
}


void convolution2D_SPM_off_NOB_print_region_3(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size)
{
	int print_F		=0;
	int print_ACGI=0;
	int print_D		=0;
	int print_E		=0;
	int print_B		=0;
	int print_H		=1;
  //Pointers to Spms and other index that i'll need for the convolution
	int backward_space=100;
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	// void* spmaddrDoff= (void*)(spm_temp);
	void* spmaddrDoff= (void*)((int*)spmaddrC-backward_space*3);

	void* dest_in_C;
	void* dest_in_B;
	void* dest_in_D;

	int k_element=0;
	int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
	int zero=0;

	int reg_0=size/3;
	int reg_1=reg_0+reg_0;

	//______________________________sub_kernel F
	CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	0;
	fm_offset= (size-1-1);
	// for(int i=1; i< size-1;i++){
	for(int i=reg_1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff;
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ) );
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset )	);
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset )	);
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_F){
		printf("Primo ciclo spmC dopo kernel F\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	//______________________________sub_kernel___________G-I
	CSR_MVSIZE(2*SIZE_OF_INT);
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(4);
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D + SIZE_OF_INT*(0),		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D + SIZE_OF_INT*(1),		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(4);
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D + SIZE_OF_INT*(2),		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D + SIZE_OF_INT*(3),		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));

	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
	CSR_MVSIZE(1*SIZE_OF_INT);

	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  			(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(0)	);
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),			(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(2)	);

	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  				(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(1)	);
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),				(void*)spmaddrDoff +4*SIZE_OF_INT+ SIZE_OF_INT*(3)	);

	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0&& print_ACGI){
		printf("Primo ciclo spmC dopo kernel F-ACGI\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	//______________________________sub_kernel D
	dest_in_D	=		(void*)spmaddrDoff;
	CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	1;
	fm_offset		=	0;
	// for(int i=1; i< size-1;i++){
	for(int i=reg_1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff;
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0&& print_D){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	//______________________________sub_kernel E
	CSR_MVSIZE((size-2)*SIZE_OF_INT);
	// for(int i=1; i< size-1;i++)
	for(int i=reg_1; i< size-1;i++)
	{
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	),
					 (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
					 (void*)	( (int*)spmaddrBoff+k_element++) );

			ksrav((void*)(	(int*)	(spmaddrDoff)	),
				  (void*)(	(int*)	(spmaddrDoff)	),
				  (int*)conv2D_out_scal);

			kaddv((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
				  (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
				  (void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	),
					 (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
					 (void*)	( (int*)spmaddrBoff+k_element++) );

			ksrav((void*)(	(int*)	(spmaddrDoff)	),
				  (void*)(	(int*)	(spmaddrDoff)	),
				  (int*)conv2D_out_scal);

			kaddv((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
				  (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
				  (void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	),
					 (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
					 (void*)	( (int*)spmaddrBoff+k_element++) );

			ksrav((void*)(	(int*)	(spmaddrDoff)	),
				  (void*)(	(int*)	(spmaddrDoff)	),
				  (int*)conv2D_out_scal);

			kaddv((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
				  (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
				  (void*)(	(int*)	(spmaddrDoff)	) );
		}
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_E){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	//______________________________sub_kernel H
	for(int i=size-1; i< size;i++)
	{
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		dest_in_D	= (void*)spmaddrDoff;
		k_element=0;
		for (int rw_pt=-1; rw_pt<1; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc(dest_in_D,			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),	(void*)	( (int*)spmaddrBoff+k_element++) );
			ksrav(dest_in_D,	dest_in_D,	(int*)conv2D_out_scal);
			kaddv	(dest_in_C, dest_in_C, dest_in_D);
			ksvmulsc(dest_in_D,			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),	(void*)	( (int*)spmaddrBoff+k_element++) );
			ksrav(dest_in_D,	dest_in_D,	(int*)conv2D_out_scal);
			kaddv	(dest_in_C, dest_in_C, dest_in_D);
			ksvmulsc(dest_in_D,			(void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),	(void*)	( (int*)spmaddrBoff+k_element++) );
			ksrav(dest_in_D,	dest_in_D,	(int*)conv2D_out_scal);
			kaddv	(dest_in_C, dest_in_C, dest_in_D);
		}
	}
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_H){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B-H\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	CSR_MVSIZE(backward_space*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
