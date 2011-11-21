all: os

os: os.o assembler.o VirtualMachine.o
	g++ -o os os.o assembler.o VirtualMachine.o

os.o: os.cpp
	g++ -c os.cpp

assembler.o: assembler.cpp
	g++ -c assembler.cpp

VirtualMachine.o: VirtualMachine.cpp
	g++ -c VirtualMachine.cpp
