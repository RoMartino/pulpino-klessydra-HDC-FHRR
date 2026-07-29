#include <stdio.h>

#include "dsp_functions.h"


// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------

void matrix_check( int* mat1, int* mat2, int size){
	printf("Checking results for errors...");
	int err=0;
	for(int i=0; i<size; i++)
	{
		for(int j=0; j<size; j++)
		{
			if ( *((int*)mat1+i*size+j) != *((int*)mat2+i*size+j) ) {
				printf("\nERROR at elements [%d][%d], %x, %x !\n",i,j, *((int*)mat1+i*size+j), *((int*)mat2+i*size+j));
				err++;
			}
		}
	}
	if (err==0)
		printf("\n Success, %d errors found.\n", err);
	else 
		printf("Fail, %d errors Detected!.\n", err);
}


void matrix_print(int* pt, int size, int mode, int start_row, int finish_row)
{
	for(int i=start_row; i<=finish_row; i++)
	{
		for(int j=0; j<size; j++)
		{
			if (mode == 0) {
				printf("\t%02d", *((int*)pt+i*size+j));
			}
			else {
				printf("\t0x%08x", *(int*)(&pt[i*size+j]));
			}
		}
		printf("\n\n");
	}
}

// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------

void conv2D(int fm_size, int krn_size, int (*matrix)[fm_size], int (*kernel)[krn_size], int (*out)[fm_size])
{
	int zpad_mat[Z_ORDER][Z_ORDER]={{0}};
	int pad_size = (krn_size-1)/2;
	// Create the Zeropadded matrix
	for (int i=pad_size; i< fm_size+pad_size; i++) {
		for (int j=pad_size; j< fm_size+pad_size; j++){
			zpad_mat[i][j] = matrix[i-pad_size][j-pad_size];
		}
	}
	//matrix_print((int*)matrix, A_ORDER, 1, 0, A_ORDER-1);
	//printf("\n\n\n\n\n");
	//matrix_print((int*)zpad_mat, Z_ORDER, 1, 0, Z_ORDER-1);

	//// Perform the convolution
	for (int i=0; i<fm_size; i++)
	{
		for (int j=0; j<fm_size; j++)
		{
			for (int k=0; k<krn_size; k++)
			{
				for (int l=0; l<krn_size; l++)
				{
					out[i][j] += zpad_mat[i+k][j+l] * kernel[k][l];
				}
			}
		}
	}
}

// loop unrolled convolution function for 3x3 kernels
// NOTE: These loop unrolled functions are made since compiler flags and directives for loop unrolling on the generic function didn't improve the speed, but rather made it worse
void conv2D_3x3(int fm_size, int (*matrix)[fm_size], int (*kernel)[3], int (*out)[fm_size])
{
	int zpad_mat[Z_ORDER][Z_ORDER]={{0}};
	// Create the Zeropadded matrix
	for (int i=1; i<fm_size+1; i++) {
		for (int j=1; j<fm_size+1; j++){
			zpad_mat[i][j] = matrix[i-1][j-1];
		}
	}

	// Perform the convolution unrolling the 3x3 kernel loops
	for (int i=0; i<fm_size; i++)
	{
		for (int j=0; j<fm_size; j++)
		{
			out[i][j] +=
				(zpad_mat[i+0][j+0] * kernel[0][0])  +
				(zpad_mat[i+0][j+1] * kernel[0][1])  +
				(zpad_mat[i+0][j+2] * kernel[0][2])  +
				(zpad_mat[i+1][j+0] * kernel[1][0])  +
				(zpad_mat[i+1][j+1] * kernel[1][1])  +
				(zpad_mat[i+1][j+2] * kernel[1][2])  +
				(zpad_mat[i+2][j+0] * kernel[2][0])  +
				(zpad_mat[i+2][j+1] * kernel[2][1])  +
				(zpad_mat[i+2][j+2] * kernel[2][2]);
		}
	}
}

