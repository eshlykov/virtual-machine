#include "Disassembler.h"
#include "Exception.h"

#include <experimental/filesystem>
#include <fstream>

CDisassembler::CDisassembler()
{
}

void CDisassembler::Disassembly( const std::string& pathToBinaryFile, const std::string& pathToAssemblerFile )
{
	init( pathToBinaryFile );
	readBytes();
	writeProgram( pathToAssemblerFile );
	clear();
}

void CDisassembler::init( const std::string& pathToBinaryFile )
{
	std::ifstream input( pathToBinaryFile, std::ios::in | std::ios::binary );
	if( !input.is_open() ) {
		throw CInvalidFile( "CDisassembler::readBytes::initCode - Cannot open binary file." );
	}

	unsigned bytesCount = std::experimental::filesystem::file_size( pathToBinaryFile );
	unsigned length = bytesCount / 4;
	code.assign( length, 0 );

	for( unsigned i = 0; i < length; ++i ) {
		input.read( reinterpret_cast< char* >( &( code[i] ) ), sizeof( unsigned ) );
	}
	input.close();

	current = 10;

	commands = {
		std::bind( &CDisassembler::decodePrint, this ),
		std::bind( &CDisassembler::decodeRead, this ),
		std::bind( &CDisassembler::decodePush, this ),
		std::bind( &CDisassembler::decodePop, this ),
		std::bind( &CDisassembler::decodeMove, this ),
		std::bind( &CDisassembler::decodeIf, this ),
		std::bind( &CDisassembler::decodeCall, this ),
		std::bind( &CDisassembler::decodeEqual, this ),
		std::bind( &CDisassembler::decodeAdd, this ),
		std::bind( &CDisassembler::decodeSubtract, this ),
		std::bind( &CDisassembler::decodePushaddr, this ),
		std::bind( &CDisassembler::decodeReturn, this ),
		std::bind( &CDisassembler::decodeExit, this ),
		std::bind( &CDisassembler::decodeStr, this ),
	};
}

void CDisassembler::decodePrint()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	append( "print " + argument1 + "\n" );
}

std::string CDisassembler::getRegisterOrNumber( unsigned value )
{
	if( value < integerShift ) {
		return getRegister( value );
	}
	return std::to_string( value - integerShift );
}

std::string CDisassembler::getRegister( unsigned value )
{
	if( value < resIndex ) {
		return "reg" + std::to_string( value - 1 );
	} else if( value == resIndex ) {
		return "res";
	}
	throw CInvalidFile( "CDisassembler::getRegister::InvalidFile - Cannot parse arguments of command." );
}

void CDisassembler::append( const std::string& string )
{
	for( int i = 0; i < tabsCount; ++i ) {
		program += "  ";
	}
	program += string;
}

void CDisassembler::decodeRead()
{
	append( "read\n" );
}

void CDisassembler::decodePush()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	append( "push " + argument1 + "\n" );
}

void CDisassembler::decodePop()
{
	append( "pop\n" );
}

void CDisassembler::decodeMove()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegister( code[current + 2] );
	append( "move " + argument1 + " " + argument2 + "\n" );
}

void CDisassembler::decodeIf()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = "label" + std::to_string( code[current + 2] );
	append( "if " + argument1 + " " + argument2 + "\n" );
}

void CDisassembler::decodeCall()
{
	std::string argument1 = "function" + std::to_string( code[current + 1] );
	append( "call " + argument1 + "\n" );
}

void CDisassembler::decodeEqual()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegisterOrNumber( code[current + 2] );
	append( "equal " + argument1 + " " + argument2 + "\n" );
}

void CDisassembler::decodeAdd()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegisterOrNumber( code[current + 2] );
	append( "add " + argument1 + " " + argument2 + "\n" );
}

void CDisassembler::decodeSubtract()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegisterOrNumber( code[current + 2] );
	append( "subtract " + argument1 + " " + argument2 + "\n" );
}

void CDisassembler::decodePushaddr()
{
	append( "pushaddr\n" );
}

void CDisassembler::decodeReturn()
{
	append( "return\n" );
}

void CDisassembler::decodeExit()
{
	append( "exit\n" );
}

void CDisassembler::decodeStr()
{
	std::string argument1 = "string" + std::to_string( code[current + 1] );
	append( "str " + argument1 + "\n" );
}

void CDisassembler::readBytes()
{
	readStrings();
	readLabels();
	readFunctions();
	readCommands();
}

void CDisassembler::readStrings()
{
	append( "strings\n" );
	++tabsCount;
	while( code[current] != 0 ) {
		std::string string = "";
		unsigned i = current;
		while( code[i] != 0 ) {
			for( int j = 24; j >= 0; j -= 8 ) {
				unsigned char c = ( code[i] >> j ) & 0xFF;
				if( c == 0 ) {
					break;
				}
				string += c;
			}
			++i;
		}
		if( string != "" ) {
			append( "string" + std::to_string( current ) + " " + string + "\n" );
		}
		current = i + 1;
	}
	--tabsCount;
	append( ".\n" );
	++current;
}

void CDisassembler::readLabels()
{
	append( "labels\n" );
	++tabsCount;
	while( code[current] != 0 ) {
		labels[code[current]] = current;
		append( "label" + std::to_string( current ) + "\n" );
		++current;
	}
	--tabsCount;
	append( ".\n" );
	++current;
}

void CDisassembler::readFunctions()
{
	append( "functions\n" );
	++tabsCount;
	while( code[current] != 0 ) {
		functions.push_back( current );
		readFunction();
	}
	--tabsCount;
	append( ".\n" );
	++current;
}

void CDisassembler::readFunction()
{
	append( "function" + std::to_string( current ) + "\n" );
	++current;
	++tabsCount;
	readStrings();
	readLabels();
	readFunctions();
	readCommands();
	--tabsCount;
}

void CDisassembler::readCommands()
{
	append( "commands\n" );
	++tabsCount;
	while( code[current] != integerShift - 1 ) {
		tryAddLabel();
		commands[code[current]]();
		current += 3;
	}
	--tabsCount;
	append( ".\n" );
	++current;
}

void CDisassembler::tryAddLabel()
{
	if( labels.find( current ) != labels.end() ) {
		append( "label label" + std::to_string( labels[current] ) + "\n" );
	}
}

void CDisassembler::writeProgram( const std::string & pathToAssemblerFile ) const
{
	std::ofstream output( pathToAssemblerFile, std::ios::out );
	output << program;
	output.close();
}

void CDisassembler::clear()
{
	code.clear();
	current = 0;
	labels.clear();
	functions.clear();
	commands.clear();
	program = "";
	tabsCount = 0;
}
