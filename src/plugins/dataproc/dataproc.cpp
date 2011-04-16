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
#     pragma comment(lib, "adwidgetsd.lib")   // static
#     pragma comment(lib, "adwplotd.lib")     // static
#     pragma comment(lib, "adportabled.lib")  // static
#     pragma comment(lib, "adplugind.lib")    // dll
#     pragma comment(lib, "adcontrolsd.lib")  // static
#     pragma comment(lib, "adutilsd.lib")     // static
#     pragma comment(lib, "acewrapperd.lib")  // static
#     pragma comment(lib, "qtwrapperd.lib")   // static
#     pragma comment(lib, "xmlwrapperd.lib")  // static
#     pragma comment(lib, "portfoliod.lib")   // dll
#     pragma comment(lib, "adutilsd.lib")     // static
#     pragma comment(lib, "qwtd.lib")
#else
#     pragma comment(lib, "adwidgets.lib")
#     pragma comment(lib, "adwplot.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adutils.lib")      // static
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "xmlwrapper.lib")
#     pragma comment(lib, "portfolio.lib")
#     pragma comment(lib, "adutils.lib")
#     pragma comment(lib, "qwt.lib")
#endif

#  if defined _DEBUG
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "adcontrollerd.lib")
#     pragma comment(lib, "adbrokerd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#  else
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "adcontroller.lib")
#     pragma comment(lib, "adbroker.lib")
#     pragma comment(lib, "adinterface.lib")
#  endif

