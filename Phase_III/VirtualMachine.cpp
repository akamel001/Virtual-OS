/*
  Filename: VirtualMachine.cpp 
  Author: Abdelrahman Kamel, Takenori Tsuruga
  Date: April 27, 2009
  Discription: This file contains the implementation of
  member functions found in VirtualMachine.h. These functions
  are suppost to simulate the 26 assembly instructions of phase
  one of our project. 
*/

#include "VirtualMachine.h"

void VirtualMachine::setcarry()			
{
	if(r[objCode.f1.RD] & 0x10000) 		
		sr = sr | 1; //set carry flag if 17th bit of RD is 1
	else 
		sr = sr & 0x1E; //else carry is 0
}

bool VirtualMachine::getcarry()	//return 1 if carry flag is set
{											
	if (sr & 1) 
		return 1;							
	else 
		return 0;
}

void VirtualMachine::init(PCB *p)
{
		p -> r.reserve(4);
		
		for(int i = 0; i < 4; i++)
			p->r[i] = 0;
		
		p->page_table.reserve(16);
	
		for(int i=0; i < 16; i++)
		{
			p->page_table[i] = new Page;
			p->page_table[i]->v_i = 0;
		}

		p->sp=256;
		p->sr=0;
		p->CPU_time=0;
		p->largest_stack_size=0;
		p->ta_time=0;
		p->io_time=0;
		p->IO_clock;
		p->ready_time_stamp=0;
		p->wait_time_stamp=0;
		p->waiting_time=0;
		p->page_request=0;
		p->page_counter=0;
		p->HR=0;
		p->pf=0;

}

void VirtualMachine::loadmem(list<PCB *> &pcb)
{
	int temp;
	
	string file;
	
	list<PCB *>::iterator itr;

	//removing the object files produced by g++ compiler
	system("rm -f VirtualMachine.o assembler.o os.o");
	system("ls *.o > objFile");
	ifstream objFile;
	objFile.open("objFile",ios::in);
	
	itr = pcb.begin();
	for(;objFile >> file;itr++)
	{	
		Page * pp = new Page;

		(*itr)->pcbfo.open(file.c_str(), ios::in);
		
		init(*itr);

		if(itr == pcb.begin())
		{
			(*itr) -> pc = 0;
			(*itr) -> base= 0;
		}
		else
		{
			(*itr) -> pc = counter;
			(*itr) -> base = counter;
		}
		
		pp->frame = counter/16;
	
		for(limit=0; limit < 16 && (*itr)->pcbfo >> temp; counter++, limit++)
			mem[counter] = temp; //cout << temp << "\t" << counter << endl;}	

		pp->v_i = 1;

		if(limit < 	16)
			for(int i=limit; i < 16; counter++, i++)	
				mem[counter] = -1;
	
		(*itr) -> limit = limit;
		
		(*itr)->pcbfo.close();

		file = file.substr(0,file.length()-2) + ".in";
		((*itr)->pcbfin).open(file.c_str(),ios::in);
		
		file = file.substr(0,file.length()-3) + ".out";
		((*itr) -> pcbfout).open(file.c_str(),ios::out);
		
		(*itr)->page_table[0] = pp;	
		//(*itr)->ref_str.push(0);
	}
	
	objFile.close();
	
	system("rm objFile");
	
}
void VirtualMachine::printMem()
{
	cout << "***MEMORY DUMP***" << endl;
	for(int i = 0; i < 256; i++)
		cout << mem[i] << endl;
	cout << "***END OF MEMORY DUMP***" << endl;

}
void VirtualMachine::saveState(PCB * p)
{
	for(int i=0; i < 4; i++)//saving registers
		p->r[i] = r[i];

	p->sr = sr;
	p->sp= sp;
	p->pc = pc;
	p->base = base;
	p->limit = limit;

	string file = (p->pName);
	file = file.substr(0,(file.length()-2));
	file += ".st"; 

	
	(p-> pcbfst).open(file.c_str(), ios::out);

	if(sp != 256)
	{
		for(int i=sp; i != 256; i++)
			(p -> pcbfst) << mem[i] << endl;
	}
	(p-> pcbfst).close();
}

