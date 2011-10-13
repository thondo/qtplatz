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

#include "targetingmethod.hpp"

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/version.hpp>
# if defined _MSC_VER
# pragma warning( disable: 4996 )
# endif
//# include <boost/archive/binary_oarchive.hpp>
//# include <boost/archive/binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>


using namespace adcontrols;

TargetingMethod::TargetingMethod()
{
}

///////////// serialize //////////////////

namespace adcontrols {

    template<> void
    //TargetingMethod::serialize( portable_binary_oarchive& /*ar*/, const unsigned int /* version */)
    TargetingMethod::serialize( portable_binary_oarchive& /*ar*/, const unsigned int /* version */)
    {
/*
  if ( version >= 0 )
  ar << boost::serialization::make_nvp( "ElementalCompositionMethod", pImpl_ );
*/
    }

    template<> void
        //TargetingMethod::serialize( portable_binary_iarchive& /*ar*/, const unsigned int /*version*/)
    TargetingMethod::serialize( portable_binary_iarchive& /*ar*/, const unsigned int /*version*/)
    {
/*
  if ( version >= 0 )
  ar >> boost::serialization::make_nvp( "ElementalCompositionMethod", pImpl_ );
*/
    }

}; // namespace adcontrols

