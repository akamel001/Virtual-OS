#include <iostream>
#include <string>
#include <map>
#include <stdexcept>

using namespace std;

class NullPointerException: public runtime_error {
public:
	NullPointerException(): runtime_error("Null Pointer!") { }
};

class Assembler {
	typedef void (Assembler::*FP)(string);

	void add(string s) { cout << "add" << s << endl; }
	void sub(string s) { cout << "sub" << s << endl; }
	void load(string s) { cout << "load" << s << endl; }
	map<string, FP> instr;
public:
	Assembler();
	int assemble();
};

Assembler::Assembler()
{
	instr["add"] = &Assembler::add;
	instr["sub"] = &Assembler::sub;
	instr["load"] = &Assembler::load;
}

int Assembler::assemble()
{
	string input;

	try {
		cin >> input;
		if (not instr[input] )
			throw NullPointerException();
		else (this->*instr[input])("something");

	} catch (NullPointerException e) {
		cerr << e.what() << endl;
		return 1;
	}
}

main()
{
	Assembler a;
	a.assemble();
}