void VirtualMachine::loadState(PCB * p)
{
	int temp;
	
	for(int i = 0; i < 4; i++)
		r[i] = p->r[i];
		
	for(int i =0; i < 16; i++)
		TLB[i] = p->page_table[i];
		

	sr = p->sr;
	sp = p->sp;
	pc = p->pc;
	base = (pc/16) * 16;
	limit = (pc/16) * 16 + 16; 
	
	string file = (p->pName);
	file = file.substr(0,(file.length()-2)) + ".st";
	
	(p->pcbfst).open(file.c_str(),ios::in);

	if(sp != 256)
	{
		for(int i = sp; (p->pcbfst) >> temp;i++)
			mem[i] = temp;
	}
	
	(p->pcbfst).close();
	frameTimeStamps[pc/16] = clock;
}

void VirtualMachine::CheckAddr(int addr)
{
	if(TLB[addr/16]->v_i == 1)
	{
		pc = ((TLB[addr/16]->frame)*16) + (addr % 16);
		
		TLB[addr/16]->base = TLB[addr/16]->frame*16;
		
		base = TLB[addr/16]->base;
		limit = base + 16;
	}
	else
	{
		current->adj_pc = false;
		current->page_request = addr/16;
		pageFault = true;
		pc--;
	}
}

void VirtualMachine::run(PCB * p)
{
	int temp = 0, time_up=0;

	current = p;
	loadState(p);
	
	time_up = clock+time_slice;	
	while( true )//entering infinit loop of fetch-execute cycle
	{
		
		ir = mem[pc];
		pc++;
		objCode.i = ir;
		
		
		(this->*fmap[objCode.f1.OP])();
		sr = sr & 0x1F;
		
		if(pageFault){
			sr = sr | 0xE0;
			pageFault = false;
			saveState(p);
			break;
		}
		
		if(objCode.f1.OP == 22 || objCode.f1.OP == 23){
			sr = sr | 0x20;//io operation
			checkRange(p);
			saveState(p);
			break;
		}

		if(sp > 256)//stack underflow
		{
			sr = sr | 0xA0;//underflow flag
			saveState(p);
			break;
		}
		
		if((objCode.f1.OP == 0 && objCode.f1.I == 0) || (objCode.f1.OP == 1))
		{
			if(clock >= (time_up+3))
			{
				sr = sr & 0x1F;
				saveState(p);
				break;
			}
		}
		else
		{
			if(clock >= time_up)
			{
				sr = sr & 0x1F;
				saveState(p);
				break;
			}
		}
		

		if(objCode.i == 49152){
			sr = sr | 0x40;//halt instr
			p->ta_time = clock;
			saveState(p);
			break;
		}
		
		if(pc >= limit)
		{
			/*
			int req_frame = pc/16;
			
			vector<Page *>::iterator itr = TLB.begin();
			for(; itr != TLB.end(); itr++)
			{
				if((*itr)->frame == req_frame)
					goto skip;
			}*/
			++(p->page_counter);

			if(getCurrentPage() < 0)//page wasnt found
					goto skip1;
				
			if(TLB[p->page_counter]->v_i == 1)
			{
				pc = (TLB[(p->page_counter)]->frame * 16) + pc % 16;
				base = pc/16;
				limit = base + 16;
				++(p->HR);
				//p->ref_str.push(p->page_counter);
				clock += 4;
				frameTimeStamps[pc/16] = clock;
				goto skip2;
			}

			skip1:;
			sr = sr | 0xE0;
			saveState(p);
			p->page_request = p->page_counter;
			p->adj_pc = true;
			break;
		}
		skip2:;
	}	
}


void VirtualMachine::checkRange(PCB* p)
{
	if(pc >= limit)
		{
			/*
			int req_frame = pc/16;
			
			vector<Page *>::iterator itr = TLB.begin();
			for(; itr != TLB.end(); itr++)
			{
				if((*itr)->frame == req_frame)
					goto skip;
			}*/
			++(p->page_counter);

			if(getCurrentPage() < 0)//page wasnt found
					goto skip1;
				
			if(TLB[p->page_counter]->v_i == 1)
			{
				pc = (TLB[(p->page_counter)]->frame * 16) + pc % 16;
				base = pc/16;
				limit = base + 16;
				++(p->HR);
				//p->ref_str.push(p->page_counter);
				clock += 4;
				frameTimeStamps[pc/16] = clock;
				goto skip2;
			}

			skip1:;
			sr = sr | 0xE0;
			saveState(p);
			p->page_request = p->page_counter;
			p->adj_pc = true;
			
		}
		skip2:;
}

