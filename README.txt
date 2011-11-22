This is a simulation project, simulating how an operating system works in C++. The project is divided into three phases.
PHASE I 
	• Create the assembler.
	• Create the virtual machine.
	• Create the OS that runs an instance of the assembler and sends it to the virtual machine.
	• Test the virtual machine with the factorial program.

Phase II 
	• Simulate the CPU running multiple programs .
	• Add three queues in OS, waiting queue and ready queue.
	• Simulate loading memory with all programs available in the current directory.
	• Add context switching in OS.
	• Implement a file to store each processes stack before context switch.
	• Append accounting information of OS at the end of every programs output file.
PHASE III 

	• Add memory management layer to OS.
	• Implement demand-paging.
	• Fix the page size to be 16 words.
	• Use two page replacement algorithms, FIFO and LRU.
	• Add page fault and Hit-Ration to accounting information.
	• Add PCB's to manage programs.
	• Add page tables for each process.
	• Implement in OS the inverted page table.

This Phase is not complete, there are some bugs in the FIFO, LRU alg.

To Compile
To compile the source code:
	• Download, extract and then navigate to the code's directory and type "make"
