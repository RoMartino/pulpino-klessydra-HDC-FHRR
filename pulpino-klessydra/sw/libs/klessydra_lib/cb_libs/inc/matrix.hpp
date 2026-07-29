// 
// ███╗░░░███╗░█████╗░████████╗██████╗░██╗██╗░░██╗
// ████╗░████║██╔══██╗╚══██╔══╝██╔══██╗██║╚██╗██╔╝
// ██╔████╔██║███████║░░░██║░░░██████╔╝██║░╚███╔╝░
// ██║╚██╔╝██║██╔══██║░░░██║░░░██╔══██╗██║░██╔██╗░
// ██║░╚═╝░██║██║░░██║░░░██║░░░██║░░██║██║██╔╝╚██╗
// ╚═╝░░░░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░╚═╝░░╚═╝╚═╝╚═╝░░╚═╝
// 
// ██╗░░░░░██╗██████╗░██████╗░░█████╗░██████╗░██╗░░░██╗
// ██║░░░░░██║██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗░██╔╝
// ██║░░░░░██║██████╦╝██████╔╝███████║██████╔╝░╚████╔╝░
// ██║░░░░░██║██╔══██╗██╔══██╗██╔══██║██╔══██╗░░╚██╔╝░░
// ███████╗██║██████╦╝██║░░██║██║░░██║██║░░██║░░░██║░░░
// ╚══════╝╚═╝╚═════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░
/*-------------------------------------------------------------------------------------------------------
Script:      Matrix.hpp
Description: Matrix management library. Fixed Point matrix declaration and matrix arithmetic operations.
			 Supported operations: sum, sub, mul, inversion, reshape, transpose
			 Accelerated operations: sum, mul, inversion
			 Other functions: print, print_F

New: 		ACCL flag allows you to choose whether to use accelerated or not accelerated operations.
			COMPARISON flag allows testing the acceleration effect on clock cycles.
			
Data:        17/05/2022
Version:     1.0
-------------------------------------------------------------------------------------------------------*/
#ifndef _matrix_hpp_
#define _matrix_hpp_

#include "fixed_point.hpp"

// Flag defines: 
#define SIZE_OF_INT 4   // Int size: used in accelerated operations.
#define ACCL 1					// Acceleration flag: set to 1 if you want to use the accelerated SUM, MULT and INVERSE operations.
#define comparison 0 		// Comparison flag:   set to 1 if you want to compare the accelerated results and the not accelerated one.
#define DEBUG 0         // Debug flag: 	      set to 1 if you want to print DEBUG infos such as: L, U, Z and inverse matrix.

// --------------------------- Matrix Library: --------------------------- 

// Matrix Definition
template<int rows,int cols>
union matrix{
 	fixed_point value[rows][cols]={{0}};			// Matrix is initialized to zero by default
  matrix& operator=(const matrix& other)		// Assign operator
  {
      if (this != &other) // not a self-assignment
      {
       	for (int i=0;i<rows;i++)
					for (int j=0;j<cols;j++)
       			value[i][j].num = other.value[i][j].num;
  	  }
      return *this;
  }
};


// Matrix functions:
// 1) Print Matrix function: print all matrix elements as integers
template<int rows,int cols>
void print(matrix<rows,cols> tmp)
{
	for (int i=0;i<rows;i++)
	{
		for (int j=0;j<cols;j++)
		{
			printf("%d ",tmp.value[i][j].num);
		}
		if (i!=rows-1)
				printf(";");
	}	
}

// 2) Print_float matrix function: print all matrix elements as floats
template<int rows,int cols> 
void print_F(matrix<rows,cols> tmp)
{
	for (int i=0;i<rows;i++)
	{
		for (int j=0;j<cols;j++)
				printf("%2.3f ",tmp.value[i][j].to_float());
			if (i!=rows-1)
				printf(";");
	}	
}

// 3) Identity Matrix: this function returns an identity matrix of size R x C
template<int rows,int cols>
matrix<rows,cols> identity()
{
	matrix<rows,cols> i_matrix;
	for (int i=0;i<rows;i++)
	  for (int j=0;j<cols;j++)
	    if (i==j)
				i_matrix.value[i][j].num=1<<SHIFT_AMOUNT;
	return i_matrix;
}


// 4) Zero Matrix: this function returns an identity matrix of size R x C
template<int rows,int cols>
matrix<rows,cols> zero()
{
	matrix<rows,cols> z_matrix;			
	return z_matrix;
}

// 4) Transpose operation: this function take as input an R x C matrix and returns its transposed as output
template<int cols,int rows>
matrix<cols,rows> transpose(matrix<rows,cols> ptr)
{
	matrix<cols,rows>result;
	for (int i=0;i<cols;i++)
	  for (int j=0;j<rows;j++)				
			result.value[i][j].num = ptr.value[j][i].num;
	return result;
}