VirtualMachine::VirtualMachine()
{
	//initializing data types
	fmap.reserve(26);
	mem.reserve(256);
	TLB.reserve(16);
	frameTimeStamps.reserve(16);
	
	for(int i=0; i < 16; i++)
	{
		TLB[i] = new Page;
		TLB[i]->v_i = 0;
	}

	avail_frames.reserve(16);
	r.reserve(4);
	counter = clock = sr = base = pc = limit = 0;
	pageFault = false;
	sp = 256;
	time_slice = 15;
	
	//building function map in a vector
	fmap[0]  = &VirtualMachine::load; 		fmap[1]  = &VirtualMachine::store;
	fmap[2]  = &VirtualMachine::add;			fmap[3]  = &VirtualMachine::addc;		
	fmap[4]  = &VirtualMachine::sub;			fmap[5]  = &VirtualMachine::subc;
	fmap[6]  = &VirtualMachine::and_;		fmap[7]  = &VirtualMachine::xor_;
	fmap[8]  = &VirtualMachine::compl_;		fmap[9]  = &VirtualMachine::shl;
	fmap[10] = &VirtualMachine::shla;		fmap[11] = &VirtualMachine::shr;
	fmap[12] = &VirtualMachine::shra;		fmap[13] = &VirtualMachine::compr;
	fmap[14] = &VirtualMachine::getstat;	fmap[15] = &VirtualMachine::putstat;
	fmap[16] = &VirtualMachine::jump;		fmap[17] = &VirtualMachine::jumpl;
	fmap[18] = &VirtualMachine::jumpe;		fmap[19] = &VirtualMachine::jumpg;
	fmap[20] = &VirtualMachine::call;		fmap[21] = &VirtualMachine::return_;
	fmap[22] = &VirtualMachine::read;		fmap[23] = &VirtualMachine::write;
	fmap[24] = &VirtualMachine::halt;		fmap[25] = &VirtualMachine::noop;


}

int VirtualMachine::getFrame(int page)
{
	if(TLB[page]->v_i == 0)
		return -1;
	else 
		return TLB[page]->frame; 
}

int VirtualMachine::getCurrentPage()
{
	int currentFrame = (pc)/16;
	
	for(int i = 0; i < 16; i++)
	{
		if(TLB[i]->frame == currentFrame)
			return i;
	}
	
	return -1;
}

void VirtualMachine::load() 
{
	if ( objCode.f1.I == 0 ) //I=0
		{
			int i = getFrame(objCode.f2.ADDR/16);

			if( i == -1)
			{
				current -> page_request = objCode.f2.ADDR/16;
				current->adj_pc = false;
				pageFault = true;
				pc--;
			}
			else
			{
				r[objCode.f2.RD] = mem[(i*16)+(objCode.f2.ADDR%16)];
					
				if(objCode.f2.ADDR > limit)
					{
						++(current->HR);
						//current->ref_str.push(objCode.f2.ADDR/16);
					}
					
				clock += 4;
			}
		}
	else //I = 1
		{
			r[objCode.f3.RD] = objCode.f3.CONST;
			clock += 1;
		}
}
		
void VirtualMachine::store() 
{
	clock += 4;
	
	int i = getFrame(objCode.f2.ADDR/16);

	if(i  == -1)
	{
		current->adj_pc = false;
		current->page_request = objCode.f2.ADDR/16;
		pageFault = true;
		pc--;
	}
	else
	{
		if(objCode.f2.ADDR > limit)
		{
			++(current->HR);
			//current->ref_str.push(objCode.f2.ADDR/16);
		}
		mem[(i*16)+objCode.f2.ADDR%16] = r[objCode.f2.RD];
		clock += 4;
	}
}
		
void VirtualMachine::add() 
{
	clock += 1;

	if ( objCode.f1.I == 0 )//I = 0
	{
		
		if(objCode.f1.RD >= 0 && objCode.f1.RS >=0 && ((objCode.f1.RD + objCode.f1.RS) < 0))
			sr = sr | 0x10;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x10;

		r[objCode.f1.RD] += r[objCode.f1.RS];
		setcarry();
     }
	else {
		
		if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
			sr = sr | 0x00000010;
		else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
			sr = sr | 0x00000010;
		
		r[objCode.f3.RD] += objCode.f3.CONST;
		setcarry();
	}
}	

