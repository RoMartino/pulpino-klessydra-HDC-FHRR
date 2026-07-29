#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

extern "C" {
	#include "dsp_functions.h"
	#include "functions.h"
	#include "klessydra_defs.h"
}

// VCU_TEST {1-->5}  Vector inputs / Vector Output Operations
// VCU_TEST {5-->} Scalar-Vector input / Vector Output Operations
// VCU_TEST {-->} Vector input / Scalar Output Operations
// VCU_TEST {-->} Other Operations (i.e. Vector Copy, and Broadcast)

#ifndef VCU_TEST
	#define VCU_TEST 1
#endif

#ifndef DEBUG
	#define DEBUG 0
#endif

#ifndef VSIZE
	#define VSIZE 27
#endif

#ifndef TEST
	#define TEST 2
#endif

#ifndef SIMD
	#define SIMD 4
#endif

#if TEST == 1
	#define SW_LOOP "SW Loop "
	#define SIMD_BYTES SIMD*4
#else
	#define SW_LOOP ""
	#define SIMD_BYTES 
#endif


#if VCU_TEST == 1
	#define VCU_INSTR "KADDV "
	#define VCU_INSTR_TYPE  "vector addition"
	#if   TEST == 1
		#define VCU_FUNC_NAME kaddv_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kaddv_complete
	#endif
#elif VCU_TEST == 2
	#define VCU_INSTR "KSUBV "
	#define VCU_INSTR_TYPE  "vector subtraction"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksubv_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksubv_complete
	#endif
#elif VCU_TEST == 3
	#define VCU_INSTR "KVMUL "
	#define VCU_INSTR_TYPE  "vector multiplication"
	#if   TEST == 1
		#define VCU_FUNC_NAME kvmul_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kvmul_complete
	#endif
#elif VCU_TEST == 4
	#define VCU_INSTR "KVDIV "
	#define VCU_INSTR_TYPE  "vector division"
	#if   TEST == 1
		#define VCU_FUNC_NAME kvdiv_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kvdiv_complete
	#endif
#elif VCU_TEST == 5
	#define VCU_INSTR "KVREM "
	#define VCU_INSTR_TYPE  "vector remainder"
	#if   TEST == 1
		#define VCU_FUNC_NAME kvrem_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kvrem_complete
	#endif
#elif VCU_TEST == 6
	#define VCU_INSTR "KVSLT "
	#define VCU_INSTR_TYPE  "vector set less than"
	#if   TEST == 1
		#define VCU_FUNC_NAME kvslt_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kvslt_complete
	#endif
#elif VCU_TEST == 7
	#define VCU_INSTR "KRELU "
	#define VCU_INSTR_TYPE  "vector rectify linear unit"
	#if   TEST == 1
		#define VCU_FUNC_NAME krelu_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME krelu_complete
	#endif
#elif VCU_TEST == 8
	#define VCU_INSTR "KSRLV "
	#define VCU_INSTR_TYPE  "vector right logic shift"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksrlv_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksrlv_complete
	#endif
#elif VCU_TEST == 9
	#define VCU_INSTR "KSRAV "
	#define VCU_INSTR_TYPE  "vector right arithmetic shift"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksrav_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksrav_complete
	#endif
#elif VCU_TEST == 10
	#define VCU_INSTR "KSVADDSC "
	#define VCU_INSTR_TYPE  "scalar-vector addition"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvaddsc_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvmulsc_complete
	#endif
#elif VCU_TEST == 11
	#define VCU_INSTR "KSVADDRF "
	#define VCU_INSTR_TYPE  "scalar-vector addition"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvaddrf_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvaddrf_complete
	#endif
#elif VCU_TEST == 12
	#define VCU_INSTR "KSVMULSC "
	#define VCU_INSTR_TYPE  "scalar-vector multiplication"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvmulsc_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvmulsc_complete
	#endif
#elif VCU_TEST == 13
	#define VCU_INSTR "KSVMULRF "
	#define VCU_INSTR_TYPE  "scalar-vector multiplication"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvmulrf_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvmulrf_complete
	#endif
