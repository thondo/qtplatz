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

#ifndef PROGRESSWND_HPP
#define PROGRESSWND_HPP

#include "adwidgets_global.hpp"
#include <QDialog>
#include <atomic>
#include <memory>
#include <map>
#include <vector>

class QGridLayout;
class QProgressBar;

namespace adwidgets {

    class Progress;

    class ADWIDGETSSHARED_EXPORT ProgressWnd : public QDialog  {
        Q_OBJECT
        ~ProgressWnd();
        explicit ProgressWnd(QWidget *parent = 0);
        static ProgressWnd * instance_;
    public:
        static ProgressWnd * instance();
        std::shared_ptr< Progress > addbar();
        void stop();

    private:
        QGridLayout * layout_;
        std::map< int, QProgressBar * > progressive_;
        //std::vector< std::pair< int, QProgressBar * > > completed_;
        std::atomic<int> idGen_;
        bool stop_requested_;
        friend class Progress;

    signals:
        void onAdd( int );
        void onProgress( int, int, int );
        void onRemove( int );

    private slots:
        void handleProgress( int, int, int );
        void handleRemove( int );
        void handleAdd( int );
    };


    class ADWIDGETSSHARED_EXPORT Progress : public std::enable_shared_from_this < Progress > {
        ProgressWnd * wnd_;
        int id_;
        Progress( const Progress& ) = delete;
        Progress& operator = ( const Progress& ) = delete;
    public:
        Progress( ProgressWnd *, int id );
        ~Progress();
        bool operator()( int, int );
    };

}

#endif // PROGRESSWND_HPP