//5) Sum operation: 
// This function is implemented in two ways:
// - Not accelerated: selected if ACCL flag is equal to 0
// - Accelerated: 	  selected if ACCL 1 is equal to 1
template<int rows_1,int cols_1, int rows_2,int cols_2>
matrix<rows_2,cols_2> sum(matrix<rows_1,cols_1> A, matrix<rows_2,cols_2> B,int core=0)
{
	matrix<rows_2,cols_2> result;
	 	
	if (rows_1 != rows_2 || cols_1 != cols_2)
		printf("\e[91mWarning:\e[39m wrong sum initialization. Used Matrices have a size of: %ix%i & %ix%i\n", rows_1, cols_1, rows_2, cols_2);

	#if ACCL==0
	else
	{
	  for (int i=0;i<rows_1;i++)
	    for (int j=0;j<cols_1;j++)
				result.value[i][j] = A.value[i][j] + B.value[i][j];
	}
	#else
	else
	{
	// Accelerated:
		#if comparison==1
			start_count();																					// Performance counting (Clock Cycle)
		#endif

  		int offset_matrix=core*rows_1*cols_1;
		CSR_MVSIZE(rows_1*cols_1*SIZE_OF_INT);   															// Size of next arithmetic instructions
		kbcast((void *)((int*)spmaddrA+offset_matrix), (void*)0);	   										// Zero initialization of SPM A
		kbcast((void *)((int*)spmaddrB+offset_matrix), (void*)0);    										// Zero initialization of SPM B
		kbcast((void *)((int*)spmaddrC+offset_matrix), (void*)0);    										// Zero initialization of SPM C

		kmemld((void *)((int *)spmaddrA+offset_matrix), &A.value[0][0],  SIZE_OF_INT*(rows_1*cols_1));		// Matrix A Load in SPM A
		kmemld((void *)((int *)spmaddrB+offset_matrix), &B.value[0][0],  SIZE_OF_INT*(rows_1*cols_1));		// Matrix B Load in SPM B

		CSR_MVSIZE(rows_1*cols_1*SIZE_OF_INT);																// Size of next arithmetic instructions

		kaddv((void *)((int *)spmaddrC+offset_matrix),(void *)((int *)spmaddrA+offset_matrix),(void *)((int *)spmaddrB+offset_matrix));	// Sum between SPM. The result is saved in SPM C
		kmemstr(&result.value[0][0],  (void *)((int *)spmaddrC+offset_matrix), rows_1*cols_1*SIZE_OF_INT);	// Memory store. The result is saved in result matrix
	
		#if comparison==1
			int cycle1=finish_count();

			// Non accelerated Sum operation:
			start_count();
			matrix<rows_2,cols_2> result2;
			for (int i=0;i<rows_1;i++)
		    	for (int j=0;j<cols_1;j++)
					result2.value[i][j] = A.value[i][j] + B.value[i][j];
			int cycle2=finish_count();
	
		// Performance results:
		if(Klessydra_get_coreID()==0)
		{	
			printf("----------------\n");
			printf("Acc: %d ---->\n",cycle1); 
			#if DEBUG==1
				print<rows_2,cols_2>(result);
				printf("\n");
			#endif
			printf("Std: %d ---->\n",cycle2);							
			#if DEBUG==1
				print<rows_2,cols_2>(result2);
				printf("\n");
			#endif
			}
			#endif
		}
	#endif
return result;
}		


// 6) Subtraction operation: function that performs subtraction between two matrices R x C
template<int rows_1,int cols_1, int rows_2,int cols_2>
matrix<rows_2,cols_2> sub(matrix<rows_1,cols_1> A, matrix<rows_2,cols_2> B)
{
 	matrix<rows_2,cols_2> result;
	if (rows_1 != rows_2 || cols_1 != cols_2)
		printf("\e[91mError:\e[39m sum between matrix with different sizes\n");
	else
	{
	  for (int i=0;i<rows_1;i++)
	    for (int j=0;j<cols_1;j++)
				result.value[i][j] = A.value[i][j] - B.value[i][j];
	}
	return result;
}

//7) Multiplication operation:
// This function performs the multiplication operation between two matrices. It is implemented in two different ways:
// - Not accelerated: selected if ACCL flag is equal to zero;
// - Accelerated:     selected if ACCL flag is equat to one.
#if ACCL==0
	template<int rows_1,int cols_1, int rows_2,int cols_2>
	matrix<rows_1,cols_2> mult(matrix<rows_1,cols_1> A, matrix<rows_2,cols_2> B,int core=0)
	{
		matrix<rows_1,cols_2> mult;
		if (cols_1!=rows_2)
		      	printf("\e[91mError:\e[39m product operations is not possible. Used matrices'sizes: (%ix%i)*(%ix%i)\n", rows_1,cols_1,rows_2,cols_2);
		else
		{
			for (int i=0;i<rows_1;i++)				// For every row i
			 for (int j=0;j<cols_2;j++)				// And every column j
			   for(int k=0;k<rows_2;k++)			// the (i-j) element will be the sum of every element in row i (first matrix) and all element in column j (second matrix)
						mult.value[i][j]= mult.value[i][j] + (A.value[i][k]*B.value[k][j]);
		}
		return mult;
	}
