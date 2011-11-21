#include <iostream>
//#include <iomanip>

using namespace std;

main()
{
	class format1 {
	public:
		unsigned UNUSED:6;
		unsigned RS:2;
		unsigned I:1;
		unsigned RD:2;
		unsigned OP:5;
	};

	class format2 {
	public:
		unsigned ADDR:8;
		unsigned I:1;
		unsigned RD:2;
		unsigned OP:5;
	};


	class format3 {
	public:
		int CONST:8;
		unsigned I:1;
		unsigned RD:2;
		unsigned OP:5;
	};

	union instruction {
		int i;
		format1 f1;
		format2 f2;
		format3 f3;
	};

	instruction ins;

	ins.i = 0xa007;
	//ins.i = 4288;
	//ins.i = 1093;

	//cout << hex;
	cout << ins.i << endl;
	cout << ins.f1.OP << " " << ins.f1.RD << " " << ins.f1.I << " " << ins.f1.RS << " " << ins.f1.UNUSED << endl;
	cout << ins.f2.OP << " " << ins.f2.RD << " " << ins.f2.I << " " << ins.f2.ADDR << endl;
	cout << ins.f3.OP << " " << ins.f3.RD << " " << ins.f3.I << " " << ins.f3.CONST << endl;
}
