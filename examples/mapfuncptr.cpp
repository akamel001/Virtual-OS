#include <iostream>
#include <string>
#include <map>

using namespace std;

class Assembler {
	typedef void (Assembler::*FP)(string);

	void add(string s) { cout << "add" << s << endl; }
	void sub(string s) { cout << "sub" << s << endl; }
	map<string, FP> instr;
public:
	Assembler();
	void assemble();
};

Assembler::Assembler()
{
	instr["add"] = &Assembler::add;
	instr["sub"] = &Assembler::sub;
}

void Assembler::assemble()
{
	string input;

	cin >> input;
	(this->*instr[input])("1");
	cin >> input;
	(*this.*instr[input])("2");
}

main()
{
	Assembler a;
	a.assemble();
}
