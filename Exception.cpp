#include "Exception.h"

CException::CException( std::string _message ) :
	message( _message )
{
}

const char* CException::what() const throw( )
{
	return message.c_str();
}

//----------------------------------------------------------------------------------------------------------------------

CSyntaxError::CSyntaxError( std::string _message ) :
	CException( _message )
{
}

//----------------------------------------------------------------------------------------------------------------------

CInvalidFile::CInvalidFile( std::string _message ) :
	CException( _message )
{
}

//----------------------------------------------------------------------------------------------------------------------

CInvalidArguments::CInvalidArguments( std::string _message ) :
	CException( _message )
{
}
