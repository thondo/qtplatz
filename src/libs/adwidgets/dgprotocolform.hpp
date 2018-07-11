/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QWidget>
#include "adwidgets_global.hpp"

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT dgProtocolForm : public QWidget {
        
        Q_OBJECT

    public:
        explicit dgProtocolForm(QWidget *parent = 0);
        ~dgProtocolForm();
        //void setProtocol( const adio::dg::protocol<adio::dg::delay_pulse_count>& );
        
    signals:

    private slots:
        
    private:

    };

}
    
