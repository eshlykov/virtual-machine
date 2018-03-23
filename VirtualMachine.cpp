#include "Exception.h"
#include "VirtualMachine.h"

#include <experimental/filesystem>
#include <iostream>

CVirtualMachine::CVirtualMachine()
{
}

void CVirtualMachine::Execute( const std::string& pathToBinaryFile )
{
	init( pathToBinaryFile );
	run();
	clear();
}

void CVirtualMachine::init( const std::string& pathToBinaryFile )
{
	std::ifstream input( pathToBinaryFile, std::ios::in | std::ios::binary );

	if( !input.is_open() ) {
		throw CInvalidFile( "CVirtualMachine::init::InvalidBinary - Cannot open file." );
	}

	unsigned bytesCount = std::experimental::filesystem::file_size( pathToBinaryFile );
	unsigned length = bytesCount / 4;
	code.assign( length, 0 );

	for( unsigned i = 0; i < length; ++i ) {
		input.read( reinterpret_cast< char* >( &( code[i] ) ), sizeof( unsigned ) );
	}

	commands = {
		std::bind( &CVirtualMachine::execPrint, this ),
		std::bind( &CVirtualMachine::execRead, this ),
		std::bind( &CVirtualMachine::execPush, this ),
		std::bind( &CVirtualMachine::execPop, this ),
		std::bind( &CVirtualMachine::execMove, this ),
		std::bind( &CVirtualMachine::execIf, this ),
		std::bind( &CVirtualMachine::execCall, this ),
		std::bind( &CVirtualMachine::execEqual, this ),
		std::bind( &CVirtualMachine::execAdd, this ),
		std::bind( &CVirtualMachine::execSubtract, this ),
		std::bind( &CVirtualMachine::execPushaddr, this ),
		std::bind( &CVirtualMachine::execReturn, this ),
		std::bind( &CVirtualMachine::execExit, this ),
		std::bind( &CVirtualMachine::execStr, this ),
	};
}

unsigned CVirtualMachine::getLength( std::ifstream& stream )
{
	stream.seekg( 0, std::ios::end );
	unsigned length = stream.tellg();
	stream.seekg( 0, std::ios::beg );
	return length;
}

void CVirtualMachine::run()
{
	do {
	} while( commands[code[code[0]]]() );
}

bool CVirtualMachine::execPrint()
{
	std::cout << getInteger( code[code[0] + 1] ) << std::endl;
	code[0] += 3;
	return true;
}

unsigned CVirtualMachine::getInteger( unsigned number )
{
	if( !isInteger( number ) ) {
		return getInteger( code[number] );
	}
	return number - integerShift;
}

bool CVirtualMachine::isInteger( unsigned number )
{
	return number >= integerShift;
}

bool CVirtualMachine::execRead()
{
	unsigned number;
	std::cin >> number;
	code[resIndex] = castToCodeData( number );
	code[0] += 3;
	return true;
}

unsigned CVirtualMachine::castToCodeData( int number )
{
	return number + integerShift;
}

bool CVirtualMachine::execPush()
{
	code[code[1]++] = castToCodeData( getInteger( code[code[0] + 1] ) );
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execPop()
{
	code[resIndex] = code[code[1] - 1];
	code[--code[1]] = 0;
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execMove()
{
	code[code[code[0] + 2]] = castToCodeData( getInteger( code[code[0] + 1] ) );
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execIf()
{
	if( getInteger( code[code[0] + 1] ) == 0 ) {
		code[0] += 3;
	} else {
		code[0] = code[code[code[0] + 2]];
	}
	return true;
}

bool CVirtualMachine::execCall()
{
	code[0] = code[code[code[0] + 1]];
	return true;
}

bool CVirtualMachine::execEqual()
{
	code[resIndex] = castToCodeData( getInteger( code[code[0] + 1] ) == getInteger( code[code[0] + 2] ) );
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execAdd()
{
	code[resIndex] = castToCodeData( getInteger( code[code[0] + 1] ) + getInteger( code[code[0] + 2] ) );
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execSubtract()
{
	unsigned minuend = getInteger( code[code[0] + 1] );
	unsigned subtrahend = getInteger( code[code[0] + 2] );
	if( minuend < subtrahend ) {
		throw CInvalidArguments( "CVirtualMachine::execSubtract::CInvalidArguments - Minuend is less than subtrahend." );
	}
	code[resIndex] = castToCodeData( minuend - subtrahend );
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execReturn()
{
	code[0] = getInteger( code[resIndex] );
	return true;
}

bool CVirtualMachine::execPushaddr()
{
	code[code[1]++] = castToCodeData( code[0] + 6 );
	code[0] += 3;
	return true;
}

bool CVirtualMachine::execExit()
{
	return false;
}

bool CVirtualMachine::execStr()
{
	for( int i = code[code[0] + 1]; ; ++i ) {
		for( int j = 24; j >= 0; j -= 8 ) {
			unsigned char c = ( code[i] >> j ) & 0xFF;
			if( c == 0 ) {
				std::cout << std::endl;
				code[0] += 3;
				return true;
			}
			std::cout << c;
		}
	}
}

void CVirtualMachine::clear()
{
	code.clear();
	commands.clear();
}
