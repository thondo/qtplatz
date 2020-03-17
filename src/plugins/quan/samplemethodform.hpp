// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2020 MS-Cheminformatics LLC
**
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminfomatics.
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
#include <adcontrols/constants.hpp>

namespace quan {

    class SampleMethodForm : public QWidget {
        Q_OBJECT

    public:
        explicit SampleMethodForm(QWidget *parent = nullptr);
        ~SampleMethodForm();

        adcontrols::Quan::QuanInlet currSelection() const;
        void setSelection( adcontrols::Quan::QuanInlet );

    signals:
        void onSampleMethodChanged( adcontrols::Quan::QuanInlet );

    private:

    };
}
