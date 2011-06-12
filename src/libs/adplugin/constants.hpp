// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#define iid_iMonitor               L"adplugin::ui::iMonitor"
#define iid_iControlMethodEditor   L"adplugin::ui::iControlMethodEditor"
#define iid_iLog                   L"adplugin::ui::iLog"
#define iid_iSequence              L"adplugin::ui::iSequence"
#define iid_iSequencesForm         L"adplugin::ui::iSequencesForm"

#if defined __APPLE__
#  define adpluginDirectory          "PlugIns/ScienceLiaison"
#else
#  define adpluginDirectory          "lib/qtplatz/plugins/ScienceLiaison"
#endif
