// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
#include"Definitions_fully_test.h"
#define DBG_MSG_SIZE 100
#define SPM_WIDTH 		64 // if SPM_addr_width is 2^14
#define SPM_HEIGH 		SPM_WIDTH
#define SPM_DIMENSION SPM_WIDTH*SPM_HEIGH*SIZE_OF_INT

// --------------------------
//Definese for operative mode
// --------------------------

// #define SINGLE_TH 1
// #define SUB_KERNELS
// #define FAST_ESTIMATION 1

#define ZEROPADDED 1
#define MULTI_TH 1
// #define NO_SPMU


// #define DEDICATED_SPMU 1 //per i FC layers


// --------------------------
//Definese for debug purposes
// --------------------------
// #define INTERCEPT 1
// #define QUICK_DEBUG 1
// #define ISEQUAL_RUNTIME 1

// #define INSIDE_TRACE	1
#define COUNT_CYCLES 1 
// --------------------------
//Definese for active layers
// --------------------------

#define ACTIVE_LAYER_0  1

#define ACTIVE_LAYER_1  1
#define ACTIVE_LAYER_2  1
#define ACTIVE_LAYER_3  1 

#define ACTIVE_LAYER_4  1
#define ACTIVE_LAYER_5  1
#define ACTIVE_LAYER_6  1

#define ACTIVE_LAYER_7  1
#define ACTIVE_LAYER_8  1
// #define CONFRONTA_8 1
#define ACTIVE_LAYER_9  1
#define ACTIVE_LAYER_10 1

#define ACTIVE_LAYER_11 1
#define ACTIVE_LAYER_12 1
// #define CONFRONTA_12 1
#define ACTIVE_LAYER_13 1
#define ACTIVE_LAYER_14 1

#define ACTIVE_LAYER_15 1 
#define ACTIVE_LAYER_16 1 
#define ACTIVE_LAYER_17 1
// // #define LAY_17_CORRECT_INTERCEPT 1
#define ACTIVE_LAYER_18 1
// // #define lay_15_to_17_keep_in_spm 1

#define ACTIVE_LAYER_19 1
#define ACTIVE_LAYER_20 1
#define ACTIVE_LAYER_21 1
#define ACTIVE_LAYER_22 1
// #define ACTIVE_LAYER_OUTPUT 1


int print_global_dbg;
int print_global_id;
int print_global_k;
int mv_fl, mv_bs, mf2, mb2;
int print_global_id;
float appoggio_exp;
int stampadbg;

//   /$$$$$$$$        /$$      /$$
//  | $$_____/       | $$$    /$$$
//  | $$             | $$$$  /$$$$  /$$$$$$   /$$$$$$   /$$$$$$$
//  | $$$$$          | $$ $$/$$ $$ |____  $$ /$$__  $$ /$$_____/
//  | $$__/          | $$  $$$| $$  /$$$$$$$| $$  \ $$|  $$$$$$
//  | $$             | $$\  $ | $$ /$$__  $$| $$  | $$ \____  $$
//  | $$             | $$ \/  | $$|  $$$$$$$| $$$$$$$/ /$$$$$$$/
//  |__/$$$$$$/$$$$$$|__/     |__/ \_______/| $$____/ |_______/
//    |______/______/                       | $$
//                                          | $$
//                                          |__/
union FeatureMaps
{
	int lay_1[64][SIZE_L1_2][SIZE_L1_2]; 		// conv
	int lay_2[64][SIZE_L1_2][SIZE_L1_2]; 		// conv
	int lay_3[64][SIZE_L3_5][SIZE_L3_5]; 		// pool

	int lay_4[128][SIZE_L3_5][SIZE_L3_5]; 		// conv
	int lay_5[128][SIZE_L3_5][SIZE_L3_5]; 		// conv
	int lay_6[128][SIZE_L6_9][SIZE_L6_9];			// pool 

	int lay_7[256][SIZE_L6_9][SIZE_L6_9];				// conv
	int lay_8[256][SIZE_L6_9][SIZE_L6_9];				// conv
	int lay_9[256][SIZE_L6_9][SIZE_L6_9];				// conv
	int lay_10[256][SIZE_L10_13][SIZE_L10_13];		// pool

	int lay_11[512][SIZE_L10_13][SIZE_L10_13];		// conv
	int lay_12[512][SIZE_L10_13][SIZE_L10_13];		// conv
	int lay_13[512][SIZE_L10_13][SIZE_L10_13];		// conv
	int lay_14[512][SIZE_L14_17][SIZE_L14_17];		// pool

	int lay_15[512][SIZE_L14_17][SIZE_L14_17];		// conv
	int lay_16[512][SIZE_L14_17][SIZE_L14_17];		// conv
	int lay_17[512][SIZE_L14_17][SIZE_L14_17];		// conv
	int lay_18[512][SIZE_L18][SIZE_L18];				// pool

	int lay_19[4096];			// fc
	int lay_20[4096];			// fc
	int lay_21[OUTPUT_SIZE];			// fc
	float lay_22[OUTPUT_SIZE];			// softmax
};
//   /$$   /$$                                         /$$
//  | $$  /$$/                                        | $$
//  | $$ /$$/   /$$$$$$   /$$$$$$  /$$$$$$$   /$$$$$$ | $$  /$$$$$$$
//  | $$$$$/   /$$__  $$ /$$__  $$| $$__  $$ /$$__  $$| $$ /$$_____/
//  | $$  $$  | $$$$$$$$| $$  \__/| $$  \ $$| $$$$$$$$| $$|  $$$$$$
//  | $$\  $$ | $$_____/| $$      | $$  | $$| $$_____/| $$ \____  $$
//  | $$ \  $$|  $$$$$$$| $$      | $$  | $$|  $$$$$$$| $$ /$$$$$$$/
//  |__/  \__/ \_______/|__/      |__/  |__/ \_______/|__/|_______/
union KernelVector
{
	int kernel_9[9];
	int kernel_4096[4096];
	int kernel_maxs[SIZE_L18*SIZE_L18*512];
};

