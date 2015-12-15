/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include <adcontrols/datareader.hpp>
#include <adplugin/plugin.hpp>
#include <boost/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace adfs { class sqlite; }

namespace acqrsinterpreter {

    class DataReader : public adcontrols::DataReader {
        DataReader( const DataReader& ) = delete;  // noncopyable
        DataReader& operator = (const DataReader&) = delete;
    public:
        ~DataReader( void );
        DataReader( const char * traceid );
        
        static std::vector< std::string > traceid_list();        

        // <===== adcontrols::DataReader 
        bool initialize( adfs::filesystem&, const boost::uuids::uuid& objid, const std::string& objtxt ) override;
        void finalize() override;
        size_t ticCount() const override;
        // =============================>

    private:
        std::unique_ptr< adcontrols::DataInterpreter > interpreter_;
        std::weak_ptr< adfs::sqlite > db_;
        boost::uuids::uuid objid_;
        std::string objtext_;
    };

}