#elif VCU_TEST == 14
	#define VCU_INSTR "KSVDIVSC "
	#define VCU_INSTR_TYPE  "scalar-vector division"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvdivsc_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvdivsc_complete
	#endif
#elif VCU_TEST == 15
	#define VCU_INSTR "KSVDIVRF "
	#define VCU_INSTR_TYPE  "scalar-vector division"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvdivrf_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvdivrf_complete
	#endif
#elif VCU_TEST == 16
	#define VCU_INSTR "KSVREMSC "
	#define VCU_INSTR_TYPE  "scalar-vector remainder"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvremsc_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvremsc_complete
	#endif
#elif VCU_TEST == 17
	#define VCU_INSTR "KSVREMRF "
	#define VCU_INSTR_TYPE  "scalar-vector remainder"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvremrf_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvremrf_complete
	#endif
#elif VCU_TEST == 18
	#define VCU_INSTR "KSVSLT "
	#define VCU_INSTR_TYPE  "scalar-vector set less than"
	#if   TEST == 1
		#define VCU_FUNC_NAME ksvslt_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME ksvslt_complete
	#endif
#elif VCU_TEST == 19
	#define VCU_INSTR "KVRED "
	#define VCU_INSTR_TYPE  "vector reduction addition"
	#if   TEST == 1
		#define VCU_FUNC_NAME kvred_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kvred_complete
	#endif
#elif VCU_TEST == 20
	#define VCU_INSTR "KDOTP "
	#define VCU_INSTR_TYPE  "vector dot product"
	#if   TEST == 1
		#define VCU_FUNC_NAME kdotp_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kdotp_complete
	#endif
#elif VCU_TEST == 21
	#define VCU_INSTR "KDOTPPS "
	#define VCU_INSTR_TYPE  "vector post scaling dot product"
	#if   TEST == 1
		#define VCU_FUNC_NAME kdotpps_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kdotpps_complete
	#endif
#elif VCU_TEST == 22
	#define VCU_INSTR "KBCAST "
	#define VCU_INSTR_TYPE  "scalar broadcast"
	#if   TEST == 1
		#define VCU_FUNC_NAME kbcast_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kbcast_complete
	#endif
#elif VCU_TEST == 23
	#define VCU_INSTR "KVCP "
	#define VCU_INSTR_TYPE  "vector copy"
	#if   TEST == 1
		#define VCU_FUNC_NAME kvcp_sw_loop_complete
	#elif TEST == 2
		#define VCU_FUNC_NAME kvcp_complete
	#endif
#endif

#ifndef TIME
	#define TIME 10
#endif

typedef enum {
    type_8b,
    type_16b,
    type_32b
} vect_e;

typedef struct {
	int vs1;
	int vs2;
	int vd;
	int vs1_scalar;
	int vs2_scalar;
	int vd_scalar;
} vect_properties_t;

vect_properties_t vcu_instr;

int8_t  vect8_1[VSIZE];
int16_t vect16_1[VSIZE];
int32_t vect32_1[VSIZE];
int8_t  vect8_2[VSIZE];
int16_t vect16_2[VSIZE];
int32_t vect32_2[VSIZE];
int8_t  testres8[VSIZE];
int16_t testres16[VSIZE];
int32_t testres32[VSIZE];
int8_t  result8[VSIZE];
int16_t result16[VSIZE];
int32_t result32[VSIZE];

void*  src8_1;
void*  src8_2;
void* src16_1;
void* src16_2;
void* src32_1;
void* src32_2;

int testperf[THREAD_POOL_SIZE], perf8[THREAD_POOL_SIZE], perf16[THREAD_POOL_SIZE], perf32[THREAD_POOL_SIZE];

int size8 =VSIZE*sizeof(char);
int size16=VSIZE*sizeof(short);
int size32=VSIZE*sizeof(int);

int power(int a,int b);

void check_kvcu(void* testres, void* vect1, void* vect2, vect_e vect);