#else
	template<int rows_1,int cols_1, int rows_2,int cols_2>
	matrix<rows_1,cols_2> mult(matrix<rows_1,cols_1> A, matrix<rows_2,cols_2> B,int core=0)
	{
		// Multiplication between two fixed point numbers can be done in two ways:
		// - optimal: every number is casted to long type, they are multiplied and finally they are shifted back in the original format
		// - Every number is preshifted by a fixed number of bytes. This avoids using long types. However, the shift certainly loose precision.
	 	matrix<rows_1,cols_2> mult;

		if (cols_1!=rows_2)
		      	printf("\e[91mError:\e[39m product operations is not possible. Used matrices'sizes: (%ix%i)*(%ix%i)\n", rows_1,cols_1,rows_2,cols_2);
		else
		{
			// Accelerated mode:
			// Algorithm explaination:
			// At the start the B matrix is loaded in SPM
			// For every iteration i, the offset allows taking every B's row. In fact:
			//  	i=0 -> j=0: spmaAddrA + 0      	 = point B[0][0] and take the entire row thanks to CSR_MVSIZE = cols_2*SIZE_OF_INT
			//			-> j=1: spmaAddrA + cols_2 	 = point B[1][0] and take the entire row thanks to CSR_MVSIZE = cols_2*SIZE_OF_INT
			//			-> j=2: spmaAddrA + 2*cols_2 = point B[2][0] and take the entire row thanks to CSR_MVSIZE = cols_2*SIZE_OF_INT
			//			-> j=3: spmaAddrA + 3*cols_2 = point B[3][0] and take the entire row thanks to CSR_MVSIZE = cols_2*SIZE_OF_INT

			// This row is then multiplicated by coefficient A[i][j]
			// This working principle is shown below:
			// i  j   
			// 0  0  (b_00,b_01,b_02,b_03)*a_00 = (a_00*b_00,a_00*b_01,a_00*b_02,a_00*b_03) Note how for every iteration we have a vector
			// 0  1  (b_10,b_11,b_12,b_13)*a_01 = (a_01*b_10,a_01*b_11,a_01*b_12,a_01*b_13) Note how for every iteration we have a vector
			// 0  2  (b_20,b_21,b_22,b_23)*a_02 = (a_02*b_20,a_02*b_21,a_02*b_22,a_02*b_23) Note how for every iteration we have a vector
			// 0  3  (b_30,b_31,b_32,b_33)*a_03 = (a_03*b_30,a_03*b_31,a_03*b_32,a_03*b_33) Note how for every iteration we have a vector
			// Result for i=0:					  (a_00*b_00+a_01*b_10+a_02*b_20+a_03*b_30, ..., ..., ....) 
			// These are exactly the desired ones.
			//start_count();

			int zero=0; int k,a; int shift=5; int offset;

			#if comparison==1
				start_count();																		// Performance counting (Clock Cycle)
			#endif

			// Different offset are used when core != 0 (default value).
				// This is usefule when multiple accelerated operations are performed in parallel.
				// All operations, in fact, are implemented using SPMs and working by addressing the data inside them.
				// However, when more than one of these operations is carried out in parallel 
				// (for example because you want thread 0 to multiply and thread 1 to do another), it happens that, pointing to the same addresses in SPMs,
				// the data will tend to "mix". In fact, having an interleaved execution, the same addresses will be used alternately by the first and 
				// the second multiplication. To solve this problem an offset proportional to the number of thread is added to every address.
			int offset_core_matrix = core*rows_1*cols_2;
			int offset_core_matrix2 = core*rows_2*cols_2;
			int offset_core_vector = core*cols_2;



			#if DEBUG==1
			if (Klessydra_get_coreID()==0)
			{	// DEBUG
				printf("Init\n");
				printf("spmaddrA -> 0x%x\n",spmaddrA);
				printf("spmaddrB -> 0x%x\n",spmaddrB);
				printf("spmaddrC -> 0x%x\n",spmaddrC);
				printf("spmaddrD -> 0x%x\n",spmaddrD);
				//printf("spmaddrG -> 0x%x\n",KLESS_SPM_G);
			}
			#endif

			// Initializations:
			CSR_MVSIZE(rows_2*cols_2*SIZE_OF_INT);   											// Size of next arithmetic instructions
			kbcast((void *)((int *)spmaddrA+offset_core_matrix2), (void*)zero);	   											// Zero initialization of SPM A
			
			CSR_MVSIZE(cols_2*SIZE_OF_INT);				   										// Size of next arithmetic instructions
			kbcast((void *)((int *)spmaddrB+offset_core_vector), (void*)zero);    											// Zero initialization of SPM B

			CSR_MVSIZE(rows_1*cols_2*SIZE_OF_INT);				   								// Size of next arithmetic instructions
			kbcast((void *)((int *)spmaddrC+offset_core_matrix), (void*)zero);    											// Zero initialization of SPM C
			
			CSR_MVSIZE(rows_2*cols_2*SIZE_OF_INT); 												// Size of next arithmetic instructions
			
			kmemld((void *)((int *)spmaddrA+offset_core_matrix2), &B.value[0][0],  SIZE_OF_INT*(rows_2*cols_2));	// Load B 
			ksrav((void*)((int *)spmaddrA+offset_core_matrix2), (void *)((int *)spmaddrA+offset_core_matrix2), (void*)shift);			// Pre shifting of B matrix's elements 

			CSR_MVSIZE(cols_2*SIZE_OF_INT);														// Arithmetic operation vector size
			for(int i=0;i<rows_1;i++)
			{
				k=0;
				kbcast((void *)((int *)spmaddrB+offset_core_vector),(void *)zero);
				for(int j=0;j<cols_1;j++)
				{
					offset=k*cols_2;
					a = (A.value[i][j].num >> shift);
					ksvmulrf((void *)((int *)spmaddrC+offset_core_matrix),(void*)((int *)spmaddrA+offset+offset_core_matrix2), (void*)a); 		// Multiplication
					kaddv((void *)((int *)spmaddrB+offset_core_vector),(void *)((int *)spmaddrB+offset_core_vector),(void *)((int *)spmaddrC+offset_core_matrix));	// Incremental add
					k++;
				}

				ksrav((void*)((int *)spmaddrB+offset_core_vector), (void *)((int *)spmaddrB+offset_core_vector), (void*)(shift));					// Shift back
				kmemstr(&mult.value[i][0],  (void *)((int *)spmaddrB+offset_core_vector),  cols_2*SIZE_OF_INT);				// Memory storing. The result is saved in mult matrix
			}

			#if comparison==1			
				int cycle1 = finish_count();
				matrix<rows_1,cols_2> mult2;
				start_count();
				// Non accelerated:
				for (int i=0;i<rows_1;i++)				// For every row 
				 for (int j=0;j<cols_2;j++)				// And every column j
			   		for(int k=0;k<rows_2;k++)			// the (i-j) element will be the sum of every element in row i (first matrix) and all element in column j (second matrix)
							mult2.value[i][j]= mult2.value[i][j] + (A.value[i][k]*B.value[k][j]);
				int cycle2=finish_count();	

				// Performance results:
				if(Klessydra_get_coreID()==0)
				{	
					printf("----------------\n");
					printf("Acc: %d\n",cycle1);
					printf("Std: %d\n",cycle2);
					
					#if DEBUG==1
						printf("ACCL: -->\n");
		    		print<rows_1,cols_2>(mult);
		    		printf("\n");
		    		printf("STD: -->\n");
		    		print<rows_1,cols_2>(mult2);
		    		printf("\n");			
		    	#endif
				}
			#endif
		}
		return mult;
	}
