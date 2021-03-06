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

#ifndef MSREFERENCETABLE_HPP
#define MSREFERENCETABLE_HPP

#include "tableview.hpp"

class QStandardItemModel;

namespace adcontrols { class MSCalibrateMethod; class MSReference; }

namespace adwidgets {

    class MSReferenceTable : public TableView  {
        Q_OBJECT
    public:
        explicit MSReferenceTable(QWidget *parent = 0);
        ~MSReferenceTable();

        enum colume_define {
            c_formula
            , c_exact_mass
            , c_enable
            , c_description
            , c_charge
            , c_num_columns
        };

        void onInitialUpdate();
        void getContents( adcontrols::MSCalibrateMethod& );
        void setContents( const adcontrols::MSCalibrateMethod& );

        void handleAddReference( const adcontrols::MSReference& );

    private:
        QStandardItemModel * model_;
        class delegate;
        class impl;

        void addReference( const adcontrols::MSReference&, int row );
        void handleValueChanged( const QModelIndex& );
        void handleContextMenu( const QPoint& pt );

    signals:
        void selectedRowsDeleted();
            
    private slots:
        void handlePaste() override;
    };

}

#endif // MSREFERENCETABLE_HPP
