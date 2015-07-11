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

#ifndef AP240FORM_HPP
#define AP240FORM_HPP

#include <QWidget>

namespace Ui {
class ap240Form;
}

namespace ap240 {

    class ap240Form : public QWidget
    {
        Q_OBJECT

    public:
        explicit ap240Form(QWidget *parent = 0);
        ~ap240Form();

        void onInitialUpdate();
        void onStatus( int );

    private slots:
        void on_pushButton_clicked();

    signals:
        void trigger_apply();

    private:
        Ui::ap240Form *ui;
    };

}

#endif // AP240FORM_HPP