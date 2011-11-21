/*
  Filename: VirtualMachine.cpp 
  Author: Abdelrahman Kamel
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

void VirtualMachine::loadmem(list<PCB *> &pcb)
{
	int temp;
	
	string file;
	
	list<PCB *>::iterator itr;

	//removing the object files produced by g++ compiler
	system("rm -f VirtualMachine.o assembler.o os.o");
	system("ls *.o > objFile");
	ifstream objFile;
	ifstream objSubFile;
	objFile.open("objFile",ios::in);
	
	itr = pcb.begin();
	for(;objFile >> file;itr++){	
		objSubFile.open(file.c_str(), ios::in);
		(*itr) -> r.reserve(4);
		
		for(int i = 0; i < 4; i++)
			(*itr)->r[i] = 0;
		
		(*itr)->sp=256;
		(*itr)->sr=0;
		(*itr)->CPU_time=0;
		(*itr)->largest_stack_size=0;
		(*itr)->ta_time=0;
		(*itr)->io_time=0;
		(*itr)->waiting_time=0;

		if(itr == pcb.begin()){
			(*itr) -> pc = 0;
			(*itr) -> base= 0;
		}
		else{
			(*itr) -> pc = counter;
			(*itr) -> base = counter;
		}
		
		for(limit=0;objSubFile >> temp; counter++, limit++)//{
				mem[counter] = temp; //cout << temp << "\t" << counter << endl;}	
		
		(*itr) -> limit = limit;
		objSubFile.close();

		file = file.substr(0,file.length()-2);
		file += ".in";
		((*itr)->pcbfin).open(file.c_str(),ios::in);
		
		file = file.substr(0,file.length()-3);
		file += ".out";
		((*itr) -> pcbfout).open(file.c_str(),ios::out);

	}
	objFile.close();
	
	system("rm objFile");
	
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
		
	sr = p->sr;
	sp = p->sp;
	pc = p->pc;
	base = p->base;
	limit=p->limit;
	
	string file = (p->pName);
	file = file.substr(0,(file.length()-2));
	file += ".st"; 

	(p->pcbfst).open(file.c_str(),ios::in);

	if(sp != 256)
	{
		for(int i = sp; (p->pcbfst) >> temp;i++)
			mem[i] = temp;
	}
	(p->pcbfst).close();
}

void VirtualMachine::run(PCB * p)
{
	int temp, time_up;

	current = p;
	
	loadState(p);

	time_up = clock+time_slice;	
	for(;;)//entering infinit loop of fetch-execute cycle
	{
		ir = mem[pc];
		pc++;
		objCode.i = ir;
		(this->*fmap[objCode.f1.OP])();
		
		sr = sr & 0x1F;//clearing vm_return status

		if(sp < (counter + 6))//stack overflow
		{
			sr = sr | 0x80;//setting overflow flag
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
				saveState(p);
				break;
			}
		}
		else
		{
			if(clock >= time_up)
			{
				saveState(p);
				break;
			}
		}
		
		if((objCode.f1.OP == 0 && objCode.f1.I == 0) ||//load RD ADDR
			(objCode.f1.OP == 1) || //store RD ADDR
			(objCode.f1.OP == 16) || //jump ADDR
			(objCode.f1.OP == 17) || //jumpl ADDR
			(objCode.f1.OP == 18) || //jumpe ADDR
			(objCode.f1.OP == 19) || //jumpg ADDR
			(objCode.f1.OP == 20)) //call ADDR
		{
			if(!((objCode.f2.ADDR+base < base + limit) 
			 && (objCode.f2.ADDR+base >= base) && !(objCode.f2.ADDR <= 0)))
			{
				sr = sr | 0x60; // out-bound was made 
				saveState(p);
				break;
			}
		}

		if(objCode.f1.OP == 22 || objCode.f1.OP == 23){
			sr = sr | 0x20;//io operation
			saveState(p);
			break;
		}
		
		if(objCode.i == 49152){
			sr = sr | 0x40;//halt instr
			p->ta_time = clock;
			saveState(p);
			break;
		}
	}	
}


VirtualMachine::VirtualMachine()
{
	//initializing data types
	fmap.reserve(26);
	mem.reserve(256);
	r.reserve(4);
	counter = clock = sr = base = pc = limit = 0;
	sp = 256;
	time_slice = 15;
	
	//building function map in a vector
	fmap[0]  = &VirtualMachine::load; 		fmap[1]  = &VirtualMachine::store;
	fmap[2]  = &VirtualMachine::add;		fmap[3]  = &VirtualMachine::addc;		
	fmap[4]  = &VirtualMachine::sub;		fmap[5]  = &VirtualMachine::subc;
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

void VirtualMachine::load() 
{
	if ( objCode.f1.I == 0 ) //I=0
		{
			r[objCode.f2.RD] = mem[objCode.f2.ADDR+base];
			clock += 4;
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
	
	mem[objCode.f2.ADDR+base] = r[objCode.f2.RD];
}
		
void VirtualMachine::add() 
{
	clock += 1;

	if ( objCode.f1.I == 0 )//I = 0
	{
		
		if(objCode.f1.RD >= 0 && objCode.f1.RS >=0 && ((objCode.f1.RD + objCode.f1.RS) < 0))
			sr = sr | 0x00000010;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x00000010;

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
			sr = sr | 0x00000010;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x00000010;
		
		if ( getcarry() ) 
			r[objCode.f1.RD] += r[objCode.f1.RS] + 1;
		else//Carry is not set
			r[objCode.f1.RD] += r[objCode.f1.RS];
		
		setcarry();
    }
	else //I=1
	{
		if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
			sr = sr | 0x00000010;//seting overflow flag
		else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
			sr = sr | 0x00000010;
		
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
			sr = sr | 0x00000010;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x00000010;

		r[objCode.f1.RD] -= r[objCode.f1.RS];
		setcarry();
   }
	else//I=1
	{	 
		if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
			sr = sr | 0x00000010;//seting overflow flag
		else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
			sr = sr | 0x00000010;
		
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
			sr = sr | 0x00000010;//seting overflow flag
		else if(objCode.f1.RD < 0 && objCode.f1.RS < 0 && ((objCode.f1.RD + objCode.f1.RS) >= 0))
			sr = sr | 0x00000010;
		
		if ( getcarry() ) 
			r[objCode.f1.RD] -= r[objCode.f1.RS] - 1;
		else//no carry
			r[objCode.f1.RD] -= r[objCode.f1.RS];

		setcarry();
    }
	else //I=1
	{
      if(objCode.f3.RD >= 0 && objCode.f3.CONST >=0 && ((objCode.f3.RD + objCode.f3.CONST) < 0))
         sr = sr | 0x00000010;//seting overflow flag
      else if(objCode.f3.RD < 0 && objCode.f3.CONST < 0 && ((objCode.f3.RD + objCode.f3.CONST) >= 0))
         sr = sr | 0x00000010;
	
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
			sr = sr & 0x00000019;//reset greater, equal flag
		}
		else if (r[objCode.f1.RD] == r[objCode.f1.RS]) {//equal
			sr = sr | 4;//set equal flag
			sr = sr & 0x00000015;//reset greater, less flags
		}
		else {//greater
			sr = sr | 2;//set greater flag
			sr = sr & 0x00000013;//reset equal, less flags
		}
	}
	else {//compri() when I is 1
		if (r[objCode.f3.RD] < objCode.f3.CONST) {//less
			sr = sr | 8;//set less flag
			sr = sr & 0x00000019;//reset greater, equal flag
		}
		else if (r[objCode.f3.RD] == objCode.f3.CONST) {//equal
			sr = sr | 4;//set equal flag
			sr = sr & 0x00000015;//reset greater, less flags
		}
		else {//greater
			sr = sr | 2;//set greater flag
			sr = sr & 0x00000013;//reset equal, less flags
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
	
	pc = objCode.f2.ADDR+base;
}

void VirtualMachine::jumpl()
{
	clock += 1;
	
	if (sr & 8)// if less is set
		pc = objCode.f2.ADDR+base;
}  

void VirtualMachine::jumpe()
{
	clock += 1;
	
	if (sr & 4)//if equal is set
		pc = objCode.f2.ADDR+base;
}     

void VirtualMachine::jumpg()
{
	clock += 1;
	
	if (sr & 2)//if greater is set
		pc = objCode.f2.ADDR+base;
}

void VirtualMachine::call()
{
	clock += 1;
	
	mem[--sp] = pc;//pushing pc
 	
	for (int i = 0; i < 4; i++)//pushing r[0]~r[3]
		mem[--sp] = r[i];
	
	mem[--sp] = sr;//pushing sr
	
	pc = objCode.f2.ADDR+base;//loading pc to jumping address

	current->largest_stack_size = (256-sp);
}      

void VirtualMachine::return_()
{
	clock += 1;
	
	sr = mem[sp++];//popping sr

	for (int i = 3; i > -1; i--)//popping r[0]~r[3]
		r[i] = mem[sp++];
		
	pc = mem[sp++];//popping pc
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
