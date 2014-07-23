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

#include "quancompound.hpp"
#include <atomic>

namespace adcontrols {
    namespace quan {

        std::atomic< uint64_t > __uniqCompoundKey;

    }
}

using namespace adcontrols;



QuanCompound::~QuanCompound()
{
}

QuanCompound::QuanCompound() : uniqId_( quan::__uniqCompoundKey++ )
                             , isISTD_( false )
                             , idISTD_( -1 )
                             , amounts_( 1 )
                             , mass_( 0 )
                             , tR_( 0 )
                             , criteria_( 0 )
{
}

QuanCompound::QuanCompound( const QuanCompound& t ) : uniqId_( t.uniqId_ )
                                                    , display_name_( t.display_name_ )
                                                    , formula_( t.formula_ )
                                                    , isISTD_( t.isISTD_ )
                                                    , idISTD_( t.idISTD_ )
                                                    , amounts_( t.amounts_ )
                                                    , description_( t.description_ )
                                                    , mass_( t.mass_ )
                                                    , tR_( t.tR_ )
                                                    , criteria_( t.criteria_ )
{
}

uint64_t
QuanCompound::uniqId() const
{
    return uniqId_;
}

void
QuanCompound::uniqId( uint64_t v )
{
    uniqId_ = v;
}

const wchar_t *
QuanCompound::display_name() const
{
    return display_name_.c_str();
}

void
QuanCompound::displya_name( const wchar_t * v )
{
    display_name_ = v;
}

const wchar_t *
QuanCompound::description() const
{
    return description_.c_str();
}

void
QuanCompound::description( const wchar_t * v )
{
    description_ = v;
}

const wchar_t *
QuanCompound::formula() const
{
    return formula_.c_str();
}

void
QuanCompound::formula( const wchar_t * v )
{
    formula_ = v;
}

bool
QuanCompound::isISTD() const
{
    return isISTD_;
}

void QuanCompound::isISTD( bool v )
{
    isISTD_ = v;
}

int32_t
QuanCompound::idISTD() const
{
    return idISTD_;
}

void
QuanCompound::idISTD( int32_t v )
{
    idISTD_ = v;
}

void
QuanCompound::levels( size_t v )
{
    amounts_.resize(v);
}

size_t
QuanCompound::levels() const
{
    return amounts_.size();
}

const double *
QuanCompound::amounts() const
{
    return amounts_.data();
}

void
QuanCompound::amounts( const double * d, size_t size )
{
    if ( size != amounts_.size() )
        amounts_.resize( size );
    std::copy( d, d + size, amounts_.begin() );
}