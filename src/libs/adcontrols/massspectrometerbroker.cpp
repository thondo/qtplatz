// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "massspectrometer_factory.hpp"
#include "adcontrols.hpp"
#include "constants.hpp"
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <algorithm>
#include <map>
#include <string>

using namespace adcontrols;

namespace adcontrols {

    class MassSpectrometerBrokerImpl;
	class massspectrometer_factory;
    
    //-------------------------
    //-------------------------
    class MassSpectrometerBrokerImpl : public massSpectrometerBroker {
		static MassSpectrometerBrokerImpl * instance_;
    public:
        ~MassSpectrometerBrokerImpl() {}

		static MassSpectrometerBrokerImpl * instance();
        
        bool register_factory( massspectrometer_factory* factory, const std::wstring& name ) {
            factories_[name] = factory;
            try {
                MassSpectrometerBroker::register_factory( factory, MassSpectrometerBroker::name_to_uuid( name ), adportable::utf::to_utf8( name ) );
            } catch ( std::exception& ex ) {
                ADDEBUG() << ex.what();
            }
            return true;
        }
        
        massspectrometer_factory * find( const std::wstring& name ) {
            auto it = factories_.find( name );
            if ( it == factories_.end() ) {
                it = std::find_if( factories_.begin(), factories_.end(), [name] ( decltype(factories_)::value_type& a ) {
                    return a.second->is_canonical_name( name.c_str() );
                } );
            }
            if ( it != factories_.end() )
                return it->second;
            return 0;
        }

        void names( std::vector< std::wstring >& list ) {
            for ( auto& factory: factories_ )
                list.push_back( factory.first );
        }
        
        void visit( adcontrols::MassSpectrometer& );
        
    private:
        std::map< std::wstring, massspectrometer_factory *> factories_;
    };

}

MassSpectrometerBrokerImpl * MassSpectrometerBrokerImpl::instance_ = 0;

MassSpectrometerBrokerImpl *
MassSpectrometerBrokerImpl::instance()
{
	if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( adcontrols::global_mutex::mutex() );
        if ( instance_ == 0 )
			instance_ = new MassSpectrometerBrokerImpl;
	}
	return instance_;
}

void
MassSpectrometerBrokerImpl::visit( adcontrols::MassSpectrometer& )
{
    //adcontrols::MassSpectrometerBroker::factory_type factory = impl.factory();
    //register_factory( factory, impl.name() );
}

/////////////////////////////////////////////

massSpectrometerBroker::massSpectrometerBroker(void)
{
}

massSpectrometerBroker::~massSpectrometerBroker(void)
{
}

bool
massSpectrometerBroker::register_factory( massspectrometer_factory * factory, const std::wstring& name )
{
    return MassSpectrometerBrokerImpl::instance()->register_factory( factory, name );
}

massspectrometer_factory*
massSpectrometerBroker::find( const std::wstring& name )
{
	return MassSpectrometerBrokerImpl::instance()->find( name );
}

// static
std::vector< std::wstring >
massSpectrometerBroker::names()
{
    std::vector< std::wstring > vec;
	MassSpectrometerBrokerImpl::instance()->names( vec );
    return vec;
}

////////////////////////////////////////////////////////////////
/////// new generation
///////////////////////////////////////////////////////////////

namespace adcontrols {
    
    class MassSpectrometerBroker::impl {
    public:
        static impl& instance() {
            static impl __impl;
            return __impl;
        }
        typedef std::pair< std::string, std::shared_ptr< massspectrometer_factory > > value_type; 

        std::map< boost::uuids::uuid, value_type > factories_;
    };

}

MassSpectrometerBroker::MassSpectrometerBroker()
{
}

MassSpectrometerBroker::~MassSpectrometerBroker()
{
}

//static
bool
MassSpectrometerBroker::register_factory( massspectrometer_factory* f, const boost::uuids::uuid& uuid, const std::string& objtext )
{
    impl::instance().factories_ [ uuid ] = std::make_pair( objtext, f->shared_from_this() );
    return true;
}

//static
massspectrometer_factory*
MassSpectrometerBroker::find_factory( const boost::uuids::uuid& uuid )
{
    auto it = impl::instance().factories_.find( uuid );
    if ( it != impl::instance().factories_.end() ) {
        if ( auto ptr = it->second.second )
            return ptr.get();
    }
    return nullptr;
}

massspectrometer_factory*
MassSpectrometerBroker::find_factory( const std::string& objtext )
{
    auto it = std::find_if( impl::instance().factories_.begin(), impl::instance().factories_.end()
                            , [&] ( const std::pair<boost::uuids::uuid, impl::value_type>& a ) { return objtext == a.second.first; } );
    if ( it != impl::instance().factories_.end() ) {
        if ( auto ptr = it->second.second )
            return ptr.get();
    }
    return nullptr;
}

std::shared_ptr< adcontrols::MassSpectrometer >
MassSpectrometerBroker::make_massspectrometer( const boost::uuids::uuid& uuid )
{
    if ( auto factory = find_factory( uuid ) ) {
        return factory->create( 0, 0 );
    }
    return nullptr;
}

const boost::uuids::uuid
MassSpectrometerBroker::name_to_uuid( const std::wstring& objtext )
{
    return boost::uuids::name_generator( iids::massspectrometer_uuid )( adportable::utf::to_utf8( objtext ) );
}

const boost::uuids::uuid
MassSpectrometerBroker::name_to_uuid( const std::string& objtext )
{
    return boost::uuids::name_generator( iids::massspectrometer_uuid )( objtext );
}
