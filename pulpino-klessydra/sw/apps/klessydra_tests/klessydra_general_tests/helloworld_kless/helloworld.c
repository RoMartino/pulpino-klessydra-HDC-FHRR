#include <stdio.h>
#include <functions.h>

int main()
{
    Klessydra_En_Int(); // enable irqs

    sync_barrier_thread_registration();
    if (Klessydra_get_coreID() == 0) {
      printf("Hello World!!!!!\n");
    }
    sync_barrier();
    return 0;
}