void conv2D_5x5(int fm_size, int (*matrix)[fm_size], int (*kernel)[5], int (*out)[fm_size]) 
{
	int zpad_mat[Z_ORDER][Z_ORDER]={{0}};

	// Create the Zeropadded matrix
	for (int i=2; i<fm_size+2; i++) {
		for (int j=2; j<fm_size+2; j++){
			zpad_mat[i][j] = matrix[i-2][j-2];
		}
	}

	// Perform the convolution unrolling the 5x5 kernel loops
	for (int i=0; i<fm_size; i++)
	{
		for (int j=0; j<fm_size; j++)
		{
			out[i][j] +=
				(zpad_mat[i+0][j+0] * kernel[0][0]) +
				(zpad_mat[i+0][j+1] * kernel[0][1]) +
				(zpad_mat[i+0][j+2] * kernel[0][2]) +
				(zpad_mat[i+0][j+3] * kernel[0][3]) +
				(zpad_mat[i+0][j+4] * kernel[0][4]) +
				(zpad_mat[i+1][j+0] * kernel[1][0]) +
				(zpad_mat[i+1][j+1] * kernel[1][1]) +
				(zpad_mat[i+1][j+2] * kernel[1][2]) +
				(zpad_mat[i+1][j+3] * kernel[1][3]) +
				(zpad_mat[i+1][j+4] * kernel[1][4]) +
				(zpad_mat[i+2][j+0] * kernel[2][0]) +
				(zpad_mat[i+2][j+1] * kernel[2][1]) +
				(zpad_mat[i+2][j+2] * kernel[2][2]) +
				(zpad_mat[i+2][j+3] * kernel[2][3]) +
				(zpad_mat[i+2][j+4] * kernel[2][4]) +
				(zpad_mat[i+3][j+0] * kernel[3][0]) +
				(zpad_mat[i+3][j+1] * kernel[3][1]) +
				(zpad_mat[i+3][j+2] * kernel[3][2]) +
				(zpad_mat[i+3][j+3] * kernel[3][3]) +
				(zpad_mat[i+3][j+4] * kernel[3][4]) +
				(zpad_mat[i+4][j+0] * kernel[4][0]) +
				(zpad_mat[i+4][j+1] * kernel[4][1]) +
				(zpad_mat[i+4][j+2] * kernel[4][2]) +
				(zpad_mat[i+4][j+3] * kernel[4][3]) +
				(zpad_mat[i+4][j+4] * kernel[4][4]);
		}
	}
}

