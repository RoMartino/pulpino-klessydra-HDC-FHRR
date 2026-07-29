#include <stdio.h>
#include "functions.h"

int arr_hart_ID[THREAD_POOL_SIZE] = {};

int main(){

	Klessydra_En_Int(); // each thread enables it's own interrupt
	int hart_ID = Klessydra_get_coreID();

	sync_barrier_thread_registration();
	if (hart_ID == 0) {
		puts("Barrier Test\n");
	}

	arr_hart_ID[hart_ID] = 1;
	if (hart_ID == 0)
	{
		for (int i=0;i<THREAD_POOL_SIZE;i++)
		{
			if (arr_hart_ID[i] == 1)
			{
				// Register Threads to be synchronized
				printf("Registered Thread %d\n", i);
			}
		}
	}
	// Register Threads
	if (hart_ID == 0) {
		puts("\nSynchronizing\n");
	}
	// Synchronise Threads
	sync_barrier();

	sync_barrier_thread_registration();
	if (hart_ID == 0) {
		puts("Sync Success !!\n");
		puts("TEST PASS\n");
	}
	sync_barrier(); 
	return 0;

}
