// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "peak.h"
using namespace adcontrols;

Peak::Peak( ) : parentId_( 0 )
              , peakid_( 0 )
              , baseid_( 0 )
              , appliedFunctions_( 0 )
              , peak_flags_( 0 )
              , startPos_( 0 )
              , topPos_( 0 )
              , endPos_( 0 )
              , startTime_( 0 )
              , topTime_( 0 )
              , endTime_( 0 )
              , startHeight_( 0 )
              , topHeight_( 0 )
              , endHeight_( 0 )
              , peakArea_( 0 )
              , peakHeight_( 0 )
              , capacityFactor_( 0 )
              , peakWidth_( 0 )
              , peakAmount_( 0 )
              , migrationTime_( 0 )
              , peakEfficiency_( 0 )
              , massOnColumn_( 0 )
              , percentArea_( 0 )
              , percentHeight_( 0 )
              , manuallyModified_( 0 )
{
}

Peak::Peak( const Peak& t ) : name_( t.name_ )
                            , parentId_( t.parentId_ )
                            , peakid_( t.peakid_ )
                            , baseid_( t.baseid_ )
                            , appliedFunctions_( t.appliedFunctions_ )
                            , peak_flags_( t.peak_flags_ )
                            , startPos_( t.startPos_ )
                            , topPos_( t.topPos_ )
                            , endPos_( t.endPos_ )
                            , startTime_( t.startTime_ )
                            , topTime_( t.topTime_ )
                            , endTime_( t.endTime_ )
                            , startHeight_( t.startHeight_ )
                            , topHeight_( t.topHeight_ )
                            , endHeight_( t.endHeight_ )
                            , peakArea_( t.peakArea_ )
                            , peakHeight_( t.peakHeight_ )
                            , asymmetry_( t.asymmetry_ )
                            , rs_( t.rs_ )
                            , ntp_( t.ntp_ )
                            , capacityFactor_( t.capacityFactor_ )
                            , peakWidth_( t.peakWidth_ )
                            , peakAmount_( t.peakAmount_ )
                            , migrationTime_( t.migrationTime_ )
                            , peakEfficiency_( t.peakEfficiency_ )
                            , massOnColumn_( t.massOnColumn_ )
                            , percentArea_( t.percentArea_ )
                            , percentHeight_( t.percentHeight_ )
                            , manuallyModified_( t.manuallyModified_ )
{
}

long
Peak::parentId() const
{
    return parentId_;
}

void
Peak::parentId(long id)
{
    parentId_ = id;
}

long
Peak::baseId() const
{
    return baseid_;
}

void
Peak::baseId(long id)
{
    baseid_ = id;
}

long
Peak::peakId() const
{
    return peakid_;
}

void
Peak::peakId(long id)
{
    peakid_ = id;
}

// long UserData() const;
// void UserData(long);

void
Peak::peakFlags(unsigned long v )
{
    peak_flags_ = v;
}

unsigned long
Peak::peakFlags() const
{
    return peak_flags_;
}

const std::wstring&
Peak::name() const
{
    return name_;
}

void
Peak::name( const std::wstring& name )
{
    name_ = name;
}

long
Peak::appliedFunctions() const
{
    return appliedFunctions_;
}

long
Peak::startPos() const
{
    return startPos_;
}

long
Peak::topPos() const
{
    return topPos_;
}

long
Peak::endPos() const
{
    return endPos_;
}

/*
void
Peak::startPos(long, const class CMCChromatogram &)
{
}

void
Peak::topPos(long, const class CMCChromatogram &)
{
}

void
Peak::endPos(long, const class CMCChromatogram &)
{
}
*/

void
Peak::startPos( long pos, peakheight_t h )
{
}

void
Peak::topPos(long pos,   peakheight_t h)
{
}

void
Peak::endPos(long pos,   peakheight_t h)
{
}

seconds_t
Peak::startTime() const
{
    return startTime_;
}

void
Peak::startTime( seconds_t newTime)
{
    startTime_ = newTime;
}

seconds_t
Peak::topTime() const
{
    return topTime_;
}

void
Peak::topTime( seconds_t newTime)
{
    topTime_ = newTime;
}

seconds_t
Peak::endTime() const
{
    return endTime_;
}

void
Peak::endTime( seconds_t newTime)
{
    endTime_ = newTime;
}

double
Peak::startHeight() const
{
    return startHeight_;
}

double
Peak::topHeight() const
{
    return topHeight_;
}

double
Peak::endHeight() const
{
    return endHeight_;
}

double
Peak::peakArea() const
{
    return peakArea_;
}

double
Peak::peakHeight() const
{
    return peakHeight_;
}

double
Peak::CapacityFactor() const
{
    return capacityFactor_;
}

double
Peak::PeakWidth() const
{
    return peakWidth_;
}

double
Peak::PeakAmount() const
{
    return peakAmount_;
}

double
Peak::MigrationTime() const
{
    return migrationTime_;
}

double
Peak::PeakEfficiency() const
{
    return peakEfficiency_;
}

double
Peak::MassOnColumn() const
{
    return massOnColumn_;
}

double
Peak::PercentArea() const
{
    return percentArea_;
}

double
Peak::PercentHeight() const
{
    return percentHeight_;
}

bool
Peak::IsManuallyModified() const
{
    return manuallyModified_;
}

void
Peak::SetManuallyModified()
{
    manuallyModified_ = true;
}
