#pragma once

#include <fstream>
#include <functional>
#include <string>
#include <vector>

class CVirtualMachine {

public:
	CVirtualMachine();

	void Execute( const std::string& pathToBinaryFile );

private:
	static const unsigned integerShift = 1 << 31;
	static const unsigned resIndex = 9;
	std::vector<unsigned> code;
	std::vector<std::function<bool()>> commands;

	void init( const std::string& pathToBinaryFile );
	void run();
	bool execPrint();
	unsigned getInteger( unsigned number ) const;
	static bool isInteger( unsigned number );
	bool execRead();
	static unsigned castToCodeData( int number );
	bool execPush();
	bool execPop();
	bool execMove();
	bool execIf();
	bool execCall();
	bool execEqual();
	bool execAdd();
	bool execSubtract();
	bool execReturn();
	bool execPushaddr();
	bool execExit();
	bool execStr();
	void clear();
};