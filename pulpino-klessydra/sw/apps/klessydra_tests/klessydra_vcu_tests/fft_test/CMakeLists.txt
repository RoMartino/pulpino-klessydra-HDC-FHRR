set(fft_SOURCES fft_accl.c)
add_application(fft_accl "${fft_SOURCES}" LABELS "klessydra_tests")
  
###LABELS "perfbench" LIBS "perfbench.core")