#endif


// 6) Constant multiplication
template<int rows, int cols>
matrix<rows,cols> mult_coeff(matrix<rows,cols> m,float coeff)
{
	fixed_point coeff_fp;
	coeff_fp=coeff;

	for (int i=0;i<rows;i++)
	  for (int j=0;j<cols;j++)
	    m.value[i][j]=m.value[i][j]*coeff_fp;	
	return m;
}


// Matrix reshape: matrix to column vector(-1,1)
template<int final_rows,int initial_rows,int initial_cols>
matrix<final_rows,1> reshape(matrix<initial_rows,initial_cols> m)
{
	matrix<final_rows,1> result;
	int count=0;
	for (int i=0;i<initial_rows;i++)
		for (int j=0;j<initial_cols;j++)
			result.value[count++][0]=m.value[i][j];

	return result;
}


// Matrix inversion with LU decomposition:
template<int rows,int cols>
matrix<rows,cols>LU_decomposition(matrix<rows,cols> matrice,int core=0)
{
	#if ACCL==0

		 	matrix<rows,cols> u=zero<rows,cols>();			// Upper triangular matrix U declaration
			matrix<rows,cols> l=zero<rows,cols>();			// Lower triangular matrix L declaration									
			int i,j,k;	int numero_colonne=cols, numero_righe=rows; 
		 
		 
		    // u11,u12,u13,u14 coefficient initialization
		    for (j=0;j<numero_colonne;j++)
		       	u.value[0][j]=matrice.value[0][j];   
		      
		    // l11,l21,l31,l41 coefficient initialization
		    for (i=0;i<numero_righe;i++)
		       	l.value[i][0]=matrice.value[i][0]/u.value[0][0];
		    
		    for (i=0;i<rows;i++)
		    {
		    	for (j=0;j<cols;j++)
		      	    if(i==j)
		        		l.value[i][j]=1;
		    }

		 	for (i=0;i<numero_righe;i++)
		 	{
		 		for (j=0;j<numero_colonne;j++)
		 		{ 
			 	   if (i<=j)
			 	   {
			 	     	u.value[i][j]=matrice.value[i][j]; 			//  u[i][j]= a[i][j]- (sommatoria su k) (u[k][j] l[i][k])
			 	     	for (k=0;k<i;k++)
			 	      		u.value[i][j]= u.value[i][j] - l.value[i][k]*u.value[k][j];       	   

				       	if (i==j)
			       		    l.value[i][j]=1;
			       		else 
						    l.value[i][j]=0;      	      
		       }     
		       else 
		       {
	          	u.value[i][j]= 0;
  	         	l.value[i][j] = matrice.value[i][j];
	           	for (k=0;k<j;k++)
		 	     			l.value[i][j] = l.value[i][j] - l.value[i][k]*u.value[k][j];       	      	
			 	     	l.value[i][j] = l.value[i][j] / u.value[j][j];
		       }
		     	}
		  	}
		  	  // Inversion:
			  //1) compute z
			  fixed_point sum=0;
			  fixed_point result=0;
			  matrix<rows,cols>inverse;
			  inverse=zero<rows,cols>();
			  matrix<rows,cols>I = identity<rows,cols>();
			  matrix<rows,cols>Z = zero<rows,cols>();
			   	   	  	   
			 	for(int col = 0; col < cols; col++) 
			 	{
				  for(int row = 0; row < rows; row++) 
				  {
						sum=0;
				    	for(int i = 0; i < rows; i++)
						    if(i != row) 		        
						      sum = sum + l.value[row][i] * Z.value[i][col];

						result = I.value[row][col] - sum;
						Z.value[row][col] = result / l.value[row][row];	   
				  }
				}
				  
				// compute inverse
				for(int col = 0; col < cols; col++)
				{
				  for(int row = rows - 1; row >= 0; row--) 
				  {
				  	sum=0;
				  	for(int i = 0; i < rows; i++) 
							if(i != row) 
						  	 sum = sum + u.value[row][i]*inverse.value[i][col];

						result = Z.value[row][col] - sum;
						inverse.value[row][col] = result / u.value[row][row];   	  
				  }    
				}
	#else
		// ACCELERATED LU DECOMPOSITION:
		// Different offset are used when core != 0 (default value).
			// This is usefule when multiple accelerated operations are performed in parallel.
			// All operations, in fact, are implemented using SPMs and working by addressing the data inside them.
			// However, when more than one of these operations is carried out in parallel 
			// (for example because you want thread 0 to multiply and thread 1 to do another), it happens that, pointing to the same addresses in SPMs,
			// the data will tend to "mix". In fact, having an interleaved execution, the same addresses will be used alternately by the first and 
			// the second multiplication. To solve this problem an offset proportional to the number of thread is added to every address.
		int offset_core_matrix = core*rows*cols;
		int offset_core_vector = core*cols;

		#if comparison==1
			int cycle1, cycle2, cycle;
			cycle1=0;
			cycle2=0;
			start_count();
		#endif

		// Initializations:
		matrix<rows,cols> u = matrice;													// Upper triangular matrix U declaration
		matrix<rows,cols> l = matrice;													// Lower triangular matrix L declaration									
		int shift = 5;  int offset_k=0; int offset_i=0; 

		// SPMs Initializations:
		CSR_MVSIZE(rows*cols*SIZE_OF_INT);   											// Size of next arithmetic instructions
		kbcast((void *)((int *)spmaddrA+offset_core_matrix), (void*)0);	   										// Zero initialization of SPM A
		kbcast((void *)((int *)spmaddrB+offset_core_matrix), (void*)0);    										// Zero initialization of SPM B
		CSR_MVSIZE(cols*SIZE_OF_INT); 
		kbcast((void *)((int *)spmaddrC+offset_core_vector), (void*)0);    										// Zero initialization of SPM C		
		kbcast((void *)((int *)spmaddrD+offset_core_vector), (void*)0);    										// Zero initialization of SPM D		
		
		// U doesn't need initialization. In fact, the first row is already equal to the first row of A.
		// L initialization need a normalization of the first column. The iterative algorithm starts from 1. 
		for (int i=0;i<rows;i++)
		{
		   	l.value[i][0]=matrice.value[i][0]/matrice.value[0][0];	
		}

		// To parallelize the LU decomposition, it's necessary paying attention to coefficient dependency.
			  		// In particular, this requires computing alternatively a row of U and a column of L
			  		// This alternation allows computing more coefficient in parallel. 
			  		// If ACCL is equal to one, in fact, the Klessydra's capability is used to compute:
			  		// - The element of i-th row of matrix U in parallel
			  		// - The element of j-th column of matrix U in parallel
		matrix<cols,rows> l_transpose;
		l_transpose=transpose<rows,cols>(l);
	 	
	 	// L & U matrix preparation:
	 	kmemld((void *)((int *)spmaddrA+offset_core_matrix), &u.value[0][0],  			SIZE_OF_INT*(rows*cols));	// Load Matrix A in SPM A: base for U matrix
	 	kmemld((void *)((int *)spmaddrB+offset_core_matrix), &l_transpose.value[0][0],  SIZE_OF_INT*(rows*cols));	// Load Matrix A in SPM B: base for L matrix  		
	 	CSR_MVSIZE(cols*SIZE_OF_INT);  
		 

		//------------------------- LU FACTORIZATION PROCEDURE: -------------------------	
	 	for (int i=1;i<rows;i++)
	 	{	
	 		offset_i=i*cols;
	 		// Parallel computation of u matrix's i-th row. Algorithm Details:
			  			// Let's make an example: suppose i=2 and a 4x4 matrix:
			  			// - u[2][0] = a[2][0] - l[2][0]u[0][0] - l[2][1]u[1][0]
			  			// - u[2][1] = a[2][1] - l[2][0]u[0][1] - l[2][1]u[1][1]
			  			// - u[2][2] = a[2][2] - l[2][0]u[0][2] - l[2][1]u[1][2]
			  			// - u[2][3] = a[2][3] - l[2][0]u[0][3] - l[2][1]u[1][3]
			  			// We want computing these terms in parallel. To do this we can see that the coefficents of the i-th row are computed using i terms.
			  			// In particular, we can see that these terms are computed using element of a previous row of U.
			  			// So, we can compute the element of a new row u[i][:] in a parallel way taking the previous rows of matrix U and multiplying them 
			  			// by a fixed coefficient l[i][k].
			  			//
			  			// Example:
			  			// In the previous example this means that it's possibile to compute the first term of coefficients u[2][0], u[2][1], u[2][2] & u[2][3]
			  			// in parallel using the row number zero of U and multiplying it by l[2][0].
			  			// The second term for every computation, indeed, will be calculated using l[2][1] and the row one of U.
			  			//
			  			// Using this approach, for every iteration k, we obtain the term number k from the formula of every element in the new row of U.
			  			//
			  			// So with this operation we obtain a (1 x cols) vector in which every element is a coefficient of the new U matrix's row (i-row).
			  			// Iterating k-times and subtracting these vector iteratively, we will obtain the desired coefficient.
			  			// The final difference between these i-rows in SPM C will return the u[i][j] element.	
	 		for (int k=0;k<i;k++)
	 		{	
				offset_k=k*cols;																										// Offset. Used to select the k-row of matrix U		 		
	 			ksrav_v2((void*)((int *)spmaddrC+offset_core_vector), 	  	 (void *)((int *)spmaddrB+offset_k+i+offset_core_matrix),	(void *)shift,SIZE_OF_INT);			// Pre shifting of A matrix's elements 
	 			CSR_MVSIZE(cols*SIZE_OF_INT); 
	 			ksrav((void*)((int *)spmaddrD+offset_core_vector), 		  	 (void *)((int *)spmaddrA+offset_k+offset_core_matrix), 	(void*)shift);							// Pre shifting of U matrix's elements   			
	 			ksvmulsc((void *)((int *)spmaddrD+offset_core_vector),		 (void *)((int *)spmaddrD+offset_core_vector), 				(void*)((int *)spmaddrC+offset_core_vector)); 				// Multiplication between i-row of U and coefficient l[i][k]. This operation return the k term of every u's coefficient belonging to i-th row.
	 			ksrav((void*)((int *)spmaddrC+offset_core_vector),    		 (void *)((int *)spmaddrD+offset_core_vector), 				(void*)(shift));						// Shift back
	 			ksubv((void *)((int *)spmaddrA+offset_core_matrix+offset_i), (void *)((int *)spmaddrA+offset_core_matrix+offset_i), 	(void *)((int *)spmaddrC+offset_core_vector));				// Subtraction
			}
			kmemstr(&u.value[i][0],  					  					 (void *)((int *)spmaddrA+offset_core_matrix+offset_i),  cols*SIZE_OF_INT);						// Memory storing. 
			
			// Parallel computation of L matrix's i-th column using l_transposed. Algorithm Details:
		  			// This operation return the k term of every L's coefficient belonging to i-th columns.
		  			// Let's make an example: suppose i=1  and a 4x4 matrix:
		  			// - l[0][2] = a[0][2] - l[0][0]u[0][2] - l[0][1]u[1][2]
		  			// - l[1][2] = a[1][2] - l[1][0]u[0][2] - l[1][1]u[1][2]
		  			// - l[2][2] = a[2][2] - l[2][0]u[0][2] - l[2][1]u[1][2]
		  			// - l[3][2] = a[3][2] - l[3][0]u[0][2] - l[3][1]u[1][2]
		  			// Using the transpose, to compute the first term we can take the first row of L transpose (It represents the first column of L) and
		  			// multiply it by u[k][i]. Using this approach, for every iteration k, we obtain the term number k from the formula of every element
		  			// in the new column of U.
		  			// Finally we subtract
			for (int k=0;k<i;k++)
	 		{	
	 			offset_k=k*cols;
	 			ksrav_v2((void*)((int *)spmaddrC+offset_core_vector),  	  	 (void *)((int *)spmaddrA+offset_core_matrix+offset_k+i),	(void *)shift, SIZE_OF_INT);			// Pre shifting of A matrix's elements 
	 			CSR_MVSIZE(cols*SIZE_OF_INT); 
	 			ksrav((void*)((int *)spmaddrD+offset_core_vector), 		  	 (void *)((int *)spmaddrB+offset_core_matrix+offset_k), 	(void *)shift);							// Pre shifting of B matrix's elements 
	 			ksvmulsc((void *)((int *)spmaddrD+offset_core_vector),	 	 (void *)((int *)spmaddrD+offset_core_vector), 				(void *)((int*)spmaddrC+offset_core_vector)); 				// Multiplication between i-row of U and coefficient l[i][k]
	 			ksrav((void*)((int *)spmaddrC+offset_core_vector),    		 (void *)((int *)spmaddrD+offset_core_vector), 								(void *)(shift));						// Shift back
	 			ksubv((void *)((int *)spmaddrB+offset_core_matrix+offset_i), (void *)((int *)spmaddrB+offset_core_matrix+offset_i), 	(void *)((int *)spmaddrC+offset_core_vector));				// Subtraction
			}
			kmemstr(&l_transpose.value[i][0], 			  (void *)((int *)spmaddrB+offset_core_matrix+offset_i),  cols*SIZE_OF_INT);						// Memory storing. 
			
			// At the end of this process L_transpose matrix is stored
					// In fact, Klessydra has not an accelerated operation for division. 
					// So, to do this, it's necessary to store the new l-column in memory,
					// divide it and, finally, load it again in SPM.
			for (int c=i;c<cols;c++)
				l_transpose.value[i][c]=l_transpose.value[i][c]/u.value[i][i];												
	
			kmemld((void *)((int *)spmaddrB+offset_core_matrix+offset_i), &l_transpose.value[i][0],  SIZE_OF_INT*cols);						// Load again the updated l matrix
		}	
		l=transpose<cols,rows>(l_transpose);																				// Transposing		
		
		// --------------------------------------------------------------------------------------------------------------------
		// Accelerated inversion: Accelerated one:
			  // parallel computation for more rows:
			  // we have a dependence on Z columns. In fact, the z-element of column j depends by
			  // the the element j of previous rows.
			  // So, obviously we can't parallelize on columns, but we can do it by row. 
			  // For example we can calculate Z_11, Z_12, ..., Z_in. in facte these values are the first
			  // calculus related to different column of C.
			  // so we can do:
			  //		
			  // 1)
			  // [Z_11, Z_12, ..., Z_1n]* [l11, l12, ..., l1n] = [z_11*l11, z_12*l12, ..., z_1n*l_1n]
			  // Compute in parallel Z_11, Z_12, Z_1n as: ([I_11, I_12, ..., I_1n] - [z_11*l11, z_12*l12, ..., z_1n*l_1n])/[l_11, l12, ...l_in]
			  // 
			  // 2)
			  // [Z_11, Z_12, ..., Z_1n]* [l21, l22, ..., l2n] = [z_11*l21, z_12*l22, ..., z_1n*l_2n]
			  // sum it to the previous vector  [z_11*l11, z_12*l12, ..., z_1n*l_1n] + [z_11*l21, z_12*l22, ..., z_1n*l_2n]
		      // and finally compute in parallel 
			  // Z_21, Z_12, Z_1n as: ([I_21, I_22, ..., I_2n] - [z_11(l21+l11), ...])/[l_21, l22, ...l_2n]
			  //
			  // To do this in parallel we can create a matrix with every rows equal to the i-row of Z, multiply (dot) it with the entire l matrix
			  // summing the rows and obtain in parallel Z_21, Z_22 ecc. -> no, sempre sequenziale il calcolo di z cosi

		
		
		// SPMs Initializations:
		CSR_MVSIZE(rows*cols*SIZE_OF_INT);   																				// Size of next arithmetic instructions
		kbcast((void *)((int *)spmaddrB+offset_core_matrix), (void*)0);    					// Zero initialization of SPM B. It will be our Z matrix
		
		CSR_MVSIZE(cols*SIZE_OF_INT); 
		kbcast((void *)((int *)spmaddrC+offset_core_vector), (void*)0);    					// Zero initialization of SPM C		
		kbcast((void *)((int *)spmaddrD+offset_core_vector), (void*)0);    					// Zero initialization of SPM C		
		
		// Matrix Initialization:
		matrix<rows,cols> inverse; 															// Inverse matrix
		matrix<rows,cols> Z;																	  // Z matrix. Initialized to zero
		matrix<rows,cols> I = identity<rows,cols>();
		
													
		// Matrix loads:
		CSR_MVSIZE(rows*cols*SIZE_OF_INT);
		kmemld((void *)((int *)spmaddrA+offset_core_matrix), &l.value[0][0],  SIZE_OF_INT*rows*cols);						// Load the entire L matrix in SPM A
		ksrav((void*)((int *)spmaddrA+offset_core_matrix), (void *)((int *)spmaddrA+offset_core_matrix), 	(void*)shift);	// Pre shifting of L matrix's elements   			
		int offset_j; fixed_point somma = 0; fixed_point result = 0; int i,j;
		

		//------------------------- Z computation: -------------------------
		for (i=0;i<rows;i++)
		{
			offset_i=i*cols;			
			for (j=0;j<cols;j++)
			{
				offset_j=j*cols;
				CSR_MVSIZE(cols*SIZE_OF_INT); 
				ksrav((void*)((int *)spmaddrC+offset_core_vector),	  		 		 (void *)((int *)spmaddrB+offset_core_matrix+offset_j), 	(void*)(shift));									// Shift back

				CSR_MVSIZE(SIZE_OF_INT); 
				kbcast((void*)((int *)spmaddrC+offset_core_vector+i),		 (void*)0);					 

				CSR_MVSIZE(cols*SIZE_OF_INT); 
				kdotpps_emul_v3((void *)((int *)spmaddrD+offset_core_vector),		 (void *)((int *)spmaddrA+offset_core_matrix+offset_i), 	(void*)((int *)spmaddrC+offset_core_vector),(void*)(shift)); 	// Multiplication between the row l[i] and the row i of Z_transpose	
				kmemstr(&somma.num,  (void *)((int *)spmaddrD+offset_core_vector),  SIZE_OF_INT);																				// Memory storing of sum
				
				result = (I.value[i][j] - somma)/l.value[i][i];
				Z.value[j][i].num = result.num;	 
				kmemld((void *)((int *)spmaddrB+offset_core_matrix+offset_j+i), &result,  SIZE_OF_INT);			// Load Z[i][j] in SPM A
			}
		}
		Z = transpose<rows,cols>(Z);



		#if DEBUG==1
			if (Klessydra_get_coreID()==0)
			{
				printf("\nZ_ACCL:\n");
				print<rows,cols>(Z);
				printf("\n");
			}
		#endif

				
		
		// SPMs Initializations:
		CSR_MVSIZE(rows*cols*SIZE_OF_INT);   																// Size of next arithmetic instructions
		//kbcast((void *)((int *)spmaddrA+offset_core_matrix), (void*)zeroi);	   							// Zero initialization of SPM A
		kbcast((void *)((int *)spmaddrB+offset_core_matrix), (void*)0);    									// Zero initialization of SPM B. It will be our Z matrix
		
		CSR_MVSIZE(cols*SIZE_OF_INT); 
		kbcast((void *)((int *)spmaddrC+offset_core_vector), (void*)0);    									// Zero initialization of SPM C		
		kbcast((void *)((int *)spmaddrD+offset_core_vector), (void*)0);    									// Zero initialization of SPM C		
		
		// Matrix loads:
		kmemld((void *)((int *)spmaddrA+offset_core_matrix), &u.value[0][0],  SIZE_OF_INT*rows*cols);			// Load L matrix in SPM A
		CSR_MVSIZE(rows*cols*SIZE_OF_INT);
		ksrav((void*)((int *)spmaddrA+offset_core_matrix), (void *)((int *)spmaddrA+offset_core_matrix), 	(void*)shift);			// Pre shifting of L matrix's elements   			
		
		//-------------------------Inverse computation:-------------------------
		for(int i = rows - 1; i >= 0; i--) 
		{
		  offset_i=i*cols;
		  for(int j = 0; j < cols; j++)
		  {
				
				offset_j=j*cols;
				CSR_MVSIZE(cols*SIZE_OF_INT); 
				ksrav((void*)((int *)spmaddrC+offset_core_vector),			  		 (void *)((int *)spmaddrB+offset_core_matrix+offset_j), 	(void*)(shift));									// Shift back

				CSR_MVSIZE(SIZE_OF_INT); 
				kbcast((void*)((int *)spmaddrC+offset_core_vector+i),		 (void*)0);					 

				CSR_MVSIZE(cols*SIZE_OF_INT);
				kdotpps_emul_v3((void *)((int *)spmaddrD+offset_core_vector),		 (void *)((int *)spmaddrA+offset_core_matrix+offset_i), 	(void*)((int *)spmaddrC+offset_core_vector),(void*)(shift)); 	// Multiplication between the row l[i] and the row i of Z_transpose	
				kmemstr(&somma.num,  (void *)((int *)spmaddrD+offset_core_vector),  SIZE_OF_INT);																				// Memory storing of sum
				result = (Z.value[i][j]-somma) / u.value[i][i];
				inverse.value[j][i].num = result.num;	
				kmemld((void *)((int *)spmaddrB+offset_core_matrix+offset_j+i), &result,  SIZE_OF_INT);			// Load Z[i][j] in SPM A
			}
		}
		inverse=transpose<rows,cols>(inverse);
		
		//DEBUG INVERSE
		#if DEBUG==1
			if (Klessydra_get_coreID()==core)
			{
				printf("\ninverse_ACCL:\n");
				print<rows,cols>(inverse);
				printf("\n");
			}
		#endif
			
		#if comparison==1
		 	fixed_point sum=0;
			cycle1 = finish_count();
			start_count();
			// Initializations of std version:
			matrix<rows,cols> u1=zero<rows,cols>();			// Upper triangular matrix U declaration
			matrix<rows,cols> l1=zero<rows,cols>();			// Lower triangular matrix L declaration									
			int k;
			int numero_colonne=cols, numero_righe=rows; 
		 
		 
		    // u11,u12,u13,u14 coefficient initialization
		    for (j=0;j<numero_colonne;j++)
		       	u1.value[0][j]=matrice.value[0][j];   
		      
		    // l11,l21,l31,l41 coefficient initialization
		    for (i=0;i<numero_righe;i++)
		       	l1.value[i][0]=matrice.value[i][0]/u1.value[0][0];
		        
		    for (i=0;i<rows;i++)
		    {
		    	for (j=0;j<cols;j++)
		    	{
		      	    if(i==j)
		      	    {
		        		l1.value[i][j]=1;
		      	    }
		    	}
		    }
		            
		 	// L & U matrix Computation:   
		 	for (i=0;i<numero_righe;i++)
		 	{
		 		for (j=0;j<numero_colonne;j++)
		 		{ 
			 	   if (i<=j)
			 	   {
			 	     	u1.value[i][j]=matrice.value[i][j]; 			//  u[i][j]= a[i][j]- (sommatoria su k) (u[k][j] l[i][k])
			 	     	for (k=0;k<i;k++)
			 	     	{
			 	      		u1.value[i][j]= u1.value[i][j] - l1.value[i][k]*u1.value[k][j];       	   
			 	     	}
				       	if (i==j)
			       		    l1.value[i][j]=1;
			       		else 
						    l1.value[i][j]=0;      	      
			       }     
			       else 
			       {
			          	u1.value[i][j]= 0;
			           	l1.value[i][j] = matrice.value[i][j];
			           	for (k=0;k<j;k++)
				 	     	l1.value[i][j] = l1.value[i][j] - l1.value[i][k]*u1.value[k][j];       	      	
				 	     		      
			 	     	l1.value[i][j] = l1.value[i][j] / u1.value[j][j];
			       }
		     	}
		  	}
		  // Inversion. The algorithm works as follows:
			  // Using the L & U matrix we can compute the inverse matrix of A remembering that:
			  // The inverse of matrix is that matrix, which multiplied with the given matrix gives the identity I: A * A^-1 = I
			  // So, for example, the first column of the inverse matrix (that we can call B for semplicity) have to be equal to the first column
			  // of identity matrix, as shown below:
			  // [a11, a12, ..., a1n	[b11]   [ 1 ]
			  //  ...  ...  ... ...     [...] = [ 0 ]
			  //  an1, an2, ..., ann]	[bn1]   [ 0 ]
			  // The same it's true for the following columns of inverse matrix.
			  //
			  // Computing inverse matrix, therefore, corresponds to find the values of this columns.
			  // This can be rewritten using the LU decomposition approach as follows:
			  //
			  // [L] [z] = [c] --> we extract Z from this equation using c (column of identity matrix) and L (lower triangular matrix)
			  // [U] [x] = [z] --> we extract x from this equation. It represents the column of inverse matrix.
			  // 
			  // equation like this, with triangular matrices, can be quickly resolved by back-substitution. 
			  // This is equal to do:
			  //	
			  // [l_11,...,l_1n]  [Z_11]   [C_11]
			  // [l_21,...,l_2n]  [Z_21] = [C_21]
			  // [... ,..., ...]  [....]   [....]
			  // [l_n1,...,l_nn]  [Z_n1]   [C_n1]	  	
			  //	
			  // but we know that L is a lower triangular matrix. So:
		  	  // [l_11, 0,   0 , ...   0]
		  	  // [l_21, l22, 0,  ...   0]
		  	  // and so on. This means that we have:
		  	  // l11 * z11 = c11 			 -> Z_11 = c11/l11
		  	  // l21 * z11 + l22 * z21 = c21 -> Z_21 = (c21-l21*z11)/l22
		  	  // ecc. 
		  	  // For this reason, for l the dependency requires to compute the value from the first to the last row.
		  	  // On the contrary U is an upper matrix and so, following the same reasoning, we have to start from the last row.
		  	  // 
		  	  // What explained is true for every column of the inverse matrix. So Z and X will be matrices rows x cols.
		  // Z computation
		  sum=0;  result=0;
		  inverse=zero<rows,cols>();
		  I = identity<rows,cols>();
		  Z = zero<rows,cols>();
		   	   	  	   
		 	for(int row = 0; row < rows; row++)
			  for(int col = 0; col < cols; col++)  
			  {
					sum=0;
			    	for(int i = 0; i < rows; i++)
			    	{
					    if(i != row) 		        
					      sum = sum + l1.value[row][i] * Z.value[i][col];
			    	} 
					
					result = I.value[row][col] - sum;
					Z.value[row][col] = result / l1.value[row][row];	  	    
			  }
				#if DEBUG==1
				if (Klessydra_get_coreID()==0)
				{
					printf("Z_STD:\n");
					print<rows,cols>(Z);
					printf("\n");
				}	  
				#endif
			inverse=zero<rows,cols>();
			for(int row = rows - 1; row >= 0; row--) 
			{
			 for(int col = 0; col < cols; col++)
			  {
			  	sum=0;
			  	for(int i = 0; i < rows; i++) 
						if(i != row) 
					  	 sum = sum + u1.value[row][i]*inverse.value[i][col];
					result = Z.value[row][col] - sum;
					inverse.value[row][col] = result / u1.value[row][row];   	  
			  }    
			}
			#if DEBUG==1
				if (Klessydra_get_coreID()==core)
				{
					printf("\ninverse_STD:\n");
					print<rows,cols>(inverse);
					printf("\n");
				}
			#endif
		  	cycle2=finish_count();
		#endif
		// Final printing:
		#if comparison==1
			if (Klessydra_get_coreID()==core)  {
			  printf("Comparison between ACCL and STD version:\n");
			  #if DEBUG==1
			  printf("\e[94mAccelerated:\e[39m\n");  
			  printf("\e[91mMatrix L:\e[39m\n");
			  print<rows,cols>(l);
			  printf("\n\e[91mMatrix U:\e[39m\n");
			  print<rows,cols>(u);
			  printf("\n\n\e[94mStandard:\e[39m\n");
			  printf("\e[91mMatrix L:\e[39m\n");
			  print<rows,cols>(l1);
			  printf("\n\e[91mMatrix U:\e[39m\n");
			  print<rows,cols>(u1);
			  printf("\n\n\n");
			  #endif
			  printf(" ACCL ----> Requested cycles: %d\n",cycle1);
			  printf(" STD  ----> Requested cycles: %d\n",cycle2); 
		    }
		#endif
	#endif
	return inverse;
}


#endif