inline void mcycle_start();
inline void mcycle_stop(int dsp_perf, int* dsp_ptr_perf);

int main()
{
	Klessydra_En_Int();
	sync_barrier_thread_registration();

	int hartID = Klessydra_get_coreID();

	int dsp_perf8  = 0;
	int dsp_perf16 = 0;
	int dsp_perf32 = 0;
	int test_perf_i = 0;
	int* dsp_ptr_perf8  = &dsp_perf8;
	int* dsp_ptr_perf16 = &dsp_perf16;
	int* dsp_ptr_perf32 = &dsp_perf32;
	int* test_ptr_perf  = &test_perf_i;

	//#if VCU_TEST < 4
	//	vcu_instr.vs1 = 1;
	//	vcu_instr.vs2 = 1;
	//	vcu_instr.vd  = 1;
	//	vcu_instr.vs1_scalar = 0;
	//	vcu_instr.vs2_scalar = 0;
	//	vcu_instr.vd_scalar  = 0;
	//#elif VCU_TEST < 6
	//	vcu_instr.vs1 = 1;
	//	vcu_instr.vs2 = 1;
	//	vcu_instr.vd  = 1;
	//	vcu_instr.vs1_scalar = 0;
	//	vcu_instr.vs2_scalar = 1;
	//	vcu_instr.vd_scalar  = 0;
	//#elif VCU_TEST < 6
	//#endif

	/******************************************************************************************/
	/********************************* Create Random Data *************************************/
	/******************************************************************************************/

	srand (TIME);


	for (int i=0; i<VSIZE; i++) 
	{
    	vect8_1[i]  = rand()  % (0x100      - 0x1) + 1;
    	vect16_1[i] = rand()  % (0x10000    - 0x1) + 1;
    	vect32_1[i] = rand()  % (0x80000000 - 0x1) + 1;
		// For shift instructions. the shift amounts below have a smaller random number range
		#if VCU_TEST == 8 || VCU_TEST == 9
			vect8_2[0]  = rand() % (0x7  - 0x1) +1;
			vect16_2[0] = rand() % (0xF  - 0x1) +1;
			vect32_2[0] = rand() % (0x1F - 0x1) +1;
		#else
			vect8_2[i]  = rand()  % (0x100      - 0x1) + 1;
			vect16_2[i] = rand()  % (0x10000    - 0x1) + 1;
			vect32_2[i] = rand()  % (0x80000000 - 0x1) + 1;
		#endif
	}



	src8_1  = (void*) vect8_1;
	src16_1 = (void*) vect16_1;
	src32_1 = (void*) vect32_1;

	#if VCU_TEST <= 7 || (VCU_TEST >= 19 && VCU_TEST <= 21)
	    src8_2  = (void*) vect8_2;
	    src16_2 = (void*) vect16_2;
	    src32_2 = (void*) vect32_2;
	#elif VCU_TEST >= 8 || VCU_TEST <= 18
	    #if VCU_TEST == 10 || VCU_TEST == 12 || VCU_TEST == 14 || VCU_TEST == 16
		    src8_2  = (void*)  &vect8_2[0];
		    src16_2 = (void*) &vect16_2[0];
		    src32_2 = (void*) &vect32_2[0];
	    #else
		    src8_2  = (void*)  vect8_2[0];
		    src16_2 = (void*) vect16_2[0];
		    src32_2 = (void*) vect32_2[0];
		#endif
	#endif
	//if (hartID == 0) {printf("src2 = %x, %x, %lx\n",  vect8_2[0], vect16_2[0], vect32_2[0]);}

	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/



	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/
	/************************************* 8-bit VCU Start ************************************/

	// ENABLE COUNTING 
	mcycle_start();

	CSR_MVTYPE(0);  // set data type to 8-bit

	// TEST KVCU8
	#if TEST == 1 // SW loop
		VCU_FUNC_NAME((void*) result8, src8_1, src8_2, size8, 8);
	#endif
	#if TEST == 2 // HW loop
		VCU_FUNC_NAME((void*) result8, src8_1,  src8_2, size8);
	#endif

	// DISABLE COUNTING AND SAVE MCYCLE OF EACH THREAD 
	mcycle_stop(dsp_perf8, dsp_ptr_perf8);
	perf8[hartID]=dsp_perf8;
	

	// Check 8-bit result 
	vect_e vect = type_8b;
	if (hartID == 0) check_kvcu((void*)testres8, vect8_1, vect8_2, vect);

	
	/************************************* 8-bit VCU END **************************************/
	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/




	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/
	/************************************ 16-bit VCU Start ************************************/
	
	// ENABLE COUNTING 
	mcycle_start();

	CSR_MVTYPE(1);  // set data type to 16-bit

	// TEST KVCU16 
	#if TEST == 1 // SW loop
		VCU_FUNC_NAME((void*) result16, src16_1, src16_2, size16, 8);
	#endif
	#if TEST == 2 // HW loop
		VCU_FUNC_NAME((void*) result16, src16_1, src16_2, size16);
	#endif

	// DISABLE COUNTING AND SAVE MCYCLE OF EACH THREAD ////////
	mcycle_stop(dsp_perf16, dsp_ptr_perf16);
	perf16[hartID]=dsp_perf16;
	
	// Check 16-bit result
	vect = type_16b;
	if (hartID == 0) check_kvcu((void*)testres16, vect16_1, vect16_2, vect);

	/************************************* 16-bit VCU END *************************************/		
	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/



	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/
	/************************************* 32-bit VCU Start ***********************************/
	
	// ENABLE COUNTING
	mcycle_start();

	CSR_MVTYPE(2);  // set data type to 32-bit
	
	// TEST KVCU32 	
	#if TEST == 1 // SW loop
		VCU_FUNC_NAME((void*) result32, src32_1, src32_2, size32, 8);
	#endif
	#if TEST == 2 // HW loop
		VCU_FUNC_NAME((void*) result32, src32_1, src32_2, size32);
	#endif
	
	// DISABLE COUNTING AND SAVE MCYCLE OF EACH THREAD 
	mcycle_stop(dsp_perf32, dsp_ptr_perf32);
	perf32[hartID]=dsp_perf32;
	
	// Check 32-bit  result
	vect = type_32b;
	if (hartID == 0) check_kvcu((void*)testres32, vect32_1, vect32_2, vect);
	
	/************************************* 32-bit VCU END *************************************/
	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/


	/******************************************************************************************/
	/*********************************** Print Cycle Counts ***********************************/
	/******************************************************************************************/

	if (hartID == 0)
	{
		printf("\n\nNumber of Vector Elements: %d\n\n",VSIZE);
		for(int i=0; i<THREAD_POOL_SIZE;i++)
		{
			if (i>0) {
				 perf8[0] += perf8[i];
				perf16[0] += perf16[i];
				perf32[0] += perf32[i];
			}
		}
		printf("Non-accelerated 08-bit " VCU_INSTR_TYPE " speed: %d Cycles\n", testperf[0]);
		printf("Non-accelerated 16-bit " VCU_INSTR_TYPE " speed: %d Cycles\n", testperf[1]);
		printf("Non-accelerated 32-bit " VCU_INSTR_TYPE " speed: %d Cycles\n", testperf[2]);
		puts("");
		printf(VCU_INSTR "08-bit " SW_LOOP "speed: %d Cycles\n",    perf8[0]/3);
		printf(VCU_INSTR "16-bit " SW_LOOP "speed: %d Cycles\n",   perf16[0]/3);
		printf(VCU_INSTR "32-bit " SW_LOOP "speed: %d Cycles\n\n", perf32[0]/3);
	}

	/******************************************************************************************/
	/******************************************************************************************/
	/******************************************************************************************/

	sync_barrier();
	return 0;
}