void conv2D_7x7(int fm_size, int (*matrix)[fm_size], int (*kernel)[7], int (*out)[fm_size]) 
{
	int zpad_mat[Z_ORDER][Z_ORDER]={{0}};

	// Create the Zeropadded matrix
	for (int i=3; i<fm_size+3; i++) {
		for (int j=3; j<fm_size+3; j++){
			zpad_mat[i][j] = matrix[i-3][j-3];
		}
	}

	// Perform the convolution unrolling the 5x5 kernel loops
	for (int i=0; i<fm_size; i++)
	{
		for (int j=0; j<fm_size; j++)
		{
			out[i][j] +=
				(zpad_mat[i+0][j+0] * kernel[0][0]) +
				(zpad_mat[i+0][j+1] * kernel[0][1]) +
				(zpad_mat[i+0][j+2] * kernel[0][2]) +
				(zpad_mat[i+0][j+3] * kernel[0][3]) +
				(zpad_mat[i+0][j+4] * kernel[0][4]) +
				(zpad_mat[i+0][j+5] * kernel[0][5]) +
				(zpad_mat[i+0][j+6] * kernel[0][6]) +
				(zpad_mat[i+1][j+0] * kernel[1][0]) +
				(zpad_mat[i+1][j+1] * kernel[1][1]) +
				(zpad_mat[i+1][j+2] * kernel[1][2]) +
				(zpad_mat[i+1][j+3] * kernel[1][3]) +
				(zpad_mat[i+1][j+4] * kernel[1][4]) +
				(zpad_mat[i+1][j+5] * kernel[1][5]) +
				(zpad_mat[i+1][j+6] * kernel[1][6]) +
				(zpad_mat[i+2][j+0] * kernel[2][0]) +
				(zpad_mat[i+2][j+1] * kernel[2][1]) +
				(zpad_mat[i+2][j+2] * kernel[2][2]) +
				(zpad_mat[i+2][j+3] * kernel[2][3]) +
				(zpad_mat[i+2][j+4] * kernel[2][4]) +
				(zpad_mat[i+2][j+5] * kernel[2][5]) +
				(zpad_mat[i+2][j+6] * kernel[2][6]) +
				(zpad_mat[i+3][j+0] * kernel[3][0]) +
				(zpad_mat[i+3][j+1] * kernel[3][1]) +
				(zpad_mat[i+3][j+2] * kernel[3][2]) +
				(zpad_mat[i+3][j+3] * kernel[3][3]) +
				(zpad_mat[i+3][j+4] * kernel[3][4]) +
				(zpad_mat[i+3][j+5] * kernel[3][5]) +
				(zpad_mat[i+3][j+6] * kernel[3][6]) +
				(zpad_mat[i+4][j+0] * kernel[4][0]) +
				(zpad_mat[i+4][j+1] * kernel[4][1]) +
				(zpad_mat[i+4][j+2] * kernel[4][2]) +
				(zpad_mat[i+4][j+3] * kernel[4][3]) +
				(zpad_mat[i+4][j+4] * kernel[4][4]) +
				(zpad_mat[i+4][j+5] * kernel[4][5]) +
				(zpad_mat[i+4][j+6] * kernel[4][6]) +
				(zpad_mat[i+5][j+0] * kernel[5][0]) +
				(zpad_mat[i+5][j+1] * kernel[5][1]) +
				(zpad_mat[i+5][j+2] * kernel[5][2]) +
				(zpad_mat[i+5][j+3] * kernel[5][3]) +
				(zpad_mat[i+5][j+4] * kernel[5][4]) +
				(zpad_mat[i+5][j+5] * kernel[5][5]) +
				(zpad_mat[i+5][j+6] * kernel[5][6]) +
				(zpad_mat[i+6][j+0] * kernel[6][0]) +
				(zpad_mat[i+6][j+1] * kernel[6][1]) +
				(zpad_mat[i+6][j+2] * kernel[6][2]) +
				(zpad_mat[i+6][j+3] * kernel[6][3]) +
				(zpad_mat[i+6][j+4] * kernel[6][4]) +
				(zpad_mat[i+6][j+5] * kernel[6][5]) +
				(zpad_mat[i+6][j+6] * kernel[6][6]);
		}
	}
}

