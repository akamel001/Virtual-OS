/*
  Filename: assembler.h 
  Author: Abdelrahman Kamel, Takenori Tsuruga
  Date: April 27, 2009
  Discription: This file contains the member function prototype that are 
  implemented in assembler.cpp
*/

#ifndef ASSEMBLER
#define ASSEMBLER

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

class Assembler {
	map<string, int> op;

public:
	Assembler();
	void assemble(string);

private:
	int instr;	
	string opcode,line,outputName;
	fstream assemblyProg; // used to read .s
	ofstream output;//used to write .o 
};

#endif
