/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "ppio.hpp"
#include <chrono>
#include <thread>

int
main()
{
    ppio pp;

    for ( ;; ) {
        auto tp = std::chrono::steady_clock::now() + std::chrono::microseconds( 1000 );
        pp << uint8_t(0xf0);
        std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );
        pp << uint8_t(0x00);
        std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
        pp << uint8_t(0x0f);
        std::this_thread::sleep_for( std::chrono::microseconds( 1 ) );
        pp << uint8_t(0x00);
        std::this_thread::sleep_until( tp );
    }
    return 0;
}