// Declare your function pointer type
typedef int8_t  (*operation8_t)(int8_t, int8_t);
typedef int16_t (*operation16_t)(int16_t, int16_t);
typedef int32_t (*operation32_t)(int32_t, int32_t);

// Function Definitions
// "k_" prefix is added to avoid conflicts with similar function names from the compiler builtin libraries
int8_t nop(int8_t a, int8_t b)        { return 0; }
int8_t k_add(int8_t a, int8_t b)      { return a + b; }
int8_t k_sub(int8_t a, int8_t b)      { return a - b; }
int8_t k_mul(int8_t a, int8_t b)      { return a * b; }
int8_t k_div(int8_t a, int8_t b)      { return a / b; }
int8_t k_rem(int8_t a, int8_t b)      { return a % b; }
int8_t k_slt(int8_t a, int8_t b)      { if(a < b) return 1; else return 0; }
int8_t k_relu(int8_t a, int8_t b)     { if(a < 0) return 0; else return a; }
int8_t k_srl(int8_t a, int8_t b)      { return (uint8_t) a >> (uint8_t) b; }
int8_t k_sra(int8_t a, int8_t b)      { return a >> b; }
int8_t k_svaddsc(int8_t a, int8_t b)  { return k_add(a, b); }
int8_t k_svaddrf(int8_t a, int8_t b)  { return k_add(a, b); }
int8_t k_svmulsc(int8_t a, int8_t b)  { return k_mul(a, b); }
int8_t k_svmulrf(int8_t a, int8_t b)  { return k_mul(a, b); }
int8_t k_svdivsc(int8_t a, int8_t b)  { return k_div(a, b); }
int8_t k_svdivrf(int8_t a, int8_t b)  { return k_div(a, b); }
int8_t k_svremsc(int8_t a, int8_t b)  { return k_rem(a, b); }
int8_t k_svremrf(int8_t a, int8_t b)  { return k_rem(a, b); }
int8_t k_svslt(int8_t a, int8_t b)    { return k_slt(a, b); }
int8_t k_red(int8_t a, int8_t b)      { static int accum = 0; return accum += a; }
int8_t k_mulacc(int8_t a, int8_t b)   { static int accum = 0; return accum += a*b; }
//int8_t k_mulaccps(int8_t a, int8_t b) { static int accum = 0; accum += a*b; return accum; }
int8_t k_bcast(int8_t a, int8_t b)    { return a; }


