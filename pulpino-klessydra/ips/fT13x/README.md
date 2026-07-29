<img src="/pics/Klessydra_Logo.png" width="400">

# KLESSYDRA-fT13 PROCESSOR

Intro: The Klessydra processing core family is a set of processors featuring full compliance with RISC-V, and pin-to-pin compatible with the PULPino Riscy cores. Klessydra-fT13 is a bare-metal 32-bit temporally and spatial redundant fault tolerant processor fully supporting the RV32IM from the RISC-V ISA, born from Klessydra-T13 microarchitecture. 

Architecture:

The redundant Architecture depicted in the following figure, exploits the architecture of the IMT processor to obtain redundancy between the three hardware threads, combining both spatial and temporal redundancy obtained thanks to the Buffered TMR technique and its own applications. Klessydra-fT13 has three threads, each having their own set of signals thus incorporating spatial redundancy, with threads that follow the same instruction in an interleaved fashion, providing temporal redundancy. At the end of the execute stage of each thread, the writeback results from the corresponding thread are buffered, and then voted and written back to the Register File (RF) only after the last thread execution, in order to avoid a Write Back (WB) of an unvoted result that might have a fault. By following this idea, it is possible to use a single voted TMR RF in the WB stage triggered during the WB of the last thread. Additionally, there is some pipeline interlocking and RF bypass logic to directly pass the operands among threads avoiding long stalls as shown in Figure 2.

<p align="center">
<img src="/pics/Klessydra-fT13_microarchitecture.png" width="800">
</p> 

Figure 1:
Registerfile and Writeback stage modifications:

<p align="center">
<img src="/pics/Klessydra-fT13_regfile_wb_modifications.png" width="700">
</p> 
Figure 2:
The fT13 furhter supports the vector accelerator present in the T13 and S1.

PROCEDURE:

To use Klessydra-fT13, please download [PULPino-Klessydra](https://github.com/RoMartino/pulpino-klessydra-HDC-FHRR) , and follow the guide over there. 

- For more details about the Klessydra processing cores, please refer to the technincal manual in Docs
- For more details about the Klessydra runtime libraries, please refer to the software runtime manual in Docs

Hope you like it :D



