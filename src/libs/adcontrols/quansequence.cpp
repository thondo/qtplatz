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

#include "quansequence.hpp"
#include <adportable/uuid.hpp>

using namespace adcontrols;

QuanSequence::~QuanSequence()
{
}

QuanSequence::QuanSequence() : uuid_(adportable::uuid()())
{
}

QuanSequence::QuanSequence( const QuanSequence& t ) : uuid_( t.uuid_ ), samples_( t.samples_ )
{
}

QuanSequence&
QuanSequence::operator << ( const QuanSample& t )
{
    samples_.push_back( t );
    return *this;
}

void
QuanSequence::uuid( const boost::uuids::uuid& uuid )
{
    uuid_ = uuid;
}

const boost::uuids::uuid&
QuanSequence::uuid() const
{
    return uuid_;
}
