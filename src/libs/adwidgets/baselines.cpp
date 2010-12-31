/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#include "baselines.h"
#include "baseline.h"
#include "import_sagraphics.h"

using namespace adwidgets::ui;

Baselines::~Baselines()
{
  if ( pi_ )
    pi_->Release();
}

Baselines::Baselines( SAGRAPHICSLib::ISADPBaselines * pi ) : pi_(pi)
{
  pi_->AddRef();
}

Baselines::Baselines( const Baselines& t )
{
   if ( t.pi_ )
     t.pi_->AddRef(); // AddRef first, in order to avoid unexpected release when self assignment happens
   if ( pi_ )
     pi_->Release();
   pi_ = t.pi_;
}

Baseline 
Baselines::operator [] ( int idx )
{
    return Baseline( pi_->Item[ idx + 1 ] );
}

size_t
Baselines::size() const
{
    return pi_->Count;
}

bool 
Baselines::visible() const
{
    return internal::variant_bool::to_native( pi_->Visible );
}

void 
Baselines::visible( bool value )
{
    pi_->Visible = internal::variant_bool::to_variant( value );
}

short 
Baselines::LineWidth() const
{
    return pi_->LineWidth;
}

void 
Baselines::LineWidth( short value )
{
    pi_->LineWidth = value;
}

LineStyle 
Baselines::lineStyle() const
{
    return static_cast< LineStyle >( pi_->GetLineStyle() );
}

void 
Baselines::lineStyle( LineStyle value )
{
    pi_->PutLineStyle( static_cast< SAGRAPHICSLib::LineStyle >( value ) );
}

Baseline 
Baselines::add()
{
    return Baseline( pi_->Add() );
}

bool 
Baselines::remove ( int idx )
{
    return pi_->Remove( idx + 1 ) == S_OK;
}

bool 
Baselines::clear ()
{
    return pi_->Clear() == S_OK;
}