int16_t nop(int16_t a, int16_t b)        { return 0; }
int16_t k_add(int16_t a, int16_t b)      { return a + b; }
int16_t k_sub(int16_t a, int16_t b)      { return a - b; }
int16_t k_mul(int16_t a, int16_t b)      { return a * b; }
int16_t k_div(int16_t a, int16_t b)      { return a / b; }
int16_t k_rem(int16_t a, int16_t b)      { return a % b; }
int16_t k_slt(int16_t a, int16_t b)      { if(a < b) return 1; else return 0; }
int16_t k_relu(int16_t a, int16_t b)     { if(a < 0) return 0; else return a; }
int16_t k_srl(int16_t a, int16_t b)      { return (uint16_t) a >> (uint16_t) b; }
int16_t k_sra(int16_t a, int16_t b)      { return a >> b; }
int16_t k_svaddsc(int16_t a, int16_t b)  { return k_add(a, b); }
int16_t k_svaddrf(int16_t a, int16_t b)  { return k_add(a, b); }
int16_t k_svmulsc(int16_t a, int16_t b)  { return k_mul(a, b); }
int16_t k_svmulrf(int16_t a, int16_t b)  { return k_mul(a, b); }
int16_t k_svdivsc(int16_t a, int16_t b)  { return k_div(a, b); }
int16_t k_svdivrf(int16_t a, int16_t b)  { return k_div(a, b); }
int16_t k_svremsc(int16_t a, int16_t b)  { return k_rem(a, b); }
int16_t k_svremrf(int16_t a, int16_t b)  { return k_rem(a, b); }
int16_t k_svslt(int16_t a, int16_t b)    { return k_slt(a, b); }
int16_t k_red(int16_t a, int16_t b)      { static int accum = 0; return accum += a; }
int16_t k_mulacc(int16_t a, int16_t b)   { static int accum = 0; return accum += a*b; }
//int16_t k_mulaccps(int16_t a, int16_t b) { static int accum = 0; accum += a*b; return accum; }
int16_t k_bcast(int16_t a, int16_t b)  	 { return a; }


