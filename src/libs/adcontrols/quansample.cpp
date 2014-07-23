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

#include "quansample.hpp"
#include <algorithm>

using namespace adcontrols;

QuanSample::~QuanSample()
{
}

QuanSample::QuanSample() : sampleType_( SAMPLE_TYPE_UNKNOWN )
                         , inletType_( Infusion )
                         , level_(0)
                         , istdId_(-1)
                         , injVol_(0)
                         , amountsAdded_( 1.0 )
                         , istd_( 0 )
                         , dataGeneration_( ASIS )
                         , scan_range_( 0, -1 )
{
}

QuanSample::QuanSample( const QuanSample& t ) : name_( t.name_ )
                                              , dataType_( t.dataType_ )
                                              , dataSource_( t.dataSource_ )
                                              , dataGuid_( t.dataGuid_ )
                                              , sampleType_( t.sampleType_ )
                                              , level_( t.level_ )
                                              , istdId_( t.istdId_ )
                                              , injVol_( t.injVol_ )
                                              , amountsAdded_( t.amountsAdded_ )
                                              , istd_( t.istd_ )
    , quanResult_( t.quanResult_ )
    , dataGeneration_( t.dataGeneration_ )
    , scan_range_( t.scan_range_ )
{
}

const wchar_t *
QuanSample::name() const
{
    return name_.c_str();
}

void
QuanSample::name( const wchar_t * v )
{
    name_ = v;
}

const wchar_t *
QuanSample::dataSource() const
{
    return dataSource_.c_str();
}

void
QuanSample::dataSource( const wchar_t * v )
{
    dataSource_ = v;
}


const wchar_t *
QuanSample::dataGuid() const
{
    return dataGuid_.c_str();
}

void
QuanSample::dataGuid( const wchar_t * v )
{
    dataGuid_ = v;
}


QuanSample::QuanSampleType
QuanSample::sampleType() const
{
    return sampleType_;
}

void
QuanSample::sampleType( QuanSampleType v )
{
    sampleType_ = v;
}
        
int32_t
QuanSample::istdId() const
{
    return istdId_;
}

void
QuanSample::istdId( int32_t v )
{
    istdId_ = v;
}

        
int32_t
QuanSample::level() const
{
    return level_;
}

void
QuanSample::level( int32_t v )
{
    level_ = v;
}
        
double
QuanSample::injVol() const
{
    return injVol_;
}

void
QuanSample::injVol( double v )
{
    injVol_ = v;
}
        
double
QuanSample::addedAmounts() const
{
    return amountsAdded_;
}

void
QuanSample::addedAmounts( double v )
{
    amountsAdded_ = v;
}

const std::vector< quan::ISTD >&
QuanSample::istd() const
{
    return istd_;
}

void
QuanSample::istd( const std::vector< quan::ISTD > & istd )
{
    istd_ = istd;
    if ( ! istd_.empty() )
        std::sort( istd_.begin(), istd_.end(), []( const quan::ISTD& a, const quan::ISTD& b ){ return a.id_ < b.id_; });
}

QuanSample&
QuanSample::operator << ( const quan::ISTD& t )
{
    auto it = std::lower_bound( istd_.begin(), istd_.end(), t.id_, [](const quan::ISTD& a, uint32_t id){ return a.id_ < id; });
    if ( it->id_ == t.id_ )  // id must be unique
        *it = t;
    else
        istd_.insert( it, t );
    return *this;
}