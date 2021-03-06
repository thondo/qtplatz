/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUERYDOCUMENT_HPP
#define QUERYDOCUMENT_HPP

#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <QObject>

namespace boost { namespace filesystem { class path; } }
namespace adcontrols { class MassSpectrometer; }

class QSettings;

namespace query {

    class QueryConnection;

    class document : public QObject {
        Q_OBJECT
        document();
    private:
        ~document();
        static std::unique_ptr< document > instance_;
    public:
        static document * instance();

        void setConnection( QueryConnection * );
        QueryConnection * connection();

        void onInitialUpdate();
        void onFinalClose();

        inline QSettings& settings() { return *settings_; }
        QString lastDataDir() const;

        void addSqlHistory( const QString& );
        QStringList sqlHistory();

        void setMassSpectrometer( std::shared_ptr< adcontrols::MassSpectrometer > );
        std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer();
        double tof() const;
        double width() const;
        void setTof( double );
        void setWidth( double );

    private:
        friend std::unique_ptr< document >::deleter_type;
        std::unique_ptr< QSettings > settings_;
        std::shared_ptr< QueryConnection > queryConnection_;
        std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer_;

    signals:
        void onConnectionChanged();
        void onHistoryChanged();
    };
}

#endif // QUERYDOCUMENT_HPP
