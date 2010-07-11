// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

class ACE_Time_Value;
class ACE_INET_Addr;

namespace acewrapper {

    class Callback {
    public:
        virtual void operator()(const char *, int, const ACE_INET_Addr& ) { /* handle input */ }
        virtual void operator()(const ACE_Time_Value&, const void * ) { /* handle_timeout */ }
    };
  
}

