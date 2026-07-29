#include <stdio.h>
#include <functions.h>

int final_perf[3][9] = {{777}};

int main()
{
    int* ptr_perf; // local variable to allow each thread to have its own copy of the performance count

    Klessydra_En_Int(); // enable irqs
    Klessydra_perf_cnt_start();

    sync_barrier_thread_registration();
    if (Klessydra_get_coreID() == 0) {
        printf("Started performance counting:\n\n");
        printf("Executing basic mathematical routines:\n");

        // Basic mathematical operations
        int a = 5;
        int b = 3;
        int sum = a + b;
        int product = a * b;
    
        printf("\tSum of %d and %d is %d\n", a, b, sum);
        printf("\tProduct of %d and %d is %d\n", a, b, product);
    
        // More complex operation: Sum of elements in an array
        int array[] = {1, 2, 3, 4, 5};
        int arraySum = 0;
        for (int i = 0; i < sizeof(array) / sizeof(array[0]); i++) {
            arraySum += array[i];
        }
        printf("\tSum of array elements is %d\n\n", arraySum);
    }
    ptr_perf = Klessydra_perf_cnt_finish();
    sync_barrier();

    for (int i=0; i<3; i++) {
      if (Klessydra_get_coreID() == i) {
        for (int j=0; j<9; j++) {
          final_perf[i][j]=ptr_perf[j];
        }
      }
    }

    if (Klessydra_get_coreID() == 0) {
      for (int i=0; i<3; i++) {
        printf(" Cycle Count = %d \n Instruction Count = %d \n Instruction wait = %d \n Load Count = %d \n Store Count = %d \n Unconditional Jump Count = %d \n Branch Count = %d \n Branch Taken Count = %d \n Branch Hit Count = %d \n Branch Hit Rate = %d%% \n\n", 
                final_perf[i][0], final_perf[i][1], final_perf[i][2], final_perf[i][3], final_perf[i][4], final_perf[i][5], final_perf[i][6], final_perf[i][7], final_perf[i][8], ((final_perf[i][8]*100)/final_perf[i][6]));
      }
    }
    sync_barrier();

    return 0;
}
