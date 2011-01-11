// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#if defined _DEBUG
#     pragma comment(lib, "adwidgetsd.lib")   // dll
#     pragma comment(lib, "adportabled.lib")  // static
#     pragma comment(lib, "adplugind.lib")    // dll
#     pragma comment(lib, "adcontrolsd.lib")  // static
#     pragma comment(lib, "adutilsd.lib")     // static
#     pragma comment(lib, "acewrapperd.lib")  // static
#     pragma comment(lib, "qtwrapperd.lib")   // static
#     pragma comment(lib, "ACEd.lib")         // dll
#     pragma comment(lib, "QAxContainerd.lib")
#     pragma comment(lib, "xmlwrapperd.lib")  // static
#     pragma comment(lib, "portfoliod.lib")   // dll
#     pragma comment(lib, "adutilsd.lib")     // static
#else
#     pragma comment(lib, "adwidgets.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adutils.lib")      // static
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "QAxContainer.lib")
#     pragma comment(lib, "xmlwrapper.lib")
#     pragma comment(lib, "portfolio.lib")
#     pragma comment(lib, "adutils.lib")
#endif
