#include <stdlib.h>
#include"dsp_functions.h"

void kaddv_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];" // copy vector 1
		"	kmemld %[SPMADDRB], %[srcB], %[sz];" // copy vector 2
		"	csrw 0xBF0, %[sz]; "
		"	kaddv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void ksubv_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];" // copy vector 1
		"	kmemld %[SPMADDRB], %[srcB], %[sz];" // copy vector 2
		"	csrw 0xBF0, %[sz]; "
		"	ksubv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}


void kvmul_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kvmul %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvdiv_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kvdiv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvrem_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kvrem %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvslt_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kvslt %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void krelu_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	krelu %[SPMADDRB], %[SPMADDRA];"
		"	kmemstr %[result], %[SPMADDRB], %[sz];"
		:
		:[sz] "r" (size),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [result] "r" (result)
	);
}

void ksrlv_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksrlv %[SPMADDRC], %[SPMADDRA], %[srcB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1), [sz] "r" (size),
		 [srcB]     "r" (src2),     [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
		:
	);
}

void ksrav_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksrav %[SPMADDRC], %[SPMADDRA], %[srcB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),     [sz]     "r" (size),
		 [srcB]     "r" (src2), [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
		:
	);
}

void ksvaddsc_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sc_sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksvaddsc %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [SPMADDRB] "r" (SPMADDRB),
		 [srcA]     "r" (src1),     [sz]       "r" (size),     [sc_sz]  "r" (scalar_size),
		 [srcB]     "r" (src2),     [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
		:
	);
}

void ksvaddrf_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld 		%[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksvaddrf    %[SPMADDRC], %[SPMADDRA], %[srcB];"
		"	kmemstr     %[result], %[SPMADDRC], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1), [sz] "r" (size),
		 [srcB]     "r" (src2),     [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
		:
	);
}

void ksvmulsc_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sc_sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksvmulsc %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [SPMADDRB] "r" (SPMADDRB),
		 [srcA]     "r" (src1),     [sz]       "r" (size),     [sc_sz]  "r" (scalar_size),
		 [srcB]     "r" (src2),     [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
		:
	);
}

void ksvmulrf_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld 		%[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksvmulrf    %[SPMADDRC], %[SPMADDRA], %[srcB];"
		"	kmemstr     %[result], %[SPMADDRC], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1), [sz] "r" (size),
		 [srcB]     "r" (src2),     [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
		:
	);
}

void ksvslt_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld 		%[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz];"
		"	ksvslt      %[SPMADDRB], %[SPMADDRA], %[srcB];"
		"	kmemstr     %[result], %[SPMADDRB], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1), [sz] "r" (size),
		 [srcB]     "r" (src2),     [SPMADDRB] "r" (SPMADDRB), [result] "r" (result)
		:
	);
}


void kvred_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kvred %[SPMADDRB], %[SPMADDRA];"
		"	kmemstr %[result], %[SPMADDRB], %[sc_sz];" // if the parameter size was zero, then kmemstr will store nonsense value in th the memory
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [sz]       "r" (size), [sc_sz]    "r" (scalar_size),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [result]   "r" (result)
	);
}

void kdotp_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kdotp %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sc_sz];" // if the parameter size was zero, then kmemstr will store nonsense value in th the memory
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [sz] "r" (size), [sc_sz] "r" (scalar_size),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}


void* kdotpps_complete(void *result, void* src1, void* src2, void* p_scal, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	csrw 0xBE0, %[p_scal]; "
		"	kdotpps %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	kmemstr %[result], %[SPMADDRC], %[sc_sz];"
		:
		:
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [sz] "r" (size), [p_scal] "r" (p_scal), [sc_sz] "r" (scalar_size),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
	return result;
}

void* kdotpps_complete_emul(void *result, void* src1, void* src2, void* p_scal, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[sz];"
		"	kmemld %[SPMADDRB], %[srcB], %[sz];"
		"	csrw 0xBF0, %[sz]; "
		"	kvmul %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
		"	ksrav %[SPMADDRC], %[SPMADDRC], %[p_scal];"
		"	kvred %[SPMADDRC], %[SPMADDRC];"
		"	kmemstr %[result], %[SPMADDRC], %[sc_sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [sz] "r" (size), [p_scal] "r" (p_scal), [sc_sz] "r" (scalar_size),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
	return result;
}


