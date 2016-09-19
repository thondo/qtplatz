//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "tcp_connection.hpp"
#include "tcp_connection_manager.hpp"
#include "tcp_server.hpp"
#include "request_handler.hpp"
#include "acqiris_protocol.hpp"
#include "document.hpp"
#include <adportable/debug.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <utility>
#include <vector>
#include <iostream>

using namespace aqdrv4::server;

connection::connection( boost::asio::ip::tcp::socket socket
                        , connection_manager& manager
                        , request_handler& handler )
    : socket_( std::move(socket) )
    , connection_manager_( manager )
    , request_handler_( handler )
    , connected_( false )
    , connection_requested_( false )
{
    ADDEBUG() << "*** connection ctor";
}

connection::~connection()
{
    ADDEBUG() << "*** connection dtor";
}

void
connection::start()
{
    do_read();
}

void
connection::stop()
{
    socket_.close();
}

void
connection::do_read()
{
    auto self(shared_from_this());

    boost::asio::async_read(
        socket_
        , response_
        , boost::asio::transfer_at_least( sizeof( acqrscontrols::aqdrv4::preamble ) )
        , [this,self]( const boost::system::error_code& ec
                       , std::size_t bytes_transferred ) {
                                 
            if ( !ec ) {
                                     
                if ( response_.size() >= sizeof( acqrscontrols::aqdrv4::preamble ) ) {

                    auto preamble = boost::asio::buffer_cast< const acqrscontrols::aqdrv4::preamble * >( response_.data() );

                    if ( acqrscontrols::aqdrv4::preamble::isOk( preamble ) && 
                         preamble->length <= response_.size() - sizeof( acqrscontrols::aqdrv4::preamble ) ) {
                        
                        if ( preamble->clsid == acqrscontrols::aqdrv4::clsid_connection_request )
                            connection_requested_ = true;

                        request_handler_.handle_request( response_, reply_ );
                        
                        if ( reply_.size() )
                            do_write();
                    }
                                         
                } else {
                                         
                    do_read();                                         

                }

            } else if ( ec != boost::asio::error::operation_aborted ) {

                ADDEBUG() << "*** do_read: abort connection ***" << ec.message();
                connection_manager_.stop(shared_from_this());

            }
        } );
}

void
connection::do_write()
{
    auto self(shared_from_this());
    
    boost::asio::async_write(
        socket_
        , reply_
        , [this, self]( boost::system::error_code ec, std::size_t ) {
            if ( !ec ) {

                if ( connection_requested_ ) {
                    connection_requested_ = false;
                    connected_ = true;
                    if ( auto server = document::instance()->server() )
                        server->setConnected();
                }

                do_read();
                
                // Initiate graceful connection closure.
                // boost::system::error_code ignored_ec;
                // socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
                
            } else { // if ( ec != boost::asio::error::operation_aborted ) {
                ADDEBUG() << "*** do_write: abort connection *** " << ec.message();
                connection_manager_.stop(shared_from_this());
            }
        });
}

void
connection::write( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_protocol > data )
{
    if ( connected_ ) {

        auto self( this );

        boost::asio::async_write(
            socket_
            , data->to_buffers()
            , [this, self, data]( boost::system::error_code ec, std::size_t ) {
                if ( ec ) {
                    ADDEBUG() << "*** do_write: abort connection *** " << ec.message();
                    connection_manager_.stop(shared_from_this());
                }
            });
    }
}
