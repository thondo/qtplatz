// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "spectrumwidget.hpp"
#include "zoomer.hpp"
#include "picker.hpp"
#include "plotcurve.hpp"
#include "annotation.hpp"
#include "annotations.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <qwt_plot_picker.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_picker_machine.h>
#include <boost/format.hpp>
#include <set>

using namespace adwplot;

namespace adwplot {
    namespace spectrumwidget {

        static Qt::GlobalColor color_table[] = {
            Qt::blue          // 0
            , Qt::red           // 1
            , Qt::green         // 2
            , Qt::cyan          // 3
            , Qt::magenta       // 4
            , Qt::yellow        // 5
            , Qt::darkRed       // 6
            , Qt::darkGreen     // 7
            , Qt::darkBlue      // 8
            , Qt::darkCyan      // 9
            , Qt::darkMagenta   // 10
            , Qt::darkYellow    // 11
            , Qt::darkGray      // 12
            , Qt::black         // 13
            , Qt::lightGray      // 14
            , Qt::white          // 15
            , Qt::transparent    // 16
            , Qt::transparent    // 17
        };

        class xSeriesData : public QwtSeriesData<QPointF>, boost::noncopyable {
        public:
            virtual ~xSeriesData() {
            }

            xSeriesData( const adcontrols::MassSpectrum& ms
                         , const QRectF& rc
                         , bool axisTime ) : ms_( ms )
                                           , rect_( rc )
                                           , axisTime_( axisTime ) {
            }

            size_t size() const override { 
                if ( ! indecies_.empty() )
                    return indecies_.size();
                return ms_.size();
            }

            QPointF sample( size_t idx ) const override {
                if ( ! indecies_.empty() && idx < indecies_.size() ) // if colored
                    idx = indecies_[ idx ];
                using namespace adcontrols::metric;
                if ( axisTime_ )
                    return QPointF( scale<double, micro>( ms_.getTime( idx  )), ms_.getIntensity( idx ) );
                else
                    return QPointF( ms_.getMass( idx ), ms_.getIntensity( idx ) );
            }

            QRectF boundingRect() const override {
                return rect_;
            }

            size_t make_color_index( unsigned char color ) {
                const unsigned char * colors = ms_.getColorArray();
                for ( size_t i = 0; i < ms_.size(); ++i ) {
                    if ( color == colors[i] )
                        indecies_.push_back( i );
                }
                return indecies_.size();
            }

        private:
            QRectF rect_;
            const adcontrols::MassSpectrum& ms_;
            std::vector< size_t > indecies_; // if centroid with color, 
            bool axisTime_;
        };
	
        class TraceData {
        public:
            TraceData( int idx ) : idx_( idx ) {  }
            TraceData( const TraceData& t ) : idx_( t.idx_ ), curves_( t.curves_ ) {   }
            ~TraceData();
            void setData( Dataplot& plot
                          , const std::shared_ptr< adcontrols::MassSpectrum>&
                          , QRectF&, SpectrumWidget::HorizontalAxis );
            std::pair<double, double> y_range( double left, double right ) const;
            
        private:
            void setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& );
            void setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& );

            int idx_;
            std::vector< PlotCurve > curves_;
            std::weak_ptr< adcontrols::MassSpectrum > pSpectrum_;
			bool isTimeAxis_;
        };
        
    } // namespace spectrumwidget

    struct SpectrumWidgetImpl {
        std::weak_ptr< adcontrols::MassSpectrum > centroid_;  // for annotation
        std::vector< Annotation > annotations_;
        std::vector< spectrumwidget::TraceData > traces_;

        void clear();
        void update_annotations( Dataplot&, const std::pair<double, double>& );
		void clear_annotations();
    };

} // namespace adwplot

SpectrumWidget::~SpectrumWidget()
{
    delete impl_;
}

SpectrumWidget::SpectrumWidget(QWidget *parent) : Dataplot(parent)
                                                , impl_( new SpectrumWidgetImpl )
                                                , autoYZoom_( true ) 
                                                , haxis_( HorizontalAxisMass )
{
    zoomer2_.reset();
	zoomer1_->autoYScale( autoYZoom_ );

    setAxisTitle(QwtPlot::xBottom, "m/z");
    setAxisTitle(QwtPlot::yLeft, "Intensity");

    // handle zoom rect by this
    if ( zoomer1_ ) {
        connect( zoomer1_.get(), SIGNAL( zoom_override( QRectF& ) ), this, SLOT( override_zoom_rect( QRectF& ) ) );
		QwtPlotZoomer * p = zoomer1_.get();
		connect( p, SIGNAL( zoomed( const QRectF& ) ), this, SLOT( zoomed( const QRectF& ) ) );
	}
	if ( picker_ ) {
		connect( picker_.get(), SIGNAL( moved( const QPointF& ) ), this, SLOT( moved( const QPointF& ) ) );
		connect( picker_.get(), SIGNAL( selected( const QRectF& ) ), this, SLOT( selected( const QRectF& ) ) );
		picker_->setEnabled( true );
	}
}