void conv2D_9x9(int fm_size, int (*matrix)[fm_size], int (*kernel)[9], int (*out)[fm_size]) 
{
	int zpad_mat[Z_ORDER][Z_ORDER]={{0}};

	// Create the Zeropadded matrix
	for (int i=4; i<fm_size+4; i++) {
		for (int j=4; j<fm_size+4; j++){
			zpad_mat[i][j] = matrix[i-4][j-4];
		}
	}

	// Perform the convolution unrolling the 5x5 kernel loops
	for (int i=0; i<fm_size; i++)
	{
		for (int j=0; j<fm_size; j++)
		{
			out[i][j] +=
				(zpad_mat[i+0][j+0] * kernel[0][0]) +
				(zpad_mat[i+0][j+1] * kernel[0][1]) +
				(zpad_mat[i+0][j+2] * kernel[0][2]) +
				(zpad_mat[i+0][j+3] * kernel[0][3]) +
				(zpad_mat[i+0][j+4] * kernel[0][4]) +
				(zpad_mat[i+0][j+5] * kernel[0][5]) +
				(zpad_mat[i+0][j+6] * kernel[0][6]) +
				(zpad_mat[i+0][j+7] * kernel[0][7]) +
				(zpad_mat[i+0][j+8] * kernel[0][8]) +
				(zpad_mat[i+1][j+0] * kernel[1][0]) +
				(zpad_mat[i+1][j+1] * kernel[1][1]) +
				(zpad_mat[i+1][j+2] * kernel[1][2]) +
				(zpad_mat[i+1][j+3] * kernel[1][3]) +
				(zpad_mat[i+1][j+4] * kernel[1][4]) +
				(zpad_mat[i+1][j+5] * kernel[1][5]) +
				(zpad_mat[i+1][j+6] * kernel[1][6]) +
				(zpad_mat[i+1][j+7] * kernel[1][7]) +
				(zpad_mat[i+1][j+8] * kernel[1][8]) +
				(zpad_mat[i+2][j+0] * kernel[2][0]) +
				(zpad_mat[i+2][j+1] * kernel[2][1]) +
				(zpad_mat[i+2][j+2] * kernel[2][2]) +
				(zpad_mat[i+2][j+3] * kernel[2][3]) +
				(zpad_mat[i+2][j+4] * kernel[2][4]) +
				(zpad_mat[i+2][j+5] * kernel[2][5]) +
				(zpad_mat[i+2][j+6] * kernel[2][6]) +
				(zpad_mat[i+2][j+7] * kernel[2][7]) +
				(zpad_mat[i+2][j+8] * kernel[2][8]) +
				(zpad_mat[i+3][j+0] * kernel[3][0]) +
				(zpad_mat[i+3][j+1] * kernel[3][1]) +
				(zpad_mat[i+3][j+2] * kernel[3][2]) +
				(zpad_mat[i+3][j+3] * kernel[3][3]) +
				(zpad_mat[i+3][j+4] * kernel[3][4]) +
				(zpad_mat[i+3][j+5] * kernel[3][5]) +
				(zpad_mat[i+3][j+6] * kernel[3][6]) +
				(zpad_mat[i+3][j+7] * kernel[3][7]) +
				(zpad_mat[i+3][j+8] * kernel[3][8]) +
				(zpad_mat[i+4][j+0] * kernel[4][0]) +
				(zpad_mat[i+4][j+1] * kernel[4][1]) +
				(zpad_mat[i+4][j+2] * kernel[4][2]) +
				(zpad_mat[i+4][j+3] * kernel[4][3]) +
				(zpad_mat[i+4][j+4] * kernel[4][4]) +
				(zpad_mat[i+4][j+5] * kernel[4][5]) +
				(zpad_mat[i+4][j+6] * kernel[4][6]) +
				(zpad_mat[i+4][j+7] * kernel[4][7]) +
				(zpad_mat[i+4][j+8] * kernel[4][8]) +
				(zpad_mat[i+5][j+0] * kernel[5][0]) +
				(zpad_mat[i+5][j+1] * kernel[5][1]) +
				(zpad_mat[i+5][j+2] * kernel[5][2]) +
				(zpad_mat[i+5][j+3] * kernel[5][3]) +
				(zpad_mat[i+5][j+4] * kernel[5][4]) +
				(zpad_mat[i+5][j+5] * kernel[5][5]) +
				(zpad_mat[i+5][j+6] * kernel[5][6]) +
				(zpad_mat[i+5][j+7] * kernel[5][7]) +
				(zpad_mat[i+5][j+8] * kernel[5][8]) +
				(zpad_mat[i+6][j+0] * kernel[6][0]) +
				(zpad_mat[i+6][j+1] * kernel[6][1]) +
				(zpad_mat[i+6][j+2] * kernel[6][2]) +
				(zpad_mat[i+6][j+3] * kernel[6][3]) +
				(zpad_mat[i+6][j+4] * kernel[6][4]) +
				(zpad_mat[i+6][j+5] * kernel[6][5]) +
				(zpad_mat[i+6][j+6] * kernel[6][6]) +
				(zpad_mat[i+6][j+7] * kernel[6][7]) +
				(zpad_mat[i+6][j+8] * kernel[6][8]) +
				(zpad_mat[i+7][j+0] * kernel[7][0]) +
				(zpad_mat[i+7][j+1] * kernel[7][1]) +
				(zpad_mat[i+7][j+2] * kernel[7][2]) +
				(zpad_mat[i+7][j+3] * kernel[7][3]) +
				(zpad_mat[i+7][j+4] * kernel[7][4]) +
				(zpad_mat[i+7][j+5] * kernel[7][5]) +
				(zpad_mat[i+7][j+6] * kernel[7][6]) +
				(zpad_mat[i+7][j+7] * kernel[7][7]) +
				(zpad_mat[i+7][j+8] * kernel[7][8]) +
				(zpad_mat[i+8][j+0] * kernel[8][0]) +
				(zpad_mat[i+8][j+1] * kernel[8][1]) +
				(zpad_mat[i+8][j+2] * kernel[8][2]) +
				(zpad_mat[i+8][j+3] * kernel[8][3]) +
				(zpad_mat[i+8][j+4] * kernel[8][4]) +
				(zpad_mat[i+8][j+5] * kernel[8][5]) +
				(zpad_mat[i+8][j+6] * kernel[8][6]) +
				(zpad_mat[i+8][j+7] * kernel[8][7]) +
				(zpad_mat[i+8][j+8] * kernel[8][8]);
		}
	}
}

