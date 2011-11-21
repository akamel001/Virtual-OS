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

OS::OS(){
	assemble_programs();	
	idle_time = 0;
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
system("rm programs");
fin.close();

//setting first job
running=readyQ.front();
readyQ.pop();
}

int OS::context_switch()
{
	switch(vm.get_vm_status()){

      case 0://Time slice occured
            readyQ.push(running);
            running -> ready_time_stamp = vm.clock;
            break;
      case 1://I/O operation occured
         	running->IO_clock = vm.clock + 26;
            waitingQ.push(running);         
            running -> wait_time_stamp= vm.clock;
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
      done:
				print_info();
				return -1;
		default:
         	cout << "Invalid vm return status " << vm.get_vm_status() << endl;
         	break;    
		}//select
   
	vm.clock += 5;//charging timer for context switch
	
	//null because job came back and was placed in right queue
	running = 0;
}

void OS::check_waitingQ(){

	//poping all ready items into readyQ
   while( !waitingQ.empty() && waitingQ.front() -> IO_clock < vm.clock){
     	 readyQ.push(waitingQ.front());
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
}//while */

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
	(*itr)->pcbfout << "Turn around time: " 
					<< (*itr)->ta_time <<endl;
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

