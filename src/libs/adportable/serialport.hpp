/**************************************************************************
** Copyright (C) 2013 Toshinobu Hondo, Ph.D.
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

#include <boost/asio.hpp>
#include <boost/function.hpp>

namespace boost { namespace system { class error_code; } }

namespace adportable {

    class serialport {
    public:
        serialport( boost::asio::io_service& io_service, const char * device_name, unsigned int baud_rate );
        inline operator boost::asio::serial_port& () { return port_; }

        void async_reader( boost::function< void (const char *, std::size_t) > reader );
        void start();
        void close();

        void write ( const char *, std::size_t );
        void async_writeln( const char *, std::size_t );
        void async_write( const char *, std::size_t );

    private:
        boost::asio::serial_port port_;
        boost::asio::streambuf read_buffer_;
        boost::function< void (const char *, std::size_t) > reader_;
        std::string inbuf_;
        std::string outbuf_;
        void initiate_read();
        void initiate_timer();
        void handle_timeout( const boost::system::error_code& );
        void handle_read(  const boost::system::error_code&, std::size_t );
        void handle_write( const boost::system::error_code&, std::size_t );
    };
}


