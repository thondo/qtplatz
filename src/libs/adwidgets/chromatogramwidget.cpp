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

#include "chromatogramwidget.h"
#include <adcontrols/chromatogram.h>
#include <adcontrols/descriptions.h>
#include <adcontrols/peaks.h>
#include <adcontrols/peak.h>
#include <adcontrols/baselines.h>
#include <adcontrols/baseline.h>
#include <adwidgets/axis.h>
#include <adwidgets/traces.h>
#include <adwidgets/trace.h>
#include <adwidgets/titles.h>
#include <adwidgets/title.h>
#include <adwidgets/peaks.h>
#include <adwidgets/peak.h>
#include <adwidgets/baselines.h>
#include <adwidgets/baseline.h>
#include <adwidgets/font.h>
#include <adwidgets/colorindices.h>
#include <adutils/dataplothelper.h>

using namespace adwidgets::ui;

ChromatogramWidget::~ChromatogramWidget()
{
}

ChromatogramWidget::ChromatogramWidget(QWidget *parent) : TraceWidget(parent)
{
    this->axisX().text( L"Time(min)");
    this->axisY().text( L"(uV)");
    this->title( 0, L"Chromatogram" );
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c )
{
    setData( c, 0 );
}

void
ChromatogramWidget::setData( const adcontrols::Chromatogram& c, int idx, bool yaxis2 )
{
	while ( int(traces().size()) <= idx )
		traces().add();

	if ( idx < 3 ) {
		adwidgets::ui::Title title = titles()[idx];
        const adcontrols::Descriptions& desc_v = c.getDescriptions();
		std::wstring ttext;
		for ( size_t i = 0; i < desc_v.size(); ++i ) {
			if ( i != 0 )
				ttext += L" :: ";
			ttext += desc_v[i].text();
		}
		title.text( ttext );
        title.visible( true );
	}

    adwidgets::ui::Trace trace = traces()[idx];

    std::pair<double, double> xrange( adcontrols::Chromatogram::toMinutes( c.timeRange() ) );
    std::pair<double, double> yrange( c.getMinIntensity(), c.getMaxIntensity() );

    std::pair<double, double> drange;
    drange.first = yrange.first < 0 ? yrange.first : 0;
	drange.second = yrange.second - drange.first < 50 ? 50 : yrange.second;

    traces().visible(false);

    this->display_range_x( xrange );
	if ( yaxis2 ) {
		display_range_y2( drange );
		trace.YAxis( Trace::Y2 );
		axisY2().visible( true );
	} else {
		display_range_y( drange );
		trace.YAxis( Trace::Y1 );
	}

    trace.colorIndex(2 + idx);
    trace.traceStyle( Trace::TS_Connected );
    if ( c.getTimeArray() ) {
        trace.setXYDirect( c.size(), c.getTimeArray(), c.getIntensityArray() );
    } else {
        boost::scoped_array<double> timearray( new double [ c.size() ] );
        for ( size_t i = 0; i < c.size(); ++i )
            timearray[i] = adcontrols::Chromatogram::toMinutes( c.timeFromDataIndex( i ) );
        trace.setXYDirect( c.size(), timearray.get(), c.getIntensityArray() );
    }

    trace.visible( true );
    traces().visible( true );

    setPeaks( c.peaks(), c.baselines(), trace );
}

void
ChromatogramWidget::setPeaks(  const adcontrols::Peaks& peaks
                             , const adcontrols::Baselines& baselines
                             , Trace& trace )
{
    trace.autoAnnotation( false );
    trace.font().size( 80000 );
    trace.font().bold( false );

    ui::Peaks pks = trace.peaks();
    pks.clear();
    for ( adcontrols::Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it ) {
        ui::Peak uipk = pks.add();
        adutils::DataplotHelper::copy( uipk, *it );
        uipk.colorIndex( this->getColorIndex( adwidgets::ui::CI_PeakMark ) );
    }
    pks.visible(true);

    ui::Baselines bss = trace.baselines();
    for ( adcontrols::Baselines::vector_type::const_iterator it = baselines.begin(); it != baselines.end(); ++it ) {
        ui::Baseline bs = bss.add();
        adutils::DataplotHelper::copy( bs, *it );
        bs.colorIndex( this->getColorIndex( adwidgets::ui::CI_BaseMark ) );
    }
    bss.visible( true );

}

