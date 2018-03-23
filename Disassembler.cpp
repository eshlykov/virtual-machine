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
	current += 3;
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
		program += "\t";
	}
	program += string;
}

void CDisassembler::decodeRead()
{
	append( "read\n" );
	current += 3;
}

void CDisassembler::decodePush()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	append( "push " + argument1 + "\n" );
	current += 3;
}

void CDisassembler::decodePop()
{
	append( "pop\n" );
	current += 3;
}

void CDisassembler::decodeMove()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegister( code[current + 2] );
	append( "move " + argument1 + " " + argument2 + "\n" );
	current += 3;
}

void CDisassembler::decodeIf()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = "label" + std::to_string( code[current + 2] );
	append( "if " + argument1 + " " + argument2 + "\n" );
	current += 3;
}

void CDisassembler::decodeCall()
{
	std::string argument1 = "functuion" + std::to_string( code[current + 2] );
	append( "call " + argument1 + "\n" );
	current += 3;
}

void CDisassembler::decodeEqual()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegisterOrNumber( code[current + 2] );
	append( "equal " + argument1 + " " + argument2 + "\n" );
	current += 3;
}

void CDisassembler::decodeAdd()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegisterOrNumber( code[current + 2] );
	append( "add " + argument1 + " " + argument2 + "\n" );
	current += 3;
	current += 3;
}

void CDisassembler::decodeSubtract()
{
	std::string argument1 = getRegisterOrNumber( code[current + 1] );
	std::string argument2 = getRegisterOrNumber( code[current + 2] );
	append( "subtract " + argument1 + " " + argument2 + "\n" );
	current += 3;
	current += 3;
}

void CDisassembler::decodePushaddr()
{
	append( "pushaddr\n" );
	current += 3;
}

void CDisassembler::decodeReturn()
{
	append( "return\n" );
	current += 3;
}

void CDisassembler::decodeExit()
{
	append( "exit\n" );
	current += 3;
}

void CDisassembler::decodeStr()
{
	std::string argument1 = "string" + std::to_string( code[current + 2] );
	append( "str " + argument1 + "\n" );
	current += 3;
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
			strings[current] = string;
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
		labels.push_back( current );
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
		append( "function" + std::to_string( current ) + "\n" );
		readStrings();
		readLabels();
		readFunctions();
		readCommands();
	}
	--tabsCount;
	append( ".\n" );
	++current;
}

void CDisassembler::readCommands()
{
	append( "functions\n" );
	++tabsCount;
	while( code[current] != integerShift - 1 ) {
		tryAddLabel();
		commands[code[current]]();
	}
	--tabsCount;
	append( ".\n" );
	++current;
}

void CDisassembler::tryAddLabel()
{
	for( unsigned index : labels ) {
		if( index == current ) {
			append( "label label" + std::to_string( current ) + "\n" );
		}
	}
}

void CDisassembler::writeProgram( const std::string & pathToAssemblerFile )
{
	std::ofstream output( pathToAssemblerFile, std::ios::out );
	output << program;
	output.close();
}

void CDisassembler::clear()
{
	code.clear();
	current = 0;
	strings.clear();
	labels.clear();
	functions.clear();
	commands.clear();
	program = "";
	tabsCount = 0;
}
