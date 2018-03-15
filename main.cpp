#include "Assembler.h"
#include "VirtualMachine.h"

#include <iostream>

int main( int argc, char** argv )
{
	try {
		CAssembler assembler;
		assembler.Assembly( "../fibonacci.asm", "../fibonacci.bin" );

		CVirtualMachine virtualMachine;
		virtualMachine.Execute( "../fibonacci.bin" );
	} catch ( const std::exception& exception ) {
		std::cout << exception.what() << std::endl;
	}

	int c;
	std::cin >> c;

	return 0;
}