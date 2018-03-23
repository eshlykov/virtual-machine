#pragma once

#include "Assembler.h"
#include "Exception.h"

#include <algorithm>
#include <bitset>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>

CAssembler::CAssembler()
{
}

void CAssembler::Assembly( const std::string& pathToAssemblerFile, const std::string& pathToBinaryFile )
{
	readProgram( pathToAssemblerFile );
	writeBytes( pathToBinaryFile );
}

void CAssembler::readProgram( const std::string& pathToAssemblerFile )
{
	input = std::ifstream( pathToAssemblerFile, std::ios::in );
	if( !input.is_open() ) {
		throw CInvalidFile( "CAssembler::readProgram::InvalidFile - Cannot open assembler file." );
	}

	initCode();
	readStrings();
	readLabels();
	readFunctions();
	setIp();
	readCommands();
	setStack();
}

void CAssembler::initCode()
{
	for( int i = 0; i < memoryLimit; ++i ) {
		code[i] = 0;
	}
	for( int i = 1; i <= registersCount - 1; ++i ) {
		registers["reg" + std::to_string( i )] = i + 1;
		code[i + 1] = integerShift;
	}
	registers["res"] = registersCount + 1;
	code[registersCount + 1] = integerShift;
	current = registersCount + 2;

	commands = {
		{ "print", std::bind( &CAssembler::doPrint, this ) },
		{ "read", std::bind( &CAssembler::doRead, this ) },
		{ "push", std::bind( &CAssembler::doPush, this ) },
		{ "pop", std::bind( &CAssembler::doPop, this ) },
		{ "move", std::bind( &CAssembler::doMove, this ) },
		{ "if", std::bind( &CAssembler::doIf, this ) },
		{ "call", std::bind( &CAssembler::doCall, this ) },
		{ "equal", std::bind( &CAssembler::doEqual, this ) },
		{ "add", std::bind( &CAssembler::doAdd, this ) },
		{ "subtract", std::bind( &CAssembler::doSubtract, this ) },
		{ "pushaddr", std::bind( &CAssembler::doPushaddr, this ) },
		{ "return", std::bind( &CAssembler::doReturn, this ) },
		{ "exit", std::bind( &CAssembler::doExit, this ) },
		{ "label", std::bind( &CAssembler::doLabel, this ) },
		{ "str", std::bind( &CAssembler::doStr, this ) },
	};
}

void CAssembler::readStrings()
{
	readAndCheckKeyword( "strings" );
	for( input >> token; token != "."; input >> token ) {
		checkStringDoubleDefinition( token );
		strings[token] = current;

		std::string string;
		input.get();
		std::getline( input, string );

		for( int i = 0; i < string.length(); ++i ) {
			code[current + i / 4] += static_cast<unsigned>( string[i] ) << ( 24 - 8 * ( i % 4 ) );
		}

		current += ( string.length() - 1 ) / 4 + 2;
	}
}

void CAssembler::readAndCheckKeyword( const std::string keyword )
{
	input >> token;
	if( token != keyword ) {
		throw CSyntaxError( "CAssembler::readAndCheckKeyword::SyntaxError - Expected '" + keyword + "', but '" + token + "' found." );
	}
}

void CAssembler::checkStringDoubleDefinition( const std::string token ) const
{
	if( strings.find( token ) != strings.end() ) {
		throw CSyntaxError( "CAssembler::checkFunctionDoubleDefinition::SyntaxError - String '" + token + "' already defined." );
	}
}

void CAssembler::readLabels()
{
	readAndCheckKeyword( "labels" );
	for( input >> token; token != "."; input >> token ) {
		checkLabelDoubleDeclaration( token );
		labels[token] = current++;
	}
}


void CAssembler::checkLabelDoubleDeclaration( const std::string token ) const
{
	if( labels.find( token ) != labels.end() ) {
		throw CSyntaxError( "CAssembler::checkLabelDoubleDeclaration::SyntaxError - Label + '" + token + "' already declarated." );
	}
}

void CAssembler::readFunctions()
{
	readAndCheckKeyword( "functions" );
	for( input >> token; token != "."; input >> token ) {
		checkFunctionDoubleDefinition( token );
		functions[token] = current++;
		readFunction( token );
	}
}


void CAssembler::checkFunctionDoubleDefinition( const std::string token ) const
{
	if( functions.find( token ) != functions.end() ) {
		throw CSyntaxError( "CAssembler::checkFunctionDoubleDefinition::SyntaxError - Function '" + token + "' already defined." );
	}
}

void CAssembler::readFunction( const std::string name )
{
	readStrings();
	readLabels();
	readFunctions();
	setFunction( name );
	readCommands();
}


void CAssembler::setFunction( const std::string name )
{
	code[functions.at( name )] = current;
}

void CAssembler::readCommands()
{
	readAndCheckKeyword( "commands" );
	for( input >> token; token != "."; input >> token ) {
		if( commands.find( token ) == commands.end() ) {
			throw CSyntaxError( "CAssembler::readCommands::SyntaxError - Unknown command '" + token + "'." );
		}
		commands[token]();
	}
}

void CAssembler::doPrint()
{
	code[current++] = 0;
	code[current++] = getIntegerOrRegister();
	code[current++] = 0;
}

