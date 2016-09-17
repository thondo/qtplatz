/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QObject>
#include <boost/numeric/ublas/fwd.hpp>
#include <boost/msm/back/state_machine.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

class QSettings;
class digitizer;

namespace aqdrv4 {

    class acqiris_method;
    class waveform;

    enum SubMethodType : unsigned int;
    namespace server {
        class tcp_server;
    }
    namespace client {
        class tcp_client;
    }
}

class AcqirisWidget;

class document : public QObject {
    Q_OBJECT
    ~document();
    document( const document& ) = delete;
    document( QObject * parent = 0 );
public:
    static document * instance();
    static class digitizer * digitizer();

    bool initialSetup();
    bool finalClose();
    QSettings * settings();

    bool digitizer_initialize();

    void push( std::shared_ptr< aqdrv4::waveform > );
    std::shared_ptr< aqdrv4::waveform > recentWaveform();

    std::shared_ptr< const aqdrv4::acqiris_method > acqiris_method();
    std::shared_ptr< const aqdrv4::acqiris_method > adapted_acqiris_method();
    void set_acqiris_method( std::shared_ptr< aqdrv4::acqiris_method > );
    void acqiris_method_adapted( std::shared_ptr< aqdrv4::acqiris_method > );

    void set_server( std::unique_ptr< aqdrv4::server::tcp_server >&& );
    void set_client( std::unique_ptr< aqdrv4::client::tcp_client >&& );

    aqdrv4::server::tcp_server * server() { return server_.get(); }
    aqdrv4::client::tcp_client * client() { return client_.get(); }

    static bool save( const std::string& file, std::shared_ptr< const aqdrv4::acqiris_method > );
    static std::shared_ptr< aqdrv4::acqiris_method > load( const std::string& file );

    void handleValueChanged( std::shared_ptr< aqdrv4::acqiris_method >, aqdrv4::SubMethodType );
    void replyTemperature( int );

signals:
    void updateData();
    void on_acqiris_method_adapted();
    
private:
    std::mutex mutex_;
    std::deque< std::shared_ptr< aqdrv4::waveform > > que_;
    std::shared_ptr< aqdrv4::acqiris_method > method_;
    std::shared_ptr< aqdrv4::acqiris_method > adapted_method_;
    std::unique_ptr< aqdrv4::server::tcp_server > server_;
    std::unique_ptr< aqdrv4::client::tcp_client > client_;
    std::unique_ptr< QSettings > settings_;
    std::vector< std::thread > tcp_threads_;
};
