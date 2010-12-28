//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "peaks.h"
#include "peak.h"
#include "baseline.h"
#include "baselines.h"

using namespace adcontrols;

Peaks::~Peaks()
{
}

Peaks::Peaks() : baselines_( new Baselines )
{
}

Peaks::Peaks( const Peaks& t ) : baselines_( new Baselines( *t.baselines_ ) )
                               , peaks_( t.peaks_ )
{
}

void
Peaks::add( const Peak& pk )
{
    peaks_.push_back( pk );
}
