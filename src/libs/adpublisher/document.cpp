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

#include "document.hpp"
#include <xmlparser/pugixml.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

using namespace adpublisher;

document::document() : doc_( std::make_shared< pugi::xml_document >() )
{
    if ( auto comment = doc_->append_child( pugi::node_comment ) ) {
        comment.set_value( "Copyright(C) 2010-2014, MS-Cheminformatics LLC, All rights reserved." );
    }
    if ( auto article = doc_->append_child( "article" ) ) {

        if ( auto title = article.append_child( "title" ) ) {
            title.text().set( "Qualitative and quantitative accuracy on LC/TOF-MS with newly developped targeting algorihm" );
        }


        if ( auto author = article.append_child( "author" ) ) {
            author.text().set( "Kristie C. Cloos, Katherine M. Schroeder and Toshinobu Hondo" );
        }

        if ( auto sec = article.append_child( "section" ) ) {
            sec.append_attribute( "id" ) = "section_1";

            if ( auto title = sec.append_child( "title" ) )
                title.text().set( "Introduction" );

            if ( auto para = sec.append_child( "paragraph" ) ) {
                para.text().set( "\
A single LC/TOFMS offers the advantage of a full scan which, by allowing all analytes to reach the detector, gives way for multiple applications. However, analyzing the data acquired from a full scan can quickly become a limiting factor in high-throughput (HT) analysis. Accurate results in qualitative and quantitative high-performance liquid chromatographic (HPLC) analysis are fundamentally dependant on the degree of separation obtained for the component peak. In a full scan, analytes can experience suppression as they might still contain contaminates which can lead to inaccurate intensities and co-eluted compounds. Another disadvantage is the inability to avoid drift which makes it difficult to target the analytes of choice using current targeting software where a time range must be entered for each compound. \
The work presented here shows how using a newly developed targeting algorithm allows an isolated mass spectrum to be targeted from a complex sample matrix. This technique allows a single LC/TOFMS to become an instrument that can be used for quantitative HT analysis." );
            }

        }
    }
}

bool
document::save_file( const char * filepath ) const
{
    return doc_->save_file( filepath );
}

bool
document::load_file( const char * filepath )
{
    return doc_->load_file( filepath );
}

bool
document::save( std::ostream& o ) const
{
    doc_->save( o );
    return true;
}

bool
document::save( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    doc_->save( device );
    return true;
}

bool
document::load( const char * xml )
{
    return doc_->load( xml );
}

std::shared_ptr< pugi::xml_document >
document::xml_document()
{
    return doc_;
}