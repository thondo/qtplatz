/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quancompounds.hpp"
#include <adportable/uuid.hpp>

using namespace adcontrols;

QuanCompounds::~QuanCompounds()
{
}

QuanCompounds::QuanCompounds() : uuid_( adportable::uuid()() )
{
}

QuanCompounds::QuanCompounds( const QuanCompounds& t ) : uuid_( t.uuid_ )
                                                       , compounds_( t.compounds_ )
{
}

QuanCompounds&
QuanCompounds::operator << ( const QuanCompound& t )
{
    compounds_.push_back( t );
    return *this;
}

void
QuanCompounds::uuid( const boost::uuids::uuid& uuid )
{
    uuid_ = uuid;
}

const boost::uuids::uuid&
QuanCompounds::uuid() const
{
    return uuid_;
}
