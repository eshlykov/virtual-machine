#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <unordered_map>

class CAssembler {

public:
	CAssembler();

	void Assembly( const std::string& pathToAssemblerFile, const std::string& pathToBinaryFile );

private:
	static const unsigned memoryLimit = 65536;
	static const unsigned registersCount = 8;
	static const unsigned ip = 0;
	static const unsigned stack = 1;
	static const unsigned integerShift = 1 << 31;
	std::ifstream input;
	std::ofstream output;
	std::string token;
	unsigned code[memoryLimit];
	unsigned current = 0;
	std::unordered_map<std::string, unsigned> strings;
	std::unordered_map<std::string, unsigned> labels;
	std::unordered_map<std::string, unsigned> functions;
	std::unordered_map<std::string, unsigned> registers;
	std::unordered_map<std::string, std::function<void()>> commands;

	void readProgram( const std::string& pathToAssemblerFile );
	void initCode();
	void readStrings();
	void readAndCheckKeyword( const std::string keyword );
	void checkStringDoubleDefinition( const std::string token ) const;
	void readLabels();
	void checkLabelDoubleDeclaration( const std::string token ) const;
	void readFunctions();
	void checkFunctionDoubleDefinition( const std::string token ) const;
	void readFunction( const std::string name );
	void setFunction( const std::string name );
	void readCommands();
	void doPrint();
	unsigned getIntegerOrRegister();
	static bool isInteger( const std::string token );
	unsigned getInteger( const std::string token );
	void checkInteger( const std::string token ) const;
	void checkTooLarge( const unsigned value ) const;
	void doRead();
	void doPush();
	void doPop();
	void doMove();
	unsigned getRegister();
	unsigned getRegister( const std::string token ) const;
	void checkRegister( const std::string token ) const;
	void doIf();
	unsigned getLabel();
	void checkLabel( const std::string token ) const;
	void doCall();
	unsigned getFunction();
	void checkFunction( const std::string token ) const;
	void doEqual();
	void doAdd();
	void doSubtract();
	void doPushaddr();
	void doReturn();
	void doExit();
	void doLabel();
	void checkLabelDoubleDefinition( const std::string token ) const;
	void doStr();
	void checkString( const std::string& token ) const;
	void writeBytes( const std::string& pathToBinaryFile );
	void setIp();
	void setStack();
	void clear();
};
