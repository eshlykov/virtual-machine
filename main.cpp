#include "Assembler.h"
#include "Disassembler.h"
#include "VirtualMachine.h"

#include <iostream>

int main( int argc, char** argv )
{
	try {
		CAssembler assembler;
		assembler.Assembly( "../fibonacci.asm", "../fibonacci.bin" );

		CVirtualMachine virtualMachine;
		virtualMachine.Execute( "../fibonacci.bin" );

		CDisassembler disassembler;
		disassembler.Disassembly( "../fibonacci.bin", "../fibonacci.disasm" );
		assembler.Assembly( "../fibonacci.disasm", "../fibonacci.bin" );
		virtualMachine.Execute( "../fibonacci.bin" );
	} catch ( const std::exception& exception ) {
		std::cout << exception.what() << std::endl;
	}
	return 0;
}
