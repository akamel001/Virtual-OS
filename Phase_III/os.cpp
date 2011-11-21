/*
   Filename: os.cpp
   Author: Abdelrahman Kamel, Takenori Tsuruga
   Date: April 27, 2009
   Discription: This program checks the suffix of the incoming
   perameter and makes sure it ends with ".s"; exits otherwise.
   It then creates 2 objects as, and vm and those two objects
   call assemble, and run. 
*/

/*modified for phase 2 undergoing */

#include "os.h"
#include <limits.h>
#include <math.h>	

OS::OS(){
	invertedPageTable = vector<InvertedPage*>(16);

	for(int i = 0; i < invertedPageTable.size(); i++)
	{
		invertedPageTable[i] = new InvertedPage;
		invertedPageTable[i]->frameOwner = "";
		invertedPageTable[i]->page = 0;
	}

	assemble_programs();
		
	idle_time = 0;
	idle_time =0;
	idle_counter=0;
	FIFO = true;
	LRU = false;
}

void OS::assemble_programs(){

//system("ls *.s > programs 2>&-");
system("ls *.s >programs");
fstream fin;
fin.open("programs",ios::in);

string prog;

while(fin >> prog){
	as.assemble(prog);
	PCB * p = new PCB;
	p -> pName = prog;
	readyQ.push(p);
	pcb.push_back(p);
}

vm.loadmem(pcb);//load mem with pcb list

for(int i = 0; i < pcb.size(); i++)
{
	invertedPageTable[pcb.front()->pc/16]->frameOwner = pcb.front()->pName;
	invertedPageTable[pcb.front()->pc/16]->page = 0;
	FIFORef.push(pcb.front()->pc/16);
	pcb.push_back(pcb.front());
	pcb.pop_front();
}

system("rm programs");
fin.close();

//setting first job
running=readyQ.front();
readyQ.pop();
}

//checks frames if SP grew too high it invalidates the frame
void OS::check_kill_frames(stack<int> kill_frames)
{
	while(!kill_frames.empty())
	{
		int k_frame = kill_frames.top();
		
		kill_frames.pop();
		string owner;
		owner =  invertedPageTable[k_frame-1]->frameOwner;
		
		invertedPageTable[k_frame]->frameOwner = "";
		for(int i = 0; i < pcb.size(); i++)
		{
			if(pcb.front()->pName == owner)
				break;
			else
			{
				pcb.push_back(pcb.front());
				pcb.pop_front();
			}
		}
		
		for(int j = 0; j < pcb.front()->page_table.size();j++)
		{
			if((pcb.front()->page_table[j])->frame == k_frame)
				{
					(pcb.front()->page_table[j])->v_i = 0;
					break;
				}
		}
	}
}

int OS::context_switch()
{
	switch(vm.get_vm_status()){

      case 0://Time slice occured
            readyQ.push(running);
            running -> ready_time_stamp = vm.clock;
				running->pf_triger = false;
			   break;
      case 1://I/O operation occured
         	running->IO_clock = vm.clock + 26;
            waitingQ.push(running);         
            running -> wait_time_stamp= vm.clock;
				running->pf_triger = false;
				break;
      case 2://Halt instr occured
         	term_jobs.push_back(running);
				if(term_jobs.size() == pcb.size())
            	goto done;
				break;
      case 3://out-bound
            (running->pcbfout) 
            	<< "An out-of-bound reference was made." << endl << "PROGRAM TERMINATED!" << endl;
            term_jobs.push_back(running);
            break;
      case 4://stack overflow occured
            (running->pcbfout) << "Stack overflow occured." << endl << "PROGRAM TERMINATED!" << endl;
            term_jobs.push_back(running);
         	break;
      case 5://Stack underflow occured
            (running->pcbfout) << "Stack underflow occured." << endl << "PROGRAM TERMINATED!" << endl;
            term_jobs.push_back(running);
      		break;
		case 7://page fault  
				running -> IO_clock = vm.clock + 40;
				waitingQ.push(running);
				running -> wait_time_stamp = vm.clock;
				running->pf_triger = true;
				++(running->pf);
				break;
      done:
				print_info();
				close_streams();
				return -1;
		default:
         	break;    
		}//select
  	 
	vm.clock += 5;//charging timer for context switch
	
	//null because job came back and was placed in right queue
	running = 0;
}

void OS::close_streams()
{
	list<PCB *>::iterator itr = term_jobs.begin();
	
	for(; itr != term_jobs.end(); itr++)
		{
			(*itr)->pcbfin.close();
			(*itr)->pcbfo.close();
			(*itr)->pcbfout.close();
			(*itr)->pcbfst.close();
		}

}

int OS::getEmptyFrame()
{
	for(int i = 0; i < invertedPageTable.size(); i++)
	{
		if(invertedPageTable[i]->frameOwner == "")
			return i;
	}

	return -1;
}

bool OS::check_page(int page, PCB *p)
{
	string file = p->pName; 
	file = file.substr(0,file.length()-2)+".o";
	
	fstream strea;
	strea.open(file.c_str(), ios::in);
	
	int counter, temp;
	
	for(counter = 0;strea >> temp; counter++);

	counter = ceil(static_cast<double>(counter)/16.0);
	if(counter >= page)
		return true;
	else
		return false;
}

