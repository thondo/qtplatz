/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#include "imfilter.hpp"
#include "transform.hpp"
#include "applycolormap.hpp"
#include "dft2d.hpp"
#include <adportable/debug.hpp>
#include <QImage>
#if HAVE_OPENCV
# include <opencv2/core/core.hpp>
# include <opencv2/opencv.hpp>
# include <opencv2/imgproc/imgproc.hpp>
#endif
#include <boost/numeric/ublas/matrix.hpp>

namespace advision {

    template< typename T > struct find_minmax {
        inline std::pair< T, T > operator()( const boost::numeric::ublas::matrix< T >& m ) const {
            std::pair<T, T> mm = { m(0, 0), m(0, 0) };
            for ( int i = 0; i < m.size1(); ++i ) {
                for ( int j = 0; j < m.size2(); ++j ) {
                    mm.first = std::min( mm.first, m( i, j ) );
                    mm.second = std::max( mm.second, m( i, j ) );
                }
            }
            return mm;
        }
    };

    namespace opencv {
        cv::Size Size( const imBlur& blur ) { return cv::Size( blur.size.first, blur.size.second ); }
    };
    
}

class QPaintEvent;

//using namespace advision;

namespace advision {

///// GrayScale
    template<>
    template<>
    QImage
    imfilter< QImage, imGrayScale >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };

        return ApplyColorMap_< QImage >(2, __levels.data(), __colors.data() )( m, float( scaleFactor ) );
    }

////// Blur
    template<>
    template<>
    QImage
    imfilter< QImage, imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        cv::Mat mat = ApplyColorMap_< cv::Mat >()( m, float( scaleFactor ) );

        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );

        cv::Size sz = size_ == 1 ? opencv::Size( std::get<0>( algos_ ) ) : cv::Size( 5, 5 );
        
        cv::GaussianBlur( mat, mat, cv::Size( 5, 5 ), 0, 0 );

        return transform_< QImage >()( mat );
    }

////// ColorMap matrix<double>
    template<>
    template<>
    QImage
    imfilter< QImage, imColorMap >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        return ApplyColorMap_<QImage>()( m, scaleFactor );
    }

    //////////////////
    //////////////////
    //////////////////
    template<>
    template<>
    QImage
    imfilter< QImage
              , imGrayScale
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };
        
        cv::Mat mat = ApplyColorMap_< cv::Mat >( 2, __levels.data(), __colors.data() )( m, float( scaleFactor ) );
        
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );

        cv::Size sz = size_ == 2 ? opencv::Size( std::get<1>( algos_ ) ) : cv::Size( 5, 5 );
        
        cv::GaussianBlur( mat, mat, sz, 0, 0 );

        return transform_< QImage >()( mat );
    }

    template<>
    template<>
    QImage
    imfilter< QImage
              , imColorMap
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        cv::Mat mat = ApplyColorMap_< cv::Mat >()( m, float( scaleFactor ) );
        
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );
        
        cv::Size sz = size_ == 2 ? opencv::Size( std::get<1>( algos_ ) ) : cv::Size( 5, 5 );

        cv::GaussianBlur( mat, mat, sz, 0, 0 );

        return transform_< QImage >()( mat );
    }

    //////// DFT
    template<>
    template<>
    QImage
    imfilter< QImage
              , imColorMap
              , imDFT >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        auto mat = transform_< cv::Mat >()( m ); // -> float *
        mat = dft2d().dft( mat );

        return ApplyColorMap_< QImage >()( mat, float( scaleFactor ) );
    }


    template<>
    template<>
    QImage
    imfilter< QImage
              , imGrayScale
              , imDFT >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        auto mat = transform_< cv::Mat >()( m ); // -> float *

        mat = dft2d().dft( mat );
        
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };

        return ApplyColorMap_< QImage >( 2, __levels.data(), __colors.data() )( mat, float( scaleFactor ) );
    }


    // ColorMap + DFT + Blur
    template<>
    template<>
    QImage
    imfilter< QImage
              , imColorMap
              , imDFT
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        // --> cv::Mat_<float>
        auto mat = transform_< cv::Mat >()( m ); // -> float *

        // --> DFT
        mat = dft2d().dft( mat );

        // --> ColorMap cv::Mat
        mat = ApplyColorMap_< cv::Mat >()( mat, float( scaleFactor ) );

        // --> Blur cv::Mat
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );
        cv::Size sz = size_ == 2 ? opencv::Size( std::get<2>( algos_ ) ) : cv::Size( 5, 5 );
        cv::GaussianBlur( mat, mat, sz, 0, 0 );

        // wrap up
        return transform_< QImage >()( mat );
    }

    // GrayScale + DFT + Blur
    template<>
    template<>
    QImage
    imfilter< QImage
              , imGrayScale
              , imDFT
              , imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >& m, double scaleFactor ) const
    {
        // --> cv::Mat_<float>
        auto mat = transform_< cv::Mat >()( m ); // -> float *

        // --> DFT
        mat = dft2d().dft( mat );

        // --> GrayScale cv::Mat
        const std::vector< float > __levels{ 0.0, 1.0 };
        const std::vector< float > __colors{ 0.0, 1.0,   0.0, 1.0,   0.0, 1.0 };
        mat = ApplyColorMap_< cv::Mat >( 2, __levels.data(), __colors.data() )( mat, float( scaleFactor ) );

        // --> Blur cv::Mat
        if ( mat.rows < 256 )
            cv::resize( mat, mat, cv::Size(0,0), 256/mat.cols, 256/mat.rows, CV_INTER_LINEAR );
        cv::Size sz = size_ == 2 ? opencv::Size( std::get<2>( algos_ ) ) : cv::Size( 5, 5 );
        cv::GaussianBlur( mat, mat, sz, 0, 0 );

        // wrap up
        return transform_< QImage >()( mat );
    }
    
}
