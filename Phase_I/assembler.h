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