void conv2D_11x11(int fm_size, int (*matrix)[fm_size], int (*kernel)[11], int (*out)[fm_size]) 
{
	int zpad_mat[Z_ORDER][Z_ORDER]={{0}};

	// Create the Zeropadded matrix
	for (int i=5; i<fm_size+5; i++) {
		for (int j=5; j<fm_size+5; j++){
			zpad_mat[i][j] = matrix[i-5][j-5];
		}
	}

	// Perform the convolution unrolling the 5x5 kernel loops
	for (int i=0; i<fm_size; i++)
	{
		for (int j=0; j<fm_size; j++)
		{
			out[i][j] +=
				(zpad_mat[i+0][j+0 ]  * kernel[0][0 ])  +
				(zpad_mat[i+0][j+1 ]  * kernel[0][1 ])  +
				(zpad_mat[i+0][j+2 ]  * kernel[0][2 ])  +
				(zpad_mat[i+0][j+3 ]  * kernel[0][3 ])  +
				(zpad_mat[i+0][j+4 ]  * kernel[0][4 ])  +
				(zpad_mat[i+0][j+5 ]  * kernel[0][5 ])  +
				(zpad_mat[i+0][j+6 ]  * kernel[0][6 ])  +
				(zpad_mat[i+0][j+7 ]  * kernel[0][7 ])  +
				(zpad_mat[i+0][j+8 ]  * kernel[0][8 ])  +
				(zpad_mat[i+0][j+9 ]  * kernel[0][9 ])  +
				(zpad_mat[i+0][j+10]  * kernel[0][10])  +
				(zpad_mat[i+1][j+0 ]  * kernel[1][0 ])  +
				(zpad_mat[i+1][j+1 ]  * kernel[1][1 ])  +
				(zpad_mat[i+1][j+2 ]  * kernel[1][2 ])  +
				(zpad_mat[i+1][j+3 ]  * kernel[1][3 ])  +
				(zpad_mat[i+1][j+4 ]  * kernel[1][4 ])  +
				(zpad_mat[i+1][j+5 ]  * kernel[1][5 ])  +
				(zpad_mat[i+1][j+6 ]  * kernel[1][6 ])  +
				(zpad_mat[i+1][j+7 ]  * kernel[1][7 ])  +
				(zpad_mat[i+1][j+8 ]  * kernel[1][8 ])  +
				(zpad_mat[i+1][j+9 ]  * kernel[1][9 ])  +
				(zpad_mat[i+1][j+10]  * kernel[1][10])  +
				(zpad_mat[i+2][j+0 ]  * kernel[2][0 ])  +
				(zpad_mat[i+2][j+1 ]  * kernel[2][1 ])  +
				(zpad_mat[i+2][j+2 ]  * kernel[2][2 ])  +
				(zpad_mat[i+2][j+3 ]  * kernel[2][3 ])  +
				(zpad_mat[i+2][j+4 ]  * kernel[2][4 ])  +
				(zpad_mat[i+2][j+5 ]  * kernel[2][5 ])  +
				(zpad_mat[i+2][j+6 ]  * kernel[2][6 ])  +
				(zpad_mat[i+2][j+7 ]  * kernel[2][7 ])  +
				(zpad_mat[i+2][j+8 ]  * kernel[2][8 ])  +
				(zpad_mat[i+2][j+9 ]  * kernel[2][9 ])  +
				(zpad_mat[i+2][j+10]  * kernel[2][10])  +
				(zpad_mat[i+3][j+0 ]  * kernel[3][0 ])  +
				(zpad_mat[i+3][j+1 ]  * kernel[3][1 ])  +
				(zpad_mat[i+3][j+2 ]  * kernel[3][2 ])  +
				(zpad_mat[i+3][j+3 ]  * kernel[3][3 ])  +
				(zpad_mat[i+3][j+4 ]  * kernel[3][4 ])  +
				(zpad_mat[i+3][j+5 ]  * kernel[3][5 ])  +
				(zpad_mat[i+3][j+6 ]  * kernel[3][6 ])  +
				(zpad_mat[i+3][j+7 ]  * kernel[3][7 ])  +
				(zpad_mat[i+3][j+8 ]  * kernel[3][8 ])  +
				(zpad_mat[i+3][j+9 ]  * kernel[3][9 ])  +
				(zpad_mat[i+3][j+10]  * kernel[3][10])  +
				(zpad_mat[i+4][j+0 ]  * kernel[4][0 ])  +
				(zpad_mat[i+4][j+1 ]  * kernel[4][1 ])  +
				(zpad_mat[i+4][j+2 ]  * kernel[4][2 ])  +
				(zpad_mat[i+4][j+3 ]  * kernel[4][3 ])  +
				(zpad_mat[i+4][j+4 ]  * kernel[4][4 ])  +
				(zpad_mat[i+4][j+5 ]  * kernel[4][5 ])  +
				(zpad_mat[i+4][j+6 ]  * kernel[4][6 ])  +
				(zpad_mat[i+4][j+7 ]  * kernel[4][7 ])  +
				(zpad_mat[i+4][j+8 ]  * kernel[4][8 ])  +
				(zpad_mat[i+4][j+9 ]  * kernel[4][9 ])  +
				(zpad_mat[i+4][j+10]  * kernel[4][10])  +
				(zpad_mat[i+5][j+0 ]  * kernel[5][0 ])  +
				(zpad_mat[i+5][j+1 ]  * kernel[5][1 ])  +
				(zpad_mat[i+5][j+2 ]  * kernel[5][2 ])  +
				(zpad_mat[i+5][j+3 ]  * kernel[5][3 ])  +
				(zpad_mat[i+5][j+4 ]  * kernel[5][4 ])  +
				(zpad_mat[i+5][j+5 ]  * kernel[5][5 ])  +
				(zpad_mat[i+5][j+6 ]  * kernel[5][6 ])  +
				(zpad_mat[i+5][j+7 ]  * kernel[5][7 ])  +
				(zpad_mat[i+5][j+8 ]  * kernel[5][8 ])  +
				(zpad_mat[i+5][j+9 ]  * kernel[5][9 ])  +
				(zpad_mat[i+5][j+10]  * kernel[5][10])  +
				(zpad_mat[i+6][j+0 ]  * kernel[6][0 ])  +
				(zpad_mat[i+6][j+1 ]  * kernel[6][1 ])  +
				(zpad_mat[i+6][j+2 ]  * kernel[6][2 ])  +
				(zpad_mat[i+6][j+3 ]  * kernel[6][3 ])  +
				(zpad_mat[i+6][j+4 ]  * kernel[6][4 ])  +
				(zpad_mat[i+6][j+5 ]  * kernel[6][5 ])  +
				(zpad_mat[i+6][j+6 ]  * kernel[6][6 ])  +
				(zpad_mat[i+6][j+7 ]  * kernel[6][7 ])  +
				(zpad_mat[i+6][j+8 ]  * kernel[6][8 ])  +
				(zpad_mat[i+6][j+9 ]  * kernel[6][9 ])  +
				(zpad_mat[i+6][j+10]  * kernel[6][10])  +
				(zpad_mat[i+7][j+0 ]  * kernel[7][0 ])  +
				(zpad_mat[i+7][j+1 ]  * kernel[7][1 ])  +
				(zpad_mat[i+7][j+2 ]  * kernel[7][2 ])  +
				(zpad_mat[i+7][j+3 ]  * kernel[7][3 ])  +
				(zpad_mat[i+7][j+4 ]  * kernel[7][4 ])  +
				(zpad_mat[i+7][j+5 ]  * kernel[7][5 ])  +
				(zpad_mat[i+7][j+6 ]  * kernel[7][6 ])  +
				(zpad_mat[i+7][j+7 ]  * kernel[7][7 ])  +
				(zpad_mat[i+7][j+8 ]  * kernel[7][8 ])  +
				(zpad_mat[i+7][j+9 ]  * kernel[7][9 ])  +
				(zpad_mat[i+7][j+10]  * kernel[7][10])  +
				(zpad_mat[i+8][j+0 ]  * kernel[8][0 ])  +
				(zpad_mat[i+8][j+1 ]  * kernel[8][1 ])  +
				(zpad_mat[i+8][j+2 ]  * kernel[8][2 ])  +
				(zpad_mat[i+8][j+3 ]  * kernel[8][3 ])  +
				(zpad_mat[i+8][j+4 ]  * kernel[8][4 ])  +
				(zpad_mat[i+8][j+5 ]  * kernel[8][5 ])  +
				(zpad_mat[i+8][j+6 ]  * kernel[8][6 ])  +
				(zpad_mat[i+8][j+7 ]  * kernel[8][7 ])  +
				(zpad_mat[i+8][j+8 ]  * kernel[8][8 ])  +
				(zpad_mat[i+8][j+9 ]  * kernel[8][9 ])  +
				(zpad_mat[i+8][j+10]  * kernel[8][10])  +
				(zpad_mat[i+9][j+0 ]  * kernel[9][0 ])  +
				(zpad_mat[i+9][j+1 ]  * kernel[9][1 ])  +
				(zpad_mat[i+9][j+2 ]  * kernel[9][2 ])  +
				(zpad_mat[i+9][j+3 ]  * kernel[9][3 ])  +
				(zpad_mat[i+9][j+4 ]  * kernel[9][4 ])  +
				(zpad_mat[i+9][j+5 ]  * kernel[9][5 ])  +
				(zpad_mat[i+9][j+6 ]  * kernel[9][6 ])  +
				(zpad_mat[i+9][j+7 ]  * kernel[9][7 ])  +
				(zpad_mat[i+9][j+8 ]  * kernel[9][8 ])  +
				(zpad_mat[i+9][j+9 ]  * kernel[9][9 ])  +
				(zpad_mat[i+9][j+10]  * kernel[9][10])  +
				(zpad_mat[i+10][j+0]  * kernel[10][0])  +
				(zpad_mat[i+10][j+1]  * kernel[10][1])  +
				(zpad_mat[i+10][j+2]  * kernel[10][2])  +
				(zpad_mat[i+10][j+3]  * kernel[10][3])  +
				(zpad_mat[i+10][j+4]  * kernel[10][4])  +
				(zpad_mat[i+10][j+5]  * kernel[10][5])  +
				(zpad_mat[i+10][j+6]  * kernel[10][6])  +
				(zpad_mat[i+10][j+7]  * kernel[10][7])  +
				(zpad_mat[i+10][j+8]  * kernel[10][8])  +
				(zpad_mat[i+10][j+9]  * kernel[10][9])  +
				(zpad_mat[i+10][j+10] * kernel[10][10]);
		}
	}
}

// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------


void conv2D_accl(int fm_size, int krn_size, void* spm_fm, void* spm_krn, void* spm_res)
{
	void* spmaddrAoff  = (void*)(spm_fm);  // Input Matrix stays in SPM_A
	void* spmaddrBoff  = (void*)(spm_krn); // Input Kernel stays in SPM_B
	void* spmaddrBoff2 = (void*)(spmaddrBoff+krn_size*krn_size*SIZE_OF_INT); // Intermediate results go to SPM_B
	void* spmaddrCoff  = (void*)(spm_res); // Final results go to SPM_C

	int kernel_element=0;

	CSR_MVSIZE(fm_size*SIZE_OF_INT);
	for(int i=0; i<fm_size; i++)
	{
		for (int k=0; k<krn_size; k++)
		{
			for (int l=0; l<krn_size; l++)
			{
				ksvmulsc((void*)( (int*)(spmaddrBoff2)) + SIZE_OF_INT*fm_size*i,
						 (void*)( (int*) spmaddrAoff    + (i+k)*(fm_size+(krn_size-1))+l),
						 (void*)( (int*) spmaddrBoff    + kernel_element++) );
				kaddv((void*)((int*)(spmaddrCoff))  + SIZE_OF_INT*fm_size*i,
					  (void*)((int*)(spmaddrCoff))  + SIZE_OF_INT*fm_size*i,
					  (void*)((int*)(spmaddrBoff2)) + SIZE_OF_INT*fm_size*i);
			}
		}
		kernel_element=0;
	}
}