void VirtualMachine::addc() 
{
	clock += 1;

	if ( objCode.f1.I == 0 ) //I=0
	{	
		if(objCode.f1.RD >= 0 && objCode.f1.RS >=0 && ((objCode.f1.RD + objCode.f1.RS) < 0))
			sr = sr | 0x10;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x10;
		
		if ( getcarry() ) 
			r[objCode.f1.RD] += r[objCode.f1.RS] + 1;
		else//Carry is not set
			r[objCode.f1.RD] += r[objCode.f1.RS];
		
		setcarry();
    }
	else //I=1
	{
		if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
			sr = sr | 0x10;//seting overflow flag
		else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
			sr = sr | 0x10;
		
		if ( getcarry() ) 
			r[objCode.f3.RD] += objCode.f3.CONST + 1;
		else//carry is not set
			r[objCode.f3.RD] += objCode.f3.CONST;
		
		setcarry();
	}
}
	
void VirtualMachine::sub() 
{
	clock += 1;
	if ( objCode.f1.I == 0 )//I=0
	{	
		if(objCode.f1.RD >= 0 && objCode.f1.RS >=0 && ((objCode.f1.RD + objCode.f1.RS) < 0))
			sr = sr | 0x10;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x10;

		r[objCode.f1.RD] -= r[objCode.f1.RS];
		setcarry();
   }
	else//I=1
	{	 
		if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
			sr = sr | 0x10;//seting overflow flag
		else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
			sr = sr | 0x10;
		
		r[objCode.f3.RD] -= objCode.f3.CONST;
		setcarry();
	}
}	
void VirtualMachine::subc()
{
	clock += 1;
	
	if ( objCode.f1.I == 0 )//I=0
	{
		if(objCode.f1.RD >= 0 && objCode.f1.RS >=0 && ((objCode.f1.RD + objCode.f1.RS) < 0))
			sr = sr | 0x10;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x10;
		
		if ( getcarry() ) 
			r[objCode.f1.RD] -= r[objCode.f1.RS] - 1;
		else//no carry
			r[objCode.f1.RD] -= r[objCode.f1.RS];

		setcarry();
    }
	else //I=1
	{
      if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
         sr = sr | 0x10;//seting overflow flag
      else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
         sr = sr | 0x10;
	
		if ( getcarry() ) 
			r[objCode.f3.RD] -= objCode.f3.CONST - 1;
		else//no carry
			r[objCode.f3.RD] -= objCode.f3.CONST;

		setcarry();
	}
}	
	
void VirtualMachine::and_()
{
	clock += 1;
	
	if ( objCode.f1.I == 0 ) //I=0
		r[objCode.f1.RD] = r[objCode.f1.RD] & r[objCode.f1.RS];
	else //I=1
		r[objCode.f3.RD] = r[objCode.f3.RD] & objCode.f3.CONST;
}	


void VirtualMachine::xor_()
{
	clock += 1;
	if ( objCode.f1.I == 0 ) 
		r[objCode.f1.RD] = r[objCode.f1.RD] ^ r[objCode.f1.RS];
	else 
		r[objCode.f3.RD] = r[objCode.f3.RD] ^ objCode.f3.CONST;
}		

void VirtualMachine::compl_()
{
	clock += 1;
	
	r[objCode.f1.RD] = ~r[objCode.f1.RD];
}

void VirtualMachine::shl()
{
	clock += 1;
	
	r[objCode.f1.RD] = r[objCode.f1.RD] << 1;
	setcarry();
}

void VirtualMachine::shla()
{
	clock += 1;
	
	if ( r[objCode.f1.RD] < 0 ) {//check if r[RD] is negative
		r[objCode.f1.RD] = r[objCode.f1.RD] << 1;
		r[objCode.f1.RD] = r[objCode.f1.RD] | 0x80000000;//fixing the sign bit to 1
	}//r[RD] >= 0
	else 
	{
		r[objCode.f1.RD] = r[objCode.f1.RD] << 1;
		r[objCode.f1.RD] = r[objCode.f1.RD] & 0x7FFFFFFF;//fixing the sign bit to 0
	}

	setcarry();
}    

void VirtualMachine::shr()
{
	clock += 1;
	
	if ( r[objCode.f1.RD] & 1)//check if 1st bit is 1
		sr = sr | 1;//set carry if 1st bit is 1

	r[objCode.f1.RD] = r[objCode.f1.RD] >> 1;
}       

void VirtualMachine::shra()
{
	clock += 1;
	
	if(r[objCode.f1.RD] & 1)// check if 1st bit is 1
		sr = sr | 1;//set carry if 1st bit is 1
			
	if ( r[objCode.f1.RD] < 0 ) {//check if r[RD] is negative
		r[objCode.f1.RD] = r[objCode.f1.RD] >> 1;
		r[objCode.f1.RD] = r[objCode.f1.RD] | 0x80000000;//fixing the sign bit to 1
	}
		else r[objCode.f1.RD] = r[objCode.f1.RD] >> 1; 
}   