int32_t nop(int32_t a, int32_t b)        { return 0; }
int32_t k_add(int32_t a, int32_t b)      { return a + b; }
int32_t k_sub(int32_t a, int32_t b)      { return a - b; }
int32_t k_mul(int32_t a, int32_t b)      { return a * b; }
int32_t k_div(int32_t a, int32_t b)      { return a / b; }
int32_t k_rem(int32_t a, int32_t b)      { return a % b; }
int32_t k_slt(int32_t a, int32_t b)      { if(a < b) return 1; else return 0; }
int32_t k_relu(int32_t a, int32_t b)     { if(a < 0) return 0; else return a; }
int32_t k_srl(int32_t a, int32_t b)      { return (uint32_t) a >> (uint32_t) b; }
int32_t k_sra(int32_t a, int32_t b)      { return a >> b; }
int32_t k_svaddsc(int32_t a, int32_t b)  { return k_add(a, b); }
int32_t k_svaddrf(int32_t a, int32_t b)  { return k_add(a, b); }
int32_t k_svmulsc(int32_t a, int32_t b)  { return k_mul(a, b); }
int32_t k_svmulrf(int32_t a, int32_t b)  { return k_mul(a, b); }
int32_t k_svdivsc(int32_t a, int32_t b)  { return k_div(a, b); }
int32_t k_svdivrf(int32_t a, int32_t b)  { return k_div(a, b); }
int32_t k_svremsc(int32_t a, int32_t b)  { return k_rem(a, b); }
int32_t k_svremrf(int32_t a, int32_t b)  { return k_rem(a, b); }
int32_t k_svslt(int32_t a, int32_t b)    { return k_slt(a, b); }
int32_t k_red(int32_t a, int32_t b)      { static int accum = 0; return accum += a; }
int32_t k_mulacc(int32_t a, int32_t b)   { static int accum = 0; return accum += a*b; }
//int32_t k_mulaccps(int32_t a, int32_t b) { static int accum = 0; accum += a*b; return accum; }
int32_t k_bcast(int32_t a, int32_t b)    { return a; }


// Create an array of function pointers
operation8_t  ops8[]  = {nop, k_add, k_sub, k_mul, k_div, k_rem, k_slt, k_relu, k_srl, k_sra,
                         k_svaddsc, k_svaddrf, k_svmulsc, k_svmulrf, k_svdivsc, k_svdivrf, k_svremsc, k_svremrf,
                         k_svslt, k_red, k_mulacc, nop, k_bcast};
operation16_t ops16[] = {nop, k_add, k_sub, k_mul, k_div, k_rem, k_slt, k_relu, k_srl, k_sra,
                         k_svaddsc, k_svaddrf, k_svmulsc, k_svmulrf, k_svdivsc, k_svdivrf, k_svremsc, k_svremrf,
                         k_svslt, k_red, k_mulacc, nop, k_bcast};
operation32_t ops32[] = {nop, k_add, k_sub, k_mul, k_div, k_rem, k_slt, k_relu, k_srl, k_sra,
                         k_svaddsc, k_svaddrf, k_svmulsc, k_svmulrf, k_svdivsc, k_svdivrf, k_svremsc, k_svremrf,
                         k_svslt, k_red, k_mulacc, nop, k_bcast};

//#pragma GCC push_options
//#pragma GCC optimize ("O0")

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

