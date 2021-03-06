/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QWidget>
#include <memory>

class QGridLayout;

namespace query {

    class QueryForm;
    class QueryResultTable;
    class QueryQuery;
    class CountingQueryDialog;
    class SqlHistoryDialog;

    class QueryWidget : public QWidget  {
        Q_OBJECT
    public:
        ~QueryWidget();
        explicit QueryWidget(QWidget *parent = 0);

    private:
        QGridLayout * layout_;
        std::unique_ptr< QueryForm > form_;
        std::unique_ptr< QueryResultTable > table_;
        CountingQueryDialog * dlg_;
        SqlHistoryDialog * hdlg_;

        void executeQuery();

    signals:
        void onQueryData( std::shared_ptr< QueryQuery > );
        void tableListChanged( const QStringList& );

    public slots :
        void handleConnectionChanged();

        void handleQuery( const QString& );
        void handlePlot();
        void buildQuery( const QString&, const QRectF&, bool );
        void accept(); // <- dialog
        void applyQuery();
        void showHistory();
    };

}
