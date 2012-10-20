/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "sdfileview.hpp"
#include "ui_sdfileview.h"

#include <openbabel/babelconfig.h>
#include <openbabel/obconversion.h>
#include <openbabel/mol.h>

SDFileView::SDFileView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SDFileView)
{
    ui->setupUi(this);

	OpenBabel::OBConversion obconversion;
    OpenBabel::OBMol mol;
    std::string fname = "Z:/SkyDrive/MOL/common-names.sdf";
	OpenBabel::OBFormat * informat = obconversion.FormatFromExt( fname.c_str() );
	obconversion.SetInFormat( informat );
	bool noteatend = obconversion.ReadFile( &mol, fname );
	while ( noteatend ) {
		std::string formula = mol.GetFormula();
		break;
	}
}

SDFileView::~SDFileView()
{
    delete ui;
}