#if 0
void CChromatogramView::UpdatePeaks(long traceIdx, bool active)
{
	SAGRAPHICSLib::ISADPPeaksPtr piPeaks = piTrace->GetPeaks();
	if (piPeaks == NULL)
		return;
	piPeaks->Clear();
	piPeaks->Visible = VARIANT_FALSE;

	SAGRAPHICSLib::ISADPBaselinesPtr piBaselines = piTrace->GetBaselines();
	if (piBaselines == NULL)
		return;
	piBaselines->Clear();
	piBaselines->Visible = VARIANT_FALSE;

	SAGRAPHICSLib::ISADPAnnotationsPtr piAnnos = piTrace->GetAnnotations();
	if (piAnnos == NULL)
		return;
	piAnnos->Clear();
	piAnnos->Visible = VARIANT_FALSE;
	piAnnos->XDecimals = 3;

	const CMCBaselines & baselines = ptr->baselines();
	for (CMCBaselines::vector_type::const_iterator ibs = baselines.begin(); ibs != baselines.end(); ++ibs)
	{
		//debug_trace(LOG_DEBUG, "baseline(id=%d, %lf -- %lf(s)", ibs->BaseId(), ibs->StartTime(), ibs->StopTime());
		SAGRAPHICSLib::ISADPBaselinePtr piBaseline = piBaselines->Add();
		if (piBaseline)
		{
			piBaseline->StartX = ibs->StartTime();
			piBaseline->StartY = ibs->StartHeight();
			piBaseline->EndX   = ibs->StopTime();
			piBaseline->EndY   = ibs->StopHeight();
			piBaseline->ColorIndex = 7;
			piBaseline->Visible = VARIANT_TRUE;
		}
	}

	short clrIndex = ((short(traceIdx) - 1) % 18) + 2;

	const CMCPeaks & peaks = ptr->peaks();
	for (CMCPeaks::vector_type::const_iterator ipk = peaks.begin(); ipk != peaks.end(); ++ipk)
	{
		//if (ipk->baseId() == ibs->BaseId())
		//{
			//#pragma message ("--- !!!workaround, To draw baselines and peak annotations, time in a CMCPeak is divided by 1000. Taka ---")
			SAGRAPHICSLib::ISADPPeakPtr piPeak = piPeaks->Add();
			if (piPeak)
			{
				piPeak->StartX = ipk->startTime();
				piPeak->StartY = ipk->startHeight();
				piPeak->CenterX = ipk->topTime();
				piPeak->CenterY = ipk->topHeight();
				piPeak->EndX = ipk->endTime();
				piPeak->EndY = ipk->endHeight();

				piPeak->StartMarkerStyle = SAGRAPHICSLib::PM_DownStick;
				piPeak->EndMarkerStyle = SAGRAPHICSLib::PM_DownStick;
				piPeak->CenterMarkerStyle = SAGRAPHICSLib::PM_UpStick;

				piPeak->ColorIndex = clrIndex;
				piPeak->StartMarkerColorIndex = clrIndex;
				piPeak->EndMarkerColorIndex = clrIndex;
				piPeak->CenterMarkerColorIndex = clrIndex;

				//piPeak->PeakFill = VARIANT_TRUE;
				//piPeak->DrawBaselineCenter = VARIANT_TRUE;
				piPeak->Marked = VARIANT_TRUE;
				piPeak->Visible = VARIANT_TRUE;
			}

			SAGRAPHICSLib::ISADPAnnotationPtr piAnno = piAnnos->Add();
			if (piAnno)
			{
				std::wostringstream o;
				std::wstring value1 = _datum_t<double, wchar_t>(ipk->topTime(), 3);
				//std::wstring value2 = _datum_t<double, wchar_t>(ipk->topHeight(), 0);
				std::wstring value2 = ipk->name();

				if (value2.length() > 0)
					value1 += (std::wstring(L"\n") + value2);

				//o << value1 << L"\n" << value2;

				piAnno->x = ipk->topTime();
				piAnno->y = ipk->topHeight();
				//piAnno->Value = CComVariant(o.str().c_str());
				piAnno->Value = CComVariant(value1.c_str());
				piAnno->ColorIndex = clrIndex;
			}

			//debug_trace(LOG_DEBUG, "\tpeak( tR = %8.3lf min H=%8.lf NTP=%8.lf)", ipk->topTime(), ipk->peakHeight(), ipk->NTP() );
		//}
	}

	piPeaks->Visible = VARIANT_TRUE;
	piBaselines->Visible = VARIANT_TRUE;
	piAnnos->Visible = VARIANT_TRUE;
}
#endif