void kbcast_complete(void *result, void* src1, void* src2, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	char scalar_size = 4;
	asm volatile(
		"	csrw  0xBF0, %[sz]; "
		"	kbcast  %[SPMADDRA], %[srcA];"
		"	kmemstr %[result], %[SPMADDRA], %[sz];"
		// if the parameter size was zero, then kmemstr will store nonsense value in th the memory
		:
		:[SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [sz]       "r" (size),   [result] "r" (result)
	);
}

void* kvcp_complete(void *result, void* src, int size)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int SPMADDRD = spmaddrD;
	asm volatile(
		"	kmemld %[SPMADDRA], %[src], %[sz];"
		"	csrw 0xBF0, %[sz];"
		"	kvcp %[SPMADDRB], %[SPMADDRA];"
		"	kmemstr %[result], %[SPMADDRB], %[sz];"
		:
		:[SPMADDRA] "r" (SPMADDRA), [src] "r" (src), [sz] "r" (size),
         [SPMADDRB] "r" (SPMADDRB), [result] "r" (result)
		:
	);
	return result;
}


void kaddv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	kaddv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	kaddv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}


void ksubv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksubv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksubv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvmul_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	kvmul %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	kvmul %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}


void kvdiv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	kvdiv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	kvdiv %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvrem_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	kvrem %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	kvrem %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvslt_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	kvslt %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	kvslt %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void kvrelu_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	krelu %[SPMADDRB], %[SPMADDRA];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	krelu %[SPMADDRB], %[SPMADDRA];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRB=spmaddrB;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRB], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRB]  "r" (SPMADDRB),  [result]     "r" (result)
	);
}

void ksrlv_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksrlv %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksrlv %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void ksrav_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksrav %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksrav %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size] "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		: [size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
          [SPMADDRC]  "r" (SPMADDRC), [result]      "r" (result)
	);
}

void ksvaddsc_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[sc_sz];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		: [size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		  [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		  [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
		  [sc_sz]    "r" (scalar_size)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksvaddsc %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksvaddsc %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size]     "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		: [size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
          [SPMADDRC]  "r" (SPMADDRC),  [result]     "r" (result)
	);
}

void ksvaddrf_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksvaddrf %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksvaddrf %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size]     "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void ksvmulsc_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	char scalar_size = 4;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	kmemld %[SPMADDRB], %[srcB], %[sc_sz];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		: [size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		  [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		  [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2),
		  [sc_sz]    "r" (scalar_size)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksvmulsc %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [SPMADDRB] "r" (SPMADDRB),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksvmulsc %[SPMADDRC], %[SPMADDRA], %[SPMADDRB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [SPMADDRB] "r" (SPMADDRB) ,
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size]     "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		: [size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
          [SPMADDRC]  "r" (SPMADDRC),  [result]     "r" (result)
	);
}

void ksvmulrf_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksvmulrf %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksvmulrf %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size]     "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}

void ksvslt_sw_loop_complete(void *result, void* src1, void* src2, int size, int SIMD_BYTES)
{
	int SPMADDRA = spmaddrA;
	int SPMADDRB = spmaddrB;
	int SPMADDRC = spmaddrC;
	int size_temp = size;
	asm volatile(
		"	kmemld %[SPMADDRA], %[srcA], %[size_temp];"
		"	csrw 0xBF0, %[SIMD_BYTES];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
		 [SPMADDRA] "r" (SPMADDRA), [srcA] "r" (src1),
		 [SPMADDRB] "r" (SPMADDRB), [srcB] "r" (src2)
	);
	for (int i=0; i<size_temp; i=i+SIMD_BYTES)
	{
		if (size >= SIMD_BYTES)
		{
			size = size-SIMD_BYTES;
			asm volatile(
				"	ksvslt %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA),
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC)
			);
			SPMADDRA+=SIMD_BYTES;
			SPMADDRB+=SIMD_BYTES;
			SPMADDRC+=SIMD_BYTES;
		}
		else
		{
			asm volatile(
				"	csrw 0xBF0, %[size];"
				"	ksvslt %[SPMADDRC], %[SPMADDRA], %[srcB];"
				:
				:
				 [SPMADDRA] "r" (SPMADDRA) ,
				 [srcB]     "r" (src2),
   		    	 [SPMADDRC] "r" (SPMADDRC) ,
   		    	 [size]     "r" (size)
			);
		}
	}
	SPMADDRC=spmaddrC;
	asm volatile(
		"	kmemstr %[result], %[SPMADDRC], %[size_temp];"
		:
		:[size_temp] "r" (size_temp), [SIMD_BYTES] "r" (SIMD_BYTES),
         [SPMADDRC] "r" (SPMADDRC), [result] "r" (result)
	);
}