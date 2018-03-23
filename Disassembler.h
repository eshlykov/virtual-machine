#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

class CDisassembler {

public:
	CDisassembler();

	void Disassembly( const std::string& pathToBinaryFile, const std::string& pathToAssemblerFile );

private:
	static const unsigned integerShift = 1 << 31;
	static const unsigned resIndex = 9;
	std::vector<unsigned> code;
	unsigned current = 0;
	std::unordered_map<unsigned, unsigned> labels;
	std::vector<unsigned> functions;
	std::vector<std::function<void()>> commands;
	std::string program = "";
	unsigned tabsCount = 0;

	void init( const std::string& pathToBinaryFile );
	void decodePrint();
	static std::string getRegisterOrNumber( unsigned value );
	static std::string getRegister( unsigned value );
	void append( const std::string& string );
	void decodeRead();
	void decodePush();
	void decodePop();
	void decodeMove();
	void decodeIf();
	void decodeCall();
	void decodeEqual();
	void decodeAdd();
	void decodeSubtract();
	void decodePushaddr();
	void decodeReturn();
	void decodeExit();
	void decodeStr();
	void readBytes();
	void readStrings();
	void readLabels();
	void readFunctions();
	void readFunction();
	void readCommands();
	void tryAddLabel();
	void writeProgram( const std::string& pathToAssemblerFile ) const;
	void clear();
};
