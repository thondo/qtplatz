/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include <compiler/disable_unused_parameter.h>
#include "manager.hpp"
#include "adplugin.hpp"
#include "lifecycle.hpp"
#include "constants.hpp"
#include "lifecycleaccessor.hpp"
#include "plugin.hpp"
#include "plugin_ptr.hpp"
#include "visitor.hpp"
#include <adportable/configuration.hpp>
#include <acewrapper/constants.hpp>
#include <adportable/configloader.hpp>
#include <adportable/debug.hpp>
#include <adportable/string.hpp>
#include <qtwrapper/qstring.hpp>
#include <QLibrary>
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <map>
#include <fstream>
#include <ace/Singleton.h>
#include <boost/smart_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/cast.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>
#include <algorithm>

//----------------------------------

using namespace adplugin;

////////////////

namespace adplugin {

    class plugin_data {
        adplugin::plugin_ptr plugin_;
    public:
		plugin_data() {
		}
        plugin_data( adplugin::plugin_ptr ptr ) : plugin_( ptr ) {
        }
        plugin_data( const plugin_data& t ) : plugin_( t.plugin_ ) {
        }
        const char * clsid() const { 
			return plugin_->clsid();
		}
        const char * iid() const { 
            adplugin::plugin * ptr = plugin_.get();
			return ptr->iid(); 
		}
        adplugin::plugin_ptr plugin() const { return plugin_; }
        adplugin::plugin * p() { return plugin_.get(); }
        bool operator == ( const adplugin::plugin& t ) const {
            if ( plugin_.get() == &t ) // equal address
                return true;
            return ( plugin_->clsid() == t.clsid() &&  plugin_->iid() == t.iid() );
        }
    };

    namespace internal {

        class manager_data : boost::noncopyable
                           , adplugin::visitor {
        public:
            virtual ~manager_data() {}
            manager_data() {}
            typedef std::map< std::string, plugin_data > map_type;
            typedef std::vector< plugin_data > vector_type;
            bool install( QLibrary&, const std::string& adplugispec );
			void populated();
            plugin_ptr select_iid( const char * regex );
            plugin_ptr select_clsid( const char * regex );
            size_t select_iids( const char * regex, std::vector< plugin_ptr >& );
            size_t select_clsids( const char * regex, std::vector< plugin_ptr >& );

            // visitor 
            void visit( adplugin::plugin * plugin, const char * adpluginspec );
        private:
            map_type plugins_;
            vector_type additionals_; // if shared-object contains more than two plugins
        };
    }

    class manager_impl : public manager {
    public:
        ~manager_impl();
        manager_impl();

        virtual void register_ior( const std::string& name, const std::string& ior );
        virtual const char * lookup_ior( const std::string& name );

    private:
        std::map< std::string, std::string > iorMap_;
    };
}
////////////////////////////////////

manager * manager::instance_ = 0;

manager *
manager::instance()
{
	if ( instance_ == 0 )
		instance_ = new manager_impl;
	return instance_;
}

std::string
manager::iorBroker()
{
    const char * p = manager::instance()->lookup_ior( acewrapper::constants::adbroker::manager::_name() );
    return p ? std::string( p ) : std::string("");
}

std::string
manager::ior( const char * name )
{
	return manager::instance()->lookup_ior( name );
}

///////////////////////////////////

manager_impl::manager_impl()
{
}

manager_impl::~manager_impl()
{
}

void
manager_impl::register_ior( const std::string& name, const std::string& ior )
{
    iorMap_[name] = ior;
    
    QDir dir = QDir::home();

	boost::filesystem::path path( qtwrapper::wstring::copy( dir.absolutePath() ) );
	path /= ".ior";
	if ( ! boost::filesystem::exists( path ) )
		boost::filesystem::create_directory( path );
    path /= name;
	path.replace_extension( ".ior" );
	boost::filesystem::ofstream of( path );

    of << ior;
}

const char *
manager_impl::lookup_ior( const std::string& name )
{
    std::map< std::string, std::string >::iterator it = iorMap_.find( name );
    if ( it != iorMap_.end() )
	return it->second.c_str();
    return 0;
}


