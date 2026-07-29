#include"functions.h"

int klessydra_lock_set(int *lock)
{
	int temp = 1;	
	int *p_temp = &temp;

	__asm__(
		"amoswap.w.aq %[temp], %[temp], (%[lock]);" // fetch the value that will be used to make a region lock
		"sw %[temp], 0(%[p_temp])"
		://no output register
		: [lock] "r" (lock), [temp] "r" (temp), [p_temp] "r" (p_temp)
		: "memory"
	);
	return temp;
}
