#include <stdio.h>
#include "functions.h"

int lock = 0;
int *plock_1;
int lock_1 = 0;

int harts_skipping = 0;

int main()
{
	Klessydra_En_Int(); // each thread enables it's own interrupt

	sync_barrier_thread_registration();

	klessydra_lock_acquire(&lock_1);
	int lock_val = klessydra_lock_set(&lock); // the first hart gets the key value "0", other harts will get a fake key
	klessydra_lock_release(&lock_1);

	if (lock_val != 0) {
		harts_skipping++;
	}
	sync_barrier();

	sync_barrier_thread_registration();
	// the following if condition will be executed exclusively by one hart only
	if (lock_val == 0) {
		printf("Procedure being handled by hart %d, thread_skip_count = %d\n", Klessydra_get_coreID(), harts_skipping);
		puts("\nTEST_PASS");
	}
	sync_barrier();

	return 0;
}
