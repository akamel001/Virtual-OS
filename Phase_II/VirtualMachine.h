/*
  Filename: VirtualMachine.h
  Author: Abdelrahman Kamel, Takenori Tsuruga
  Date: April 27, 2009
  Discription: This file contains all of the 26 instructions
  of the assembly language designed in phase 1 of our project.
  This header file also contains helper functions and helper 
  classes. Only member functions are in this header file. 
  Implementation of the member functions are found in the 
  VirtualMachine.cpp.
*/

#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector> 
#include <map>
#include <list>
#include <cstdio>
#include <cstdlib>

using namespace std;

class format1 {
	public:
		unsigned UNUSED:6;//UNUSED[5:0]
		unsigned RS:2;		//RS[7:6]
		unsigned I:1;		//I[8]
		unsigned RD:2;		//RD[10:9]
		unsigned OP:5;		//OP[15:11]
	};

class format2 {
	public:
		unsigned ADDR:8;	//ADDR[7:0]
		unsigned I:1;		//I[8]
		unsigned RD:2;		//RD[10:9]
		unsigned OP:5;		//OP[15:11]
	};

class format3 {
	public:
		int CONST:8;		//CONST[7:0]
		unsigned I:1;		//I[8]
		unsigned RD:2;		//RD[10:9]
		unsigned OP:5;		//OP[15:11]
	};

union instruction {
		int i;
		format1 f1;	
		format2 f2;	
		format3 f3;			
	};

struct PCB{
	vector<int> r;
	int pc,sr,sp,base,limit, IO_clock;
	int CPU_time, largest_stack_size, ta_time, io_time, waiting_time; 
	int wait_time_stamp, ready_time_stamp;
	string pName;
	
	ifstream pcbfin;//.in
	ofstream pcbfout;//.out
	fstream pcbfst;//.st
};

class VirtualMachine{
	public:
		VirtualMachine();

		void load();			
		void store();			
		void add();			
		void addc();			
		void sub();			
		void subc();			
		void and_();			
		void xor_()	;		
		void compl_();			
		void shl();			
		void shla();			
		void shr();			
		void shra();			
		void compr();			
		void getstat();			
		void putstat();		
		void jump();			
		void jumpl();			
		void jumpe();			
		void jumpg();			
		void call();			
		void return_();			
		void read(); 			
		void write();			
		void halt();			
		void noop();			
		
		void setcarry();		//function to set carry flag if needed
		bool getcarry();		//function to grub carry if carry flag is set
		int  get_vm_status() {return ((sr & 0xE0) >> 5);}
		void run(PCB *);		
		void saveState(PCB *);
		void loadState(PCB *);
		void loadmem(list<PCB *> &);

	private:
		typedef void (VirtualMachine::*FP)();
		vector<int> mem, r;
		instruction objCode;
		string wfile;
		string rfile;
		PCB * current;
		int ir,sr,limit,pc,base,sp,clock, time_slice, counter;
		vector<FP> fmap;

		friend class OS;
};

#endif 
