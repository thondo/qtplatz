/**************************************************************************
** Copyright (C) 2019-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019-2020 MS-Cheminformatics LLC
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

int addtwo P(( int, int));

:Begin:
:Function:       addtwo
:Pattern:        AddTwo[i_Integer, j_Integer]
:Arguments:      { i, j }
:ArgumentTypes:  { Integer, Integer }
:ReturnType:     Integer
:End:
:Evaluate: AddTwo::usage = "AddTwo[x, y] gives the sum of two machine integers x and y."

/*******/
int counter P((int));

:Begin:
:Function:       counter
:Pattern:        Counter[i_Integer]
:Arguments:      { i }
:ArgumentTypes:  { Integer }
:ReturnType:     Integer
:End:
:Evaluate: Counter::usage = "Counter[x] increment counter value."

/**************************************
*/
// void isotopeCluster P(( const char *, double ));

:Begin:
:Function:       isotopeCluster
:Pattern:        IsotopeCluster[i_String, j_Real]
:Arguments:      { i, j }
:ArgumentTypes:  { String, Real64 }
:ReturnType:     Manual
:End:

:Evaluate: IsotopeCluster::usage = "IsotopeCluster[x,r] gives isotope cluster for given formula x and resolving power r."

/**************************************
*/
double monoIsotopicMass P(( const char * ));

:Begin:
:Function:       monoIsotopicMass
:Pattern:        MonoIsotopicMass[i_String]
:Arguments:      { i }
:ArgumentTypes:  { String }
:ReturnType:     Real64
:End:

:Evaluate: MonoIsotopicMass::usage = "MonoIsotopicMass[x] gives the mono isotopic mass for formula x."

/**************************************
*/

int adFileOpen P(( const char * ));
:Begin:
:Function:       adFileOpen
:Pattern:        adFileOpen[i_String]
:Arguments:      { i }
:ArgumentTypes:  { String }
:ReturnType:     Integer
:End:

:Evaluate: adfsOpen::usage = "adFileOpen[x] open .adfs datafile."


