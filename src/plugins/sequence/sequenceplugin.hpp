// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef SEQUENCEPLUGIN_H
#define SEQUENCEPLUGIN_H

#include <extensionsystem/iplugin.h>
#if QT_VERSION >= 0x050000 && ! defined Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif

namespace adextension { class iSequence; }

namespace sequence {

    class Mode;
    class MainWindow;

    namespace internal {

        class SequenceManager;

        class SequencePlugin : public ExtensionSystem::IPlugin {
            Q_OBJECT;
        public:
            ~SequencePlugin();
            explicit SequencePlugin();

            bool initialize(const QStringList& arguments, QString* error_message);
            void extensionsInitialized();
            void shutdown();
        private:
            MainWindow * mainWindow_;
            boost::scoped_ptr< Mode > mode_;
        signals:

        private slots:
			void handleFileNew( bool );

        };
    }
}

#endif // SEQUENCEPLUGIN_H
