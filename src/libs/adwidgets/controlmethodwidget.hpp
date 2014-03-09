/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef CONTROLMETHODWIDGET_HPP
#define CONTROLMETHODWIDGET_HPP

#include <QWidget>
#include "adwidgets_global.hpp"

class QTabWidget;

namespace adcontrols { namespace controlmethod { class MethodItem; } }

namespace adwidgets {

    class ControlMethodTable;

    class  ADWIDGETSSHARED_EXPORT ControlMethodWidget : public QWidget {
        Q_OBJECT
    public:
        explicit ControlMethodWidget(QWidget *parent = 0);
		QSize sizeHint() const override;

        void addWidget( QWidget *, const QString& label );
        void addWidget( QWidget *, const QIcon&, const QString& );

    private:
        ControlMethodTable * table_;
        QTabWidget * tab_;
        
    signals:
            
    public slots:
        void handleAdd( const adcontrols::controlmethod::MethodItem& );
        
    };

}

#endif // CONTROLMETHODWIDGET_HPP