//////////////////////////////////////
////////////////////////////////////////
//////////////////////

manager::manager(void) : d_( new internal::manager_data() )
{
}

manager::~manager(void)
{
    delete d_;
}

bool
manager::install( QLibrary& lib, const std::string& adpluginspec )
{
    return d_->install( lib, adpluginspec );
}

void
manager::populated()
{
	return d_->populated();
}

plugin_ptr
manager::select_iid( const char * regex )
{
    return d_->select_iid( regex );
}

plugin_ptr
manager::select_clsid( const char * regex )
{
    return d_->select_clsid( regex );
}

size_t
manager::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return d_->select_iids( regex, vec );
}

size_t
manager::select_clsids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return d_->select_clsids( regex, vec );
}

//////////////////

using namespace adplugin::internal;

void
manager_data::visit( adplugin::plugin * plugin, const char * adpluginspec )
{
    if ( adpluginspec == 0 || plugin == 0 )
        return;

    // make it unique
	auto it = std::find_if( plugins_.begin(), plugins_.end(), [&](const map_type::value_type& d){
		return d.second == (*plugin);
	});
	if ( it == plugins_.end() )
		additionals_.push_back( plugin_data( plugin ) );
}

void
manager_data::populated()
{
#if defined _DEBUG || defined DEBUG
    std::cout << "=============== install ===============================" << std::endl;
    for ( auto it = plugins_.begin(); it != plugins_.end(); ++it ) {
        adplugin::plugin * xptr = it->second.p();
        std::cout << "\t" << std::hex << unsigned(xptr) << ", " << xptr->iid() << std::endl;
    }
    std::cout << "=============== end install ===========================" << std::endl;
#endif
}

bool
manager_data::install( QLibrary& lib, const std::string& adpluginspec )
{
    if ( plugins_.find( adpluginspec ) != plugins_.end() )
        return true; // already in, so that does not need unload() call
    typedef adplugin::plugin * (*factory)();

    if ( lib.isLoaded() ) {
        factory f = reinterpret_cast< factory >( lib.resolve( "adplugin_plugin_instance" ) );
        if ( f ) {
            adplugin::plugin * pptr = f();
#ifdef DEBUG
            boost::filesystem::path path( adpluginspec );
            std::string cls = path.leaf().string();
            std::cout << " ** install plugin: " << std::hex << pptr << ", " << pptr->iid() 
                      << " ** " << cls << std::endl;
#endif            
            adplugin::plugin_ptr ptr( pptr, false ); // ref count start with 1 so don't increment when instatnce created
            if ( ptr ) {
                ptr->clsid_ = adpluginspec;
				plugins_[ adpluginspec ] = plugin_data( ptr );
                ptr->accept( *this, adpluginspec.c_str() );

                return true;
            }
        }
    }
    return false;
}

plugin_ptr
manager_data::select_iid( const char * regex )
{
	boost::regex re( regex );
    boost::cmatch matches;

	auto itr = std::find_if( plugins_.begin(), plugins_.end(), [&]( const map_type::value_type& m ){
		return boost::regex_match( m.second.iid(), matches, re );
	});
	if ( itr != plugins_.end() )
		return itr->second.plugin();
    return 0;
}

plugin_ptr
manager_data::select_clsid( const char * regex )
{
    return 0;
}

size_t
manager_data::select_iids( const char * regex, std::vector< plugin_ptr >& vec )
{
	boost::regex re( regex );
	boost::cmatch matches;

	std::for_each( plugins_.begin(), plugins_.end(), [&]( const map_type::value_type& m ){
		const char * tgt = m.second.iid();
		std::cout << "regex: " << regex << std::endl;
		std::cout << "targt: " << tgt << std::endl;
		if ( boost::regex_match( m.second.iid(), matches, re ) )
			vec.push_back( m.second.plugin() );
		} );
    return vec.size();
}

size_t
manager_data::select_clsids( const char * regex, std::vector< plugin_ptr >& vec )
{
    return 0;
}