void
SpectrumWidget::override_zoom_rect( QRectF& rc )
{
    if ( autoYZoom_ ) {
        using spectrumwidget::TraceData;
        double bottom = rc.bottom();
        double top = rc.top();
        for ( const TraceData& trace: impl_->traces_ ) {
            std::pair<double, double> y = trace.y_range( rc.left(), rc.right() );
            if ( bottom > y.first )
                bottom = y.first;
            if ( top < y.second )
                top = y.second; // rc.setTop( y.second );
        }
        rc.setBottom( bottom );
        rc.setTop( top + ( top - bottom ) * 0.12 );  // increase 12% for annotation
    }
}

void
SpectrumWidget::zoom( const QRectF& rect )
{
	Dataplot::zoom( rect );
    impl_->update_annotations( *this, std::make_pair<>( rect.left(), rect.right() ) );
}
void
SpectrumWidget::zoomed( const QRectF& rect )
{
    impl_->update_annotations( *this, std::make_pair<>( rect.left(), rect.right() ) );
}

void
SpectrumWidget::moved( const QPointF& pos )
{
	(void)pos;
}

void
SpectrumWidget::selected( const QPointF& pos )
{
	emit onSelected( pos );
}

void
SpectrumWidget::selected( const QRectF& rect )
{
	emit onSelected( rect );
}

void
SpectrumWidget::clear()
{
    impl_->clear();
    zoomer1_->setZoomBase();
}

void
SpectrumWidget::setAxis( HorizontalAxis haxis )
{
    clear();
    haxis_ = haxis;
    setAxisTitle(QwtPlot::xBottom, haxis_ == HorizontalAxisMass ? "m/z" : "Time(microseconds)");
}

void
SpectrumWidget::setData( const std::shared_ptr< adcontrols::MassSpectrum >& ptr, int idx, bool yaxis2 )
{
    using spectrumwidget::TraceData;

    bool addedTrace = impl_->traces_.size() <= size_t( idx );

    while ( int( impl_->traces_.size() ) <= idx ) 
		impl_->traces_.push_back( TraceData( impl_->traces_.size() ) );

    TraceData& trace = impl_->traces_[ idx ];

    QRectF rect;
    trace.setData( *this, ptr, rect, haxis_ );

    setAxisScale( QwtPlot::xBottom, rect.left(), rect.right() );
    setAxisScale( yaxis2 ? QwtPlot::yRight : QwtPlot::yLeft, rect.top(), rect.bottom() );

    QRectF z = zoomer1_->zoomRect();
    
    zoomer1_->setZoomBase();
    if ( ! addedTrace )
        zoomer1_->zoom( z );

    // todo: annotations
    if ( ptr->isCentroid() ) {
        impl_->centroid_ = ptr;
        impl_->clear_annotations();
        impl_->update_annotations( *this, ptr->getAcquisitionMassRange() );
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace adwplot::spectrumwidget;

TraceData::~TraceData()
{
    curves_.clear();
}

void
TraceData::setProfileData( Dataplot& plot, const adcontrols::MassSpectrum& ms, const QRectF& rect )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( ms );
    for ( auto& seg: segments ) {
        curves_.push_back( PlotCurve( plot ) );

        PlotCurve &curve = curves_.back();
        if ( idx_ )
            curve.p()->setPen( QPen( color_table[ idx_ ] ) );
        curve.p()->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
    }
}

void
TraceData::setCentroidData( Dataplot& plot, const adcontrols::MassSpectrum& _ms, const QRectF& rect )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( _ms );

    for ( auto& seg: segments ) {
        if ( const unsigned char * colors = seg.getColorArray() ) {
            std::set< unsigned char > color;
            for ( size_t i = 0; i < seg.size(); ++i )
                color.insert( colors[i] );

            for ( const auto& c: color ) {
                xSeriesData * xp = new xSeriesData( seg, rect, isTimeAxis_ );
                if ( size_t ndata = xp->make_color_index( c ) ) {
                    curves_.push_back( PlotCurve( plot ) );
                    PlotCurve &curve = curves_.back();
                    curve.p()->setData( xp );
                    curve.p()->setPen( QPen( color_table[ c ] ) );
                    curve.p()->setStyle( QwtPlotCurve::Sticks );
                }
            }

        } else {
            curves_.push_back( PlotCurve( plot ) );
            PlotCurve &curve = curves_.back();
            curve.p()->setPen( QPen( color_table[ 0 ] ) );
            curve.p()->setData( new xSeriesData( seg, rect, isTimeAxis_ ) );
            curve.p()->setStyle( QwtPlotCurve::Sticks );
        }
    }
}

