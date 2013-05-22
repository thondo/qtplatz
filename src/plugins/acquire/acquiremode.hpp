// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#ifndef ACQUIREMODE_H
#define ACQUIREMODE_H

#include <coreplugin/basemode.h>

namespace Acquire {
    namespace internal {
    
        class AcquireMode : public Core::BaseMode {
            Q_OBJECT
                ;
        public:
            ~AcquireMode();
            explicit AcquireMode(QObject *parent = 0);

        signals:

        public slots:

        private:

        };
        //---------
    }
}

#endif // ACQUIREMODE_H