void OS::load_page(PCB * p)
{	
	int empty_frame = getEmptyFrame();
		
	int temp, limit;
	
	if(!check_page(p->page_request, p))
	{
		cout << "PAGE NOT IN RANGE" << endl;
		exit(1);
	}

	if(empty_frame == -1)
	{
		int time = LONG_MAX;
		
		if(FIFO)
		{	
			empty_frame = FIFORef.front();
			FIFORef.push(FIFORef.front());
			FIFORef.pop();
		}

		if(LRU)
		{
			for(int i = 0; i < 16; i++)
			{
				if(vm.frameTimeStamps[i] < time)
				{
					empty_frame = i;		
					time = vm.frameTimeStamps[i];
				}
			}
		}
		int j =0;
		PCB* replacingPCB;

		for(int i = 0; i < pcb.size(); i++)
		{
			if(invertedPageTable[empty_frame]->frameOwner == pcb.front()->pName)
			{
				for(; j < 16; j++)
				{
					if(pcb.front()->page_table[j]->frame == empty_frame)
					{
						pcb.front()->page_table[j]->v_i = 0;
						replacingPCB = pcb.front();
						break;
					}	
				}
			}
			pcb.push_back(pcb.front());
			pcb.pop_front();
		}  
		
	}
		
	Page * pp = new Page;
		
	pp->v_i = 1;
	pp->frame = empty_frame;
	FIFORef.push(empty_frame);
		
	invertedPageTable[empty_frame]->frameOwner = p->pName;
	invertedPageTable[empty_frame]->page = p->page_request;
	
	string file = p->pName; 
	file = file.substr(0,file.length()-2)+".o";
	
	(p->pcbfo).open(file.c_str(),ios::in);
		
	for(int i = 0; i < p->page_request*16; i++)
		p->pcbfo >> temp;
		
	for(limit=0; limit < 16 && p->pcbfo >> temp; limit++)
		vm.mem[16 * empty_frame + limit] = temp;	
		//vm.mem.push_back(temp);
		
	pp->limit = limit;
	p->limit = limit;
		
	if(limit < 16)
	{
		for(limit; limit < 16; limit++);
			//vm.mem[16*empty_frame+limit] = 0;
			//vm.mem.push_back(-1);
	}
		
	if(p->adj_pc)	
		p->pc = 16*empty_frame;// + p->displ;
		
	p->adj_pc = false;	

	pp->base = 16*empty_frame;
	p->base = 16*empty_frame;
	p->page_table[p->page_request] = pp;
	
	p->pf_triger = false;	
	
	(p->pcbfo).close();
}

void OS::check_waitingQ(){

	//poping all ready items into readyQ
   while( !waitingQ.empty() && waitingQ.front() -> IO_clock < vm.clock){
     	 readyQ.push(waitingQ.front());
		 
		 if(waitingQ.front()->pf_triger)
				load_page(waitingQ.front());
		
		 waitingQ.front() -> ready_time_stamp = vm.clock ;
		 waitingQ.front() -> io_time += (vm.clock - (waitingQ.front() -> wait_time_stamp));
       waitingQ.pop();
	}
	
}

void OS::next_job(){
	
	//assigning the next task to run
	if(!readyQ.empty()){
		running = readyQ.front();
		readyQ.front() -> waiting_time += (vm.clock - (running -> ready_time_stamp));
		readyQ.pop();
	}else running =0;

}

void OS::idle(){

		for(idle_counter=0; waitingQ.front() -> IO_clock >= vm.clock; vm.clock++, idle_counter++);
      	idle_time += idle_counter;
		
}

void OS::run(){

bool first_run = false;
int temp=0;

while( true ){
	
	check_waitingQ();
	

	if(!first_run){
       temp = vm.clock-5;
       vm.run(running);
       running->CPU_time += (vm.clock-temp-5);
   }else{
       temp = vm.clock;
		 vm.run(running);
       running->CPU_time += (vm.clock-temp);   
   }
	
	//preforming context_switch
	if(context_switch() == -1)
		goto exit;	
	
	//idle case
   while(readyQ.empty() && running == 0)
   {
		idle();
		check_waitingQ();
   }
	
	next_job();

	first_run = true;
	
}//while 

exit:;
}

void OS::print_info(){

list<PCB *>::iterator itr;
itr = term_jobs.begin();

double io_time=0;
for(;itr != term_jobs.end(); itr++)
{
	(*itr)->pcbfout << "\nCPU Time: " << (*itr)->CPU_time << endl;
	(*itr)->pcbfout << "Largest stack size: " << (*itr)->largest_stack_size << endl;
	(*itr)->pcbfout << "I/O Time in waitingQ: " << (*itr)->io_time << endl;
	(*itr)->pcbfout << "Waiting Time in readyQ: " << (*itr)->waiting_time << endl;
	(*itr)->pcbfout << "Turn around time: " << (*itr)->ta_time <<endl;
	(*itr)->pcbfout << "Number of page fault: " << (*itr)->pf << endl;
	(*itr)->pcbfout << "Hit ratio: " << (*itr)->HR << endl;
	io_time += (*itr)->io_time;
}

itr = term_jobs.begin();
for(;itr != term_jobs.end(); itr++)
{
	(*itr)->pcbfout << "\nCPU idled for: " << idle_time << " ticks." << endl;
	(*itr)->pcbfout << "\nSystem Information: " << endl;
	(*itr)->pcbfout  << "\tCPU Utilization: " << setprecision(4) 
		 << static_cast<double>((vm.clock-idle_time))/static_cast<double>(vm.clock)*100 << "%"<<endl;
	(*itr)->pcbfout  << "\tThroughput: " << (static_cast<double>(pcb.size())/(static_cast<double>(vm.clock)))*1000<< " seconds" << endl;
}
}

main(){
OS os;
os.run();
}//main

