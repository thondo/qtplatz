/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "digitizer.hpp"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <chrono>
#include <iostream>
#include <thread>

int
main(int argc, char* argv[])
{
    namespace po = boost::program_options;
    
    try {
        po::variables_map vm;
        
        po::options_description desc("options");
        desc.add_options()
            ( "help,h", "print help message" )
            ( "delay", po::value<double>()->default_value(0.0), "start delay in microseconds" )
            ( "width", po::value<double>()->default_value(10.0), "duration (width) in microseconds" )
            ( "mode", po::value<int>()->default_value(0), "config mode (0 = digitizer, 2 = averager)" )
            ;

        po::store( po::command_line_parser( argc, argv ).options( desc ).run(), vm );
        po::notify( vm );

        if ( vm.count( "help" ) ) {
            std::cout << desc;
            return 0;
        }
        ap240::method m;
        m.hor_.delay = vm["delay"].as<double>() * 1.0e-6;
        m.hor_.width = vm["width"].as<double>() * 1.0e-6;
        m.hor_.mode = vm["mode"].as<int>();

        ap240::digitizer aqrs;

        aqrs.connect_reply( []( const std::string key, const std::string value ){
                std::cout << key << "\t" << value << std::endl;
            });

        aqrs.connect_waveform( []( const ap240::waveform * wform, ap240::method& proto ){
                auto self( wform->shared_from_this() );
                std::cout << "wform: " << wform->serialnumber_ << " size: " << wform->size() << "/" << wform->d_.size();
                std::cout << "\thorPos: " << wform->meta_.horPos
                          << "\tindexFirst: " << wform->meta_.indexFirstPoint
                          << "\ttime: " << wform->meta_.initialXTimeSeconds
                          << std::endl;
                auto p = wform->begin<int8_t>();
                for ( int i = 0; i < 10; ++i )
                    std::cout << int(*p++) << ", ";
                std::cout << std::endl;
                return false;
            });

        aqrs.peripheral_initialize();
        aqrs.peripheral_prepare_for_run( m );

        std::this_thread::sleep_for( std::chrono::seconds(30) );

        aqrs.peripheral_terminate();
        
    }  catch (std::exception& e)  {
        std::cerr << "exception: " << e.what() << "\n";
    }
    std::cout << "bye." << std::endl;
}
