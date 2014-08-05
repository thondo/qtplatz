/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANPLOTWND_HPP
#define QUANPLOTWND_HPP

#include <QWidget>
#include <boost/uuid/uuid.hpp>
#include <map>
#include <memory>

namespace adwplot { class Dataplot; }
class QwtPlotMarker;
class QwtPlotCurve;

namespace quan {

    namespace detail { struct calib_curve; struct calib_data; }

    class QuanCmpdWidget;
    class QuanResultTable;
    class QuanResultWidget;

    class QuanResultWnd : public QWidget
    {
        Q_OBJECT
    public:
        explicit QuanResultWnd(QWidget *parent = 0);

    private:
        QuanCmpdWidget * cmpdWidget_;
        QuanResultWidget * respTable_;
        std::shared_ptr< adwplot::Dataplot > calibplot_;
        std::vector< std::shared_ptr< QwtPlotMarker > > markers_;
        std::vector< std::shared_ptr< QwtPlotCurve > > curves_;
        std::map< boost::uuids::uuid, std::shared_ptr< detail::calib_curve > > calib_curves_;
        std::map< boost::uuids::uuid, std::shared_ptr< detail::calib_data > > calib_data_;
        
        void handleConnectionChanged();
        void handleCompoundSelected( const QModelIndex& );

        void plot_calib_curve_xy( adwplot::Dataplot *, const detail::calib_curve&, const detail::calib_data& );
        void plot_calib_curve_yx( adwplot::Dataplot *, const detail::calib_curve&, const detail::calib_data& );

    signals:

    public slots:

    };

}

#endif // QUANPLOTWND_HPP