unsigned CAssembler::getIntegerOrRegister()
{
	input >> token;
	if( isInteger( token ) ) {
		return getInteger( token );
	}
	return getRegister( token );
}

bool CAssembler::isInteger( const std::string token )
{
	return !token.empty() && ( std::find_if( token.begin(), token.end(),
		[] ( char c )
	{
		return !std::isdigit( c );
	} ) == token.end() );
}

unsigned CAssembler::getInteger( const std::string token )
{
	checkInteger( token );
	unsigned value = std::atoi( token.c_str() );
	checkTooLarge( value );
	return value + integerShift;
}

void CAssembler::checkInteger( const std::string token ) const
{
	if( !isInteger( token ) ) {
		throw CSyntaxError( "CAssembler::checkInteger::SyntaxError - The number expected, but '" + token + "' found." );
	}
}

void CAssembler::checkTooLarge( const unsigned value ) const
{
	if( value >= integerShift ) {
		throw CSyntaxError( "CAssembler::checkInteger::SyntaxError - The number '" + token + "' is too large." );
	}
}

void CAssembler::doRead()
{
	code[current++] = 1;
	code[current++] = 0;
	code[current++] = 0;
}

void CAssembler::doPush()
{
	code[current++] = 2;
	code[current++] = getIntegerOrRegister();
	code[current++] = 0;
}

void CAssembler::doPop()
{
	code[current++] = 3;
	code[current++] = 0;
	code[current++] = 0;
}

void CAssembler::doMove()
{
	code[current++] = 4;
	code[current++] = getIntegerOrRegister();
	code[current++] = getRegister();
}


unsigned CAssembler::getRegister()
{
	input >> token;
	return getRegister( token );
}

unsigned CAssembler::getRegister( const std::string token ) const
{
	checkRegister( token );
	return registers.at( token );
}


void CAssembler::checkRegister( const std::string token ) const
{
	if( registers.find( token ) == registers.end() ) {
		throw CSyntaxError( "CAssembler::checkRegister::SyntaxError - Unknown register '" + token + "'." );
	}
}

void CAssembler::doIf()
{
	code[current++] = 5;
	code[current++] = getIntegerOrRegister();
	code[current++] = getLabel();
}

unsigned CAssembler::getLabel()
{
	input >> token;
	checkLabel( token );
	return labels.at( token );
}

void CAssembler::checkLabel( const std::string token ) const
{
	if( labels.find( token ) == labels.end() ) {
		throw CSyntaxError( "CAssembler::checkLabel::SyntaxError - Unknown label '" + token + "'." );
	}
}

void CAssembler::doCall()
{
	code[current++] = 6;
	code[current++] = getFunction();
	code[current++] = 0;
}

unsigned CAssembler::getFunction()
{
	input >> token;
	checkFunction( token );
	return functions.at( token );
}

void CAssembler::checkFunction( const std::string token ) const
{
	if( functions.find( token ) == functions.end() ) {
		throw CSyntaxError( "CAssembler::checkFunction::SyntaxError - Unknown function '" + token + "'." );
	}
}

void CAssembler::doEqual()
{
	code[current++] = 7;
	code[current++] = getIntegerOrRegister();
	code[current++] = getIntegerOrRegister();
}

void CAssembler::doAdd()
{
	code[current++] = 8;
	code[current++] = getIntegerOrRegister();
	code[current++] = getIntegerOrRegister();
}

void CAssembler::doSubtract()
{
	code[current++] = 9;
	code[current++] = getIntegerOrRegister();
	code[current++] = getIntegerOrRegister();
}

void CAssembler::doPushaddr()
{
	code[current++] = 10;
	code[current++] = 0;
	code[current++] = 0;
}

void CAssembler::doReturn()
{
	code[current++] = 11;
	code[current++] = 0;
	code[current++] = 0;
}

void CAssembler::doExit()
{
	code[current++] = 12;
	code[current++] = 0;
	code[current++] = 0;
}

void CAssembler::doLabel()
{
	input >> token;
	checkLabel( token );
	checkFunctionDoubleDefinition( token );
	code[labels.at( token )] = current;
}

void CAssembler::checkLabelDoubleDefinition( const std::string token ) const
{
	if( code[labels.at( token )] != 0 ) {
		throw CSyntaxError( "CAssembler::checkLabelDoubleDefinition::SyntaxError - Label + '" + token + "' already defined." );
	}
}

void CAssembler::doStr()
{
	input >> token;
	checkString( token );
	code[current++] = 13;
	code[current++] = strings.at( token );
	code[current++] = 0;
}

void CAssembler::checkString( const std::string& token ) const
{
	if( strings.find( token ) == strings.end() ) {
		throw CSyntaxError( "CAssembler::checkFunction::SyntaxError - Unknown string '" + token + "'." );
	}
}

void CAssembler::writeBytes( const std::string& pathToBinaryFile )
{
	output = std::ofstream( pathToBinaryFile, std::ios::out | std::ios::binary );
	char* c = reinterpret_cast<char*>( code );
	output.write( c, sizeof( unsigned ) * memoryLimit );
}

void CAssembler::setIp()
{
	code[0] = current;
}

void CAssembler::setStack()
{
	code[1] = current;
}