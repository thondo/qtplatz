// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "massspectrometerbroker.hpp"
#include "massspectrometer.hpp"
#if defined _MSC_VER
#pragma warning(disable:4996)
#endif
#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>

#include <map>
#include <string>
#include <QLibrary>
#include <adportable/string.hpp>

using namespace adcontrols;

namespace adcontrols {

    class MassSpectrometerBrokerImpl;
    
    //-------------------------
    namespace singleton {
        typedef ACE_Singleton<MassSpectrometerBrokerImpl, ACE_Recursive_Thread_Mutex> MassSpectrometerBrokerImpl;
    }
    
    //-------------------------
    class MassSpectrometerBrokerImpl : public MassSpectrometerBroker {
    public:
        ~MassSpectrometerBrokerImpl() {}
        
        bool register_library( const std::wstring& sharedlib_name );
        
        bool register_factory( factory_type factory, const std::wstring& name ) {
            factories_[name] = factory;
            return true;
        }
        
        factory_type find( const std::wstring& name ) {
            std::map< std::wstring, factory_type >::iterator it = factories_.find( name );
            if ( it != factories_.end() )
                return it->second;
            return 0;
        }
        
        void visit( adcontrols::MassSpectrometer& );
        
    private:
        std::map< std::wstring, factory_type > factories_;
    };
    
    typedef ACE_Singleton< MassSpectrometerBrokerImpl, ACE_Recursive_Thread_Mutex > impl_type;
}

bool
MassSpectrometerBrokerImpl::register_library( const std::wstring& sharedlib )
{
    std::string mbs = adportable::string::convert( sharedlib );
    QLibrary lib( mbs.c_str() );
    if ( lib.load() ) {
        typedef adcontrols::MassSpectrometer * (*instance_type)();
        instance_type getMassSpectrometer = reinterpret_cast<instance_type>( lib.resolve( "getMassSpectrometer" ) );
        if ( getMassSpectrometer ) {
            MassSpectrometer * p = getMassSpectrometer();
            if ( p )
                p->accept( *this );
        }
    }
    return false;
}

void
MassSpectrometerBrokerImpl::visit( adcontrols::MassSpectrometer& impl )
{
    adcontrols::MassSpectrometerBroker::factory_type factory = impl.factory();
    register_factory( factory, impl.name() );
}

/////////////////////////////////////////////

MassSpectrometerBroker::MassSpectrometerBroker(void)
{
}

MassSpectrometerBroker::~MassSpectrometerBroker(void)
{
}

bool
MassSpectrometerBroker::register_library( const std::wstring& sharedlib )
{
	return singleton::MassSpectrometerBrokerImpl::instance()->register_library( sharedlib );
}

bool
MassSpectrometerBroker::register_factory( factory_type factory, const std::wstring& name )
{
    return singleton::MassSpectrometerBrokerImpl::instance()->register_factory( factory, name );
}

MassSpectrometerBroker::factory_type
MassSpectrometerBroker::find( const std::wstring& name )
{
	return singleton::MassSpectrometerBrokerImpl::instance()->find( name );
}
