// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "traceaccessor.hpp"

using namespace adcontrols;

TraceAccessor::~TraceAccessor()
{
}

TraceAccessor::TraceAccessor() : pos_(-1)
                               , isConstantSampleInterval_( true )
                               , sampInterval_(0)
{
}

TraceAccessor::TraceAccessor( const TraceAccessor& t ) : traceX_( t.traceX_ )
                                                       , traceY_( t.traceY_ )
                                                       , events_( t.events_ )
                                                       , minTime_( t.minTime_ )
                                                       , isConstantSampleInterval_( t.isConstantSampleInterval_ )
                                                       , sampInterval_( t.sampInterval_ )
{
}

void
TraceAccessor::clear()
{
    traceX_.clear();
    traceY_.clear();
    events_.clear();
}

size_t
TraceAccessor::size() const
{
    return traceY_.size();
}

long
TraceAccessor::pos() const
{
    return pos_;
}

void
TraceAccessor::pos( long pos )
{
    pos_ = pos;
}

bool
TraceAccessor::isConstantSampleInterval() const
{
    return isConstantSampleInterval_;
}

void
TraceAccessor::sampInterval( unsigned long value )
{
    sampInterval_ = value;
}

unsigned long
TraceAccessor::sampInterval() const
{
    return sampInterval_;
}

const seconds_t&
TraceAccessor::getMinimumTime() const
{
    return minTime_;
}

void
TraceAccessor::setMinimumTime( const seconds_t& value )
{
    minTime_ = value;
}

void
TraceAccessor::push_back( double value, unsigned long events, const seconds_t& s )
{
    isConstantSampleInterval_ = false;
    traceX_.push_back( s.seconds );
    traceY_.push_back( value );
    events_.push_back( events );
}

void
TraceAccessor::push_back( double value, unsigned long events )
{
    traceY_.push_back( value );
    events_.push_back( events );
}

const double *
TraceAccessor::getIntensityArray() const
{
    if ( traceY_.empty() )
        return 0;
    return &traceY_[0];
}

const double *
TraceAccessor::getTimeArray() const   // null if isConstantSampleInterval is set
{
    if ( traceX_.empty() )
        return 0;
    return &traceX_[0];
}

const unsigned long *
TraceAccessor::getEventsArray() const
{
    if ( events_.empty() )
        return 0;
    return &events_[0];
}

