//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////
#include "brokertoken.h"
#include <time.h>

BrokerToken::~BrokerToken()
{
}

BrokerToken::BrokerToken()
{
}

BrokerToken::BrokerToken( const BrokerToken& t ) : connString_(t.connString_)
												 , terminalId_(t.terminalId_)
												 , sessionGuid_(t.sessionGuid_)
												 , ownerId_(t.ownerId_)
												 , ownerName_(t.ownerName_)
												 , connDate_(t.connDate_)
												 , revision_(t.revision_)
{
}

const std::wstring& 
BrokerToken::connDate() const
{
  return connDate_;
}

const std::wstring& 
BrokerToken::connString() const
{
  return connString_;
}

const std::wstring& 
BrokerToken::terminalId() const
{
  return terminalId_;
}

const std::wstring& 
BrokerToken::sessionGuid() const
{
  return sessionGuid_;
}

const std::wstring& 
BrokerToken::ownerId() const
{
  return ownerId_;
}

const std::wstring& 
BrokerToken::ownerName() const
{
  return ownerName_;
}

const std::wstring& 
BrokerToken::revision() const
{
  return revision_;
}

//static 
BrokerToken
BrokerToken::Create( const std::wstring& ownerId
					 , const std::wstring& ownerName
					 , const std::wstring& sessionGuid
					 , const std::wstring& terminalId
					 , const std::wstring& connString
                                         , const std::wstring& revision )
{
  BrokerToken token;
  token.ownerId_     = ownerId;
  token.ownerName_   = ownerName;
  token.sessionGuid_ = sessionGuid;
  token.terminalId_  = terminalId;
  token.connString_  = connString;
  token.revision_    = revision;
  
  time_t tm;
  time(&tm);
  // _date_t today(tm);
  // token.connDate_ = _wstr_t::towcs( std::string( today ) );

  return token;
}
  
//static
BrokerToken
BrokerToken::fromXml( const std::wstring& xml )
{
  return BrokerToken();
}


//static
BrokerToken
BrokerToken::fromXml( const XMLElement& )
{
  return BrokerToken();
}

//static 
bool 
BrokerToken::toXml( const BrokerToken &, std::wstring& xml )
{
  return false;
}
