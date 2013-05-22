// -*- C++ -*-
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

#include "centroidmethod.hpp"

using namespace adcontrols;

CentroidMethod::~CentroidMethod(void)
{
}

CentroidMethod::CentroidMethod() : baselineWidth_(500.0)
                                 , rsConstInDa_(1.0)
                                 , rsPropoInPpm_(200.0)
                                 , rsTofInDa_(0.1)
                                 , rsTofAtMz_(600.0)
                                 , attenuation_(5.0)
                                 , bCentroidAreaIntensity_(true)
                                 , peakCentroidFraction_(0.5)
                                 , peakWidthMethod_(ePeakWidthTOF)
{
}

CentroidMethod::CentroidMethod(const CentroidMethod& t)
{
	operator=(t);
}

CentroidMethod&
CentroidMethod::operator = ( const CentroidMethod& rhs )
{
	peakWidthMethod( rhs.peakWidthMethod() );
	rsTofInDa( rhs.rsTofInDa() );
	rsTofAtMz( rhs.rsTofAtMz() );
	rsPropoInPpm( rhs.rsPropoInPpm() );
	rsConstInDa( rhs.rsConstInDa() );
	baselineWidth( rhs.baselineWidth() );
	attenuation( rhs.attenuation() );
	centroidAreaIntensity( rhs.centroidAreaIntensity() );
	peakWidthMethod( rhs.peakWidthMethod() );
	peakCentroidFraction( rhs.peakCentroidFraction() );

	return * this;
}

bool
CentroidMethod::operator == ( const CentroidMethod & rhs ) const
{
	return	peakWidthMethod() == rhs.peakWidthMethod() &&
				rsTofInDa() == rhs.rsTofInDa() &&
				rsTofAtMz() == rhs.rsTofAtMz() &&
				rsPropoInPpm() == rhs.rsPropoInPpm() &&
				rsConstInDa() == rhs.rsConstInDa() &&
				baselineWidth() == rhs.baselineWidth() &&
				attenuation() == rhs.attenuation() &&
				centroidAreaIntensity() == rhs.centroidAreaIntensity() &&
				peakWidthMethod() == rhs.peakWidthMethod() &&
				peakCentroidFraction() == rhs.peakCentroidFraction();
}

bool
CentroidMethod::operator != ( const CentroidMethod & rhs ) const
{
	return ! operator == ( rhs );
}

double 
CentroidMethod::baselineWidth() const
{
	return baselineWidth_;
}

double 
CentroidMethod::rsConstInDa() const
{
  return rsConstInDa_;
}

double 
CentroidMethod::rsPropoInPpm() const
{
	return rsPropoInPpm_;
}

double 
CentroidMethod::rsTofInDa() const
{
	return rsTofInDa_;
}

double 
CentroidMethod::rsTofAtMz() const
{
	return rsTofAtMz_;
}

double 
CentroidMethod::attenuation() const
{
	return attenuation_;
}

CentroidMethod::ePeakWidthMethod
CentroidMethod::peakWidthMethod() const
{
  return peakWidthMethod_;
}

bool
CentroidMethod::centroidAreaIntensity() const
{
  return bCentroidAreaIntensity_;
}

double
CentroidMethod::peakCentroidFraction() const
{
  return peakCentroidFraction_;
}

void  
CentroidMethod::baselineWidth(double v)
{
	baselineWidth_ = v;
}

void  
CentroidMethod::rsConstInDa(double v)
{
	rsConstInDa_ = v;
}

void  
CentroidMethod::rsPropoInPpm(double v)
{
	rsPropoInPpm_ = v;
}

void  
CentroidMethod::rsTofInDa(double v)
{
	rsTofInDa_ = v;
}

void  
CentroidMethod::rsTofAtMz(double v)
{
	rsTofAtMz_ = v;
}

void  
CentroidMethod::attenuation(double v)
{
	attenuation_ = v;
}

void
CentroidMethod::peakWidthMethod(ePeakWidthMethod e)
{
	peakWidthMethod_ = e;
}

void 
CentroidMethod::centroidAreaIntensity(bool f)
{
	bCentroidAreaIntensity_ = f;
}

void
CentroidMethod::peakCentroidFraction(double v)
{
	peakCentroidFraction_ = v;
}