// Unrolling the KSVMULSC in the LOOP for a 16x16 matrix and 3x3 (or 9 elements) kernel

// [0][0]*[0], [0][1]*[0], [0][2]*[0], [0][3]*[0], [0][4]*[0], .... ,[0][15]*[0],
// [0][1]*[1], [0][2]*[1], [0][3]*[1], [0][4]*[1], [0][5]*[1], .... ,[0][16]*[1],
// [0][2]*[2], [0][3]*[2], [0][4]*[2], [0][5]*[2], [0][6]*[2], .... ,[0][17]*[2],

// [1][0]*[3], [1][1]*[3], [1][2]*[3], [1][3]*[3], [1][4]*[3], .... ,[1][15]*[3],
// [1][1]*[4], [1][2]*[4], [1][3]*[4], [1][4]*[4], [1][5]*[4], .... ,[1][16]*[4],
// [1][2]*[5], [1][3]*[5], [1][4]*[5], [1][5]*[5], [1][6]*[5], .... ,[1][17]*[5],

// [2][0]*[6], [2][1]*[6], [2][2]*[6], [2][3]*[6], [2][4]*[6], .... ,[2][15]*[6],
// [2][1]*[7], [2][2]*[7], [2][3]*[7], [2][4]*[7], [2][5]*[7], .... ,[2][16]*[7],
// [2][2]*[8], [2][3]*[8], [2][4]*[8], [2][5]*[8], [2][6]*[8], .... ,[2][17]*[8],


