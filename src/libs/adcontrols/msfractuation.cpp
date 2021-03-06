// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
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

#include "msfractuation.hpp"
#include "lockmass.hpp"
#include <adportable/float.hpp>
#include <adportable/unique_ptr.hpp>
#include <type_traits>
#include <map>

namespace adcontrols {

    class MSFractuation::impl {
    public:
        std::map< int64_t, std::pair< double, double > > values_;
        std::map< int64_t /* rowid */, lockmass::fitter > fitter_;
        std::map< int64_t /* rowid */, lockmass::fitter > interporated_;
    };

}

using namespace adcontrols;

MSFractuation::~MSFractuation(void)
{
}

MSFractuation::MSFractuation() : impl_( std::make_unique< impl >() )
{
    static_assert( std::is_base_of< std::enable_shared_from_this< MSFractuation >, MSFractuation >::value
                             , "MSFractuation must be decendent of std::enable_shared_from_this<>" );
}

// static
std::shared_ptr< MSFractuation >
MSFractuation::create()
{
    return std::make_shared< MSFractuation >();
}

void
MSFractuation::insert( int64_t rowid, const lockmass::mslock& mslock )
{
    impl_->fitter_[ rowid ] = mslock.fitter();

    for ( auto& ref : mslock ) 
        impl_->values_ [ rowid ] = std::make_pair( ref.exactMass(), ref.matchedMass() );
}

bool
MSFractuation::has_a( int64_t rowid ) const
{
    return impl_->fitter_.find( rowid ) != impl_->fitter_.end();
}

const lockmass::fitter
MSFractuation::find( int64_t rowid, bool interporate )
{
    auto it = impl_->fitter_.lower_bound( rowid );

    if ( it == impl_->fitter_.end() ) 
        return lockmass::fitter();

    if ( it == impl_->fitter_.begin() || it->first == rowid )
        return it->second;

    auto prev = it;
    --prev;
    if ( interporate ) {
        auto xit = impl_->values_.lower_bound( rowid );
        lockmass::mslock lk;
        auto value = xit->second;
        //lk << lockmass::reference( "", xit->second.first, xit->second.second, 0 );
        --xit;

        if ( adportable::compare<double>::essentiallyEqual( value.first, xit->second.first ) ) {
            lk << lockmass::reference( "", value.first, ( value.second + xit->second.second ) / 2, 0);
        } else {
            lk << lockmass::reference( "", value.first, value.second , 0 );
            lk << lockmass::reference( "", xit->second.first, xit->second.second, 0 );
        }
        if ( lk.fit() )
            impl_->interporated_ [ rowid ] = lk.fitter();
        return impl_->interporated_ [ rowid ];
    }
    // find closest or lower if equal distance.
    if ( rowid - prev->first <= it->first - rowid )
        return prev->second;
    return it->second;
}

