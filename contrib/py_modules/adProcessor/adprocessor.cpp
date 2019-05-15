// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
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

#include "dataprocessor.hpp"
#include "datareader.hpp"
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/datareader.hpp>
#include <memory>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace boost::python;
void exportUUID();

boost::uuids::uuid
gen_uuid() {
    return boost::uuids::random_generator()();
}

void set_uuid( const std::string& uuid ) {
    ADDEBUG() << "***** invoke set_uuid: " << uuid;
}

std::vector< std::string > uuids() {
    std::vector< std::string > a;
    for ( int i = 0; i < 5;  ++i )
        a.emplace_back( boost::uuids::to_string ( gen_uuid() ) );
    return a;
}

boost::python::tuple
my_tuple()
{
    return boost::python::make_tuple( std::string("abcde"), 1.0, std::string("xyz" ) );
}

std::vector< boost::python::tuple >
my_tuples()
{
    std::vector< boost::python::tuple > a;
    for ( size_t i = 0; i < 5; ++i )
        a.emplace_back( boost::python::make_tuple( std::string("abcde"), 1.0, gen_uuid() ) );
        //a.emplace_back( boost::python::make_tuple( std::string("abcde"), 1.0, std::to_string(i) ) );
    return a;
}

BOOST_PYTHON_MODULE( adProcessor )
{
    exportUUID();

    register_ptr_to_python< std::shared_ptr< DataReader > >();

    def( "gen_uuid", gen_uuid );
    def( "set_uuid", set_uuid );
    def( "uuids", &uuids );
    def( "tuple", my_tuple );
    def( "tuples", my_tuples );

    class_< std::vector< std::string > >("std_vector_string")
        .def( vector_indexing_suite< std::vector< std::string > >() )
        ;

    class_< std::vector< boost::python::tuple > >("std_vector_tuple")
        .def( vector_indexing_suite< std::vector< boost::python::tuple >,true >() )
        ;

    class_< std::vector< std::shared_ptr< DataReader > > >("std_vector_std_shared_ptr_DataReader")
        .def( vector_indexing_suite< std::vector< std::shared_ptr< DataReader > >, true >() )
        ;

    class_< dataProcessor >( "processor" )
        .def( "open", &dataProcessor::open )
        .def( "dataReaderTuples", &dataProcessor::dataReaderTuples )
        .def( "dataReaders", &dataProcessor::dataReaders )
        ;

    class_< DataReader >( "dataReader", no_init )
        .def( "objuuid", &DataReader::objuuid )
        .def( "objtext", &DataReader::objtext )
        .def( "display_name", &DataReader::display_name )
        ;
}
