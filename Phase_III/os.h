#ifndef OS_H
#define OS_H

#include "assembler.h"
#include "VirtualMachine.h"
#include <iomanip>
#include <list>
#include <queue>
#include <stack>

class OS{

public:
	OS();
	void run();

private:
	struct InvertedPage
	{ 
		string frameOwner;
		int page;
	};

	vector<InvertedPage *> invertedPageTable;
	
	queue<int> FIFORef;

	void assemble_programs();
	void print_info();
	void check_waitingQ();
	void load_page(PCB * p);
	void check_kill_frames(stack<int>);
	bool check_page(int, PCB *);
	void close_streams();
	void idle();
	void next_job();
	int context_switch();
	int getEmptyFrame();
	Assembler as;
	VirtualMachine vm;
	list<PCB *> pcb;
	list<PCB *> term_jobs;
	queue<PCB *> waitingQ, readyQ, runningQ;
	PCB * running;
	bool FIFO, LRU;
	int idle_time, idle_counter;
};

#endif 