void
TraceData::setData( Dataplot& plot
                    , const std::shared_ptr< adcontrols::MassSpectrum >& ms
                    , QRectF& rect
                    , SpectrumWidget::HorizontalAxis haxis )
{
    curves_.clear();

    pSpectrum_ = ms;
    isTimeAxis_ = haxis == SpectrumWidget::HorizontalAxisTime;

	double top = adcontrols::segments_helper::max_intensity( *ms );
    double bottom = adcontrols::segments_helper::min_intensity( *ms );
	
	if ( ms->isCentroid() )
		bottom = 0;
	top = top + ( top - bottom ) * 0.12; // add 12% margine for annotation

    if ( isTimeAxis_ ) {

        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
        std::pair< double, double > time_range = std::make_pair( std::numeric_limits<double>::max(), 0 );
        for ( auto& m: segments ) {
            std::pair< double, double > range = m.getMSProperty().instTimeRange();
            time_range.first = std::min( time_range.first, range.first );
            time_range.second = std::max( time_range.second, range.second );
        }

        using adcontrols::metric::micro;
        using adcontrols::metric::scale;
        rect.setCoords( scale<double, micro>(time_range.first), bottom, scale<double, micro>(time_range.second), top );

    } else {

        const std::pair< double, double >& mass_range = ms->getAcquisitionMassRange();
        rect.setCoords( mass_range.first, bottom, mass_range.second, top );

    }
    if ( ms->isCentroid() ) { // sticked
        setCentroidData( plot, *ms, rect );
    } else { // Profile
        setProfileData( plot, *ms, rect );
    }
}

std::pair< double, double >
TraceData::y_range( double left, double right ) const
{
    double top = 100;
    double bottom = -10;

    if ( const std::shared_ptr< adcontrols::MassSpectrum > ms = pSpectrum_.lock() ) {
        if ( ms->isCentroid() ) {
            adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *ms );
            for ( auto& seg: segments ) {
				const double * x = isTimeAxis_ ? seg.getTimeArray() : seg.getMassArray();

                size_t idleft = std::distance( x, std::lower_bound( x, x + seg.size(), left ) );
                size_t idright = std::distance( x, std::lower_bound( x, x + seg.size(), right ) );

                if ( idleft < idright ) {
                    const double * y = seg.getIntensityArray();
                    
                    double min = *std::min_element( y + idleft, y + idright );
                    double max = *std::max_element( y + idleft, y + idright );
                    
                    bottom = std::min( bottom, min );
                    top = std::max( top, max );
                }
            }
        }
    }
    return std::make_pair<>(bottom, top);
}

void
SpectrumWidgetImpl::clear_annotations()
{
	annotations_.clear();
}

void
SpectrumWidgetImpl::update_annotations( Dataplot& plot
                                       , const std::pair<double, double>& range )
{
    using adportable::array_wrapper;

    typedef std::tuple< size_t, size_t, int, double, double > peak; // fcn, idx, color, mass, intensity
    enum { c_fcn, c_idx, c_color, c_intensity, c_mass };

    if ( auto centroid = centroid_.lock() ) {

        std::vector< peak > peaks;
        adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( *centroid );
        
        adcontrols::annotations auto_annotations;
        adcontrols::annotations annotations;
        
        double max_y = adcontrols::segments_helper::max_intensity( *centroid );
        //double h_limit = max_y / 25;
        //double w_limit = std::fabs( range.second - range.first ) / 100.0;
        
        for ( size_t fcn = 0; fcn < segments.size(); ++fcn ) {
            const adcontrols::MassSpectrum& ms = segments[ fcn ];
            const unsigned char * colors = ms.getColorArray();
            
            array_wrapper< const double > masses( ms.getMassArray(), ms.size() );
            size_t beg = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.first ) );
            size_t end = std::distance( masses.begin(), std::lower_bound( masses.begin(), masses.end(), range.second ) );
            
            if ( beg < end ) {
                const adcontrols::annotations& attached = ms.get_annotations();
                for ( auto& a : attached ) {
                    if ( ( int(beg) <= a.index() && a.index() <= int(end) ) || ( range.first < a.x() && a.x() < range.second ) ) {
                        annotations << a;
                    }
                }
                
                // generate auto-annotation
                for ( size_t idx = beg; idx <= end; ++idx ) {
                    if ( std::find_if( attached.begin()
                                       , attached.end()
                                       , [idx]( const adcontrols::annotation& a ){ return a.index() == idx; } ) == attached.end() ) {
                        int pri = ms.getIntensity( idx ) / max_y * 1000;
                        if ( colors )
                            pri *= 100;
                        adcontrols::annotation annot( ( boost::wformat( L"%.4lf" ) % ms.getMass( idx ) ).str()
                                                      , ms.getMass( idx ), ms.getIntensity( idx ), ( fcn << 24 | idx ), pri );
                        auto_annotations << annot;
                    }
                }
            }
        }
        
        auto_annotations.sort();
        annotations.sort();
        
        annotations_.clear();
        Annotations annots(plot, annotations_);
        
        size_t n = 0;
        for ( const auto& a: annotations ) {
            if ( ++n <= 20 ) {
                Annotation anno = annots.add( a.x(), a.y(), a.text() );
                anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );
            }
        }
        
        for ( const auto& a: auto_annotations ) {
            if ( ++n <= 20 ) {
                Annotation anno = annots.add( a.x(), a.y(), a.text() );
                anno.setLabelAlighment( Qt::AlignTop | Qt::AlignCenter );
            }
        }
    }
}

void
SpectrumWidgetImpl::clear()
{
    centroid_.reset();
    annotations_.clear();
    traces_.clear();
}
