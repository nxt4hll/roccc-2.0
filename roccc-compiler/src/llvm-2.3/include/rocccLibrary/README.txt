The files in this directory are library and configuration files used by 
ROCCC.  

The file ROCCCHelper.vhdl defines various helper functions used by ROCCC
that have no configuration necessary.

The file DELAY.txt contains a list of the various costs associated with
different operations in hardware.  This is used by the ROCCC compiler to
determine when to merge different operations into the same pipeline stage. 

The files rocccLibrary.txt and rocccLibrary.vhdl are generated and modified 
by the ROCCC compiler and should not be changed by hand.

The format of the simple file is as follows:

NAME ( Delay , portName : in : bitSize , portName2 : out : bitSize , ... )

The bitsize can be a number or a question mark followed by a number such as 
"?2"  This signifies that the bit size is parameterizable and actually 
references a generic in the VHDL.  The generic is instantiated in the system
code by the size of the data elements passed to the function. 