int 	performance=0;
int 	perf=0;
int* 	ptr_perf = &perf;
char 	msg[DBG_MSG_SIZE];
int 	single_value_minus	=-1;
int 	single_value_zero		=0;
//    /$$$$$$   /$$
//   /$$__  $$ | $$
//  | $$  \__//$$$$$$    /$$$$$$            /$$$$$$   /$$$$$$   /$$$$$$   /$$$$$$$
//  | $$     |_  $$_/   /$$__  $$          /$$__  $$ /$$__  $$ /$$__  $$ /$$_____/
//  | $$       | $$    | $$  \__/         | $$  \__/| $$$$$$$$| $$  \ $$|  $$$$$$
//  | $$    $$ | $$ /$$| $$               | $$      | $$_____/| $$  | $$ \____  $$
//  |  $$$$$$/ |  $$$$/| $$               | $$      |  $$$$$$$|  $$$$$$$ /$$$$$$$/
//   \______/   \___/  |__//$$$$$$ /$$$$$$|__/       \_______/ \____  $$|_______/
//                        |______/|______/                     /$$  \ $$
//                                                            |  $$$$$$/
//                                                             \______/
int priority_0=1;
int priority_1=0;
int go_1=0;
int go_0=1;
int glb_i=0;
int glb_k=0;
int glb_sub_i=0;
int glb_sub_k=0;
//   /$$$$$$$$                             /$$
//  | $$_____/                            | $$
//  | $$    /$$   /$$ /$$$$$$$   /$$$$$$$/$$$$$$
//  | $$$$$| $$  | $$| $$__  $$ /$$_____/_  $$_/
//  | $$__/| $$  | $$| $$  \ $$| $$       | $$
//  | $$   | $$  | $$| $$  | $$| $$       | $$ /$$
//  | $$   |  $$$$$$/| $$  | $$|  $$$$$$$ |  $$$$/
//  |__/    \______/ |__/  |__/ \_______/  \___/$$$$$$
//                                            |______/
void 	spmReset(int size);
void 	matrixReset_SPM(int size, int *matrix);
void 	start_count();
void 	finish_count(char msgp[]);
void	finish_count_nomsg();
void 	printmod_sel(int layer,int ch, int dim_i, int dim_j, int *input,int shift);
void 	printmod_full(int layer,int ch, int dim_i, int dim_j, int *input, int shift);
void 	dbg_count_zero(int ch, int dim_i, int dim_j,int *input);
void 	spegni_threads();
void 	display_spm_matrix(int size_r,int size_c,void* pt_spm_mat);
void 	display_spm_matrix_X(int size_r,int size_c,void* pt_spm_mat);
void 	display_spm_matrix_scal(int size_r,int size_c,void* pt_spm_mat, int shift);
void 	isequal(int layer, int row, int col, int (*pt1)[row], const int *pt2, int id);
void 	isequal_v2(int layer, int row, int col, int *pt1, const int *pt2, int id);
void convolution2D_SPM_off						(void* spm_dest, void* spm_fm, void* spm_krn, int size, int mem_off);
void convolution2D_SPM_off_ALT				(void* spm_dest, void* spm_fm, void* spm_krn, void* spm_bank, int size); //for layer 7 going forward
void convolution2D_SPM_off_ALT_2x2		(void* spm_dest, void* spm_fm, void* spm_krn, void* spm_bank, int size); //for layer 15
void convolution2D_SPM_off_NOB				(void* spm_dest, void* spm_fm, void* spm_krn,	void* spm_temp,  int size);//latest iteration of the convolution code
void convolution2D_SPM_off_NOB_2x2(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void convolution2D_SPM_off_NOB_print(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
//void convolution2D_SPM_off_NOB_multi				(void* spm_dest, void* spm_fm, void* spm_krn,	void* spm_temp,  int size);//latest iteration of the convolution code
void convolution2D_SPM_off_NOB_zeropadded(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size);
void matrix_check_v2( int* mat1, int* mat2, int size);
volatile int depe=0;
// noinline void chekmark(){
void __attribute__ ((noinline))mark(){
	depe++;
	if (depe==1000){
		printf("CIAO!\n");
	}
}
//   /$$      /$$           /$$
//  | $$$    /$$$          |__/
//  | $$$$  /$$$$  /$$$$$$  /$$ /$$$$$$$
//  | $$ $$/$$ $$ |____  $$| $$| $$__  $$
//  | $$  $$$| $$  /$$$$$$$| $$| $$  \ $$
//  | $$\  $ | $$ /$$__  $$| $$| $$  | $$
//  | $$ \/  | $$|  $$$$$$$| $$| $$  | $$
//  |__/     |__/ \_______/|__/|__/  |__/

int perf_results[4]={0};
union FeatureMaps odds;
union FeatureMaps even;
int input_int[CHANNEL_SIZE][INPUT_SIZE][INPUT_SIZE];
int flattened[SIZE_L18*SIZE_L18*512];

int main(int argc, char *argv[]){
	#ifdef SINGLE_TH
		spegni_threads();
	#endif
	#ifdef MULTI_TH
		__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt
		sync_barrier_reset(); sync_barrier_thread_registration();	sync_barrier();

		sync_barrier_reset(); sync_barrier_thread_registration();
		if(Klessydra_get_coreID()==0) printf("\n--- Mode: MULTI_THREAD ---\n----------------------------------------\n\tStart_program_Vgg_Fully_fixed\n----------------------------------------\n\n");
		sync_barrier();
	#endif
	#ifdef NO_SPMU
		__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt
		sync_barrier_reset(); sync_barrier_thread_registration();	sync_barrier();

		sync_barrier_reset(); sync_barrier_thread_registration();
		if(Klessydra_get_coreID()==0) printf("\n--- Mode: MULTI_THREAD_NO_SPMU ---\n----------------------------------------\n\tStart_program_Vgg_Fully_fixed\n----------------------------------------\n\n");
		sync_barrier();
	#endif
	union KernelVector kern;
			__asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt

	print_global_dbg = 0;
	print_global_id = 0;
	print_global_k = 0;

	// pointer to features and bias to fetch from main memory
	const int* pt_to_fl;
	const int* pt_to_bs;
	// pointer needed for execution
	int *punt, *punt2, *punt_in,*punt_out;
	int bias;
	
	// values and pointers for layer 18-22 execution
  int total = 0;
	int monoindice=0;
	int errori=0;
	int dimension_out_19=sizeof(odds.lay_19)/SIZE_OF_INT;
	int dimension_out_20=sizeof(even.lay_20)/SIZE_OF_INT;
	int dimension_out_21=sizeof(odds.lay_21)/SIZE_OF_INT;
 	float *punt_lay22;
	float max = 0;
  int maxpos;
	int zero_value=0;
	// value for debug purposes
	int output_per_layer=0;
	int input_per_layer=0;
	int jollypt=0;
	const int * dml[100];
	volatile int kload		=0;
	volatile int kcast		=0;
	volatile int kstore	 = 0;

	int dimension_spm=SPM_DIMENSION;
	int current_dimension;
	int uscite_parallele;
	int ingressi_paralleli;
	int	krn_slide[512] ={0};
	for(unsigned int i=0;i< ( sizeof(krn_slide)/SIZE_OF_INT );i++){//1024 è il numero di locazioni disponibili
		krn_slide[i]=	i*9	;
	}

	int	mem_off_BASE_B[512] ={0};
	int limite_B=16; // Fixed parametr depending on SPM_B size, 16 is it's upper limit in parallel computation
	current_dimension=limite_B*limite_B;//non comprendere il conto degli integers
	mem_off_BASE_B[0]	=	0;
	for(unsigned int i=0; i<( sizeof(mem_off_BASE_B)/SIZE_OF_INT );i++){
		mem_off_BASE_B[i]	=	i*(current_dimension);
	}
	int mem_off_BASE_AC[512] ={0};

	int nano_i=0; 
	int nano_k=0;
	int ripet=0;
									// NB la moltiplicazione 16*16 è necessaria per evitare il problema della grana fine
									// quando potresti andare sotto al fattore di parallelo=16
	int divisor_0=0;
	int divisor_1=0;
	
	//i quickly set the register for the post scaling
	spmReset(SPM_DIMENSION);
	CSR_MVSIZE(2*SIZE_OF_INT);
	kdotpps_v3((void*)spmaddrA,	(void*)spmaddrA,	(void*)spmaddrB, (void*) conv2D_out_scal);
	
	int lay_index			;
	int run_SIZE			;
	int *run_pt_input	;
	int *run_pt_output;
	int	nano_divisor_0;
	int nano_divisor_1;
	int nano_divisor_2;
	
	int thread_outputs[2]={0};
	int th_id;
	int th_output_low;
	int th_output_high;
	int isequal_number=0;
	int uni_ind=0;
	int input_int[CHANNEL_SIZE][INPUT_SIZE][INPUT_SIZE];

	#ifndef NO_SPMU //quindi c'è l'unità di accelerazione
		#ifndef ACTIVE_LAYER_0
		//Layer_0 Zero-mean on the input image
		for (int k = 0; k < 64; k++) {
			matrixReset_SPM(INPUT_SIZE, &odds.lay_1[k][0][0]);
			matrixReset_SPM(INPUT_SIZE, &even.lay_1[k][0][0]);
		}
		#endif
	#else // non c'è l'unità di accelerazione
		#ifndef ACTIVE_LAYER_0
			for (int k = 0; k < 64; k++)
			{
				matrixReset(INPUT_SIZE, &odds.lay_1[k][0][0]);
				matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
			}
		#endif
	#endif
//   /$$                                   /$$$$$$
//  | $$                                  /$$$_  $$
//  | $$        /$$$$$$  /$$   /$$       | $$$$\ $$
//  | $$       |____  $$| $$  | $$       | $$ $$ $$
//  | $$        /$$$$$$$| $$  | $$       | $$\ $$$$
//  | $$       /$$__  $$| $$  | $$       | $$ \ $$$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$       |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$\______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/	
#ifdef ACTIVE_LAYER_0
	//Layer_0 Zero-mean on the input image
	for (int k = 0; k < 64; k++) {
		matrixReset_SPM(INPUT_SIZE, &odds.lay_1[k][0][0]);
		matrixReset_SPM(INPUT_SIZE, &even.lay_1[k][0][0]);
	}
	dml[0]=&dut_matrix_L0_0[0];
	dml[1]=&dut_matrix_L0_1[0];
	dml[2]=&dut_matrix_L0_2[0];

	
	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset(); sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==0){
			printf("Inizio Layer_0\n");
			CSR_MVSIZE(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT);
			for (int k = 0; k < 1; k++)
			{
				jollypt=k*(INPUT_SIZE*INPUT_SIZE);
				kmemld((void*)((int*)spmaddrA + jollypt ),	(void*)((const int*)input				+jollypt),	(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT));
				kmemld((void*)((int*)spmaddrB + jollypt ),	(void*)((const int*)layer0_bias +jollypt),	(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT));
				ksvmulrf((void*)((int*)spmaddrB + jollypt ),		(void*)((int*)spmaddrB + jollypt ),	(void*)((int*)single_value_minus)); //change the sign for the kaddv
				kaddv((void*)((int*)spmaddrC + jollypt),
								(void*)((int*)spmaddrA + jollypt),
								(void*)((int*)spmaddrB + jollypt)
								);
				kmemstr((void*)((int*)input_int+k*(INPUT_SIZE*INPUT_SIZE)),	(void*)((int*)spmaddrC + jollypt),	INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT	);
			}
		}
		if (Klessydra_get_coreID()==1){
			CSR_MVSIZE(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT);
			for (int k = 1; k < 2; k++)
			{
				jollypt=k*(INPUT_SIZE*INPUT_SIZE);
				kmemld((void*)((int*)spmaddrA + jollypt ),	(void*)((const int*)input				+jollypt),	(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT));
				kmemld((void*)((int*)spmaddrB + jollypt ),	(void*)((const int*)layer0_bias +jollypt),	(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT));
				ksvmulrf((void*)((int*)spmaddrB + jollypt ),		(void*)((int*)spmaddrB + jollypt ),	(void*)((int*)single_value_minus)); //change the sign for the kaddv
				kaddv((void*)((int*)spmaddrC + jollypt),
								(void*)((int*)spmaddrA + jollypt),
								(void*)((int*)spmaddrB + jollypt)
								);
				kmemstr((void*)((int*)input_int+k*(INPUT_SIZE*INPUT_SIZE)),	(void*)((int*)spmaddrC + jollypt),	INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT	);
			}
		}
		if (Klessydra_get_coreID()==2){
			CSR_MVSIZE(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT);
			for (int k = 2; k < 3; k++)
			{
				jollypt=k*(INPUT_SIZE*INPUT_SIZE);
				kmemld((void*)((int*)spmaddrA + jollypt ),	(void*)((const int*)input				+jollypt),	(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT));
				kmemld((void*)((int*)spmaddrB + jollypt ),	(void*)((const int*)layer0_bias +jollypt),	(INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT));
				ksvmulrf((void*)((int*)spmaddrB + jollypt ),		(void*)((int*)spmaddrB + jollypt ),	(void*)((int*)single_value_minus)); //change the sign for the kaddv
				kaddv((void*)((int*)spmaddrC + jollypt),
								(void*)((int*)spmaddrA + jollypt),
								(void*)((int*)spmaddrB + jollypt)
								);
				kmemstr((void*)((int*)input_int+k*(INPUT_SIZE*INPUT_SIZE)),	(void*)((int*)spmaddrC + jollypt),	INPUT_SIZE*INPUT_SIZE*SIZE_OF_INT	);
			}
		}
		spmReset(SPM_DIMENSION);
		// CSR_MVSIZE(SPM_DIMENSION);
		// kbcast((void*)spmaddrA,(void*)single_value_zero);
		// kbcast((void*)spmaddrD,(void*)single_value_zero);
		sync_barrier();

		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
			strncpy(msg, "Th_0:\t After Layer_0", 100); finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
			strncpy(msg, "Th_1:\t After Layer_0", 100); finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
			strncpy(msg, "Th_2:\t After Layer_0", 100); finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
					for(int k=0; k<CHANNEL_SIZE; k++){
						isequal(0,INPUT_SIZE,INPUT_SIZE, input_int[k], dml[k],k);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		#ifdef COUNT_CYCLES
			start_count();
		#endif	
		sync_barrier_reset();		sync_barrier_thread_registration();
		
		if(Klessydra_get_coreID()==0){
			for (int k = 0; k < 21; k++) {
				matrixReset(INPUT_SIZE, &odds.lay_1[k][0][0]);
				matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
			}
		}
		if(Klessydra_get_coreID()==1){
			for (int k = 21; k < 42; k++) {
				matrixReset(INPUT_SIZE, &odds.lay_1[k][0][0]);
				matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
			}
		}
		if(Klessydra_get_coreID()==1){
			for (int k = 42; k < 64; k++) {
				matrixReset(INPUT_SIZE, &odds.lay_1[k][0][0]);
				matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
			}
		}
		// sync_barrier();
		
		// sync_barrier_reset();		sync_barrier_thread_registration();
		if(Klessydra_get_coreID()==0){
			for (int k = 0; k < 1; k++)
			{
				for (int i = 0; i < INPUT_SIZE; i++)
				{
					for (int j = 0; j < INPUT_SIZE; j++)
					{
						uni_ind=k*(INPUT_SIZE*INPUT_SIZE)+i*INPUT_SIZE+j;
						input_int[k][i][j] = input[uni_ind] - layer0_bias[uni_ind];
					}
				}
			}
		}
		if(Klessydra_get_coreID()==1){
			for (int k = 1; k < 2; k++)
			{
				for (int i = 0; i < INPUT_SIZE; i++)
				{
					for (int j = 0; j < INPUT_SIZE; j++)
					{
						uni_ind=k*(INPUT_SIZE*INPUT_SIZE)+i*INPUT_SIZE+j;
						input_int[k][i][j] = input[uni_ind] - layer0_bias[uni_ind];
					}
				}
			}
		}
		if(Klessydra_get_coreID()==2){
			for (int k = 2; k < 3; k++)
			{
				for (int i = 0; i < INPUT_SIZE; i++)
				{
					for (int j = 0; j < INPUT_SIZE; j++)
					{
						uni_ind=k*(INPUT_SIZE*INPUT_SIZE)+i*INPUT_SIZE+j;
						input_int[k][i][j] = input[uni_ind] - layer0_bias[uni_ind];
					}
				}
			}
		}
		sync_barrier();
		
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
			strncpy(msg, "Th_0:\t After Layer_0", 100); finish_count(msg);
			#endif
		}
		sync_barrier();
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU

#endif
//   /$$                                     /$$          /$$$$$$        /$$$$$$
//  | $$                                   /$$$$         /$$__  $$      /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$        |__/  \ $$     |__/  \ $$
//  | $$       |____  $$| $$  | $$          | $$          /$$$$$$/        /$$$$$/
//  | $$        /$$$$$$$| $$  | $$          | $$         /$$____/        |___  $$
//  | $$       /$$__  $$| $$  | $$          | $$        | $$            /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$      | $$$$$$$$     |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/$$$$$$|________/$$$$$$\______/
//                       /$$  | $$|______/      |______/        |______/
//                      |  $$$$$$/
//                       \______/
//   /$$                                     /$$
//  | $$                                   /$$$$
//  | $$        /$$$$$$  /$$   /$$        |_  $$
//  | $$       |____  $$| $$  | $$          | $$
//  | $$        /$$$$$$$| $$  | $$          | $$
//  | $$       /$$__  $$| $$  | $$          | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_1
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<CHANNEL_SIZE; k++){
		for(int i=0; i<SIZE_L1_2; i++){
			for(int j=0; j<SIZE_L1_2; j++){
				input_int[k][i][j]=24642726+k*SIZE_L1_2*SIZE_L1_2+i*SIZE_L1_2+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L1_0[0];isequal_number++;
		dml[1]=&dut_matrix_L1_1[0];isequal_number++;
		dml[2]=&dut_matrix_L1_2[0];isequal_number++;
	#endif

	// pointers to filters and bias
	pt_to_fl = &layer1_filters[0];
	pt_to_bs = &layer1_bias[0];
	
	// I set some parameter for parallel execution and multithreading
	#ifdef SUB_KERNELS
		// I set some parameter for parallel execution and multithreading
		current_dimension=(SIZE_L1_2)*(SIZE_L1_2);
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	#ifdef ZEROPADDED
		// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L1_2+2)*(SIZE_L1_2+2);
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=CHANNEL_SIZE;
	
	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH

		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_1\n");
		}
		sync_barrier();
		
		input_per_layer	=CHANNEL_SIZE; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=64;  	//for loop "i"
		#else
			output_per_layer=4; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;
		
		lay_index				=1								;
		run_SIZE				=SIZE_L1_2				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
	
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==0){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)
						{
							kmemld		((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(input_int[(k)+sub_k]),		(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
						}
					}
					if(Klessydra_get_coreID()==1){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						//PRESCALING of the weights
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)
						{
							kmemld	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)pt_to_fl+	9*((k)+(i+sub_i)*input_per_layer) ),		(ingressi_paralleli*9*SIZE_OF_INT)	);
							ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();
					// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==2){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i = 0;	sub_i < divisor_0;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==1){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i = divisor_0;	sub_i < divisor_1;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==0){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i = divisor_1;	sub_i < uscite_parallele;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					sync_barrier();
				}
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==2){
					for(int sub_i = 0;	sub_i < divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &odds.lay_1[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if (Klessydra_get_coreID()==1){
					for(int sub_i = divisor_0;	sub_i < divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						// Be sure to update the pointer to the ouput
						punt_out = &odds.lay_1[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if (Klessydra_get_coreID()==0){
					for(int sub_i = divisor_1;	sub_i < uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &odds.lay_1[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();

				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					#ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt<isequal_number;	jollypt++)
							{
								//beware of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_1[jollypt], dml[jollypt],jollypt);
							}
						}
					#endif
				}
				sync_barrier();

			}
		#endif

		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)input_int[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_1[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_1[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)input_int[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_1[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_1[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)input_int[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_1[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_1[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_1", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_1", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_1", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
						{
							//beware of
							//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
							//--------------------------------CHANGE this pointer
							isequal(lay_index,run_SIZE,run_SIZE, odds.lay_1[jollypt], dml[jollypt],jollypt);
						}
			}
			sync_barrier();
		#endif

		#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_1\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=CHANNEL_SIZE;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=64; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=1								;
		run_SIZE				=SIZE_L1_2				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &odds.lay_1[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, input_int[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &odds.lay_1[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, input_int[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &odds.lay_1[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, input_int[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_1", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_1[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                    /$$$$$$
//  | $$                                   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |__/  \ $$ 
//  | $$       |____  $$| $$  | $$          /$$$$$$/
//  | $$        /$$$$$$$| $$  | $$         /$$____/
//  | $$       /$$__  $$| $$  | $$        | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$        | $$$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|________/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_2
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<64; k++){
		for(int i=0; i<SIZE_L1_2; i++){
			for(int j=0; j<SIZE_L1_2; j++){
				odds.lay_1[k][i][j]=24642726+k*SIZE_L1_2*SIZE_L1_2+i*SIZE_L1_2+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L2_0[0];isequal_number++;
		dml[1]=&dut_matrix_L2_1[0];isequal_number++;
		dml[2]=&dut_matrix_L2_2[0];isequal_number++;
		dml[3]=&dut_matrix_L2_3[0];isequal_number++;
		dml[4]=&dut_matrix_L2_4[0];isequal_number++;
		dml[5]=&dut_matrix_L2_5[0];isequal_number++;
	#endif
	
	pt_to_fl = &layer2_filters[0];
	pt_to_bs = &layer2_bias[0];

	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_1
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L1_2)*(SIZE_L1_2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_1
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L1_2+2)*(SIZE_L1_2+2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// // I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	
	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_2\n");
		}
		sync_barrier();
		
		input_per_layer	=64;	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=64; 	//for loop "i"
		#else
			output_per_layer=4; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		lay_index				=2								;
		run_SIZE				=SIZE_L1_2				;
		
		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING	@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		LOADING	@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		LOADING	@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==0){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)
						{
							kbcastld		((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_1[(k)+sub_k]),		(run_SIZE*run_SIZE*SIZE_OF_INT));
							kcast++;
							// ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
						}
					}
					if(Klessydra_get_coreID()==1){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)
						{
							kbcastld	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)pt_to_fl+	9*((k)+(i+sub_i)*input_per_layer) ),		(ingressi_paralleli*9*SIZE_OF_INT)	);
							kcast++;
							// ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();
					
					//@@@@@@@@@@@@@@@@		PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		PRESCALING			@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					
					if(Klessydra_get_coreID()==0){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)	ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
					}		
					if(Klessydra_get_coreID()==1){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)	ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
					}		
					if(Klessydra_get_coreID()==2){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)	ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
					}						
					if(Klessydra_get_coreID()==0){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)	ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
					}				
					if(Klessydra_get_coreID()==1){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)	ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
						}				
					if(Klessydra_get_coreID()==2){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)	ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
					}				
					
					sync_barrier();
					// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
					// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
					// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==2){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i = 0;	sub_i < divisor_0;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==1){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i = divisor_0;	sub_i < divisor_1;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==0){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i = divisor_1;	sub_i < uscite_parallele;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					sync_barrier();
				}
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==2){
					for(int sub_i = 0;	sub_i < divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2]) ,pt_to_bs,	(sub_i+i)		);
						kload++;
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &even.lay_2[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						kstore++;
					}
				}
				if (Klessydra_get_coreID()==1){
					for(int sub_i = divisor_0;	sub_i < divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1]) ,pt_to_bs,	(sub_i+i)		);
						kload++;
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						// Be sure to update the pointer to the ouput
						punt_out = &even.lay_2[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						kstore++;
					}
				}
				if (Klessydra_get_coreID()==0){
					for(int sub_i = divisor_1;	sub_i < uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0]) ,pt_to_bs,	(sub_i+i)		);
						kload++;
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &even.lay_2[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						kstore++;
					}
				}
				sync_barrier();

				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					#ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt<isequal_number;	jollypt++)
							{
								//beware of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, even.lay_2[jollypt], dml[jollypt],jollypt);
							}
						}
					#endif
				}
				sync_barrier();
			}
		#endif
	
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)odds.lay_1[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_2[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_2[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)odds.lay_1[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_2[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_2[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)odds.lay_1[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_2[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_2[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "After Layer_2", 100);	finish_count(msg);
				printf("kbcastld=%d \t kload=%d \t kmemst=%d \n",kcast, kload, kstore); kcast=0; kload=0; kstore=0;
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "After Layer_2", 100);	finish_count(msg);
				printf("kbcastld=%d \t kload=%d \t kmemst=%d \n",kcast, kload, kstore); kcast=0; kload=0; kstore=0;
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "After Layer_2", 100);	finish_count(msg);
				printf("kbcastld=%d \t kload=%d \t kmemst=%d \n",kcast, kload, kstore); kcast=0; kload=0; kstore=0;
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_2[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD

	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_2\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=64;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=64; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif
		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;
		lay_index				=2								;
		run_SIZE				=SIZE_L1_2				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_2[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_1[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_2[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_1[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &even.lay_2[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_1[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_2", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_2[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
	
#endif
//   /$$                                   /$$$$$$
//  | $$                                  /$$__  $$
//  | $$        /$$$$$$  /$$   /$$       |__/  \ $$
//  | $$       |____  $$| $$  | $$          /$$$$$/
//  | $$        /$$$$$$$| $$  | $$         |___  $$
//  | $$       /$$__  $$| $$  | $$        /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$       |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$\______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_3
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<64; k++){
		for(int i=0; i<SIZE_L1_2; i++){
			for(int j=0; j<SIZE_L1_2; j++){
				even.lay_2[k][i][j]=24642726+k*SIZE_L1_2*SIZE_L1_2+i*SIZE_L1_2+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L3_0[0];isequal_number++;
		dml[1]=&dut_matrix_L3_1[0];isequal_number++;
		dml[2]=&dut_matrix_L3_2[0];isequal_number++;
	#endif

	#ifdef COUNT_CYCLES
		start_count();
	#endif

	// ------------------> EXECUTION IN MULTITHREAD	
	#ifdef MULTI_TH
		output_per_layer=64;
		divisor_0=output_per_layer/3; //in this case divisor_0 is different, because its a maxpool layer
		divisor_1=divisor_0*2;
		lay_index = 3;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_3\n");
			for (int k = 0; k < divisor_0; k++)
			{
				punt_in 	= &even.lay_2[k][0][0];
				punt_out	= &odds.lay_3[k][0][0];
				maxpool(SIZE_L1_2, punt_in, SIZE_L3_5, punt_out);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int k = divisor_0; k < divisor_1; k++)
			{
				punt_in 	= &even.lay_2[k][0][0];
				punt_out	= &odds.lay_3[k][0][0];
				maxpool(SIZE_L1_2, punt_in, SIZE_L3_5, punt_out);
			}	
		}
		if (Klessydra_get_coreID()==0){
			for (int k = divisor_1; k < output_per_layer; k++)
			{
				punt_in 	= &even.lay_2[k][0][0];
				punt_out	= &odds.lay_3[k][0][0];
				maxpool(SIZE_L1_2, punt_in, SIZE_L3_5, punt_out);
			}	
		}
		sync_barrier();
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			printf("Completed %d maxpools of Layer_%d\n",output_per_layer,lay_index);
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_3", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_3", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_3", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				for (int k = 0; k < divisor_0; k++)
				{
						if (k<3){
							isequal(lay_index,SIZE_L3_5,SIZE_L3_5, odds.lay_3[k], dml[k],k);
						}
				}
			}
			sync_barrier();
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
#endif
//   /$$                             /$$   /$$      /$$$$$$$       /$$$$$$
//  | $$                            | $$  | $$     | $$____/      /$$__  $$
//  | $$        /$$$$$$  /$$   /$$  | $$  | $$     | $$          | $$  \__/
//  | $$       |____  $$| $$  | $$  | $$$$$$$$     | $$$$$$$     | $$$$$$$
//  | $$        /$$$$$$$| $$  | $$  |_____  $$     |_____  $$    | $$__  $$
//  | $$       /$$__  $$| $$  | $$        | $$      /$$  \ $$    | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$        | $$     |  $$$$$$/    |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|__/$$$$$$\______/$$$$$$\______/
//                       /$$  | $$|______/  |______/      |______/
//                      |  $$$$$$/
//                       \______/
//   /$$                             /$$   /$$
//  | $$                            | $$  | $$
//  | $$        /$$$$$$  /$$   /$$  | $$  | $$
//  | $$       |____  $$| $$  | $$  | $$$$$$$$
//  | $$        /$$$$$$$| $$  | $$  |_____  $$
//  | $$       /$$__  $$| $$  | $$        | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$        | $$
//  |________/ \_______/ \____  $$ /$$$$$$|__/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_4
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<64; k++){
		for(int i=0; i<SIZE_L3_5; i++){
			for(int j=0; j<SIZE_L3_5; j++){
				odds.lay_3[k][i][j]=24642726+k*SIZE_L3_5*SIZE_L3_5+i*SIZE_L3_5+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L4_0[0];isequal_number++;
		dml[1]=&dut_matrix_L4_1[0];isequal_number++;
		dml[2]=&dut_matrix_L4_2[0];isequal_number++;
	#endif

	pt_to_fl = &layer4_filters[0];
	pt_to_bs = &layer4_bias[0];

	#ifdef SUB_KERNELS
		// I set some parameter for parallel execution and multithreading
		current_dimension=SIZE_L3_5*SIZE_L3_5;
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	#ifdef ZEROPADDED
		// I set some parameter for parallel execution and multithreading
		current_dimension=(SIZE_L3_5+2)*(SIZE_L3_5+2);
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH

		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_4\n");
		}
		sync_barrier();
		
		input_per_layer	=64; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=128;  	//for loop "i"
		#else
			output_per_layer=4; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;
	
		lay_index				=4								;
		run_SIZE				=SIZE_L3_5				;
		
		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==0){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)
						{
							kmemld		((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_3[(k)+sub_k]),		(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
						}
					}
					if(Klessydra_get_coreID()==1){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						//PRESCALING of the weights, beware that the offest needs to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)
						{
							kmemld	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)pt_to_fl+	9*((k)+(i+sub_i)*input_per_layer) ),		(ingressi_paralleli*9*SIZE_OF_INT)	);
							ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();
					// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==2){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==1){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==0){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					sync_barrier();
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==2){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &even.lay_4[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if (Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						// Be sure to update the pointer to the ouput
						punt_out = &even.lay_4[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if (Klessydra_get_coreID()==0){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &even.lay_4[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();

				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					#ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt<isequal_number;	jollypt++)
							{
								//beware of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, even.lay_4[jollypt], dml[jollypt],jollypt);
							}
						}
					#endif
				}
				sync_barrier();

			}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)odds.lay_3[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_4[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_4[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)odds.lay_3[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_4[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_4[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)odds.lay_3[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_4[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_4[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_4", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_4", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_4", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();	
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_4[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_4\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=64;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=128; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=4								;
		run_SIZE				=SIZE_L3_5				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_4[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_3[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_4[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_3[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &even.lay_4[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_3[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_4", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_4[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                  /$$$$$$$
//  | $$                                 | $$____/
//  | $$        /$$$$$$  /$$   /$$       | $$
//  | $$       |____  $$| $$  | $$       | $$$$$$$
//  | $$        /$$$$$$$| $$  | $$       |_____  $$
//  | $$       /$$__  $$| $$  | $$        /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$       |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$\______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_5
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<128; k++){
		for(int i=0; i<SIZE_L3_5; i++){
			for(int j=0; j<SIZE_L3_5; j++){
				even.lay_4[k][i][j]=24642726+k*SIZE_L3_5*SIZE_L3_5+i*SIZE_L3_5+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L5_0[0];isequal_number++;
		dml[1]=&dut_matrix_L5_1[0];isequal_number++;
		dml[2]=&dut_matrix_L5_2[0];isequal_number++;
		dml[3]=&dut_matrix_L5_3[0];isequal_number++;
		dml[4]=&dut_matrix_L5_4[0];isequal_number++;
		dml[5]=&dut_matrix_L5_5[0];isequal_number++;
	#endif
	
	pt_to_fl = &layer5_filters[0];
	pt_to_bs = &layer5_bias[0];

	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_4// I set some parameter for parallel execution and multithreading
			current_dimension=SIZE_L3_5*SIZE_L3_5;
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_4// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L3_5+2)*(SIZE_L3_5+2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_5\n");
		}
		sync_barrier();
		
		input_per_layer	=128; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=128;  	//for loop "i"
		#else
			output_per_layer=4; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;
	
		lay_index				=5								;
		run_SIZE				=SIZE_L3_5				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==0){
						//FETCHING the features, beware of SIZE parameter and from with FEATURE MAP you're fetching
						for(int sub_k=0;	sub_k<ingressi_paralleli;	sub_k++)
						{
							kmemld		((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_4[(k)+sub_k]),		(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav_v2((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(int*)conv2D_scaling_factor,	(run_SIZE*run_SIZE*SIZE_OF_INT));
						}
					}
					if(Klessydra_get_coreID()==1){
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						//PRESCALING of the weights, beware that the offest needs to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						CSR_MVSIZE(ingressi_paralleli*9*SIZE_OF_INT);
						for(int sub_i=0;	sub_i<uscite_parallele;	sub_i++)
						{
							kmemld	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)pt_to_fl+	9*((k)+(i+sub_i)*input_per_layer) ),		(ingressi_paralleli*9*SIZE_OF_INT)	);
							ksrav	((void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(void*)((int*)spmaddrB+mem_off_BASE_B[sub_i]),		(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();
					// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
					sync_barrier_reset();		sync_barrier_thread_registration();
					if(Klessydra_get_coreID()==2){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==1){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					if(Klessydra_get_coreID()==0){
						//I do the convolution on different parts of SPM_Cm beware of the SIZE parameter
						for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
						{
							for(int sub_k=0; sub_k<ingressi_paralleli; sub_k++)
							{
								convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																					(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																					(void*)(	(int*)spmaddrB+	mem_off_BASE_B [sub_i]	+ krn_slide[sub_k]),
																					(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																					run_SIZE);
							}
						}
					}
					sync_barrier();
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==2){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &odds.lay_5[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if (Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						// Be sure to update the pointer to the ouput
						punt_out = &odds.lay_5[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if (Klessydra_get_coreID()==0){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0]) ,pt_to_bs,	(sub_i+i)		);
						// Beware of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//Be sure to update the pointer to the ouput
						punt_out = &odds.lay_5[sub_i+i][0][0];
						//Beware of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();

				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					#ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt<isequal_number;	jollypt++)
							{
								//beware of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_5[jollypt], dml[jollypt],jollypt);
							}
						}
					#endif
				}
				sync_barrier();

			}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_4[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_5[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_5[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_4[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_5[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_5[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_4[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_5[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_5[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_5", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_5", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_5", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_5[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_5\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=128;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=128; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=5								;
		run_SIZE				=SIZE_L3_5				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &odds.lay_5[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_4[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &odds.lay_5[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_4[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &odds.lay_5[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_4[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_5", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_5[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                   /$$$$$$
//  | $$                                  /$$__  $$
//  | $$        /$$$$$$  /$$   /$$       | $$  \__/
//  | $$       |____  $$| $$  | $$       | $$$$$$$
//  | $$        /$$$$$$$| $$  | $$       | $$__  $$
//  | $$       /$$__  $$| $$  | $$       | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$       |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$\______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_6
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<128; k++){
		for(int i=0; i<SIZE_L3_5; i++){
			for(int j=0; j<SIZE_L3_5; j++){
				odds.lay_5[k][i][j]=24642726+k*SIZE_L3_5*SIZE_L3_5+i*SIZE_L3_5+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		dml[0]=&dut_matrix_L6_0[0];isequal_number++;
		dml[1]=&dut_matrix_L6_1[0];isequal_number++;
		dml[2]=&dut_matrix_L6_2[0];isequal_number++;
	#endif
	for (int k = 0; k < 64; k++)
	{
		matrixReset_SPM(INPUT_SIZE, &even.lay_1[k][0][0]);
	}
	#ifdef COUNT_CYCLES
		start_count();
	#endif

	
	// ------------------> EXECUTION IN MULTITHREAD	
	#ifdef MULTI_TH
		output_per_layer=128;
		divisor_0=output_per_layer/3; //in this case divisor_0 is different, because its a maxpool layer
		divisor_1=divisor_0*2;
		lay_index = 6;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_6\n");
			for (int k = 0; k < divisor_0; k++)
			{
				punt_in = &odds.lay_5[k][0][0];
				punt_out = &even.lay_6[k][0][0];
				maxpool(SIZE_L3_5, punt_in, SIZE_L6_9, punt_out);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int k = divisor_0; k < divisor_1; k++)
			{
				punt_in = &odds.lay_5[k][0][0];
				punt_out = &even.lay_6[k][0][0];
				maxpool(SIZE_L3_5, punt_in, SIZE_L6_9, punt_out);
			}	
		}
		if (Klessydra_get_coreID()==0){
			for (int k = divisor_1; k < output_per_layer; k++)
			{
				punt_in = &odds.lay_5[k][0][0];
				punt_out = &even.lay_6[k][0][0];
				maxpool(SIZE_L3_5, punt_in, SIZE_L6_9, punt_out);
			}	
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			printf("Completed %d maxpools of Layer_%d\n",output_per_layer,lay_index);
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_6", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_6", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_6", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				printf("Inizio Layer_6\n");
				for (int k = 0; k < divisor_0; k++)
				{
					if (k<3){
					isequal(lay_index,SIZE_L6_9,SIZE_L6_9, even.lay_6[k], dml[k],k);
					}
				}
			}
			sync_barrier();			
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
#endif
//   /$$                                   /$$$$$$$$        /$$$$$$
//  | $$                                  |_____ $$/       /$$__  $$
//  | $$        /$$$$$$  /$$   /$$             /$$/       | $$  \ $$
//  | $$       |____  $$| $$  | $$            /$$/        |  $$$$$$/
//  | $$        /$$$$$$$| $$  | $$           /$$/          >$$__  $$
//  | $$       /$$__  $$| $$  | $$          /$$/          | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$/           |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|__/$$$$$$/$$$$$$\______/
//                       /$$  | $$|______/  |______/______/
//                      |  $$$$$$/
//                       \______/
//                                   /$$$$$$         /$$    /$$$$$$
//                                  /$$__  $$      /$$$$   /$$$_  $$
//                                 | $$  \ $$     |_  $$  | $$$$\ $$
//                                 |  $$$$$$$       | $$  | $$ $$ $$
//                                  \____  $$       | $$  | $$\ $$$$
//                                  /$$  \ $$       | $$  | $$ \ $$$
//                                 |  $$$$$$/      /$$$$$$|  $$$$$$/
//                                  \______/$$$$$$|______/ \______/
//                                        |______/
//   /$$                                   /$$$$$$$$
//  | $$                                  |_____ $$/
//  | $$        /$$$$$$  /$$   /$$             /$$/
//  | $$       |____  $$| $$  | $$            /$$/
//  | $$        /$$$$$$$| $$  | $$           /$$/
//  | $$       /$$__  $$| $$  | $$          /$$/
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$/
//  |________/ \_______/ \____  $$ /$$$$$$|__/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_7
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<128; k++){
		for(int i=0; i<SIZE_L6_9; i++){
			for(int j=0; j<SIZE_L6_9; j++){
				even.lay_6[k][i][j]=24642726+k*SIZE_L6_9*SIZE_L6_9+i*SIZE_L6_9+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L7_0[0];isequal_number++;
		dml[1]=&dut_matrix_L7_1[0];isequal_number++;
		dml[2]=&dut_matrix_L7_2[0];isequal_number++;
	#endif

	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;
	pt_to_fl = &layer7_filters[0];
	pt_to_bs = &layer7_bias[0];
	#ifdef SUB_KERNELS
		// I set some parameter for parallel execution and multithreading
		current_dimension=SIZE_L6_9*SIZE_L6_9;
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	#ifdef ZEROPADDED
		// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L6_9+2)*(SIZE_L6_9+2);
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	
	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_7\n");
		}
		sync_barrier();
		
		input_per_layer	=128; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=256; 	//for loop "i"
		#else
			output_per_layer=16; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=7								;
		run_SIZE				=SIZE_L6_9				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;

		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@

					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();
					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_6[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_6[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_6[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																										(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																										(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																										(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																										run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							sync_barrier();
						}
					}
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if(Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_7[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_7[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_7[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();


				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt< isequal_number;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_7[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();

			}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)even.lay_6[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_7[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_7[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)even.lay_6[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_7[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_7[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							// #ifdef SINGLE_SPMU
								kmemld(
											(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
											//-------------CHANGE this pointer
											(void*)	(	(int*)even.lay_6[k] + (row_pointer*run_SIZE) ),
											SIZE_OF_INT*(run_SIZE)	
								);
							// #endif
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_7[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_7[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		print_global_dbg=0;
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_7", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_7", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_7", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			//I can scan if the stored matrixes are correct
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_7[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_7\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=128;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=256; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=7								;
		run_SIZE				=SIZE_L6_9				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &odds.lay_7[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_6[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &odds.lay_7[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_6[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &odds.lay_7[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_6[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_7", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_7[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
	#endif
//   /$$    
//   /$$                                   /$$$$$$
//  | $$                                  /$$__  $$
//  | $$        /$$$$$$  /$$   /$$       | $$  \ $$
//  | $$       |____  $$| $$  | $$       |  $$$$$$/
//  | $$        /$$$$$$$| $$  | $$        >$$__  $$
//  | $$       /$$__  $$| $$  | $$       | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$       |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$\______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_8
	# ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<256; k++){
		for(int i=0; i<SIZE_L6_9; i++){
			for(int j=0; j<SIZE_L6_9; j++){
				odds.lay_7[k][i][j]=24642726+k*SIZE_L6_9*SIZE_L6_9+i*SIZE_L6_9+j;
			}
		}
	}
	# endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
			isequal_number=0;
			dml[0]=&dut_matrix_L8_0[0];isequal_number++;
			dml[1]=&dut_matrix_L8_1[0];isequal_number++;
			dml[2]=&dut_matrix_L8_2[0];isequal_number++;
			dml[3]=&dut_matrix_L8_3[0];isequal_number++;
			dml[4]=&dut_matrix_L8_4[0];isequal_number++;
			dml[5]=&dut_matrix_L8_5[0];isequal_number++;
			dml[6]=&dut_matrix_L8_6[0];isequal_number++;
			dml[7]=&dut_matrix_L8_7[0];isequal_number++;
		#ifdef CONFRONTA_8
			dml[8]=&dut_matrix_L8_8[0];
			dml[9]=&dut_matrix_L8_9[0];
			dml[10]=&dut_matrix_L8_10[0];
			dml[11]=&dut_matrix_L8_11[0];
			dml[12]=&dut_matrix_L8_12[0];
			dml[13]=&dut_matrix_L8_13[0];
			dml[14]=&dut_matrix_L8_14[0];
			dml[15]=&dut_matrix_L8_15[0];
			dml[16]=&dut_matrix_L8_16[0];
			dml[17]=&dut_matrix_L8_17[0];
			dml[18]=&dut_matrix_L8_18[0];
			dml[19]=&dut_matrix_L8_19[0];
			dml[20]=&dut_matrix_L8_20[0];
			dml[21]=&dut_matrix_L8_21[0];
			dml[22]=&dut_matrix_L8_22[0];
			dml[23]=&dut_matrix_L8_23[0];
			dml[24]=&dut_matrix_L8_24[0];
			dml[25]=&dut_matrix_L8_25[0];
			dml[26]=&dut_matrix_L8_26[0];
			dml[27]=&dut_matrix_L8_27[0];
			dml[28]=&dut_matrix_L8_28[0];
			dml[29]=&dut_matrix_L8_29[0];
			dml[30]=&dut_matrix_L8_30[0];
			dml[31]=&dut_matrix_L8_31[0];
			dml[32]=&dut_matrix_L8_32[0];
			dml[33]=&dut_matrix_L8_33[0];
			dml[34]=&dut_matrix_L8_34[0];
			dml[35]=&dut_matrix_L8_35[0];
			dml[36]=&dut_matrix_L8_36[0];
			dml[37]=&dut_matrix_L8_37[0];
			dml[38]=&dut_matrix_L8_38[0];
			dml[39]=&dut_matrix_L8_39[0];
			dml[40]=&dut_matrix_L8_40[0];
			dml[41]=&dut_matrix_L8_41[0];
			dml[42]=&dut_matrix_L8_42[0];
			dml[43]=&dut_matrix_L8_43[0];
			dml[44]=&dut_matrix_L8_44[0];
			dml[45]=&dut_matrix_L8_45[0];
			dml[46]=&dut_matrix_L8_46[0];
			dml[47]=&dut_matrix_L8_47[0];
			dml[48]=&dut_matrix_L8_48[0];
			dml[49]=&dut_matrix_L8_49[0];
			dml[50]=&dut_matrix_L8_50[0];
			dml[51]=&dut_matrix_L8_51[0];
			dml[52]=&dut_matrix_L8_52[0];
			dml[53]=&dut_matrix_L8_53[0];
			dml[54]=&dut_matrix_L8_54[0];
			dml[55]=&dut_matrix_L8_55[0];
			dml[56]=&dut_matrix_L8_56[0];
			dml[57]=&dut_matrix_L8_57[0];
			dml[58]=&dut_matrix_L8_58[0];
			dml[59]=&dut_matrix_L8_59[0];
			dml[60]=&dut_matrix_L8_60[0];
			dml[61]=&dut_matrix_L8_61[0];
			dml[62]=&dut_matrix_L8_62[0];
			dml[63]=&dut_matrix_L8_63[0];
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading	
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B; 
	pt_to_fl = &layer8_filters[0];
	pt_to_bs = &layer8_bias[0];
	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_7
			// I set some parameter for parallel execution and multithreading
			current_dimension=SIZE_L6_9*SIZE_L6_9;
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_7
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L6_9+2)*(SIZE_L6_9+2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif


	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_8\n");
		}
		sync_barrier();
		
		input_per_layer	=256; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=256;  	//for loop "i"
		#else
			output_per_layer=16; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=8								;
		run_SIZE				=SIZE_L6_9				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@

					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();
					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_7[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_7[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_7[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																										(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																										(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																										(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																										run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							sync_barrier();
						}
					}
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if(Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &even.lay_8[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &even.lay_8[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &even.lay_8[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();


				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						#ifdef CONFRONTA_8
							if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
							{
								if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
								for(jollypt=0; jollypt< 64;	jollypt++) //BEWARE:compare just how many dut's you have allocated
								{
									//BEWARE of
									//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
									isequal(lay_index,run_SIZE,run_SIZE, even.lay_8[jollypt], dml[jollypt],jollypt);
								}
							}
						#endif
						#ifndef CONFRONTA_8
							if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
							{
								if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
								for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
								{
									//BEWARE of
									//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
									isequal(lay_index,run_SIZE,run_SIZE, even.lay_8[jollypt], dml[jollypt],jollypt);
								}
							}
						#endif
					# endif
				}
				sync_barrier();

			}
			
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)odds.lay_7[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_8[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_8[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)odds.lay_7[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_8[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_8[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)odds.lay_7[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &even.lay_8[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_8[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		print_global_dbg=0;
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_8", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_8", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_8", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_8[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_8\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=256;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=256; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=8						;
		run_SIZE				=SIZE_L6_9				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_8[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_7[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_8[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_7[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &even.lay_8[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_7[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_8", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_8[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$ 
//   /$$                                   /$$$$$$
//  | $$                                  /$$__  $$
//  | $$        /$$$$$$  /$$   /$$       | $$  \ $$
//  | $$       |____  $$| $$  | $$       |  $$$$$$$
//  | $$        /$$$$$$$| $$  | $$        \____  $$
//  | $$       /$$__  $$| $$  | $$        /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$       |  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$\______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_9
	# ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<256; k++){
		for(int i=0; i<SIZE_L6_9; i++){
			for(int j=0; j<SIZE_L6_9; j++){
				even.lay_8[k][i][j]=-24642726+k*SIZE_L6_9*SIZE_L6_9+i*SIZE_L6_9+j;
			}
		}
	}
	# endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L9_0[0];isequal_number++;
		dml[1]=&dut_matrix_L9_1[0];isequal_number++;
		dml[2]=&dut_matrix_L9_2[0];isequal_number++;
		dml[3]=&dut_matrix_L9_3[0];isequal_number++;
		dml[4]=&dut_matrix_L9_4[0];isequal_number++;
		dml[5]=&dut_matrix_L9_5[0];isequal_number++;
	#endif
	
	pt_to_fl = &layer9_filters[0];
	pt_to_bs = &layer9_bias[0];
	
	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_7
			// I set some parameter for parallel execution and multithreading
			current_dimension=SIZE_L6_9*SIZE_L6_9;
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_7
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L6_9+2)*(SIZE_L6_9+2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_9\n");
		}
		sync_barrier();
		
		input_per_layer	=256; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=256;  	//for loop "i"
		#else
			output_per_layer=8; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=1;
		print_global_id=0;
		print_global_k=0;
		print_global_dbg=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=9								;
		run_SIZE				=SIZE_L6_9				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@

					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();
					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_8[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_8[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_8[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																										(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																										(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																										(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																										run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							sync_barrier();
						}
					}
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				sync_barrier_reset();		sync_barrier_thread_registration();
				if(Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_9[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_9[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_9[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();

				//I can scan if the stored matrixes are correct
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_9[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();
			}
		#endif

		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)even.lay_8[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					punt_out = &odds.lay_9[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									(void*)	(	 &odds.lay_9[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)even.lay_8[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					punt_out = &odds.lay_9[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									(void*)	(	 &odds.lay_9[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)even.lay_8[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					punt_out = &odds.lay_9[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									(void*)	(	 &odds.lay_9[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_9", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_9", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_9", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< 6;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_9[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_9\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=256;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=256; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=9								;
		run_SIZE				=SIZE_L6_9				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &odds.lay_9[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_8[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &odds.lay_9[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_8[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &odds.lay_9[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_8[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_9", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_9[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$$_  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$$$\ $$
//  | $$       |____  $$| $$  | $$          | $$  | $$ $$ $$
//  | $$        /$$$$$$$| $$  | $$          | $$  | $$\ $$$$
//  | $$       /$$__  $$| $$  | $$          | $$  | $$ \ $$$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_10
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<256; k++){
		for(int i=0; i<SIZE_L6_9; i++){
			for(int j=0; j<SIZE_L6_9; j++){
				odds.lay_9[k][i][j]=24642726+k*SIZE_L6_9*SIZE_L6_9+i*SIZE_L6_9+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		dml[0]=&dut_matrix_L10_0[0];
		dml[1]=&dut_matrix_L10_1[0];
		dml[2]=&dut_matrix_L10_2[0];
	#endif
	
	#ifdef COUNT_CYCLES
		start_count();
	#endif

	// ------------------> EXECUTION IN MULTITHREAD	
	#ifdef MULTI_TH
		output_per_layer=256;
		divisor_0=output_per_layer/3; //in this case divisor_0 is different, because its a maxpool layer
		divisor_1=divisor_0*2;
		lay_index = 10;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_10\n");
			for (int k = 0; k < divisor_0; k++)
			{
				punt_in 	= &odds.lay_9[k][0][0];
				punt_out	= &even.lay_10[k][0][0];
				maxpool(SIZE_L6_9, punt_in, SIZE_L10_13, punt_out);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int k = divisor_0; k < divisor_1; k++)
			{
				punt_in 	= &odds.lay_9[k][0][0];
				punt_out	= &even.lay_10[k][0][0];
				maxpool(SIZE_L6_9, punt_in, SIZE_L10_13, punt_out);
			}	
		}
		if (Klessydra_get_coreID()==0){
			for (int k = divisor_1; k < output_per_layer; k++)
			{
				punt_in 	= &odds.lay_9[k][0][0];
				punt_out	= &even.lay_10[k][0][0];
				maxpool(SIZE_L6_9, punt_in, SIZE_L10_13, punt_out);
			}	
		}
		sync_barrier();
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			printf("Completed %d maxpools of Layer_%d\n",output_per_layer,lay_index);
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_10", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_10", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_10", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				for (int k = 0; k < divisor_0; k++)
				{
					if (k<3){
						isequal(lay_index,SIZE_L10_13,SIZE_L10_13, even.lay_10[k], dml[k],k);
					}
				}
			}
			sync_barrier();
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD

#endif
//   /$$                                     /$$    /$$           /$$    /$$$$$$
//  | $$                                   /$$$$  /$$$$         /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$ |_  $$        |_  $$  |__/  \ $$
//  | $$       |____  $$| $$  | $$          | $$   | $$          | $$    /$$$$$$/
//  | $$        /$$$$$$$| $$  | $$          | $$   | $$          | $$   /$$____/
//  | $$       /$$__  $$| $$  | $$          | $$   | $$          | $$  | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$/$$$$$$       /$$$$$$| $$$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|______/______/$$$$$$|______/|________/
//                       /$$  | $$|______/             |______/
//                      |  $$$$$$/
//                       \______/
//                                            /$$    /$$$$$$         /$$ /$$   /$$
//                                          /$$$$   /$$__  $$      /$$$$| $$  | $$
//                                         |_  $$  |__/  \ $$     |_  $$| $$  | $$
//                                           | $$     /$$$$$/       | $$| $$$$$$$$
//                                           | $$    |___  $$       | $$|_____  $$
//                                           | $$   /$$  \ $$       | $$      | $$
//                                          /$$$$$$|  $$$$$$/      /$$$$$$    | $$
//                                  /$$$$$$|______/ \______/$$$$$$|______/    |__/
//                                 |______/               |______/
//
//   /$$                                     /$$    /$$
//  | $$                                   /$$$$  /$$$$
//  | $$        /$$$$$$  /$$   /$$        |_  $$ |_  $$
//  | $$       |____  $$| $$  | $$          | $$   | $$
//  | $$        /$$$$$$$| $$  | $$          | $$   | $$
//  | $$       /$$__  $$| $$  | $$          | $$   | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$/$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|______/______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_11
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<256; k++){
		for(int i=0; i<SIZE_L10_13; i++){
			for(int j=0; j<SIZE_L10_13; j++){
				even.lay_10[k][i][j]=24642726+k*SIZE_L10_13*SIZE_L10_13+i*SIZE_L10_13+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L11_0[0];isequal_number++;
		dml[1]=&dut_matrix_L11_1[0];isequal_number++;
		dml[2]=&dut_matrix_L11_2[0];isequal_number++;
	#endif

	pt_to_fl = &layer11_filters[0];
	pt_to_bs = &layer11_bias[0];

	#ifdef SUB_KERNELS
		// I set some parameter for parallel execution and multithreading
		current_dimension=SIZE_L10_13*SIZE_L10_13;
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	#ifdef ZEROPADDED
		// I set some parameter for parallel execution and multithreading
		current_dimension=(SIZE_L10_13+2)*(SIZE_L10_13+2);
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_11\n");
		}
		sync_barrier();

		input_per_layer	=256; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=512;  	//for loop "i"
		#else
			output_per_layer=16; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=11								;
		run_SIZE				=SIZE_L10_13				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;

		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@

					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();

					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_10[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_10[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_10[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																										(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																										(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																										(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																										run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							sync_barrier();
						}
					}
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if(Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_11[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_11[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_11[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();


				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_11[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();

			}	
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_10[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_11[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_11[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_10[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_11[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_11[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_10[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_11[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_11[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();

		#endif

		// REPORT COUNTERS VALUES
		print_global_dbg=0;
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_11", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_11", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_11", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_11[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_11\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=256;	//for loop "k"

		#ifdef QUICK_DEBUG 
			output_per_layer=6; 	
		#endif

		#ifdef FAST_ESTIMATION
				output_per_layer=512/8; 	//for loop "i"
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=11								;
		run_SIZE				=SIZE_L10_13				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &odds.lay_11[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_10[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &odds.lay_11[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_10[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &odds.lay_11[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_10[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_11", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_11[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  |__/  \ $$
//  | $$       |____  $$| $$  | $$          | $$    /$$$$$$/
//  | $$        /$$$$$$$| $$  | $$          | $$   /$$____/
//  | $$       /$$__  $$| $$  | $$          | $$  | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$| $$$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|______/|________/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_12
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L10_13; i++){
			for(int j=0; j<SIZE_L10_13; j++){
				odds.lay_11[k][i][j]=24642726+k*SIZE_L10_13*SIZE_L10_13+i*SIZE_L10_13+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L12_0[0];isequal_number++;
		dml[1]=&dut_matrix_L12_1[0];isequal_number++;
		dml[2]=&dut_matrix_L12_2[0];isequal_number++;
		dml[3]=&dut_matrix_L12_3[0];isequal_number++;
		dml[4]=&dut_matrix_L12_4[0];isequal_number++;
		dml[5]=&dut_matrix_L12_5[0];isequal_number++;
	#endif
	
	pt_to_fl = &layer12_filters[0];
	pt_to_bs = &layer12_bias[0];

	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_11
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L10_13)*(SIZE_L10_13);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_11
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L10_13+2+1)*(SIZE_L10_13+2+1);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_12\n");
		}
		sync_barrier();

		input_per_layer	=512; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=512;  	//for loop "i"
		#else
			output_per_layer=64; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;
		print_global_dbg=0;		

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=12								;
		run_SIZE				=SIZE_L10_13				;
		
		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele){
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@

					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();

					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_11[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_11[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_11[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																										(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																										(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																										(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																										run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							sync_barrier();
						}
					}
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if(Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &even.lay_12[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &even.lay_12[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &even.lay_12[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();


				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
							for(jollypt=0; jollypt< isequal_number;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, even.lay_12[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();

			}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)odds.lay_11[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					punt_out = &even.lay_12[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									(void*)	(	 &even.lay_12[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)odds.lay_11[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					punt_out = &even.lay_12[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									(void*)	(	 &even.lay_12[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)odds.lay_11[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					punt_out = &even.lay_12[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									(void*)	(	 &even.lay_12[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_12", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_12", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_12", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_12[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_12\n");
		}
		sync_barrier();
		
		output_per_layer=512;  	//for loop "i"
		input_per_layer	=512; 	//for loop "k"
		
		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_12[i][0][0];
				for (int k = 0; k < 512; k++)
				{
					punt = &odds.lay_11[k][0][0];
					getWeights(pt_to_fl, 9, kern.kernel_9,i*512+k);
					convolution2D(SIZE_L10_13, odds.lay_11[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(SIZE_L10_13, punt2, bias);
				relu(SIZE_L10_13, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_12[i][0][0];
				for (int k = 0; k < 512; k++)
				{
					punt = &odds.lay_11[k][0][0];
					getWeights(pt_to_fl, 9, kern.kernel_9,i*512+k);
					convolution2D(SIZE_L10_13, odds.lay_11[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(SIZE_L10_13, punt2, bias);
				relu(SIZE_L10_13, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < 512; i++)
			{
				punt2 = &even.lay_12[i][0][0];
				for (int k = 0; k < 512; k++)
				{
					punt = &odds.lay_11[k][0][0];
					getWeights(pt_to_fl, 9, kern.kernel_9,i*512+k);
					convolution2D(SIZE_L10_13, odds.lay_11[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(SIZE_L10_13, punt2, bias);
				relu(SIZE_L10_13, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_12", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_12\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=512;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=512; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=12								;
		run_SIZE				=SIZE_L10_13				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_12[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_11[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_12[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_11[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &even.lay_12[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_11[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_12", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_12[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  |__/  \ $$
//  | $$       |____  $$| $$  | $$          | $$     /$$$$$/
//  | $$        /$$$$$$$| $$  | $$          | $$    |___  $$
//  | $$       /$$__  $$| $$  | $$          | $$   /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_13
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L10_13; i++){
			for(int j=0; j<SIZE_L10_13; j++){
				even.lay_12[k][i][j]=24642726+k*SIZE_L10_13*SIZE_L10_13+i*SIZE_L10_13+j;
			}
		}
	}
	#endif
	
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L13_0[0];isequal_number++;
		dml[1]=&dut_matrix_L13_1[0];isequal_number++;
		dml[2]=&dut_matrix_L13_2[0];isequal_number++;
	#endif

	pt_to_fl = &layer13_filters[0];
	pt_to_bs = &layer13_bias[0];

	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_11
			// I set some parameter for parallel execution and multithreading
			current_dimension=SIZE_L10_13*SIZE_L10_13;
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_11
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L10_13+2)*(SIZE_L10_13+2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=dimension_spm/(current_dimension*SIZE_OF_INT);
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_13\n");
		}
		sync_barrier();

		input_per_layer	=512; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=512;  	//for loop "i"
		#else
			output_per_layer=16; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=13								;
		run_SIZE				=SIZE_L10_13				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;

		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@

					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();

					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_12[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_12[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_12[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																										(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																										(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																										(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																										run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB((void*)(	(int*)spmaddrC+	mem_off_BASE_AC[sub_i]		),
																							(void*)(	(int*)spmaddrA+	mem_off_BASE_AC[sub_k]		),
																							(void*)(	(int*)spmaddrB+	mem_off_BASE_B [nano_i]	+ krn_slide[nano_k]),
																							(void*)(	(int*)spmaddrD+	mem_off_BASE_AC[sub_i]		),
																							run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							sync_barrier();
						}
					}
				}	
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@

				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if(Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_13[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_13[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				if(Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_13[sub_i+i][0][0];
						//BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					}
				}
				sync_barrier();


				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_13[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();

			}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_12[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_13[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_13[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_12[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_13[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_13[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_12[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_13[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_13[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		print_global_dbg=0;
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_13", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_13", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_13", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_13[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_13\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=512;	//for loop "k"
		#ifndef QUICK_DEBUG 
			output_per_layer=512; 	//for loop "i"
		#else
			output_per_layer=6; 	
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=13								;
		run_SIZE				=SIZE_L10_13				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &odds.lay_13[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_12[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &odds.lay_13[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_12[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &odds.lay_13[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, even.lay_12[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_13", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_13[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                     /$$ /$$   /$$
//  | $$                                   /$$$$| $$  | $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$| $$  | $$
//  | $$       |____  $$| $$  | $$          | $$| $$$$$$$$
//  | $$        /$$$$$$$| $$  | $$          | $$|_____  $$
//  | $$       /$$__  $$| $$  | $$          | $$      | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$    | $$
//  |________/ \_______/ \____  $$ /$$$$$$|______/    |__/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_14
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L10_13; i++){
			for(int j=0; j<SIZE_L10_13; j++){
				odds.lay_13[k][i][j]=24642726+k*SIZE_L10_13*SIZE_L10_13+i*SIZE_L10_13+j;
			}
		}
	}
	#endif
	
	#ifdef ISEQUAL_RUNTIME
		// I set some matrixes to be compared runtime
		dml[0]=&dut_matrix_L14_0[0];
		dml[1]=&dut_matrix_L14_1[0];
		dml[2]=&dut_matrix_L14_2[0];
	#endif
	
	#ifdef COUNT_CYCLES
		start_count();
	#endif

	// ------------------> EXECUTION IN MULTITHREAD	
	#ifdef MULTI_TH
		output_per_layer=512;
		divisor_0=output_per_layer/3; //in this case divisor_0 is different, because its a maxpool layer
		divisor_1=divisor_0*2;
		lay_index = 14;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_14\n");
			for (int k = 0; k < divisor_0; k++)
			{
				punt_in = &odds.lay_13[k][0][0];
				punt_out = &even.lay_14[k][0][0];
				maxpool(SIZE_L10_13,punt_in, SIZE_L14_17, punt_out);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int k = divisor_0; k < divisor_1; k++)
			{
				punt_in = &odds.lay_13[k][0][0];
				punt_out = &even.lay_14[k][0][0];
				maxpool(SIZE_L10_13,punt_in, SIZE_L14_17, punt_out);
			}	
		}
		if (Klessydra_get_coreID()==0){
			for (int k = divisor_1; k < output_per_layer; k++)
			{
				punt_in = &odds.lay_13[k][0][0];
				punt_out = &even.lay_14[k][0][0];
				maxpool(SIZE_L10_13,punt_in, SIZE_L14_17, punt_out);
			}	
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			printf("Completed %d maxpools of Layer_%d\n",output_per_layer,lay_index);
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_14", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_14", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();		
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_14", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				printf("Inizio Layer_14\n");
				for (int k = 0; k < divisor_0; k++)
				{
					punt_in = &odds.lay_13[k][0][0];
					punt_out = &even.lay_14[k][0][0];
					maxpool(SIZE_L10_13,punt_in, SIZE_L14_17, punt_out);
						if (k<3){
							isequal(lay_index,SIZE_L14_17,SIZE_L14_17, even.lay_14[k], dml[k],k);
						}
				}
				sync_barrier();
			}
		#endif
		
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
#endif
//   /$$                                     /$$   /$$$$$$$         /$$    /$$$$$$
//  | $$                                   /$$$$  | $$____/       /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$           |_  $$  | $$  \__/
//  | $$       |____  $$| $$  | $$          | $$  | $$$$$$$        | $$  | $$$$$$$
//  | $$        /$$$$$$$| $$  | $$          | $$  |_____  $$       | $$  | $$__  $$
//  | $$       /$$__  $$| $$  | $$          | $$   /$$  \ $$       | $$  | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/      /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/$$$$$$|______/ \______/
//                       /$$  | $$|______/               |______/
//                      |  $$$$$$/
//                       \______/
//                                         /$$  /$$$$$$$$          /$$    /$$$$$$
//                                       /$$$$ |_____ $$/        /$$$$   /$$__  $$
//                                      |_  $$      /$$/        |_  $$  | $$  \ $$
//                                        | $$     /$$/           | $$  |  $$$$$$/
//                                        | $$    /$$/            | $$   >$$__  $$
//                                        | $$   /$$/             | $$  | $$  \ $$
//                                       /$$$$$$/$$/             /$$$$$$|  $$$$$$/
//                                      |______/__/$$$$$$/$$$$$$|______/ \______/
//                                               |______/______/
//
//   /$$                                     /$$   /$$$$$$$
//  | $$                                   /$$$$  | $$____/
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$
//  | $$       |____  $$| $$  | $$          | $$  | $$$$$$$
//  | $$        /$$$$$$$| $$  | $$          | $$  |_____  $$
//  | $$       /$$__  $$| $$  | $$          | $$   /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_15
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L14_17; i++){
			for(int j=0; j<SIZE_L14_17; j++){
				even.lay_14[k][i][j]=24642726+k*SIZE_L14_17*SIZE_L14_17+i*SIZE_L14_17+j;
			}
		}
	}
	#endif

	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L15_0[0];isequal_number++;
		dml[1]=&dut_matrix_L15_1[0];isequal_number++;
		dml[2]=&dut_matrix_L15_2[0];isequal_number++;
	#endif
	
	pt_to_fl = &layer15_filters[0];
	pt_to_bs = &layer15_bias[0];

	#ifdef SUB_KERNELS
		// I set some parameter for parallel execution and multithreading
		current_dimension=SIZE_L14_17*SIZE_L14_17; 
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	#ifdef ZEROPADDED
		// I set some parameter for parallel execution and multithreading
		current_dimension=(SIZE_L14_17+2)*(SIZE_L14_17+2);
		for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
			mem_off_BASE_AC[i]	=	i*(current_dimension);
		}
	#endif
	
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=512;
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;

	
	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_15\n");
		}
		sync_barrier();

		input_per_layer	=512; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=512;  	//for loop "i"
		#else
			//output_per_layer=512;  	//for loop "i"
			output_per_layer=4; 	
		#endif

		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=15									;
		run_SIZE				=SIZE_L14_17				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		
		#ifdef SUB_KERNELS
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@
					
					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();
					
					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							// #ifndef lay_15_to_17_keep_in_spm
								//path_A: Standard method, I fetch and prescale Feature Maps into SPM_A
								kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_14[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							// #endif
							
							// #ifdef lay_15_to_17_keep_in_spm
								//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
								// ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							// #endif
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							// #ifndef lay_15_to_17_keep_in_spm
								//path_A: I fetch and prescale Feature Maps into SPM_A
								kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_14[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							// #endif
							
							// #ifdef lay_15_to_17_keep_in_spm
								//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
								// ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							// #endif
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							// #ifndef lay_15_to_17_keep_in_spm
								//path_A: I fetch and prescale Feature Maps into SPM_A
								kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_14[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							// #endif
							
							// #ifdef lay_15_to_17_keep_in_spm
								//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
								// ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							// #endif
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
									nano_k=0;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
									nano_k=0;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
									nano_k=0;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																									(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																									(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																									(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																									run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;				
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																									(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																									(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																									(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																									run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																									(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																									(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																									(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																									run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;		
							}
							sync_barrier();
						}
					}
				}
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@
				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_15[sub_i+i][0][0];
						
						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
							// BEWARE of this SIZE parameter
							relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
							krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
							# ifdef ISEQUAL_RUNTIME
								// I do a ReLU just for having some matrix in memory for debug purposes
								if(sub_i<3)
								{
									// BEWARE of this SIZE parameter
									relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
								}
								#endif
						#endif
					}
				}
				if (Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_15[sub_i+i][0][0];

						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
							// BEWARE of this SIZE parameter
							relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
							krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
						#endif
					}		
				}
				if (Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_15[sub_i+i][0][0];

						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
							// BEWARE of this SIZE parameter
							relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
							krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
						#endif
					}		
				}
				sync_barrier();
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_15[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();
				
			}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_14[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_15[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_15[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_14[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_15[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_15[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_14[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_15[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_15[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "After Layer_15", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifndef lay_15_to_17_keep_in_spm
				//path_A: Standard method, so i clean every spm with this function
				// spmReset(SPM_DIMENSION);
			#endif
			#ifdef lay_15_to_17_keep_in_spm
				//path_B: I won't clear SPM_A, because it's holding values for the next layer
				// jollypt=0;
				// CSR_MVSIZE(SPM_DIMENSION);
				// spmA non va assolutamente pulita, ha le matrici salvate lì
				// kbcast((void*)spmaddrB,(void*)jollypt);
				// kbcast((void*)spmaddrC,(void*)jollypt); // questo rimane sicuramente perchè la sola funzione krelu non ha pulito la spmC
				// kbcast((void*)spmaddrD,(void*)jollypt);	// per sicurezza questo lo lascio
			#endif
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_15", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_15", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
				for(jollypt=0; jollypt< isequal_number;	jollypt++)
				{
					//beware of
					//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
					//--------------------------------CHANGE this pointer
					isequal(lay_index,run_SIZE,run_SIZE, odds.lay_15[jollypt], dml[jollypt],jollypt);
					// matrix_check_v2(even.lay_14[jollypt], dml[jollypt], run_SIZE);
				}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_15\n");
		}
		sync_barrier();
		for (int k = 0; k < 64; k++){
			matrixReset(INPUT_SIZE, &even.lay_1[k][0][0]);
		}

		input_per_layer	=512;	//for loop "k"
		output_per_layer=512; 	//for loop "i"

		#ifdef QUICK_DEBUG 
			output_per_layer=6; 	
		#endif
		
		#ifdef FAST_ESTIMATION
				output_per_layer=512/8; 	//for loop "i"
		#endif

		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;

		lay_index				=15								;
		run_SIZE				=SIZE_L14_17				;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_14[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_15[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_14[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_15[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < output_per_layer; i++)
			{
				punt2 = &even.lay_14[i][0][0];
				for (int k = 0; k < input_per_layer; k++)
				{
					getWeights(pt_to_fl, 9, kern.kernel_9,i*input_per_layer+k);
					convolution2D(run_SIZE, odds.lay_15[k], kern.kernel_9, punt2);
				}
				bias = getBias(pt_to_bs,i);
				addBias(run_SIZE, punt2, bias);
				relu(run_SIZE, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_15", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer) isequal_number=output_per_layer;
					for(jollypt=0; jollypt < isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_15[jollypt], dml[jollypt],jollypt);
					}
			}
			sync_barrier();
		#endif
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU
#endif
//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$  \__/
//  | $$       |____  $$| $$  | $$          | $$  | $$$$$$$
//  | $$        /$$$$$$$| $$  | $$          | $$  | $$__  $$
//  | $$       /$$__  $$| $$  | $$          | $$  | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_16
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L14_17; i++){
			for(int j=0; j<SIZE_L14_17; j++){
				odds.lay_15[k][i][j]=-24642726+k*SIZE_L14_17*SIZE_L14_17+i*SIZE_L14_17+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		dml[0]=&dut_matrix_L16_0[0];
		dml[1]=&dut_matrix_L16_1[0];
		dml[2]=&dut_matrix_L16_2[0];
		dml[3]=&dut_matrix_L16_3[0];
		dml[4]=&dut_matrix_L16_4[0];
		dml[5]=&dut_matrix_L16_5[0];
	#endif
	
	pt_to_fl = &layer16_filters[0];
	pt_to_bs = &layer16_bias[0];

	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_15
			// I set some parameter for parallel execution and multithreading
			current_dimension=SIZE_L14_17*SIZE_L14_17; 
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_15
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L14_17+2)*(SIZE_L14_17+2); 
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=512; //this value is not obtained by calculations, it's a particular case
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;
	
	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_16\n");
		}
		sync_barrier();

		input_per_layer	=512; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=512;  	//for loop "i"
		#else
			// output_per_layer=512;  	//for loop "i"
			output_per_layer=4; 	
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;
		print_global_dbg=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=16									;
		run_SIZE				=SIZE_L14_17				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		#ifdef SUB_KERNELS
		for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
			for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
			{
				//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
				//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@
				
				//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
				//PRESCALING the features, BEWARE of SIZE parameter
				sync_barrier_reset();		sync_barrier_thread_registration();
				
				if (Klessydra_get_coreID()==0){
					CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
					for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
					{
						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I fetch and prescale Feature Maps into SPM_A
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_15[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						#endif
					}
				}
				if (Klessydra_get_coreID()==1){
					CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
					for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
					{
						#ifndef lay_15_to_17_keep_in_spm
							//path_A: I fetch and prescale Feature Maps into SPM_A
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_15[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						#endif
					}
				}
				if (Klessydra_get_coreID()==2){
					CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
					for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
					{
						#ifndef lay_15_to_17_keep_in_spm
							//path_A: I fetch and prescale Feature Maps into SPM_A
							kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(odds.lay_15[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
							ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
						#endif
					}
				}
				sync_barrier();

				for (int macro_i=0; macro_i < ripet; macro_i++)
				{
					for (int macro_k=0 ; macro_k < ripet; macro_k++)
					{
						//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
						//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
						sync_barrier_reset();		sync_barrier_thread_registration();
						//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
						//PRESCALING of the weights
						if (Klessydra_get_coreID()==0){
							nano_i=nano_divisor_0;
							CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
							for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
							{
								for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
								{
									//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
									kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
									ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
								}
								nano_i++;
								nano_k=0;
							}
						}
						if (Klessydra_get_coreID()==1){
							nano_i=nano_divisor_1;
							CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
							for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
							{
								for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
								{
									//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
									kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
									ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
								}
								nano_i++;
								nano_k=0;
							}
						}
						if (Klessydra_get_coreID()==2){
							nano_i= nano_divisor_2;
							CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
							for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
							{
								for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
								{
									//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
									kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
									ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
								}
								nano_i++;
								nano_k=0;
							}
						}
						sync_barrier();

						// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
						sync_barrier_reset();		sync_barrier_thread_registration();
						if (Klessydra_get_coreID()==0){
							//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
							nano_k=0;
							nano_i=0;
							for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
							{
								nano_i=nano_divisor_0;
								for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
								{
									convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																								(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																								(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																								(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																								run_SIZE);
									nano_i++;
								}
								nano_k++;
							}
							nano_i=0;
							nano_k=0;				
						}
						if (Klessydra_get_coreID()==1){
							//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
							nano_k=0;
							nano_i=0;
							for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
							{
								nano_i=nano_divisor_1;
								for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
								{
									convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																								(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																								(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																								(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																								run_SIZE);
									nano_i++;
								}
								nano_k++;
							}
							nano_i=0;
							nano_k=0;
						}
						if (Klessydra_get_coreID()==2){
							//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
							nano_k=0;
							nano_i=0;
							for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
							{
								nano_i= nano_divisor_2;
								for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																								(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																								(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																								(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																								run_SIZE);
									nano_i++;
								}
								nano_k++;
							}
							nano_i=0;
							nano_k=0;		
						}
						sync_barrier();
					}
				}
			}
			// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@
			//The parallel OutputMaps can be stored back in memory
			
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
				{
					getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
					//BEWARE of this SIZE parameter
					addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
											(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
											(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
											run_SIZE);
					//BEWARE to update the pointer to the output!!
					punt_out = &even.lay_16[sub_i+i][0][0];
					
					#ifndef lay_15_to_17_keep_in_spm
						//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
						// BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					#endif
					
					#ifdef lay_15_to_17_keep_in_spm
						//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
						krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
						# ifdef ISEQUAL_RUNTIME
							// I do a ReLU just for having some matrix in memory for debug purposes
							if(sub_i<3)
							{
								// BEWARE of this SIZE parameter
								relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
							}
							#endif
					#endif
				}
			}
			if (Klessydra_get_coreID()==1){
				for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
				{
					getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
					//BEWARE of this SIZE parameter
					addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
											(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
											(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
											run_SIZE);
					//BEWARE to update the pointer to the output!!
					punt_out = &even.lay_16[sub_i+i][0][0];

					#ifndef lay_15_to_17_keep_in_spm
						//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
						// BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					#endif
					
					#ifdef lay_15_to_17_keep_in_spm
						//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
						krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
					#endif
				}		
			}
			if (Klessydra_get_coreID()==2){
				for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
				{
					getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
					//BEWARE of this SIZE parameter
					addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
											(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
											(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
											run_SIZE);
					//BEWARE to update the pointer to the output!!
					punt_out = &even.lay_16[sub_i+i][0][0];

					#ifndef lay_15_to_17_keep_in_spm
						//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
						// BEWARE of this SIZE parameter
						relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
					#endif
					
					#ifdef lay_15_to_17_keep_in_spm
						//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
						krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
					#endif
				}		
			}
			sync_barrier();
			
			sync_barrier_reset();		sync_barrier_thread_registration();
			//I can scan if the stored matrixes are correct
			if (Klessydra_get_coreID()==2){
				#ifdef INSIDE_TRACE
					// CHANGE the printing "layer_X"
					printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
				#endif
				# ifdef ISEQUAL_RUNTIME
					if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
					{
						for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
						{
							//BEWARE of
							//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
							isequal(lay_index,run_SIZE,run_SIZE, even.lay_16[jollypt], dml[jollypt],jollypt);
						}
					}
				# endif
			}
			sync_barrier();
			
		}
		#endif
		
		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				// for (int i = 0; i < 2; i++)	{ // per tutte le uscite di cui si occupa thread_0
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)odds.lay_15[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_15[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_16[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				// for (int i = 2; i < 4; i++)	{ // per tutte le uscite di cui si occupa thread_0
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)odds.lay_15[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_15[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_16[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				// for (int i = 4; i < 6; i++)	{ // per tutte le uscite di cui si occupa thread_0
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)odds.lay_15[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_15[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &even.lay_16[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();

			#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_16", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifndef lay_15_to_17_keep_in_spm
				//path_A: Standard method, so i clean every spm with this function
				// spmReset(SPM_DIMENSION);
			#endif
			#ifdef lay_15_to_17_keep_in_spm
				//path_B: I won't clear SPM_A, because it's holding values for the next layer
				jollypt=0;
				CSR_MVSIZE(SPM_DIMENSION);
				// spmA non va assolutamente pulita, ha le matrici salvate lì
				// kbcast((void*)spmaddrB,(void*)jollypt);
				kbcast((void*)spmaddrC,(void*)jollypt); // questo rimane sicuramente perchè la sola funzione krelu non ha pulito la spmC
				kbcast((void*)spmaddrD,(void*)jollypt);	// per sicurezza questo lo lascio	
			#endif
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_16", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_16", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
				for(jollypt=0; jollypt< 6;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, even.lay_16[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_12[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD

	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_16\n");
		}
		sync_barrier();
		
		output_per_layer=512;  	//for loop "i"
		input_per_layer	=512; 	//for loop "k"
		
		divisor_0=output_per_layer/3;
		divisor_1=divisor_0*2;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++)
			{
				punt2 = &even.lay_16[i][0][0];
				for (int k = 0; k < 512; k++)
				{
					punt = &odds.lay_15[k][0][0];
					getWeights(pt_to_fl, 9, kern.kernel_9,i*512+k);
					convolution2D(SIZE_L14_17, odds.lay_15[k], kern.kernel_9, punt2);
				}
				// printf("Terminate le 512 conv di Layer_16 di riga:%d\n",(i));
				bias = getBias(pt_to_bs,i);
				addBias(SIZE_L14_17, punt2, bias);
				relu(SIZE_L14_17, punt2);
			}
			// # ifdef ISEQUAL_RUNTIME
				// for(jollypt=0; jollypt< 3;	jollypt++)
				// {
					// isequal(lay_index,run_SIZE,run_SIZE, even.lay_16[jollypt], dml[jollypt],jollypt);
				// }
			// # endif
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++)
			{
				punt2 = &even.lay_16[i][0][0];
				for (int k = 0; k < 512; k++)
				{
					punt = &odds.lay_15[k][0][0];
					getWeights(pt_to_fl, 9, kern.kernel_9,i*512+k);
					convolution2D(SIZE_L14_17, odds.lay_15[k], kern.kernel_9, punt2);
				}
				// printf("Terminate le 512 conv di Layer_16 di riga:%d\n",(i));
				bias = getBias(pt_to_bs,i);
				addBias(SIZE_L14_17, punt2, bias);
				relu(SIZE_L14_17, punt2);
			}
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < 512; i++)
			{
				punt2 = &even.lay_16[i][0][0];
				for (int k = 0; k < 512; k++)
				{
					punt = &odds.lay_15[k][0][0];
					getWeights(pt_to_fl, 9, kern.kernel_9,i*512+k);
					convolution2D(SIZE_L14_17, odds.lay_15[k], kern.kernel_9, punt2);
				}
				// printf("Terminate le 512 conv di Layer_16 di riga:%d\n",(i));
				bias = getBias(pt_to_bs,i);
				addBias(SIZE_L14_17, punt2, bias);
				relu(SIZE_L14_17, punt2);
			}
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_16", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU

#endif
//   /$$                                     /$$  /$$$$$$$$
//  | $$                                   /$$$$ |_____ $$/
//  | $$        /$$$$$$  /$$   /$$        |_  $$      /$$/
//  | $$       |____  $$| $$  | $$          | $$     /$$/
//  | $$        /$$$$$$$| $$  | $$          | $$    /$$/
//  | $$       /$$__  $$| $$  | $$          | $$   /$$/
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$/$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/__/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_17
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L14_17; i++){
			for(int j=0; j<SIZE_L14_17; j++){
				even.lay_16[k][i][j]=24642726+k*SIZE_L14_17*SIZE_L14_17+i*SIZE_L14_17+j;
			}
		}
	}
	#endif
	#ifdef LAY_17_CORRECT_INTERCEPT
	// I intercept CORRECT parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L14_17; i++){
			for(int j=0; j<SIZE_L14_17; j++){
				even.lay_16[k][i][j]=dut_matrix_L1600_0[k*SIZE_L14_17*SIZE_L14_17 + i*SIZE_L14_17 + j];
			}
		}
	}
	#endif
		
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L17_0[0];isequal_number++;
		dml[1]=&dut_matrix_L17_1[0];isequal_number++;
		dml[2]=&dut_matrix_L17_2[0];isequal_number++;
	#endif
	mv_fl=0;
	mv_bs=0;

	pt_to_fl = &layer17_filters[0];
	pt_to_bs = &layer17_bias[0];

	#ifdef SUB_KERNELS
		#ifndef ACTIVE_LAYER_15
			// I set some parameter for parallel execution and multithreading
			current_dimension=SIZE_L14_17*SIZE_L14_17; 
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	#ifdef ZEROPADDED
		#ifndef ACTIVE_LAYER_15
			// I set some parameter for parallel execution and multithreading
			current_dimension=(SIZE_L14_17+2)*(SIZE_L14_17+2);
			for(unsigned int i=0; i<( sizeof(mem_off_BASE_AC)/SIZE_OF_INT );i++){
				mem_off_BASE_AC[i]	=	i*(current_dimension);
			}
		#endif
	#endif
	// I set some parameter for parallel execution and multithreading
	uscite_parallele=512;
	ingressi_paralleli=uscite_parallele;
	ripet=uscite_parallele/limite_B;

	// ------------------> EXECUTION IN MULTITHREAD
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_17\n");
		}
		sync_barrier();

		input_per_layer	=512; 	//for loop "k"
		#ifndef QUICK_DEBUG
			output_per_layer=512;  	//for loop "i"
		#else
			// output_per_layer=512;  	//for loop "i"
			output_per_layer=4; 	 
		#endif
		
		divisor_0=uscite_parallele/3;
		divisor_1=divisor_0*2;

		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		nano_divisor_0=	0;
		nano_divisor_1=	limite_B/3;
		nano_divisor_2=	nano_divisor_1*2;
		
		lay_index				=17									;
		run_SIZE				=SIZE_L14_17				;

		thread_outputs[0]=output_per_layer/3;
		thread_outputs[1]=thread_outputs[0]*2;
		#ifdef SUB_KERNELS	
			for (int i = 0; i < output_per_layer; i = i+uscite_parallele)
			{
				for(int k = 0; k < input_per_layer; k = k+ingressi_paralleli)
				{
					//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
					//@@@@@@@@@@@@@@@@		The FEATURE MAPS						@@@@@@@@@@@@@@@@
					
					//Fetching the features, BEWARE of SIZE parameter and which FEATURE MAP you're fetching from
					//PRESCALING the features, BEWARE of SIZE parameter
					sync_barrier_reset();		sync_barrier_thread_registration();
					
					if (Klessydra_get_coreID()==0){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	0;	sub_k	< divisor_0 ;	sub_k++)
						{
							#ifndef lay_15_to_17_keep_in_spm
								//path_A: Standard method, I fetch and prescale Feature Maps into SPM_A
								kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_16[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							#endif
							
							#ifdef lay_15_to_17_keep_in_spm
								//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							#endif
						}
					}
					if (Klessydra_get_coreID()==1){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_0;	sub_k	< divisor_1 ;	sub_k++)
						{
							#ifndef lay_15_to_17_keep_in_spm
								//path_A: I fetch and prescale Feature Maps into SPM_A
								kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_16[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							#endif
							
							#ifdef lay_15_to_17_keep_in_spm
								//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							#endif
						}
					}
					if (Klessydra_get_coreID()==2){
						CSR_MVSIZE(run_SIZE*run_SIZE*SIZE_OF_INT);
						for(int sub_k =	divisor_1;	sub_k	< ingressi_paralleli ;	sub_k++)
						{
							#ifndef lay_15_to_17_keep_in_spm
								//path_A: I fetch and prescale Feature Maps into SPM_A
								kmemld((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),	(void*)(even.lay_16[(k)+sub_k]),	(run_SIZE*run_SIZE*SIZE_OF_INT));
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							#endif
							
							#ifdef lay_15_to_17_keep_in_spm
								//path_B: The output of previous layer are still in SPM_A, so there is no need to fetch them from memory
								ksrav((void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_k]),(int*)conv2D_scaling_factor);
							#endif
						}
					}
					sync_barrier();

					for (int macro_i=0; macro_i < ripet; macro_i++)
					{
						for (int macro_k=0 ; macro_k < ripet; macro_k++)
						{
							//@@@@@@@@@@@@@@@@		LOADING		&	PRESCALING			@@@@@@@@@@@@@@@@
							//@@@@@@@@@@@@@@@@		cycles of Kernel Maps			@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							//FETCHING of the weights, beware that the offest needs .to be updated, and the widht is the same as the lenght of upper limit of loop in "k"
							//PRESCALING of the weights
							if (Klessydra_get_coreID()==0){
								nano_i=nano_divisor_0;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_0 ;	sub_i < (macro_i*limite_B) + nano_divisor_1 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
									nano_k=0;
								}
							}
							if (Klessydra_get_coreID()==1){
								nano_i=nano_divisor_1;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i=  (macro_i*limite_B) + nano_divisor_1  ;	sub_i < (macro_i*limite_B) + nano_divisor_2 ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
									nano_k=0;
								}
							}
							if (Klessydra_get_coreID()==2){
								nano_i= nano_divisor_2;
								CSR_MVSIZE(limite_B*9*SIZE_OF_INT);
								for( int sub_i= (macro_i*limite_B) + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
								{
									for( int sub_k = macro_k*limite_B ; sub_k < (macro_k+1)*limite_B ; sub_k = sub_k + limite_B )//it's as if the loop is not present, it's taken only once
									{
										//FETCHING & PRESCALING of the weights, BEWARE that the offest needs to be updated, it's widht is the same as the lenght of upper limit of loop in "k"
										kmemld((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),			(void*)((int*)pt_to_fl+	9*( (k+sub_k)+((i+sub_i)*input_per_layer) )	),		limite_B*(9*SIZE_OF_INT)		);
										ksrav((void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),		(void*)((int*)spmaddrB+mem_off_BASE_B[nano_i]+krn_slide[0]),	(int*)conv2D_scaling_factor	);
									}
									nano_i++;
									nano_k=0;
								}
							}
							sync_barrier();

							// @@@@@@@@@@@@@@@@		CONVOLUTIONS		@@@@@@@@@@@@@@@@
							sync_barrier_reset();		sync_barrier_thread_registration();
							if (Klessydra_get_coreID()==0){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_0;
									for( int sub_i= macro_i*limite_B + nano_divisor_0;	sub_i < macro_i*limite_B + nano_divisor_1 ; sub_i++)
									{
										convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																									(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																									(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																									(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																									run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;				
							}
							if (Klessydra_get_coreID()==1){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i=nano_divisor_1;
									for( int sub_i=  macro_i*limite_B + nano_divisor_1  ;	sub_i <  macro_i*limite_B + nano_divisor_2 ; sub_i++)
									{
										convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																									(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																									(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																									(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																									run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;
							}
							if (Klessydra_get_coreID()==2){
								//I do the convolution on different parts of SPM_Cm, BEWARE of the SIZE parameter!
								nano_k=0;
								nano_i=0;
								for( int sub_k = macro_k*limite_B ;	sub_k < (macro_k+1)*limite_B ; sub_k++ )
								{
									nano_i= nano_divisor_2;
									for( int sub_i= macro_i*limite_B + nano_divisor_2 ;	sub_i < (macro_i+1)*limite_B ; sub_i++)
									{
										convolution2D_SPM_off_NOB_2x2 ((void*)(	(int*)spmaddrC+	mem_off_BASE_AC	[sub_i]												)	,
																									(void*)(	(int*)spmaddrA+	mem_off_BASE_AC	[sub_k]												)	,
																									(void*)(	(int*)spmaddrB+	mem_off_BASE_B	[nano_i]	+krn_slide[nano_k]	),
																									(void*)(	(int*)spmaddrD+	mem_off_BASE_AC	[sub_i]												),
																									run_SIZE);
										nano_i++;
									}
									nano_k++;
								}
								nano_i=0;
								nano_k=0;		
							}
							sync_barrier();
						}
					}
				}
				// @@@@@@@@@@@@@@@@		END OF LOOP IN K step INGRESSI_PARALLELI		@@@@@@@@@@@@@@@@
				//The parallel OutputMaps can be stored back in memory
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				if (Klessydra_get_coreID()==0){
					for(int sub_i	=	0	;	sub_i	<	divisor_0;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[0])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[0]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_17[sub_i+i][0][0];
						
						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
							// BEWARE of this SIZE parameter
							relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
							krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
							# ifdef ISEQUAL_RUNTIME
								// I do a ReLU just for having some matrix in memory for debug purposes
								if(sub_i<3)
								{
									// BEWARE of this SIZE parameter
									relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
								}
								#endif
						#endif
					}
				}
				if (Klessydra_get_coreID()==1){
					for(int sub_i	= divisor_0;	sub_i	<	divisor_1;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[1])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[1]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_17[sub_i+i][0][0];

						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
							// BEWARE of this SIZE parameter
							relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
							krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
						#endif
					}		
				}
				if (Klessydra_get_coreID()==2){
					for(int sub_i	=	divisor_1;	sub_i	< uscite_parallele;	sub_i++)
					{
						getBias_SPM((void*)((int*)spmaddrB+mem_off_BASE_B[2])	,pt_to_bs,	(sub_i+i)		);
						//BEWARE of this SIZE parameter
						addBias_SPM((void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]),
												(void*)((int*)spmaddrB+mem_off_BASE_B[2]),
												run_SIZE);
						//BEWARE to update the pointer to the output!!
						punt_out = &odds.lay_17[sub_i+i][0][0];

						#ifndef lay_15_to_17_keep_in_spm
							//path_A: Standard method, I do the ReLU on the output matrix and I also store it in memory
							// BEWARE of this SIZE parameter
							relu_SPM(	(void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i]), run_SIZE, (int*) punt_out);
						#endif
						
						#ifdef lay_15_to_17_keep_in_spm
							//path_B: relu_SPM without storing back in memory, so it's only krelu(dest, src): The output matrix remains on SPM_A
							krelu(	(void*)((int*)spmaddrA+mem_off_BASE_AC[sub_i]),	 (void*)((int*)spmaddrC+mem_off_BASE_AC[sub_i])		);
						#endif
					}		
				}
				sync_barrier();
				
				sync_barrier_reset();		sync_barrier_thread_registration();
				//I can scan if the stored matrixes are correct
				if (Klessydra_get_coreID()==2){
					#ifdef INSIDE_TRACE
						// CHANGE the printing "layer_X"
						printf("Completed the groups of %d matrixes of Layer_%d starting from row:%d/%d\n",uscite_parallele,lay_index,i,output_per_layer);
					#endif
					# ifdef ISEQUAL_RUNTIME
						if (i<uscite_parallele) //I'll check on just the first batch of parallel outputs
						{
							for(jollypt=0; jollypt< 3;	jollypt++) //BEWARE:compare just how many dut's you have allocated
							{
								//BEWARE of
								//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
								isequal(lay_index,run_SIZE,run_SIZE, odds.lay_17[jollypt], dml[jollypt],jollypt);
							}
						}
					# endif
				}
				sync_barrier();
				
			}
		#endif

		#ifdef ZEROPADDED
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0){
				th_id=0;
				th_output_low	=0;
				th_output_high=thread_outputs[0];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_16[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_17[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_17[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==1){
				th_id=1;
				th_output_low	=thread_outputs[0];
				th_output_high=thread_outputs[1];
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_16[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_17[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_17[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			if (Klessydra_get_coreID()==2){
				th_id=2;
				th_output_low	=thread_outputs[1];
				th_output_high=output_per_layer;
				for (int i = th_output_low; i < th_output_high; i++)	{ // per tutte le uscite di cui si occupa thread_0
 					for (int k = 0 ; k < input_per_layer;	 k++)	{  // prenderò tutte le FM in ingresso, prese singolarmente
						// LOADING & PRESCALING Feature Maps
						CSR_MVSIZE(run_SIZE*SIZE_OF_INT);
						for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
							kmemld(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										//-------------CHANGE this pointer
										(void*)	(	(int*)even.lay_16[k] + (row_pointer*run_SIZE) ),
										SIZE_OF_INT*(run_SIZE)	
							);
							ksrav(
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(void*)	(	(int*)spmaddrA + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1) ),
										(int*)conv2D_scaling_factor
							);
						}	//end loop row_pt
						// LOADING & PRESCALING Kernel Maps, depending on which outputs are made from this thread
						CSR_MVSIZE(9*SIZE_OF_INT);
						kmemld(
										(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
										(void*)	(	(int*)pt_to_fl	+	(	9*(i*input_per_layer)	+ 9*(k)	)	),
										(9*SIZE_OF_INT)
						);
						ksrav(
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(void*)	(	(int*)spmaddrB	+	mem_off_BASE_AC[th_id]),
									(int*)conv2D_scaling_factor
						);
						// CONVOLUTIONS
						convolution2D_SPM_off_NOB_zeropadded(
									(void*)(	(int*)spmaddrC	+	 mem_off_BASE_AC[th_id]	),
									(void*)(	(int*)spmaddrA	+	 mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  mem_off_BASE_AC[th_id]	), 
									(void*)(	(int*)spmaddrB	+  (mem_off_BASE_AC[th_id]+9)	), 
									// (void*)(	(int*)spmaddrD	+	mem_off_BASE_AC[th_id]	),
									(run_SIZE+2)
						);
					}	//end for K loop
					// Add BIAS & RELU and storing back to main memory
					getBias_SPM(	(void*)((int*)spmaddrB + mem_off_BASE_AC	[th_id]	),	pt_to_bs,	i	);
					ksvaddsc_v2(	
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrC	+	mem_off_BASE_AC[th_id]),
											(void*)((int*)spmaddrB	+	mem_off_BASE_AC[th_id]	),
											((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT)
					);
					//--------CHANGE this pointer
					punt_out = &odds.lay_17[i][0][0];
					CSR_MVSIZE((run_SIZE+2)*(run_SIZE+2)*SIZE_OF_INT);
					krelu((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]));
					for (int row_pointer=0; row_pointer<run_SIZE; row_pointer++){
						kmemstr(
									//--------CHANGE this pointer
									(void*)	(	 &odds.lay_17[i][0][0] + (row_pointer*run_SIZE) ),
									(void*)	(	(int*)spmaddrC + mem_off_BASE_AC[th_id] + ((row_pointer+1)*(run_SIZE+2) + 1)	),
									SIZE_OF_INT*(run_SIZE)	
						);
					} //end kmemstr loop
					kbcast((void*)((int*)spmaddrC+mem_off_BASE_AC[th_id]), (void*)zero_value	);
				}	//end for I loop
			}	// end If_klessydra
			spmReset(SPM_DIMENSION);
			sync_barrier();
			
		#endif
		
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_17", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_17", 100);	finish_count(msg);
			#endif
			#ifndef lay_15_to_17_keep_in_spm
				//path_A: Standard method, so i clean every spm with this function
				// spmReset(SPM_DIMENSION);
			#endif
			#ifdef lay_15_to_17_keep_in_spm
				//path_B: I won't clear SPM_A, because it's holding values for the next layer
				jollypt=0;
				CSR_MVSIZE(SPM_DIMENSION);
				// spmA non va assolutamente pulita, ha le matrici salvate lì
				// kbcast((void*)spmaddrB,(void*)jollypt);
				kbcast((void*)spmaddrC,(void*)jollypt); // questo rimane sicuramente perchè la sola funzione krelu non ha pulito la spmC
				kbcast((void*)spmaddrD,(void*)jollypt);	// per sicurezza questo lo lascio
			#endif
		}
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_17", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					if (isequal_number >= output_per_layer)  isequal_number=output_per_layer;
					for(jollypt=0; jollypt< isequal_number;	jollypt++)
					{
						//beware of
						//layer index		//SIZE PARAMETERS	//output pointer	//that you have enough matrixes for comparison
						//--------------------------------CHANGE this pointer
						isequal(lay_index,run_SIZE,run_SIZE, odds.lay_17[jollypt], dml[jollypt],jollypt);
						// matrix_check_v2(even.lay_16[jollypt], dml[jollypt], run_SIZE);
					}
			}
			sync_barrier();
		#endif
	
	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
#endif
//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$  \ $$
//  | $$       |____  $$| $$  | $$          | $$  |  $$$$$$/
//  | $$        /$$$$$$$| $$  | $$          | $$   >$$__  $$
//  | $$       /$$__  $$| $$  | $$          | $$  | $$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_18
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for(int k=0; k<512; k++){
		for(int i=0; i<SIZE_L14_17; i++){
			for(int j=0; j<SIZE_L14_17; j++){
				odds.lay_17[k][i][j]=24642726+k*SIZE_L14_17*SIZE_L14_17+i*SIZE_L14_17+j;
			}
		}
	}
	#endif
	// I set some matrixes to be compared runtime
	#ifdef ISEQUAL_RUNTIME
		isequal_number=0;
		dml[0]=&dut_matrix_L18_0[0];isequal_number++; //this is the only vector i need for comparison
	#endif
	mv_fl=0;
	mv_bs=0;

	#ifdef COUNT_CYCLES
		start_count();
	#endif

	
	// ------------------> EXECUTION IN MULTITHREAD	
	#ifdef MULTI_TH
		output_per_layer=512;
		divisor_0=output_per_layer/3; //in this case divisor_0 is different, because its a maxpool layer
		divisor_1=divisor_0*2;
		lay_index = 18;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_18\n");
			for (int k = 0; k < divisor_0; k++)
			{
			punt_in = &odds.lay_17[k][0][0];
			punt_out = &even.lay_18[k][0][0];
			maxpool(SIZE_L14_17, punt_in, SIZE_L18, punt_out);
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int k = divisor_0; k < divisor_1; k++)
			{
				punt_in = &odds.lay_17[k][0][0];
				punt_out = &even.lay_18[k][0][0];
				maxpool(SIZE_L14_17, punt_in, SIZE_L18, punt_out);
			}	
		}
		if (Klessydra_get_coreID()==0){
			for (int k = divisor_1; k < output_per_layer; k++)
			{
				punt_in = &odds.lay_17[k][0][0];
				punt_out = &even.lay_18[k][0][0];
				maxpool(SIZE_L14_17, punt_in, SIZE_L18, punt_out);
			}	
		}
		sync_barrier();
	
		// REPORT COUNTERS VALUES
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Completed %d maxpools of Layer_%d\n",output_per_layer,lay_index);
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_2\t After Layer_18", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==1){
			// spmReset(SPM_DIMENSION); // FORSE QUESTO POSSO TOGLIERLO			
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_1\t After Layer_18", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){		
			// Convert even.lay_18 into a unidimensional vector
			for (int i = 0; i < 512; i++)
			{
				for (int j = 0; j < SIZE_L18; j++)
				{
					for (int k = 0; k < SIZE_L18; k++)
					{
						flattened[total] = even.lay_18[i][k][j];
						total += 1;
					}
				}
			}
			finish_count_nomsg();
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_18", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
				printf("Inizio Layer_18\n");
				for (int k = 0; k < divisor_0; k++)
				{
						if (k<1){
							isequal(lay_index,SIZE_L18,SIZE_L18, even.lay_18[k], dml[k],k);
						}
				}
			}
			sync_barrier();
		#endif

	#endif
	// ------------------> END OF EXECUTION IN MULTITHREAD
#endif
//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$  \ $$
//  | $$       |____  $$| $$  | $$          | $$  |  $$$$$$$
//  | $$        /$$$$$$$| $$  | $$          | $$   \____  $$
//  | $$       /$$__  $$| $$  | $$          | $$   /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/$$$$$$
//                       /$$  | $$|______/               |______/
//                      |  $$$$$$/
//                       \______/
//                    /$$$$$$   /$$$$$$        /$$$$$$    /$$
//                   /$$__  $$ /$$$_  $$      /$$__  $$ /$$$$
//                  |__/  \ $$| $$$$\ $$     |__/  \ $$|_  $$
//                    /$$$$$$/| $$ $$ $$       /$$$$$$/  | $$
//                   /$$____/ | $$\ $$$$      /$$____/   | $$
//                  | $$      | $$ \ $$$     | $$        | $$
//                  | $$$$$$$$|  $$$$$$/     | $$$$$$$$ /$$$$$$
//                  |________/ \______/$$$$$$|________/|______/
//                                   |______/
//
//

//   /$$                                     /$$    /$$$$$$
//  | $$                                   /$$$$   /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |_  $$  | $$  \ $$
//  | $$       |____  $$| $$  | $$          | $$  |  $$$$$$$
//  | $$        /$$$$$$$| $$  | $$          | $$   \____  $$
//  | $$       /$$__  $$| $$  | $$          | $$   /$$  \ $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$         /$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|______/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_19
	#ifdef INTERCEPT
	total=0;
	// I intercept fake parameters for quick debug
	for (int i = 0; i < 512; i++)	{
		for (int j = 0; j < SIZE_L18; j++){
			for (int k = 0; k < SIZE_L18; k++){
				flattened[total] = 24642726+k*SIZE_L18*SIZE_L18+i*SIZE_L18+j;
				total += 1;
			} 
		}
	}	
	#endif
	// pointers to filters and bias
	pt_to_fl = &layer19_filters[0];
	pt_to_bs = &layer19_bias[0];
	mv_fl=0;
	mv_bs=0;
	# ifdef COUNT_CYCLES
		start_count();
	# endif

	punt = &flattened[0];
	//Mini-batch
	int lay_19_ripetition_factor=4;

	//this is not actually multithreaded
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_19\n");
		}
		sync_barrier();
		
		
		print_global_dbg=0;
		print_global_id=0;
		print_global_k=0;

		#if DEDICATED_SPMU==1
			getFeatures_SPM_fully((void*)spmaddrA,	(SIZE_L18*SIZE_L18*512), punt);
		#else
			sync_barrier_reset();	sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0) getFeatures_SPM_fully((void*)spmaddrA,	(SIZE_L18*SIZE_L18*512), punt);
			sync_barrier();
		#endif
		
		//per fare del debug veloce
		#if QUICK_DEBUG
			dimension_out_19=6;
		#endif
		divisor_0=	dimension_out_19/3;
		divisor_1=	divisor_0*2;
		sync_barrier_reset();	sync_barrier_thread_registration();
		
		#if DEDICATED_SPMU==1
			if (Klessydra_get_coreID()==0) {
				for (int cmpt_spm = 0; cmpt_spm < divisor_0; cmpt_spm++){
					getWeights_SPM		( (void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*0 ), 	pt_to_fl,	(SIZE_L18*SIZE_L18*512),	cmpt_spm);
					fullyconnect_SPM	(	(void*)((int*)spmaddrC + cmpt_spm), 
															(void*)spmaddrA,
															(void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*0 ), 
															(SIZE_L18*SIZE_L18*512)
															);
				}
			}
			if (Klessydra_get_coreID()==1) {
				for (int cmpt_spm = divisor_0; cmpt_spm < divisor_1; cmpt_spm++){
					getWeights_SPM		( (void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*1 ), 	pt_to_fl,	(SIZE_L18*SIZE_L18*512),	cmpt_spm);
					fullyconnect_SPM	(	(void*)((int*)spmaddrC + cmpt_spm), 
															(void*)spmaddrA,                               
															(void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*1 ), 
															(SIZE_L18*SIZE_L18*512)
															);
				}
			}
			if (Klessydra_get_coreID()==2) {
				// for (int cmpt_spm = divisor_1; cmpt_spm < 4096; cmpt_spm++){
				for (int cmpt_spm = divisor_1; cmpt_spm < dimension_out_19; cmpt_spm++){
					getWeights_SPM		( (void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*2 ), 	pt_to_fl,	(SIZE_L18*SIZE_L18*512),	cmpt_spm);
					fullyconnect_SPM	(	(void*)((int*)spmaddrC + cmpt_spm), 
															(void*)spmaddrA,                               
															(void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*2 ), 
															(SIZE_L18*SIZE_L18*512)
															);
				}
			}
			// sync_barrier();		
		#else
			if (Klessydra_get_coreID()==0) {
				for (int cmpt_spm = 0; cmpt_spm < divisor_0; cmpt_spm++){
					getWeights_SPM		( (void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*0 ), 	pt_to_fl,	(SIZE_L18*SIZE_L18*512),	cmpt_spm);
					fullyconnect_SPM	(	(void*)((int*)spmaddrC + cmpt_spm), 
															(void*)spmaddrA,
															(void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*0 ), 
															(SIZE_L18*SIZE_L18*512)
															);
				}
			}
			if (Klessydra_get_coreID()==1) {
				for (int cmpt_spm = divisor_0; cmpt_spm < divisor_1; cmpt_spm++){
					getWeights_SPM		( (void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*1 ), 	pt_to_fl,	(SIZE_L18*SIZE_L18*512),	cmpt_spm);
					fullyconnect_SPM	(	(void*)((int*)spmaddrC + cmpt_spm), 
															(void*)spmaddrA,                               
															(void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*1 ), 
															(SIZE_L18*SIZE_L18*512)
															);
				}
			}
			if (Klessydra_get_coreID()==2) {
				// for (int cmpt_spm = divisor_1; cmpt_spm < 4096; cmpt_spm++){
				for (int cmpt_spm = divisor_1; cmpt_spm < dimension_out_19; cmpt_spm++){
					getWeights_SPM		( (void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*2 ), 	pt_to_fl,	(SIZE_L18*SIZE_L18*512),	cmpt_spm);
					fullyconnect_SPM	(	(void*)((int*)spmaddrC + cmpt_spm), 
															(void*)spmaddrA,                               
															(void*)((int*)spmaddrB+(SIZE_L18*SIZE_L18*512)*2 ), 
															(SIZE_L18*SIZE_L18*512)
															);
				}
			}
			// sync_barrier();	
		#endif
		
		#if DEDICATED_SPMU==1
			// sync_barrier_reset();	sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0) {
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
				CSR_MVSIZE(4096*SIZE_OF_INT);
				kaddv((void*)spmaddrC, (void*)spmaddrC, (void*)spmaddrD);
				//store back in main memory
				punt_out = &odds.lay_19[0];
				// relu_SPM_fully((void*)spmaddrC, (void*)spmaddrC, dimension_out_19, (int*) punt_out);
				relu_SPM_fully( (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (divisor_0-0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==1) {
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
				CSR_MVSIZE(4096*SIZE_OF_INT);
				kaddv((void*)spmaddrC, (void*)spmaddrC, (void*)spmaddrD);
				//store back in main memory
				punt_out = &odds.lay_19[divisor_0];
				// relu_SPM_fully((void*)spmaddrC, (void*)spmaddrC, dimension_out_19, (int*) punt_out);
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (divisor_1-divisor_0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==2) {
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
				CSR_MVSIZE(4096*SIZE_OF_INT);
				kaddv((void*)spmaddrC, (void*)spmaddrC, (void*)spmaddrD);
				//store back in main memory
				punt_out = &odds.lay_19[divisor_1];
				// relu_SPM_fully((void*)spmaddrC, (void*)spmaddrC, dimension_out_19, (int*) punt_out);
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (dimension_out_19-divisor_1), (int*) punt_out);
			}
			// sync_barrier();
			// spmReset(SPM_DIMENSION);
		#else
			getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
			if (Klessydra_get_coreID()==0) {
				CSR_MVSIZE((divisor_0-0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrD+(0)));
				punt_out = &odds.lay_19[0];
				relu_SPM_fully( (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (divisor_0-0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==1) {
				CSR_MVSIZE((divisor_1-divisor_0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrD+(divisor_0)));
				punt_out = &odds.lay_19[divisor_0];
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (divisor_1-divisor_0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==2) {
				CSR_MVSIZE((dimension_out_19-divisor_1)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrD+(divisor_1)));
				punt_out = &odds.lay_19[divisor_1];
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (dimension_out_19-divisor_1), (int*) punt_out);
			}
			// sync_barrier();
			// spmReset(SPM_DIMENSION);
		#endif
	
	
		// REPORT COUNTERS VALUES
		// REPORT COUNTERS VALUES
		// REPORT COUNTERS VALUES
		// sync_barrier_reset();		sync_barrier_thread_registration();
		// if (Klessydra_get_coreID()==2){
			// #ifdef COUNT_CYCLES
				// strncpy(msg, "Th_2\t After Layer_19", 100);	finish_count(msg);
			// #endif
		// }
		// sync_barrier();
		// sync_barrier_reset();		sync_barrier_thread_registration();
		// if (Klessydra_get_coreID()==1){
			// // spmReset(SPM_DIMENSION);
			// // CSR_MVSIZE(SPM_DIMENSION);	kbcast((void*)spmaddrD,(void*)single_value_zero);
			// #ifdef COUNT_CYCLES
				// strncpy(msg, "Th_1\t After Layer_19", 100);	finish_count(msg);
			// #endif
		// }
		// sync_barrier();
		// sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_19", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
	
		//I can scan if the stored matrixes are correct
		#ifdef ISEQUAL_RUNTIME//°°°°°°°°°°check if is equal
			sync_barrier_reset();	sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0) {
					for(int cmpt_spm = 0; cmpt_spm < dimension_out_19; cmpt_spm++)
					{
						if(odds.lay_19[cmpt_spm]!=dut_matrix_L19_0[0*512+cmpt_spm]){
							errori++;
							printf("[%d]Error: SPM=%d != EXP=%d\n",0*512+cmpt_spm+1,	odds.lay_19[0*512+cmpt_spm],	dut_matrix_L19_0[0*512+cmpt_spm]);
						}
					}
					if (errori==0)		printf("Success in fully-connected layer!\n");
					else printf("There are %d ERRORS\n",errori);
					errori=0;
			}
			sync_barrier();
		#endif//°°°°°°°°°°°°°°°°°°°°°°°°°°check if is equal

		#endif

	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_19\n");
		}
		sync_barrier();
		
		divisor_0=	4096/3;
		divisor_1=	divisor_0*2;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++){
				getWeights(pt_to_fl, (SIZE_L18*SIZE_L18*512), kern.kernel_maxs,i);
				odds.lay_19[i] += fullyconnect((SIZE_L18*SIZE_L18*512), punt, kern.kernel_maxs);
				bias = getBias(pt_to_bs,i);
				odds.lay_19[i] +=bias;
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++){
				getWeights(pt_to_fl, (SIZE_L18*SIZE_L18*512), kern.kernel_maxs,i);
				odds.lay_19[i] += fullyconnect((SIZE_L18*SIZE_L18*512), punt, kern.kernel_maxs);
				bias = getBias(pt_to_bs,i);
				odds.lay_19[i] +=bias;
			}		
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_0; i < 4096; i++){
				getWeights(pt_to_fl, (SIZE_L18*SIZE_L18*512), kern.kernel_maxs,i);
				odds.lay_19[i] += fullyconnect((SIZE_L18*SIZE_L18*512), punt, kern.kernel_maxs);
				bias = getBias(pt_to_bs,i);
				odds.lay_19[i] +=bias;
			}		
		}
		sync_barrier();
				
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_19", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU

#endif
//   /$$                                    /$$$$$$   /$$$$$$
//  | $$                                   /$$__  $$ /$$$_  $$
//  | $$        /$$$$$$  /$$   /$$        |__/  \ $$| $$$$\ $$
//  | $$       |____  $$| $$  | $$          /$$$$$$/| $$ $$ $$
//  | $$        /$$$$$$$| $$  | $$         /$$____/ | $$\ $$$$
//  | $$       /$$__  $$| $$  | $$        | $$      | $$ \ $$$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$        | $$$$$$$$|  $$$$$$/
//  |________/ \_______/ \____  $$ /$$$$$$|________/ \______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_20
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for (int i = 0; i < 4096; i++)	{
		odds.lay_19[i]= -24642726 + i;
	}	
	#endif
	errori=0;
	// pointers to filters and bias
	pt_to_fl = &layer20_filters[0];
	pt_to_bs = &layer20_bias[0];
	punt = &odds.lay_19[0];
	mv_fl=0;
	mv_bs=0;

	
	//this is not actually multithreaded
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_20\n");
		}
		sync_barrier();
	
		#if DEDICATED_SPMU==1
			getFeatures_SPM_fully((void*)spmaddrA,	4096, punt);
		#else
			sync_barrier_reset();	sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0) 		getFeatures_SPM_fully((void*)spmaddrA,	4096, punt);
			sync_barrier();
		#endif
		
		// per fare del debug veloce
		#if QUICK_DEBUG
			dimension_out_20=6;
		#endif
		divisor_0=	dimension_out_20/3;
		divisor_1=	divisor_0*2;
		sync_barrier_reset();		sync_barrier_thread_registration();
		#if DEDICATED_SPMU==1
			if (Klessydra_get_coreID()==0){
				for (int cmpt_spm = 0; cmpt_spm < divisor_0; cmpt_spm++)
				{
					getWeights_SPM	((void*)spmaddrB, 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)spmaddrB, 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==1){
				for (int cmpt_spm = divisor_0; cmpt_spm < divisor_1; cmpt_spm++)
				{
					getWeights_SPM	((void*)spmaddrB, 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)spmaddrB, 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==2){
				for (int cmpt_spm = divisor_1; cmpt_spm < dimension_out_20; cmpt_spm++)
				{
					getWeights_SPM	((void*)spmaddrB, 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)spmaddrB, 	
													 4096);
				}
			}
			// sync_barrier();
		#else
			if (Klessydra_get_coreID()==0){
				for (int cmpt_spm = 0; cmpt_spm < divisor_0; cmpt_spm++)
				{
					getWeights_SPM	((void*)((int*)spmaddrB+0*4096), 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)((int*)spmaddrB+0*4096), 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==1){
				for (int cmpt_spm = divisor_0; cmpt_spm < divisor_1; cmpt_spm++)
				{
					getWeights_SPM	((void*)((int*)spmaddrB+1*4096), 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)((int*)spmaddrB+1*4096), 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==2){
				for (int cmpt_spm = divisor_1; cmpt_spm < dimension_out_20; cmpt_spm++)
				{
					getWeights_SPM	((void*)((int*)spmaddrB+2*4096), 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)((int*)spmaddrB+2*4096), 	
													 4096);
				}
			}
			// sync_barrier();	
		#endif
	
		#if DEDICATED_SPMU==1
			// sync_barrier_reset();		sync_barrier_thread_registration();		
			if (Klessydra_get_coreID()==0){
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
				// AddBias done simply in main
				CSR_MVSIZE(4096*SIZE_OF_INT);
				kaddv((void*)spmaddrC, (void*)spmaddrC, (void*)spmaddrD);
				// Relu and Store back in memory
				punt_out = &even.lay_20[0];
				relu_SPM_fully((void*)spmaddrC, (void*)spmaddrC, (divisor_0-0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==1){
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
				// AddBias done simply in main
				CSR_MVSIZE(4096*SIZE_OF_INT);
				kaddv((void*)spmaddrC, (void*)spmaddrC, (void*)spmaddrD);
				// Relu and Store back in memory
				punt_out = &even.lay_20[divisor_0];
				relu_SPM_fully((void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (divisor_1-divisor_0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==2){
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
				// AddBias done simply in main
				CSR_MVSIZE(4096*SIZE_OF_INT);
				kaddv((void*)spmaddrC, (void*)spmaddrC, (void*)spmaddrD);
				// Relu and Store back in memory
				punt_out = &even.lay_20[divisor_1];
				relu_SPM_fully((void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (dimension_out_20-divisor_1), (int*) punt_out);
			}
			// sync_barrier();
			// spmReset(SPM_DIMENSION);
		#else
			getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 4096	);
			if (Klessydra_get_coreID()==0) {
				CSR_MVSIZE((divisor_0-0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrD+(0)));
				punt_out = &even.lay_20[0];
				relu_SPM_fully( (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (divisor_0-0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==1) {
				CSR_MVSIZE((divisor_1-divisor_0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrD+(divisor_0)));
				punt_out = &even.lay_20[divisor_0];
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (divisor_1-divisor_0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==2) {
				CSR_MVSIZE((dimension_out_20-divisor_1)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrD+(divisor_1)));
				punt_out =&even.lay_20[divisor_1];
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (dimension_out_19-divisor_1), (int*) punt_out);
			}
		#endif
		
		// REPORT COUNTERS VALUES
		// REPORT COUNTERS VALUES
		// REPORT COUNTERS VALUES
		// sync_barrier_reset();		sync_barrier_thread_registration();
		// if (Klessydra_get_coreID()==2){
			// #ifdef COUNT_CYCLES
				// strncpy(msg, "Th_2\tr After Layer_20", 100);	finish_count(msg);
			// #endif
		// }
		// sync_barrier();
		// sync_barrier_reset();		sync_barrier_thread_registration();
		// if (Klessydra_get_coreID()==1){
			// #ifdef COUNT_CYCLES
				// strncpy(msg, "Th_1\tr After Layer_20", 100);	finish_count(msg);
			// #endif
		// }
		// sync_barrier();
		// sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==0){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\tr After Layer_20", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();

		#ifdef ISEQUAL_RUNTIME//°°°°°°°°°°check if is equal
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					for(int cmpt_spm = 0; cmpt_spm < dimension_out_20 ; cmpt_spm++)
					{
						if(even.lay_20[cmpt_spm]!=dut_matrix_L20_0[cmpt_spm]){
							errori++;
							printf("[%d]Error: SPM=%d != EXP=%d\n",cmpt_spm,	even.lay_20[cmpt_spm],	dut_matrix_L20_0[cmpt_spm] );
						}
					}
					if (errori==0)		printf("Success in fully-connected layer!\n");
					else printf("There are %d ERRORS\n",errori);
					errori=0;
			}
			sync_barrier();
		#endif//°°°°°°°°°°check if is equal	
		
	#endif
	
	// ------------------> EXECUTION With NO_SPMU
	#ifdef NO_SPMU
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_20\n");
		}
		sync_barrier();
		divisor_0=	4096/3;
		divisor_1=	divisor_0*2;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int i = 0; i < divisor_0; i++){
				getWeights(pt_to_fl, 4096, kern.kernel_4096,i);
				even.lay_20[i] = fullyconnect(4096, punt, kern.kernel_4096);
				bias = getBias(pt_to_bs,i);
				even.lay_20[i]+= bias;
			}
		}
		if (Klessydra_get_coreID()==1){
			for (int i = divisor_0; i < divisor_1; i++){
				getWeights(pt_to_fl, 4096, kern.kernel_4096,i);
				even.lay_20[i] = fullyconnect(4096, punt, kern.kernel_4096);
				bias = getBias(pt_to_bs,i);
				even.lay_20[i]+= bias;
			}		
		}
		if (Klessydra_get_coreID()==0){
			for (int i = divisor_1; i < 4096; i++){
				getWeights(pt_to_fl, 4096, kern.kernel_4096,i);
				even.lay_20[i] = fullyconnect(4096, punt, kern.kernel_4096);
				bias = getBias(pt_to_bs,i);
				even.lay_20[i]+= bias;
			}		
		}
		sync_barrier();
				
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\t After Layer_20", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
	#endif
	// ------------------> END OF EXECUTION With NO_SPMU

#endif
//   /$$                                    /$$$$$$    /$$
//  | $$                                   /$$__  $$ /$$$$
//  | $$        /$$$$$$  /$$   /$$        |__/  \ $$|_  $$
//  | $$       |____  $$| $$  | $$          /$$$$$$/  | $$
//  | $$        /$$$$$$$| $$  | $$         /$$____/   | $$
//  | $$       /$$__  $$| $$  | $$        | $$        | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$        | $$$$$$$$ /$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|________/|______/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_21
	#ifdef INTERCEPT
		// I intercept fake parameters for quick debug
		for (int i = 0; i < 4096; i++)	{
			even.lay_20[i]= 24642726 + i;
		}	
	#endif
	errori=0;
	// pointers to filters and bias
	pt_to_fl = &layer21_filters[0];
	pt_to_bs = &layer21_bias[0];
	mv_fl=0;
	mv_bs=0;
	punt = &even.lay_20[0];

	
	//this is not actually multithreaded
	#ifdef MULTI_TH // this is still designed for 4kB memory for a single spm, could be improved
		sync_barrier_reset();		sync_barrier_thread_registration();
		#ifdef COUNT_CYCLES
			start_count();
		#endif
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_21\n");
		}
		sync_barrier();
		
		
		#if DEDICATED_SPMU==1
			getFeatures_SPM_fully((void*)spmaddrA,	4096, punt);
		#else
			sync_barrier_reset();	sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==0) 		getFeatures_SPM_fully((void*)spmaddrA,	4096, punt);
			sync_barrier();
		#endif
		
		// per fare del debug veloce
		#if QUICK_DEBUG
			dimension_out_21=6;
		#endif
		divisor_0=	dimension_out_21/3;
		divisor_1=	divisor_0*2;
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		#if DEDICATED_SPMU==1
			if (Klessydra_get_coreID()==0){
				for (int cmpt_spm = 0; cmpt_spm < divisor_0; cmpt_spm++)
				{
					getWeights_SPM	((void*)spmaddrB, 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)spmaddrB, 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==1){
				for (int cmpt_spm = divisor_0; cmpt_spm < divisor_1; cmpt_spm++)
				{
					getWeights_SPM	((void*)spmaddrB, 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)spmaddrB, 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==2){
				for (int cmpt_spm = divisor_1; cmpt_spm < dimension_out_21; cmpt_spm++)
				{
					getWeights_SPM	((void*)spmaddrB, 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)spmaddrB, 	
													 4096);
				}
			}
			// sync_barrier();
		#else
			if (Klessydra_get_coreID()==0){
				for (int cmpt_spm = 0; cmpt_spm < divisor_0; cmpt_spm++)
				{
					getWeights_SPM	((void*)((int*)spmaddrB+0*4096), 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)((int*)spmaddrB+0*4096), 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==1){
				for (int cmpt_spm = divisor_0; cmpt_spm < divisor_1; cmpt_spm++)
				{
					getWeights_SPM	((void*)((int*)spmaddrB+1*4096), 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)((int*)spmaddrB+1*4096), 	
													 4096);
				}
			}
			if (Klessydra_get_coreID()==2){
				for (int cmpt_spm = divisor_1; cmpt_spm < dimension_out_21; cmpt_spm++)
				{
					getWeights_SPM	((void*)((int*)spmaddrB+2*4096), 	pt_to_fl,	4096,	cmpt_spm);
					fullyconnect_SPM((void*)((int*)spmaddrC+cmpt_spm),
													 (void*)spmaddrA, 
													 (void*)((int*)spmaddrB+2*4096), 	
													 4096);
				}
			}
			// sync_barrier();	
		#endif
	
		#if DEDICATED_SPMU==1
			// sync_barrier_reset();		sync_barrier_thread_registration();		
			if (Klessydra_get_coreID()==0){
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 10	);
				// AddBias done simply in main
				CSR_MVSIZE((divisor_0-0)*SIZE_OF_INT);
				kaddv((void*)spmaddrC+0, (void*)spmaddrC+0, (void*)spmaddrD+0);
				// Relu and Store back in memory
				punt_out = &odds.lay_21[0];
				relu_SPM_fully((void*)spmaddrC+0, (void*)spmaddrC+0, (divisor_0-0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==1){
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 10	);
				// AddBias done simply in main
				CSR_MVSIZE((divisor_1-divisor_0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrD+(divisor_0)));
				// Relu and Store back in memory
				punt_out = &odds.lay_21[divisor_0];
				relu_SPM_fully((void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (divisor_1-divisor_0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==2){
				getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 10	);
				// AddBias done simply in main
				CSR_MVSIZE((dimension_out_21-divisor_1)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrD+(divisor_1)));
				// Relu and Store back in memory
				punt_out = &odds.lay_21[divisor_1];
				relu_SPM_fully((void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (dimension_out_21-divisor_1), (int*) punt_out);
			}
			// sync_barrier();
			// spmReset(SPM_DIMENSION);
		#else
			getBias_SPM_fully(	(void*)spmaddrD,	pt_to_bs,	mv_bs, 10	);
			if (Klessydra_get_coreID()==0) {
				CSR_MVSIZE((divisor_0-0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrD+(0)));
				punt_out = &odds.lay_21[0];
				relu_SPM_fully( (void*)((int*)spmaddrC+(0)), (void*)((int*)spmaddrC+(0)), (divisor_0-0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==1) {
				CSR_MVSIZE((divisor_1-divisor_0)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrD+(divisor_0)));
				punt_out = &odds.lay_21[divisor_0];
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_0)), (void*)((int*)spmaddrC+(divisor_0)), (divisor_1-divisor_0), (int*) punt_out);
			}
			if (Klessydra_get_coreID()==2) {
				CSR_MVSIZE((dimension_out_21-divisor_1)*SIZE_OF_INT);
				kaddv((void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrD+(divisor_1)));
				punt_out =&odds.lay_21[divisor_1];
				relu_SPM_fully( (void*)((int*)spmaddrC+(divisor_1)), (void*)((int*)spmaddrC+(divisor_1)), (dimension_out_19-divisor_1), (int*) punt_out);
			}
		#endif
		
		if (Klessydra_get_coreID()==0){		
			#ifdef COUNT_CYCLES
				strncpy(msg, "Th_0\tr After Layer_21", 100);	finish_count(msg);
			#endif
		}
		sync_barrier();
		
		#ifdef ISEQUAL_RUNTIME//°°°°°°°°°°check if is equal
			sync_barrier_reset();		sync_barrier_thread_registration();
			if (Klessydra_get_coreID()==2){
					for(int cmpt_spm = 0; cmpt_spm < dimension_out_21 ; cmpt_spm++)
					{
						if(odds.lay_21[cmpt_spm]!=dut_matrix_L21_0[cmpt_spm]){
							errori++;
							printf("[%d]Error: SPM=%d != EXP=%d\n",cmpt_spm,	odds.lay_21[cmpt_spm],	dut_matrix_L21_0[cmpt_spm] );
						}
						else printf("[%d]: SPM=%d == EXP=%d\n",cmpt_spm,	odds.lay_21[cmpt_spm],	dut_matrix_L21_0[cmpt_spm] );
					}
					if (errori==0)		printf("Success in fully-connected layer!\n");
					else printf("There are %d ERRORS\n",errori);
					errori=0;
			}
			sync_barrier();
		#endif//°°°°°°°°°°check if is equal	
		
		
		
	#endif

#endif
//   /$$                                    /$$$$$$   /$$$$$$
//  | $$                                   /$$__  $$ /$$__  $$
//  | $$        /$$$$$$  /$$   /$$        |__/  \ $$|__/  \ $$
//  | $$       |____  $$| $$  | $$          /$$$$$$/  /$$$$$$/
//  | $$        /$$$$$$$| $$  | $$         /$$____/  /$$____/
//  | $$       /$$__  $$| $$  | $$        | $$      | $$
//  | $$$$$$$$|  $$$$$$$|  $$$$$$$        | $$$$$$$$| $$$$$$$$
//  |________/ \_______/ \____  $$ /$$$$$$|________/|________/
//                       /$$  | $$|______/
//                      |  $$$$$$/
//                       \______/
#ifdef ACTIVE_LAYER_22
	#ifdef INTERCEPT
	// I intercept fake parameters for quick debug
	for (int i = 0; i < 10; i++){
		odds.lay_21[i] = 24642726 + i;
	}
	#endif
	mv_fl=0;
	mv_bs=0;

	
	//this is not actually multithreaded
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			printf("Inizio Layer_22\n");
			#ifdef COUNT_CYCLES
				start_count();
			#endif
		}
		sync_barrier();

		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			// for (int k = 0; k < 64; k++)
			// {
				// matrixReset_SPM(INPUT_SIZE, &even.lay_1[k][0][0]);
			// }
			for (int k = 0; k < 10; k++)
			{
				// RISCY_matrixReset(OUTPUT_SIZE, &even.lay_22[k]);
				even.lay_22[k]=0;
			}
			punt = &odds.lay_21[0];
			punt_lay22 = &even.lay_22[0];
			softmax(OUTPUT_SIZE, punt, punt_lay22);
		}			
		sync_barrier();
		
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			#ifdef COUNT_CYCLES
				strncpy(msg, "After Layer_22", 100);	finish_count(msg);
			#endif
		}
		if (Klessydra_get_coreID()==1){
			spmReset(SPM_DIMENSION);
			// CSR_MVSIZE(SPM_DIMENSION);	kbcast((void*)spmaddrD,(void*)single_value_zero);
			finish_count_nomsg();
		}
		if (Klessydra_get_coreID()==0){		
			finish_count_nomsg();
		}
		sync_barrier();
	#endif


#endif
//    /$$$$$$              /$$                           /$$
//   /$$__  $$            | $$                          | $$
//  | $$  \ $$ /$$   /$$ /$$$$$$    /$$$$$$  /$$   /$$ /$$$$$$
//  | $$  | $$| $$  | $$|_  $$_/   /$$__  $$| $$  | $$|_  $$_/
//  | $$  | $$| $$  | $$  | $$    | $$  \ $$| $$  | $$  | $$
//  | $$  | $$| $$  | $$  | $$ /$$| $$  | $$| $$  | $$  | $$ /$$
//  |  $$$$$$/|  $$$$$$/  |  $$$$/| $$$$$$$/|  $$$$$$/  |  $$$$/
//   \______/  \______/    \___/  | $$____/  \______/    \___/
//                                | $$
//                                | $$
//                                |__/
#ifdef ACTIVE_LAYER_OUTPUT
	#ifdef MULTI_TH
		sync_barrier_reset();		sync_barrier_thread_registration();
		if (Klessydra_get_coreID()==2){
			for (int j = 0; j < OUTPUT_SIZE; j++)
			{
			// printf("even.lay_22[%d]=%f\n",j,even.lay_22[j]);
			printf("even.lay_22[%d]=%d\n",j,(int)((even.lay_22[j])*pow(2,21)) );
				if (even.lay_22[j] > max)
			{
						max = even.lay_22[j];
							maxpos = j;
				}
			}
			printf("The category is the [%d] element of file \"classification.txt\"\n",maxpos);
			// printf("The probability is: %f\n",max);
			printf("The probability is: %d/(2^21)",(int)((max)*pow(2,21)) );

			printf("\n------------------------\n\t\tEnd_program_Vgg_Fully_fixed_Semigruppi\n\n");
		}
		sync_barrier();
	#endif
#endif


	EXIT:
	return 0;
}

//   /$$$$$$$            /$$
//  | $$__  $$          | $$
//  | $$  \ $$  /$$$$$$ | $$$$$$$  /$$   /$$  /$$$$$$
//  | $$  | $$ /$$__  $$| $$__  $$| $$  | $$ /$$__  $$
//  | $$  | $$| $$$$$$$$| $$  \ $$| $$  | $$| $$  \ $$
//  | $$  | $$| $$_____/| $$  | $$| $$  | $$| $$  | $$
//  | $$$$$$$/|  $$$$$$$| $$$$$$$/|  $$$$$$/|  $$$$$$$
//  |_______/  \_______/|_______/  \______/  \____  $$
//                                           /$$  \ $$
//                                          |  $$$$$$/
//                                           \______/
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
void convolution2D_SPM_off_NOB(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution

	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spm_temp);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;


	//______________________________sub_kernel F
  CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	0;
	fm_offset= (size-1-1);
	for(int i=1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ) );
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset )	);
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset )	);
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}


	// kern_offset	=	0;
	// fm_offset= (size-1-1);
	// for(int i=1; i< size-1;i++){
		// CSR_MVSIZE(2*SIZE_OF_INT);
		// dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		// dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		// kdotpps		(dest_in_D+4,			(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ) );
		// kdotpps		(dest_in_D+8,			(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset )	);
		// kdotpps		(dest_in_D+12,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset )	);
    // // kaddv(dest_in_C, dest_in_C, dest_in_D);
    // // kaddv(dest_in_C, dest_in_C, dest_in_D);
		// // kaddv(dest_in_C, dest_in_C, dest_in_D);
		// CSR_MVSIZE(3*SIZE_OF_INT);
		// kvred32(dest_in_C,dest_in_D);
	// }
// // int kbacst16_v2(void* rd, void* rs1, int size)
// // {
	// // __asm__(
		// // "csrw 0xBF0, %[size];"
		// // "kvred16 %[rd], %[rs1];"
		// // ://no output register
		// // :[size] "r" (size), [rd] "r" (rd), [rs1] "r" (rs1)
		// // :/*no clobbered registers*/
	// // );

	// // return 1;
// // }


  //______________________________sub_kernel___________A-C-G-I
	CSR_MVSIZE(2*SIZE_OF_INT);
  //______________________________sub_kernel A
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1);
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+4,		(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1);
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D-4,		(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1);
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+4,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1);
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D-4,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));

	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
  	CSR_MVSIZE(1*SIZE_OF_INT);

	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  			(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),			(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1));

  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	  	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1) + 4	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),      		 	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1) - 4	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  				(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1) +4	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),				(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1) -4	);



	//______________________________sub_kernel D
  CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	1;
	fm_offset		=	0;
	for(int i=1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_C, dest_in_C, dest_in_D);
	}



  //______________________________sub_kernel E
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=1; i< size-1;i++)
	{
		// dest_in_C	= (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		// dest_in_D	= (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);
		}
	}



  // //______________________________sub_kernel B
  // CSR_MVSIZE((size-2)*SIZE_OF_INT);
  for(int i=0; i< 1;i++)
  {
    dest_in_C	= (void*)spmaddrCoff  + 1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff  + 1*SIZE_OF_INT;
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



	//______________________________sub_kernel H
  // CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=size-1; i< size;i++)
	{
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
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

	CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
	// CSR_MVSIZE(3*3*SIZE_OF_INT);
	// ksvmulrf((void*)spmaddrBoff,(void*)spmaddrBoff,(void*)zero);
	// // kbcast((void*)spmaddrBoff,(void*)shift_spmB);
}
void convolution2D_SPM_off_NOB_print(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print_F		=0;
	int print_ACGI=0;
	int print_D		=0;
	int print_E		=0;
	int print_B		=0;
	int print_H		=1;
  //Pointers to Spms and other index that i'll need for the convolution

	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spm_temp);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;

	// if(print_global_dbg && print_global_id%64 == 0){
	// 	printf("\n\n****************************\t Stampo matrice %d -> %d\t****************************\n",print_global_k,print_global_id);
	// }

	//______________________________sub_kernel F
  CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	0;
	fm_offset= (size-1-1);
	for(int i=1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
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


  //______________________________sub_kernel___________A-C-G-I
	CSR_MVSIZE(2*SIZE_OF_INT);
  //______________________________sub_kernel A
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1);
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+4,		(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1);
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D-4,		(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1);
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+4,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1);
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D-4,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));

	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
  	CSR_MVSIZE(1*SIZE_OF_INT);

	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  			(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),			(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1));

  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	  	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1) + 4	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),      		 	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1) - 4	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  				(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1) +4	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),				(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1) -4	);

	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0&& print_ACGI){
		printf("Primo ciclo spmC dopo kernel F-ACGI\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	//______________________________sub_kernel D
  CSR_MVSIZE(2*SIZE_OF_INT);
	kern_offset	=	1;
	fm_offset		=	0;
	for(int i=1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i);
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
	for(int i=1; i< size-1;i++)
	{
		// dest_in_C	= (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		// dest_in_D	= (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);
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
    dest_in_D	= (void*)spmaddrDoff  + 1*SIZE_OF_INT;
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
	if(print_global_dbg && print_global_id%64 == 0 && print_global_k%8 == 0 && print_B){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }


	//______________________________sub_kernel H
  // CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=size-1; i< size;i++)
	{
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
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
	// if(print_H && print_global_id%4 == 0){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B-H\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);

	k_element=0;
	CSR_MVSIZE(size*size*SIZE_OF_INT);
	kbcast((void*)spmaddrDoff,(void*)k_element);

	// CSR_MVSIZE(3*3*SIZE_OF_INT);
	// ksvmulrf((void*)spmaddrBoff,(void*)spmaddrBoff,(void*)zero);

	// // kbcast((void*)spmaddrBoff,(void*)shift_spmB);
}
void convolution2D_SPM_off_NOB_2x2(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution

	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spm_temp);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;


  //______________________________sub_kernel___________A-C-G-I
	CSR_MVSIZE(2*SIZE_OF_INT);
  //______________________________sub_kernel A
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	// dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1);
	dest_in_D	=		(void*)spmaddrDoff;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D+(SIZE_OF_INT*0),			(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+(SIZE_OF_INT*1),			(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));

	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D+(SIZE_OF_INT*2),			(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+(SIZE_OF_INT*3),			(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	
		CSR_MVSIZE(1*SIZE_OF_INT);
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff + (SIZE_OF_INT*0)	); 	
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1), 	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	 	(void*)spmaddrDoff + (SIZE_OF_INT*1)	);  
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff + (SIZE_OF_INT*2)  );
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),      	 	(void*)spmaddrDoff + (SIZE_OF_INT*3)  );
	
	
	CSR_MVSIZE(2*SIZE_OF_INT);
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	// dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1);
	dest_in_D	=		(void*)spmaddrDoff;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D+(SIZE_OF_INT*0),			(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+(SIZE_OF_INT*1),		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D+(SIZE_OF_INT*2),			(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_D+(SIZE_OF_INT*3),		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));

	CSR_MVSIZE(1*SIZE_OF_INT);
	kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  			(void*)spmaddrDoff +(SIZE_OF_INT*0)	);  
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),			(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  			(void*)spmaddrDoff +(SIZE_OF_INT*1)	);  
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),			(void*)spmaddrDoff +(SIZE_OF_INT*2)	);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),			(void*)spmaddrDoff +(SIZE_OF_INT*3) );
	
	
	CSR_MVSIZE(size*size*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
	// CSR_MVSIZE(3*3*SIZE_OF_INT);
	// ksvmulrf((void*)spmaddrBoff,(void*)spmaddrBoff,(void*)zero);
	// // kbcast((void*)spmaddrBoff,(void*)shift_spmB);
}
void convolution2D_SPM_off_NOB_zeropadded(void* spm_dest, 	 void* spm_fm,	 void* spm_krn,	 void* spm_temp,  int size){
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)(spm_krn );
	void* spmaddrCoff= (void*)(spm_dest);
	void* spmaddrDoff= (void*)(spm_temp);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int zero=0;
  //______________________________sub_kernel E
  CSR_MVSIZE((size-2)*SIZE_OF_INT); //però in questo caso la dimesione è quella della matrice zeropadded
	for(int i=1; i< size-1;i++)
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

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
						 (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
						 (void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	),
        			 (void*)( (int*)	spmaddrAoff + (i+rw_pt)*size	+1 ),
               (void*)( (int*)	spmaddrBoff	+	k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	),
							(void*)(	(int*)	(spmaddrDoff)	),
							(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
						 (void*)(	(int*)	(spmaddrDoff)	) );

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	),
							 (void*)( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
							 (void*)( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	),
							(void*)(	(int*)	(spmaddrDoff)	),
							(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
						 (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
						 (void*)(	(int*)	(spmaddrDoff)	) );
		}
	}
  CSR_MVSIZE((size-2)*SIZE_OF_INT); //però in questo caso la dimesione è quella della matrice zeropadded
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)zero);
}
void convolution2D_SPM_off_ALT_2x2(void* spm_dest, void* spm_fm, void* spm_krn, void* spm_bank, int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution

	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm		);
	void* spmaddrBoff= (void*)(spm_krn 	);
	void* spmaddrCoff= (void*)(spm_dest	);
	void* spmaddrDoff= (void*)(spm_bank	);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int shift_spmB=1;

	shift_spmB=0;
  //______________________________sub_kernel___________A-C-G-I
  //______________________________sub_kernel A
	CSR_MVSIZE(2*SIZE_OF_INT);
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=1
    shift_spmB++;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps_v3(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ), (void*) conv2D_out_scal);
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=2
    shift_spmB++;
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=3
    shift_spmB++;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=4
    shift_spmB++;
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
  CSR_MVSIZE(1*SIZE_OF_INT);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),	(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1));

  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrBoff+SIZE_OF_INT*9);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrBoff+SIZE_OF_INT*10);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  				(void*)spmaddrBoff+SIZE_OF_INT*11);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),	(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),				(void*)spmaddrBoff+SIZE_OF_INT*12);

	// if(print){
		// printf("Primo ciclo spmC dopo kernel ACGI\n");
		// display_spm_matrix(size,size, (void*)spmaddrCoff);
	// }

	CSR_MVSIZE(size*size*SIZE_OF_INT);
	shift_spmB=0;
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)shift_spmB);
	CSR_MVSIZE((3+1)*(3+1)*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrBoff,(void*)spmaddrBoff,(void*)shift_spmB);
	// kbcast((void*)spmaddrBoff,(void*)shift_spmB);
}
void convolution2D_SPM_off_ALT(void* spm_dest, void* spm_fm, void* spm_krn, void* spm_bank, int size){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution

	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm		);
	void* spmaddrBoff= (void*)(spm_krn 	);
	void* spmaddrCoff= (void*)(spm_dest	);
	void* spmaddrDoff= (void*)(spm_bank	);

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int shift_spmB=1;

	// printf("----------------------------------------------------------------------------\n");
	// // display_spm_matrix_X(8,8,(void*)spmaddrAoff);
	// // printf("****************************\n");
	// display_spm_matrix_X(3,3,(void*)spmaddrBoff);
	// printf("****************************\n");



	//______________________________sub_kernel F
  CSR_MVSIZE(2*SIZE_OF_INT);
	for(int i=1; i< size-1;i++){
	// for(int i=1; i< 2; i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_B	= (void*)spmaddrBoff + SIZE_OF_INT*9;
		kern_offset	=	0;
		fm_offset= (size-1-1);
		kdotpps_v3(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ), (void*) conv2D_out_scal);
		kdotpps		(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_D, dest_in_B, dest_in_D);
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print){
		printf("Primo ciclo spmC dopo kernel F\n");
		// display_spm_matrix(size,size, (void*)spmaddrCoff);
	}
	shift_spmB=0;
  //______________________________sub_kernel___________A-C-G-I
  //______________________________sub_kernel A
	CSR_MVSIZE(2*SIZE_OF_INT);
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=1
    shift_spmB++;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=2
    shift_spmB++;
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=3
    shift_spmB++;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=4
    shift_spmB++;
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
  CSR_MVSIZE(1*SIZE_OF_INT);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),	(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1));

  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrBoff+SIZE_OF_INT*9);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrBoff+SIZE_OF_INT*10);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  				(void*)spmaddrBoff+SIZE_OF_INT*11);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),	(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),				(void*)spmaddrBoff+SIZE_OF_INT*12);
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	//______________________________sub_kernel D
  CSR_MVSIZE(2*SIZE_OF_INT);
  shift_spmB=1;
	for(int i=1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i);
    dest_in_B	= (void*)spmaddrBoff + SIZE_OF_INT*12*shift_spmB; //shift_spmB=1
		kern_offset	=	1;
		fm_offset		=	0;
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
		kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_D, dest_in_B, dest_in_D);
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



  //______________________________sub_kernel E
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=1; i< size-1;i++)
	{
		// dest_in_C	= (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		// dest_in_D	= (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);
		}
	}
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



  // //______________________________sub_kernel B
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
  for(int i=0; i< 1;i++)
  {
    dest_in_C	= (void*)spmaddrCoff  + 1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff  + 1*SIZE_OF_INT;
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
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



	//______________________________sub_kernel H
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=size-1; i< size;i++)
	{
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
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
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B-H\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	// display_spm_matrix(size,size, (void*)spmaddrBoff);
	CSR_MVSIZE(size*size*SIZE_OF_INT);
	shift_spmB=0;
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)shift_spmB);
	CSR_MVSIZE((3+1)*(3+1)*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrBoff,(void*)spmaddrBoff,(void*)shift_spmB);
	// kbcast((void*)spmaddrBoff,(void*)shift_spmB);
}
void convolution2D_SPM_off(void* spm_dest, void* spm_fm, void* spm_krn, int size, int mem_off){
	int print=0;
  //Pointers to Spms and other index that i'll need for the convolution

	// void* spmaddrAoff= (void*)((int*)spm_fm + mem_off );
	void* spmaddrAoff= (void*)(spm_fm);
	void* spmaddrBoff= (void*)((int*)spm_krn 	+ mem_off );
	void* spmaddrCoff= (void*)((int*)spm_dest + mem_off );
	void* spmaddrDoff= (void*)((int*)spmaddrD + mem_off );

	void* dest_in_C;
  void* dest_in_B;
  void* dest_in_D;

  int k_element=0;
  int mat_int_shift=0; //internal shifting for properly pointing insied the spms while making kaddv

	int jump_kr_row=3; // determina il salto della riga per la matrice kernel zeropadded
	int kern_offset=0;
	int fm_offset=0;
  int shift_spmB=1;

	// printf("----------------------------------------------------------------------------\n");
	// // display_spm_matrix_X(8,8,(void*)spmaddrAoff);
	// // printf("****************************\n");
	// display_spm_matrix_X(3,3,(void*)spmaddrBoff);
	// printf("****************************\n");



	//______________________________sub_kernel F
  CSR_MVSIZE(2*SIZE_OF_INT);
	for(int i=1; i< size-1;i++){
	// for(int i=1; i< 2; i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+ SIZE_OF_INT*(1)*(size-1);
		dest_in_B	= (void*)spmaddrBoff + SIZE_OF_INT*9;
		kern_offset	=	0;
		fm_offset= (size-1-1);
		kdotpps_v3(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ), (void*) conv2D_out_scal);
		kdotpps		(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_D, dest_in_B, dest_in_D);
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps		(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print){
		printf("Primo ciclo spmC dopo kernel F\n");
		// display_spm_matrix(size,size, (void*)spmaddrCoff);
	}
	shift_spmB=0;
  //______________________________sub_kernel___________A-C-G-I
  //______________________________sub_kernel A
	CSR_MVSIZE(2*SIZE_OF_INT);
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1); //[0]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=1
    shift_spmB++;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel C
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1); //[4]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=2
    shift_spmB++;
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(0)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(1)*size			+ fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel G
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1); //[20]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=3
    shift_spmB++;
	kern_offset	=	1;
	fm_offset		=	0;
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	//______________________________sub_kernel I
	dest_in_C	=		(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1); //[24]
	dest_in_D	=		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1);
  dest_in_B	= 	(void*)spmaddrBoff + SIZE_OF_INT*(9+shift_spmB); //move_b_pt=4
    shift_spmB++;
	kern_offset	=	0;
	fm_offset		=	(size-1-1);
	kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(size-1-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
	kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(size-1)	*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
	// //______________________________sommo i parziali prodotti dei sub_kernels A-C-G-I
  CSR_MVSIZE(1*SIZE_OF_INT);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrDoff + SIZE_OF_INT*(0)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrDoff + SIZE_OF_INT*(1)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  		(void*)spmaddrDoff + SIZE_OF_INT*(size)*(size-1));
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),	(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),		(void*)spmaddrDoff + SIZE_OF_INT*(size+1)*(size-1));

  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(0)*(size-1),	       	(void*)spmaddrBoff+SIZE_OF_INT*9);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),	    	(void*)spmaddrCoff + SIZE_OF_INT*(1)*(size-1),        	(void*)spmaddrBoff+SIZE_OF_INT*10);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),		(void*)spmaddrCoff + SIZE_OF_INT*(size)*(size-1),  				(void*)spmaddrBoff+SIZE_OF_INT*11);
  kaddv((void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),	(void*)spmaddrCoff + SIZE_OF_INT*(size+1)*(size-1),				(void*)spmaddrBoff+SIZE_OF_INT*12);
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}


	//______________________________sub_kernel D
  CSR_MVSIZE(2*SIZE_OF_INT);
  shift_spmB=1;
	for(int i=1; i< size-1;i++){
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i);
		dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i);
    dest_in_B	= (void*)spmaddrBoff + SIZE_OF_INT*12*shift_spmB; //shift_spmB=1
		kern_offset	=	1;
		fm_offset		=	0;
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i-1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(0)*jump_kr_row+	kern_offset ));
		kdotpps(dest_in_B,		(void*)(	(int*)spmaddrAoff+	(i)*size				+fm_offset	),	(void*) ( (int*)spmaddrBoff+(1)*jump_kr_row+	kern_offset ));
    kaddv(dest_in_D, dest_in_B, dest_in_D);
    kaddv(dest_in_C, dest_in_C, dest_in_D);
		kdotpps(dest_in_D,		(void*)(	(int*)spmaddrAoff+	(i+1)*size			+fm_offset	),	(void*) ( (int*)spmaddrBoff+(2)*jump_kr_row+	kern_offset ));
		kaddv(dest_in_C, dest_in_C, dest_in_D);
	}
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



  //______________________________sub_kernel E
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=1; i< size-1;i++)
	{
		// dest_in_C	= (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		// dest_in_D	= (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
		k_element=0;
		for (int rw_pt=-1; rw_pt<2; rw_pt++) //rw_pt is an index i use to point to the correct row, regarding this loop that is executed three times
		//instead of making 9 different ksvmulrf
		{
			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+0 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+1 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);

			ksvmulsc((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
        			  (void*)	( (int*)spmaddrAoff + (i+rw_pt)*size	+2 ),
                (void*)	( (int*)spmaddrBoff+k_element++) );

				ksrav((void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
								(int*)conv2D_out_scal);

  		kaddv ((void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrCoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT,
              (void*)(	(int*)	(spmaddrDoff)	) + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT);
		}
	}
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



  // //______________________________sub_kernel B
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
  for(int i=0; i< 1;i++)
  {
    dest_in_C	= (void*)spmaddrCoff  + 1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff  + 1*SIZE_OF_INT;
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
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
  }



	//______________________________sub_kernel H
  CSR_MVSIZE((size-2)*SIZE_OF_INT);
	for(int i=size-1; i< size;i++)
	{
		dest_in_C	= (void*)spmaddrCoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
    dest_in_D	= (void*)spmaddrDoff + SIZE_OF_INT*(size*i)+1*SIZE_OF_INT;
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
	if(print){
		printf("Primo ciclo spmC dopo kernel F-ACGI-D-E-B-H\n");
		display_spm_matrix(size,size, (void*)spmaddrCoff);
	}

	// display_spm_matrix(size,size, (void*)spmaddrBoff);
	CSR_MVSIZE(size*size*SIZE_OF_INT);
	shift_spmB=0;
	ksvmulrf((void*)spmaddrDoff,(void*)spmaddrDoff,(void*)shift_spmB);
	CSR_MVSIZE((3+1)*(3+1)*SIZE_OF_INT);
	ksvmulrf((void*)spmaddrBoff,(void*)spmaddrBoff,(void*)shift_spmB);
	// kbcast((void*)spmaddrBoff,(void*)shift_spmB);
}
//------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------
void spmReset(int size){
	// kmemld((void*)spmaddrA,(void*)((int*)spm_clean),(size*size*SIZE_OF_INT));
	CSR_MVSIZE(size);
	int zero=0;
	// i don't know why, but using ksvmulrf means having errors in the main
	// ksvmulrf((void*)spmaddrA,(void*)spmaddrA,(void*)zero);
	// ksvmulrf((void*)spmaddrB,(void*)spmaddrB,(void*)zero);
	// ksvmulrf((void*)spmaddrC,(void*)spmaddrC,(void*)zero);
	// ksvmulrf((void*)spmaddrD,(void*)spmaddrD,(void*)zero);
	
	//provo a far resettare solamente la SPM_D, che per sicurezza credo sia l'unica che va pulita
	kbcast((void*)spmaddrA,(void*)zero);
	kbcast((void*)spmaddrB,(void*)zero);
	kbcast((void*)spmaddrC,(void*)zero);
	kbcast((void*)spmaddrD,(void*)zero);
}
void matrixReset_SPM(int size, int *matrix){
	// kmemstr((void*)(matrix+stride*(size*size)),	(void*)spmaddrA,	(size*size*SIZE_OF_INT));
	// NB modificare la funzione e mettere oppure tutti zeri preventivamente sulla matrice spmaddrA!!!!!!!!!!!!!!!!!
	kmemstr((void*)(matrix),	(void*)spmaddrB,	(size*size*SIZE_OF_INT));
}
void start_count(){
	// printf("(start_counting):\n");
	performance=0;
	int cnt_en=0;
  // __asm__("csrrw zero, mcycle, zero;"
			// "csrrw zero, 0x7A0, 0x00000001");
  __asm__(	
						// resetto registri
						"csrrw zero, 0xB00, 	zero;"
						"csrrw zero, 0xB02, 	zero;"
						"csrrw zero, 0xB06, 	zero;"
						"csrrw zero, 0xB07, 	zero;"
						//abilito tutto
						// "li %[cnt_en], 0x00000FF3;"
						"li %[cnt_en], 0x00000063;"
						"csrrw zero, 0x7A0, %[cnt_en];"
						:
						:[cnt_en] "r" (cnt_en)	);
}
void finish_count(char msgp[]){
	// __asm__("csrrw zero, 0x7A0, 0x00000000;"
			// "csrrw %[perf], mcycle, zero;"
			// "sw %[perf], 0(%[ptr_perf]);"
			// :
			// :[perf] "r" (perf), [ptr_perf] "r" (ptr_perf)
			// );
	// DISABLE COUNTING AND SAVE MCYCLE -------------------------------------------------------

	__asm__("csrrw zero, 0x7A0, 0x00000000");
	
  __asm__("csrrw %[perf], 0xB00, zero;"
      "sw %[perf], 0(%[ptr_perf]);"
      :
      :[perf] "r" (perf), 	[ptr_perf] "r" (ptr_perf)
      );
  perf_results[0]=perf;//CICLI
	__asm__("csrrw %[perf], 0xB02, zero;"
		"sw %[perf], 0(%[ptr_perf]);"
		:
		:[perf] "r" (perf), 	[ptr_perf] "r" (ptr_perf)
		);
  perf_results[1]=perf;//ISTRUZIONI

	__asm__("csrrw %[perf], 0xB06, zero;"
		"sw %[perf], 0(%[ptr_perf]);"
		:
		:[perf] "r" (perf), 	[ptr_perf] "r" (ptr_perf)
		);
  perf_results[2]=perf;//Load
	
	__asm__("csrrw %[perf], 0xB07, zero;"
		"sw %[perf], 0(%[ptr_perf]);"
		:
		:[perf] "r" (perf), 	[ptr_perf] "r" (ptr_perf)
		);
  perf_results[3]=perf;//Store
			
			
	performance=perf;
	printf("Fine_Conteggio:\t");
	for (int i =0 ; i <4; i++){ 
		printf("{%d}=%d\t",i,perf_results[i]);
		perf_results[i]=0;
	}
	printf("\n");
	perf=0;
	performance=0;
  // printf("%s\t-->Cycles=%d\n\n",msgp, performance);
	// perf=0;
	// performance=0;
}
void finish_count_nomsg(){
	__asm__("csrrw zero, 0x7A0, 0x00000000;"
			"csrrw %[perf], mcycle, zero;"
			"sw %[perf], 0(%[ptr_perf]);"
			:
			:[perf] "r" (perf), [ptr_perf] "r" (ptr_perf)
			);
	performance=perf;
	perf=0;
	performance=0;
}
void printmod_sel(int layer,int ch, int dim_i, int dim_j, int *input,int shift){
	//this will print on Transcript only the specified slice of the matrix: E.g. ch=64 will print matrix[63]
	int k,i,j;
	k=ch-1; //per stampare proprio la matrice selezionata
	printf("-----DBG_print_sel:\tOUTPUT_%d_layer_%d\n",ch,layer);
	for (int i = 0; i < dim_i; i++)
	{
		for (int j = 0; j < dim_j; j++)
		{
			printf("\t%d", (int)(((float)(input[k*dim_i*dim_j+i*dim_j+j]))/pow(2,shift)) );
		}
		printf("\n");
	}
}
void printmod_full(int layer,int ch, int dim_i, int dim_j, int *input, int shift){
	//this will print on Transcript the whole matrix
	int k,i,j;
	printf("DBG_print_full:\tOUTPUT_completo_layer_%d\n",layer);
	for (int k = 0; k < ch; k++)
	{
		for (int i = 0; i < dim_i; i++)
		{
			for (int j = 0; j < dim_j; j++)
			{
				printf("%d ", (int)(((float)(input[k*dim_i*dim_j+i*dim_j+j]))/pow(2,shift)));
			}
			printf("\n");
		}
	}
}
void dbg_count_zero(int ch, int dim_i, int dim_j,int *input){
	printf("Dentro_Counter_zero\t");
	int counter=0;
	int k,i,j;
	for (int k = 0; k < ch; k++)
	{
		for (int i = 0; i < dim_i; i++)
		{
			for (int j = 0; j < dim_j; j++)
			{
				if((int)input[ k*(dim_i*dim_j) + i*(dim_j) + j] == 0)
				{
					counter++;
				}
			}
		}
	}
	printf("DBG_Counter:%d\n",counter);
}
void spegni_threads(){
	for (int i=0;i<5;i++)
	{
		if (Klessydra_get_coreID()==0);
		else
		{
			Klessydra_WFI();
		}
	}
  printf("\n---Mode:Single Thread_0---\n");
}
void display_spm_matrix(int size_r,int size_c,void* pt_spm_mat){
  printf("\n--------------------------------------\n");
  printf("\t\t-------Display_matrix-----%d\n",pt_spm_mat);
  int pref_buff[32*32]={0};
  kmemstr((int*)pref_buff, (void*)(pt_spm_mat), (size_r*size_c)*SIZE_OF_INT);
  for(int i=0; i<size_r; i++)
  {
    for(int j=0; j<size_c; j++)
    {
      printf("\t%8d",pref_buff[i*size_r+j]);
    }
    printf("\n");
  }
  printf("\t\t-------Display_matrix-----\n");
  printf("--------------------------------------\n\n");
}
void display_spm_matrix_X(int size_r,int size_c,void* pt_spm_mat){
  printf("\n--------------------------------------\n");
  printf("\t\t-------Display_matrix-----\n");
  int pref_buff[32*32]={0};
  kmemstr((int*)pref_buff, (void*)(pt_spm_mat), (size_r*size_c)*SIZE_OF_INT);
  for(int i=0; i<size_r; i++)
  {
    for(int j=0; j<size_c; j++)
    {
      printf("\t%d",(pref_buff[i*size_r+j])<<8	);
    }
    printf("\n");
  }
  printf("\t\t-------Display_matrix-----\n");
  printf("--------------------------------------\n\n");
}
void display_spm_matrix_scal(int size_r,int size_c,void* pt_spm_mat, int shift){
  printf("\n--------------------------------------\n");
  printf("\t\t-------Display_matrix-----\n");
	int k=0;
  int pref_buff[32*32]={0};
  kmemstr((int*)pref_buff, (void*)((int*)pt_spm_mat + k*size_r*size_c), (size_r*size_c)*SIZE_OF_INT);
  for(int i=0; i<size_r; i++)
  {
    for(int j=0; j<size_c; j++)
    {
			printf("\t%8d", (int)(((float)(pref_buff[i*size_r+j]))/pow(2,shift)));
      // printf("\t%d", (	(pref_buff[i*size_r+j])	>>	prnt_sc	)		);
    }
    printf("\n");
  }
  printf("\t\t-------Display_matrix-----\n");
  printf("--------------------------------------\n\n");
}
void matrix_check_v2( int* mat1, int* mat2, int size){
	printf("Checking if there is an error (V2)...\n");
  int err=0;
	for(int i=0; i<size; i++)
	{
		for(int j=0; j<size; j++)
		{
			if ( *((int*)mat1+i*size+j) != *((int*)mat2+i*size+j) ) {
				printf("\t{%3d}",*((int*)mat1+i*size+j));
        err++;
			}
      else{
        printf("\t%5d",*((int*)mat1+i*size+j));
      }
		}
    printf("\n");
	}
  if (err==0){
    printf("No errors.\n");
  }
}
void isequal(int layer, int row, int col, int (*pt1)[row], const int *pt2, int id){
	int err=0;
	int interno=0;
	int esterno=0;
	for(int i=0;	i<row;	i++)
		{
			for(int j=0;	j<col;	j++)
			{
				// interno= pt1[i][j];
				interno= pt1[i][j];
				esterno= pt2[i*row+j];
				if( interno != esterno )
				{
					//printf("ERROR at elements [%d][%d]!\t\tSPM=%d\tEXT=%d\n",i,j,interno,esterno);
					err++;
				}
			}
		}
  if (err==0){
    printf("Matrix Layer_%d output [%d]: SUCCESS.\n",layer,id);
  }
	else{
		printf("Matrix: Layer_%d output [%d]:\t\t%d errors!!!\n",layer,id,err);
	}
}
void isequal_v2(int layer, int row, int col, int *pt1, const int *pt2, int id){
	int err=0;
	int interno=0;
	int esterno=0;
	for(int i=0;	i<row;	i++)
		{
			for(int j=0;	j<col;	j++)
			{
				// interno= pt1[i][j];
				interno= pt1[i*row+j];
				esterno= pt2[i*row+j];
				if( interno != esterno )
				{
					printf("ERROR at elements [%d][%d]!\t\tSPM=%d\tEXT=%d\n",i,j,interno,esterno);
					err++;
				}
			}
		}
  if (err==0){
    printf("Matrix Layer_%d output row[%d]: No errors.\n",layer,id);
  }
	else{
		printf("Matrix: Layer_%d output row[%d]:\t\t%d errors!!!\n",layer,id,err);
	}
}
//------------------------------------------------------------------------------------------------------
/*
 echo && echo && tail execution_0.txt && echo && echo && tail execution_1.txt && echo && echo && tail execution_2.txt && echo && echo 
 */

