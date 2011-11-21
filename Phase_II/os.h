#ifndef OS_H
#define OS_H

#include "assembler.h"
#include "VirtualMachine.h"
#include <iomanip>
#include <list>
#include <queue>


class OS{

public:
	OS();
	void run();

private:
	void assemble_programs();
	void print_info();
	void check_waitingQ();
	void idle();
	void next_job();
	int context_switch();
	Assembler as;
	VirtualMachine vm;
	list<PCB *> pcb;
	list<PCB *> term_jobs;
	queue<PCB *> waitingQ, readyQ, runningQ;
	PCB * running;
	int idle_time, idle_counter;
};

#endif 
