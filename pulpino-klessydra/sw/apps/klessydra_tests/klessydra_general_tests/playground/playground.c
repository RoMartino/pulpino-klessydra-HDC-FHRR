// NOTE: This test is like a sketchpad where we try to compile and run random pieces of code

#include <stdio.h>
#include <functions.h>


int main()
{
    Klessydra_En_Int(); // enable irqs
    float x = 1.0;
    float y = 2.0;
    float z1, z2, z3, z4;
    asm(
     "flw  f1, 0(%[p_x]);"
     "flw  f2, 0(%[p_y]);"
     "fadd.s f3, f1, f2;"
     "fsub.s f4, f1, f2;"
     "fmul.s f5, f1, f2;"
     "fdiv.s f6, f1, f2;"
     "fsw  f3, 0(%[p_z1]);"
     "fsw  f4, 0(%[p_z2]);"
     "fsw  f5, 0(%[p_z3]);"
     "fsw  f6, 0(%[p_z4]);"
     :
     : [p_x] "r" (&x), [p_y] "r" (&y),
       [p_z1] "r" (&z1), [p_z2] "r" (&z2),
       [p_z3] "r" (&z3), [p_z4] "r" (&z4)
    );

    sync_barrier_thread_registration();
    if (Klessydra_get_coreID() == 0) {
      printf("%d+%d=%d\n", (int)x, (int)y, (int)z1);
    }
    sync_barrier();


    //float z;
    //z = x + y;
    //sync_barrier_thread_registration();
    //if (Klessydra_get_coreID() == 0) {
    //  printf("%f+%f=%f\n", x, y, z);
    //}
    //sync_barrier();

    //Klessydra_En_Int(); // enable irqs

    //sync_barrier_thread_registration();
    //if (Klessydra_get_coreID() == 0) {
    //  printf("Hello World!!!!!\n");
    //}
    //sync_barrier();
    return 0;
}
