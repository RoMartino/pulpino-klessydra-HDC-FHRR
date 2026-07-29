//	███████╗██╗██╗░░██╗███████╗██████╗░░░██████╗░░█████╗░██╗███╗░░██╗████████╗
//	██╔════╝██║╚██╗██╔╝██╔════╝██╔══██╗░░██╔══██╗██╔══██╗██║████╗░██║╚══██╔══╝
//	█████╗░░██║░╚███╔╝░█████╗░░██║░░██║░░██████╔╝██║░░██║██║██╔██╗██║░░░██║░░░
//	██╔══╝░░██║░██╔██╗░██╔══╝░░██║░░██║░░██╔═══╝░██║░░██║██║██║╚████║░░░██║░░░
//	██║░░░░░██║██╔╝╚██╗███████╗██████╔╝░░██║░░░░░╚█████╔╝██║██║░╚███║░░░██║░░░
//	╚═╝░░░░░╚═╝╚═╝░░╚═╝╚══════╝╚═════╝░░░╚═╝░░░░░░╚════╝░╚═╝╚═╝░░╚══╝░░░╚═╝░░░
//
//	██╗░░░░░██╗██████╗░██████╗░░█████╗░██████╗░██╗░░░██╗
//	██║░░░░░██║██╔══██╗██╔══██╗██╔══██╗██╔══██╗╚██╗░██╔╝
//	██║░░░░░██║██████╦╝██████╔╝███████║██████╔╝░╚████╔╝░
//	██║░░░░░██║██╔══██╗██╔══██╗██╔══██║██╔══██╗░░╚██╔╝░░
//	███████╗██║██████╦╝██║░░██║██║░░██║██║░░██║░░░██║░░░
//	╚══════╝╚═╝╚═════╝░╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░╚═╝░░░╚═╝░░░
/*-------------------------------------------------------------------------------------------------------
Script:      fixed_point.hpp
Description: Fixed point type definition. Fixed Point arithmetic and other debug functions
Data:        17/05/2022
Version:     1.0
-------------------------------------------------------------------------------------------------------*/
#ifndef _FIXED_POINT_HPP
#define _FIXED_POINT_HPP

// Library import:
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
extern "C" {								// Klessydra dsp_libraries are written in C and so they're imported as extern:
#include "dsp_functions.h"
#include "functions.h"
}

// Bits used for fractionary part in fixed point rapresentation
#define SHIFT_AMOUNT 15								

// --------------------------- Fixed point type, class declaration: --------------------------- 
// Fixed point class definition:
class fixed_point
{
	public:
		int num;									//Integer value

	public:
		
		//------------------ Constructors: ------------------
		fixed_point()
		{
			num=0;
		}

		// Float constructor:
		fixed_point(float value)
		{
			num = (int)(round(value * (1 << SHIFT_AMOUNT)));
		}

		// Integer constructor:
  	fixed_point(int value)
	  {
			num = value;
		}


		//------------------ Arithmetic operators: ------------------

		// 1) Assign operator: Float to fixed_point
		void operator=(float f_num)
		{
			num = (int)(round(f_num * (1 << SHIFT_AMOUNT)));
		}

	
		// 2) Sum operation between two fixed_point numbers
		fixed_point operator+(fixed_point value)
		{
			fixed_point result;
			result.num = num + value.num;
		  return result;
		}

		// 3) Sub operation between two fixed_point numbers:
		fixed_point operator-(fixed_point value)
		{
			fixed_point result;
			result.num = num - value.num;
		  return result;
		}


		// 4) Multiplication operation:
		fixed_point operator*(fixed_point value)
		{
			fixed_point result;
			result.num = ((long long)num*(long long)value.num)/(1 << SHIFT_AMOUNT);	// Multiplication with long long casting. Operands are casted to long, multiplied and shifted back
			return result;																													// (Long Long is 8 byte in a 32 bit architecture)
		}

		// 5) Division operation:
		fixed_point operator/(fixed_point value)
		{
			fixed_point result;
			result.num = ((long long)num << SHIFT_AMOUNT)/value.num; 				// Division with long long casting. 
			return result;																									// Long Long is 8 byte in a 32 bit architecture
		}


		// ------------------ Other functions: ------------------
		// Fixed point to float:
		float to_float()
		{
    		return ((float)num/(float)(1 << SHIFT_AMOUNT));
		}
};




// --------------------------- Performance counting: --------------------------- 
// Count functions:
inline void start_count();
inline int finish_count();
inline int   perf=0;
inline int*  ptr_perf = &perf;

inline void start_count(){
  int enable_perf_cnt=0;
  __asm__("csrrw zero, mcycle, zero;"    	// reset cycle count
          "li %[enable], 0x000003E7;"    		// enable performance counters
          "csrrw zero, 0x7A0, %[enable]" 		// enable performance counters
          :
          :[enable] "r" (enable_perf_cnt)
          );
}

inline int finish_count(){
  __asm__("csrrw zero, 0x7A0, 0x00000000");  // disable performance counters
  __asm__("csrrw %[perf], mcycle, zero;"
          "sw %[perf], 0(%[ptr_perf]);"
          :
          :[perf] "r" (perf),   [ptr_perf] "r" (ptr_perf)
          );
 return perf;
}

#endif