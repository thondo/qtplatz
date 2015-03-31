// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include "adbroker_global.h"
#include <adplugin/orbbroker.hpp>

namespace adplugin { class orbServant; class plugin; }

namespace adbroker {

    class orbBroker : public adplugin::orbBroker {
        
    public:
        orbBroker();
        virtual ~orbBroker();
        
        // orbServant creator -- histrical
        // virtual adplugin::orbServant * operator()( adplugin::plugin * ) const override;

        // Borker instance
        virtual adplugin::orbServant * create_instance() const override;
        virtual bool orbmgr_init( int ac, char * av [] ) const override;
        virtual void orbmgr_shutdown() override;
        virtual bool orbmgr_fini() override;
        virtual bool orbmgr_wait() override;
    };

}


