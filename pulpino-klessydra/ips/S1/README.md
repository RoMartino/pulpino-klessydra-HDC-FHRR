<img src="/pics/Klessydra_Logo.png" width="400">

# KLESSYDRA-S1 IN-ORDER EXECUTION PROCESSOR

Intro: The Klessydra processing core family is a set of processors featuring full compliance with RISC-V, and pin-to-pin compatible with the PULPino Riscy cores. Klessydra-S1 is a bare-metal 32-bit processor fully supporting the RV32IM from the RISC-V ISA, and one instruction from the Atomic "A" extension. 'S1' further extends the instruction set with a set of custom vector instructions.

Architecture: S1 as its S0 predecessor vesions is also a single hart processor, it supports superscalar execution between the units in the execute stage, it implements basic 1-bit branch prediction, and data-dependency checking logic, as well as registerfile bypass logics. It also introduces a custom vector coprocessor, to accelerate the executuon of the vector arithmetic operations.

PROCEDURE:

To use Klessydra-S1, please download [PULPino-Klessydra](https://github.com/RoMartino/pulpino-klessydra-HDC-FHRR) , and follow the guide over there. 

- For more details about the Klessydra processing cores, please refer to the technincal manual in Docs
- For more details about the Klessydra runtime libraries, please refer to the software runtime manual in Docs

Hope you like it :D
