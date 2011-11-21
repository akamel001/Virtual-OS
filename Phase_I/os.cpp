#include "assembler.h"
#include "VirtualMachine.h"

main(int argc, char *argv[])
{	
	if(argc == 1 || argc >= 3)
		cout << "Missing argument or too many arguments.\n";
	else
	{
		string file = argv[1];
		if(*(file.end()-2) == '.' && *(file.end()-1) == 's')
			{
				Assembler as;
				VirtualMachine vm;
				as.assemble(file);
				vm.run(file);
			}
		else
			cout << "Improper suffix used, please rename your assembly program with proper suffix.\n";
	}
}
