// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019-2020 MS-Cheminformatics LLC
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

#include "chemicalformula.hpp"
#include "isotopecluster.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost::python;

BOOST_PYTHON_MODULE( adControls )
{
    // register_ptr_to_python<std::shared_ptr< adcontrols::ChemicalFormula > >();
    register_ptr_to_python< std::shared_ptr< adcontrols::MassSpectrum > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MassSpectrum > >();

    class_< std::vector< boost::python::tuple > >("std_vector_tuple")
        .def( vector_indexing_suite< std::vector< boost::python::tuple >,true >() )
        ;

    class_< std::vector< boost::python::dict > >("std_vector_dict")
        .def( vector_indexing_suite< std::vector< boost::python::dict >,true >() )
        ;

    class_< py_module::ChemicalFormula >( "ChemicalFormula", boost::python::init< const std::string >() )
        .def( "formula",          &py_module::ChemicalFormula::formula )
        .def( "monoIsotopicMass", &py_module::ChemicalFormula::monoIsotopicMass )
        .def( "standardFormula",  &py_module::ChemicalFormula::standardFormula )
        .def( "formatFormula",    &py_module::ChemicalFormula::formatFormula )
        .def( "composition",      &py_module::ChemicalFormula::composition )
        .def( "composition_dict", &py_module::ChemicalFormula::composition_dict )
        .def( "charge",           &py_module::ChemicalFormula::charge )
        ;

    class_< py_module::IsotopeCluster >( "IsotopeCluster", boost::python::init< const std::string >() )
        .def( "formula",           &py_module::IsotopeCluster::formula )
        .def( "setCharge",         &py_module::IsotopeCluster::setCharge )
        .def( "charge",            &py_module::IsotopeCluster::charge )
        .def( "setResolvingPower", &py_module::IsotopeCluster::setResolvingPower )
        .def( "resolvingPower",    &py_module::IsotopeCluster::resolvingPower )
        .def( "compute",           &py_module::IsotopeCluster::compute )
        ;

    class_< adcontrols::MassSpectrum >( "MassSpectrum" )
        .def( "__len__",       &adcontrols::MassSpectrum::size )
        .def( "size",          &adcontrols::MassSpectrum::size )
        .def( "resize",        &adcontrols::MassSpectrum::resize )
        .def( "getMass",       &adcontrols::MassSpectrum::mass )
        .def( "getTime",       &adcontrols::MassSpectrum::time )
        .def( "getIntensity",  &adcontrols::MassSpectrum::intensity )
        .def( "numProtocols",  &adcontrols::MassSpectrum::numSegments )
        .def( "getProtocol",   &adcontrols::MassSpectrum::getProtocol )
        ;

    class_< adcontrols::Chromatogram >( "Chromatogram" )
        .def( "__len__",       &adcontrols::Chromatogram::size )
        .def( "size",          &adcontrols::Chromatogram::size )
        .def( "getTime",       &adcontrols::Chromatogram::time )
        .def( "getIntensity",  &adcontrols::Chromatogram::intensity )
        .def( "protocol",      &adcontrols::Chromatogram::protocol )
        ;
}