// [1][0]*[0], [1][1]*[0], [1][2]*[0], [1][3]*[0], [1][4]*[0], .... ,[1][15]*[0],
// [1][1]*[1], [1][2]*[1], [1][3]*[1], [1][4]*[1], [1][5]*[1], .... ,[1][16]*[1],
// [1][2]*[2], [1][3]*[2], [1][4]*[2], [1][5]*[2], [1][6]*[2], .... ,[1][17]*[2],

// [2][0]*[3], [2][1]*[3], [2][2]*[3], [2][3]*[3], [2][4]*[3], .... ,[2][15]*[3],
// [2][1]*[4], [2][2]*[4], [2][3]*[4], [2][4]*[4], [2][5]*[4], .... ,[2][16]*[4],
// [2][2]*[5], [2][3]*[5], [2][4]*[5], [2][5]*[5], [2][6]*[5], .... ,[2][17]*[5],

// [3][0]*[6], [3][1]*[6], [3][2]*[6], [3][3]*[6], [3][4]*[6], .... ,[3][15]*[6],
// [3][1]*[7], [3][2]*[7], [3][3]*[7], [3][4]*[7], [3][5]*[7], .... ,[3][16]*[7],
// [3][2]*[8], [3][3]*[8], [3][4]*[8], [3][5]*[8], [3][6]*[8], .... ,[3][17]*[8],

//                           ...
//                           ...
//                           ...
//                           ...

// [15][0]*[0], [15][1]*[0], [15][2]*[0], [15][3]*[0], [15][4]*[0], .... ,[15][15]*[0],
// [15][1]*[1], [15][2]*[1], [15][3]*[1], [15][4]*[1], [15][5]*[1], .... ,[15][16]*[1],
// [15][2]*[2], [15][3]*[2], [15][4]*[2], [15][5]*[2], [15][6]*[2], .... ,[15][17]*[2],

// [16][0]*[3], [16][1]*[3], [16][2]*[3], [16][3]*[3], [16][4]*[3], .... ,[16][15]*[3],
// [16][1]*[4], [16][2]*[4], [16][3]*[4], [16][4]*[4], [16][5]*[4], .... ,[16][16]*[4],
// [16][2]*[5], [16][3]*[5], [16][4]*[5], [16][5]*[5], [16][6]*[5], .... ,[16][17]*[5],

// [17][0]*[6], [17][1]*[6], [17][2]*[6], [17][3]*[6], [17][4]*[6], .... ,[17][15]*[6],
// [17][1]*[7], [17][2]*[7], [17][3]*[7], [17][4]*[7], [17][5]*[7], .... ,[17][16]*[7],
// [17][2]*[8], [17][3]*[8], [17][4]*[8], [17][5]*[8], [17][6]*[8], .... ,[17][17]*[8],


// // --------------------------------------------------------------------------------------------------------------------------------
// // --------------------------------------------------------------------------------------------------------------------------------

