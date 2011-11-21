/*
  Filename: assembler.cpp
  Author: Abdelrahman Kamel, Takenori Tsuruga
  Date: April 27, 2009
  Discription: This file contains the implementation of
  member functions for assembler.h 
*/

#include "assembler.h"

Assembler::Assembler()
{
	op["load"] = 0;		op["loadi"] = 0;	op["store"] = 1;	
	op["add"] = 2;		op["addi"] = 2;		op["addc"] = 3;		
	op["addci"] = 3;	op["sub"] = 4;		op["subi"] = 4;		
	op["subc"] = 5;		op["subci"] = 5;	op["and"] = 6;
	op["andi"] = 6;		op["xor"] = 7;		op["xori"] = 7;
	op["compl"] = 8;	op["shl"] = 9;		op["shla"] = 10;
	op["shr"] = 11;		op["shra"] = 12;	op["compr"] = 13;
	op["compri"] = 13; 	op["getstat"] = 14;	op["putstat"] = 15;
	op["jump"] = 16;	op["jumpl"] = 17;	op["jumpe"] = 18;
	op["jumpg"] = 19;	op["call"] = 20;	op["return"] = 21;
	op["read"] = 22;	op["write"] = 23;	op["halt"] = 24;
	op["noop"] = 25;
}

void Assembler::assemble(string file)
{

	assemblyProg.open(file.c_str(),ios::in);

	if(!assemblyProg)
	{ 
		cout << "Error opening file." << endl; 
		exit(1);
	}
	
	outputName.assign(file,0,file.length()-2);
	outputName += ".o";
	output.open(outputName.c_str(),ios::out);

	getline(assemblyProg, line);
	
	while(!assemblyProg.eof()){

		int rd=-1, rs=-1;
		istringstream str(line.c_str());
		str >> opcode >> rd >> rs;		
		if(line[0] == '!' || line.empty())
			goto skip;
		else if(rd < 0 && rs < 0)
				instr = op[opcode] << 11;
		else if (rd >= 0 && rd < 256 && rs < 0 && 
				(opcode == "call" || opcode == "jumpg" || opcode == "jumpe" || 
				 opcode == "jumpl" || opcode == "jump"))//opcode ADDR
					instr = (op[opcode] << 11) + rd; 
		else if (rd >= 0 && rd < 4 && (opcode == "compl" || opcode == "shl" || opcode == "shla" || 
				opcode == "shr" || opcode == "shra" || opcode == "getstat" || 
				opcode == "putstat" || opcode == "read" || opcode == "write"))//opcode RD
				{
					instr = op[opcode] << 11;
					instr += rd << 9; 
				}
		else if( rd >= 0 && rd < 4 && rs >= -128 && rs < 128 && (opcode == "loadi" || 
				opcode == "addci" || opcode == "subi" || opcode == "subci" || 
				opcode == "addi" || opcode == "xori" || opcode == "compri" || 
				opcode == "andi"))//opcode RD CONST where 0 =< CONST < 128
				{
					instr = op[opcode] << 11;
					instr += rd << 9;
					instr += 1 << 8; // the I bit which is always one in this case
					rs = rs & 0x000000ff;
					instr += rs;
				}
		else if (rd >= 0 && rd < 4 && rs >= 0 && rs < 256 && (opcode == "load" || 
				opcode == "store"))//opcode RD ADDR
				{
					instr = op[opcode] << 11;
					instr += rd << 9;
					instr += rs;
				}
		else if (rd >=0 && rd < 4 && rs >= 0 && rs < 4 && (opcode == "add" || opcode == "addc" || 
				opcode == "sub" || opcode == "subc" || opcode == "and" || 
				opcode == "xor" || opcode == "compr"))//opcode RD RS
				{
					instr = op[opcode] << 11;
					instr += rd << 9;
					instr += rs << 6; 
				} 
		else
		{
			cout << "Failed to assemble the following instruction:   ";
			
			if(rd != -1 && rs != -1)
				cout << opcode << " " << rd << " " << rs << endl;
			else
				cout << opcode << endl;

			if(rd != -1 && rs == -1)
				cout << opcode << " " << rd << endl;
		
			cout << "Assembler is now exiting!\n";
			exit(1);
		}	
		
		output << instr << endl;
		skip:
		getline(assemblyProg, line);
	}
	assemblyProg.close();
	output.close();
}
