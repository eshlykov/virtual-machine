#pragma once

#include <exception>
#include <string>

class CException : public std::exception {

public:
	CException( std::string _message );

	const char* what() const throw ( );

private:
	std::string message;
};

//----------------------------------------------------------------------------------------------------------------------

class CSyntaxError : public CException {

public:
	CSyntaxError( std::string _message );
};

//----------------------------------------------------------------------------------------------------------------------

class CInvalidFile : public CException {

public:
	CInvalidFile( std::string _message );
};

//----------------------------------------------------------------------------------------------------------------------

class CInvalidArguments : public CException {

public:
	CInvalidArguments( std::string _message );
};
