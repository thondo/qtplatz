// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace adportable {

    namespace binary {

        template<typename, typename>
        struct has_archive;

        template<typename T, typename Ret, typename... Args>
        struct has_archive<T, Ret(Args...)> {
            template<typename U, U> struct SFINAE;
            template<typename U> static std::true_type test( SFINAE<Ret(*)(Args...), &U::archive>* ); // check 'static' mem_fun
            template<typename U> static std::false_type test(...);
            static const bool value = decltype(test<T>(0))::value;
        };

        template<typename, typename>
        struct has_restore;

        template<typename T, typename Ret, typename... Args>
        struct has_restore<T, Ret(Args...)> {
            template<typename U, U> struct SFINAE;
            template<typename U> static std::true_type test( SFINAE<Ret(*)(Args...), &U::restore>* ); // check 'static' mem_fun
            template<typename U> static std::false_type test(...);
            static const bool value = decltype(test<T>(0))::value;
        };

        template<class T> class archive_functor {
            std::ostream& os_;
        public:
            archive_functor( std::ostream& os ) : os_( os ){}
            void operator & ( const T& t ) {
                T::archive( os_, t );
            }
        };

        template<class T> class restore_functor {
            std::istream& is_;
        public:
            restore_functor( std::istream& is ) : is_( is ){}
            void operator & ( T& t ) {
                T::restore( is_, t );
            }
        };

        //----------------------------------------
        template<bool, class Archiver, class Functor> struct IF { typedef Archiver type; };
        template<class Archiver, class Functor> struct IF<true, Archiver, Functor> { typedef Functor type; };

        // if class 'T' has a static member function "archive(ostream&,const T&)", call it as serializer
        // otherwise call Archiver := portable_binary_oarchive
        // background: serializer implemented in the dll can't be called across dll boundary on apple clang++
        // that raise std::bad_cast although it works on vc12.

        template<class Archiver = portable_binary_oarchive> class serialize {
        public:
            template<class T> bool operator()( const T& data, std::ostream& output ) const {

                typename IF< has_archive<T,bool(std::ostream&,const T&)>::value
                             , Archiver, archive_functor<T> >::type ar( output );
                ar & data;

                output.flush();
                return true;
            }

            template<class T> bool operator()( const T& data, std::string& output ) const {

                boost::iostreams::back_insert_device< std::string > inserter( output );
                boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );

                typename IF< has_archive<T,bool(std::ostream&,const T&)>::value
                             , Archiver, archive_functor<T> >::type ar( device ); // SFINAE dispatch

                ar & data;

                device.flush();
                return true;
            }
        };

        // ---------------------------- deserialize ----------------------------------

        template<class Archiver = portable_binary_iarchive> class deserialize {
        public:
            template<class T> bool operator()( T& data, std::istream& strm ) {
                
                typename IF<has_restore<T,bool(std::istream&,T&)>::value
                            , Archiver, restore_functor<T> >::type ar( strm ); // SFINAE dispatch

                ar & data;

                return true;
            }

            template<class T> bool operator()( T& data, const char * s, std::size_t size ) {
				if (size == 0)
					return false;
                boost::iostreams::basic_array_source< char > device( s, size );
                boost::iostreams::stream< boost::iostreams::basic_array_source< char > > strm( device );

                typename IF<has_archive<T,bool(std::istream&,T&)>::value
                            , Archiver, restore_functor<T> >::type ar( strm );
                ar & data;
                return true;
            }
        };
    } // namespace binary

    //----------------------------------------
}