void VirtualMachine::compr()
{
	clock += 1;
	
	if ( objCode.f1.I == 0 ) {//I=0
		if (r[objCode.f1.RD] < r[objCode.f1.RS]) {// less
			sr = sr | 8;//set less flag
			sr = sr & 0x19;//reset greater, equal flag
		}
		else if (r[objCode.f1.RD] == r[objCode.f1.RS]) {//equal
			sr = sr | 4;//set equal flag
			sr = sr & 0x15;//reset greater, less flags
		}
		else {//greater
			sr = sr | 2;//set greater flag
			sr = sr & 0x13;//reset equal, less flags
		}
	}
	else {//compri() when I is 1
		if (r[objCode.f3.RD] < objCode.f3.CONST) {//less
			sr = sr | 8;//set less flag
			sr = sr & 0x19;//reset greater, equal flag
		}
		else if (r[objCode.f3.RD] == objCode.f3.CONST) {//equal
			sr = sr | 4;//set equal flag
			sr = sr & 0x15;//reset greater, less flags
		}
		else {//greater
			sr = sr | 2;//set greater flag
			sr = sr & 0x13;//reset equal, less flags
		}
	}
}    

void VirtualMachine::getstat()
{
	clock += 1;
	
	r[objCode.f1.RD] = sr;
}

void VirtualMachine::putstat()
{
	clock += 1;
	
	sr = r[objCode.f1.RD];
}   

void VirtualMachine::jump()
{
	clock += 1;

	CheckAddr(objCode.f2.ADDR);	
	//pc = objCode.f2.ADDR+base;
}

void VirtualMachine::jumpl()
{
	clock += 1;
	
	if (sr & 8)// if less is set
		CheckAddr(objCode.f2.ADDR);	
		//pc = objCode.f2.ADDR+base;
}  

void VirtualMachine::jumpe()
{
	clock += 1;
	
	if (sr & 4)//if equal is set
		CheckAddr(objCode.f2.ADDR);	
		//pc = objCode.f2.ADDR+base;
}     

void VirtualMachine::jumpg()
{
	clock += 1;
	
	if (sr & 2)//if greater is set
		CheckAddr(objCode.f2.ADDR);	
		//pc = objCode.f2.ADDR+base;
}

void VirtualMachine::call()
{
	clock += 1;
	
	current->kill_frames.push(sp/16);
	
	if(TLB[objCode.f2.ADDR/16]->v_i == 0)
		{
			current->page_request = objCode.f2.ADDR/16;
			current->adj_pc = false;
			pc--;
			pageFault = true;
			return;
		}

	mem[--sp] = getCurrentPage(); //page #
	mem[--sp] = pc%16; //offset

	//mem[--sp] = pc;//pushing pc
 	
	for (int i = 0; i < 4; i++)//pushing r[0]~r[3]
		mem[--sp] = r[i];
	
	mem[--sp] = sr;//pushing sr
	
	CheckAddr(objCode.f2.ADDR);	
	//pc = objCode.f2.ADDR+base;//loading pc to jumping address

	current->largest_stack_size = (256-sp);
}      

void VirtualMachine::return_()
{
	int newPC;

	clock += 1;
	current->kill_frames.pop();
		
	if(TLB[mem[sp+6]]->v_i == 0)
	{
			current->page_request = mem[sp+6];
			current->adj_pc = false;
			pc--;
			pageFault = true;
			return;
	}
	sr = mem[sp++];//popping sr

	for (int i = 3; i > -1; i--)//popping r[0]~r[3]
		r[i] = mem[sp++];
		
	//pc = mem[sp++];//popping pc

	newPC = mem[sp++]; //offset
	newPC += (getFrame(mem[sp++]) * 16);

	pc = newPC;
}   

void VirtualMachine::read()
{
	clock += 2;
	
	(current->pcbfin) >> r[objCode.f1.RD];//read from .in file and put in r[RD]
}  

void VirtualMachine::write()
{
	clock += 2;
	
	if((r[objCode.f1.RD] & 0x00008000))
		r[objCode.f1.RD] |= 0xffff0000;
	
	(current->pcbfout) << r[objCode.f1.RD] << endl;//write r[RD] in .out file
}    

void VirtualMachine::halt()
{
	clock += 1;
	
}

void VirtualMachine::noop()
{
	clock += 1;
	
}