void check_kvcu(void* testres, void* vect1, void* vect2, vect_e vect)
{
	const char* type_width;
	int  idx = 0;
	int  test_perf_i = 0;
	int* test_ptr_perf  = &test_perf_i;
	int  pass = 0;
	int  hartID = Klessydra_get_coreID();

	if (vect == type_8b)
	{
	    type_width = "08";
		idx = 0;
	}
	else if (vect == type_16b) {
	    type_width = "16";
		idx = 1;
	}
	else {
	    type_width = "32";
		idx = 2;
	}

	int zero = 0;

	int i = 0;
	int *j, *k;

	mcycle_start();

	// perform the vector operation with the scalar core
	for (i=0; i<VSIZE; i++)
	{
		#if VCU_TEST <= 7 // vector-vector, output vector
			j = &i; k = &i;
		#elif VCU_TEST >= 8 && VCU_TEST <= 18 // vector-scalar, output vector
			j = &zero; k = &i;
		#elif VCU_TEST == 19 || VCU_TEST == 20 || VCU_TEST == 21// vector-vector, output scalar
			j = &i; k = &zero;			
		#endif

		switch(vect)
		{
			case type_8b:
			{
			    ((int8_t*)testres)[*k] = ops8[VCU_TEST](((int8_t*)vect1)[i], ((int8_t*)vect2)[*j]);
				break;
			}
			case type_16b:
			{
			    ((int16_t*)testres)[*k] = ops16[VCU_TEST](((int16_t*)vect1)[i], ((int16_t*)vect2)[*j]);
				break;
			}
			case type_32b:
			{
			    ((int32_t*)testres)[*k] = ops32[VCU_TEST](((int32_t*)vect1)[i], ((int32_t*)vect2)[*j]);
				break;
			}
		}
	}

	mcycle_stop(test_perf_i, test_ptr_perf);

	// Count the passed results
	for (i=0; i<VSIZE; i++)
	{
		#if DEBUG == 1
		if (vect == type_8b)  { printf("\nv1[%d] = 0x%x,  v2[%d] = 0x%x,  res8[%d]  = 0x%x,  t_res = 0x%x",  i, ((int8_t*) vect1)[i], *j, ((int8_t*) vect2)[*j], *k, result8[*k],  ((int8_t*)testres)[*k]);  }
		if (vect == type_16b) { printf("\nv1[%d] = 0x%x,  v2[%d] = 0x%x,  res16[%d] = 0x%x,  t_res = 0x%x",  i, ((int16_t*)vect1)[i], *j, ((int16_t*)vect2)[*j], *k, result16[*k], ((int16_t*)testres)[*k]); }
		if (vect == type_32b) { printf("\nv1[%d] = 0x%x,  v2[%d] = 0x%x,  res32[%d] = 0x%x,  t_res = 0x%x",  i, ((int32_t*)vect1)[i], *j, ((int32_t*)vect2)[*j], *k, result32[*k], ((int32_t*)testres)[*k]); }
		#endif
		if (((( result8[*k] == (( int8_t*)testres)[*k]) && (vect == type_8b ))  ||
		     ((result16[*k] == ((int16_t*)testres)[*k]) && (vect == type_16b))) ||
		     ((result32[*k] == ((int32_t*)testres)[*k]) && (vect == type_32b)))
		{
			pass++;
		}
	}

	testperf[idx]=test_perf_i;

	if (pass==VSIZE)
		printf("\nPASSED %s-bit " VCU_INSTR SW_LOOP VCU_INSTR_TYPE, type_width);
	else 
		printf("\nFAILED %s-bit " VCU_INSTR SW_LOOP VCU_INSTR_TYPE, type_width);
}

#pragma GCC diagnostic pop
//#pragma GCC pop_options


inline void mcycle_start() {
	__asm__("csrrw zero, mcycle, zero;"
			"csrrw zero, 0x7A0, 0x00000001"
	);
}

inline void mcycle_stop(int dsp_perf, int* dsp_ptr_perf) {
	__asm__("csrrw zero, 0x7A0, 0x00000000;"
	    	"csrrw %[dsp_perf], mcycle, zero;"
			"sw %[dsp_perf], 0(%[dsp_ptr_perf]);"
			:
			:[dsp_perf] "r" (dsp_perf), [dsp_ptr_perf] "r" (dsp_ptr_perf)
	);
}
