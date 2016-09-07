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
#include "request_handler.hpp"
#include "acqiris_protocol.hpp"
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
{
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

    boost::asio::async_read( socket_
                             , response_
                             , boost::asio::transfer_at_least( 1 )
                             , [this,self]( const boost::system::error_code& ec
                                            , std::size_t bytes_transferred ) {

                                 if ( !ec ) {
                                     
                                     std::cout << "read_some: "
                                               << bytes_transferred << "; " << response_.size() << std::endl;
                                     
                                     if ( response_.size() >= sizeof( aqdrv4::preamble ) ) {

                                         request_handler_.handle_request( response_, ack_ );
                                         // reply_ = reply::stock_reply( reply::bad_request );
                                         
                                         do_write();

                                     } else {
                                         
                                         do_read();                                         
                                     }

                                 } else if ( ec != boost::asio::error::operation_aborted ) {
                                     
                                     std::cout << "close connection." << std::endl;
                                     connection_manager_.stop(shared_from_this());

                                 }
                             } );
}

void
connection::do_write()
{
    auto self(shared_from_this());
    
    boost::asio::async_write(socket_
                             , ack_ // reply_.to_buffers(),
                             , [this, self]( boost::system::error_code ec, std::size_t ) {
                                 if ( !ec ) {
                                     do_read();
                                     // Initiate graceful connection closure.
                                     //boost::system::error_code ignored_ec;
                                     //socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both,
                                     //                  ignored_ec);
                                 }
                                 if ( ec != boost::asio::error::operation_aborted ) {
                                     connection_manager_.stop(shared_from_this());
                                 }
                             });
}

