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

#include "datasequencetree.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/descriptions.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QComboBox>
#include <QHeaderView>
#include <QKeyEvent>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <functional>
#include <array>
#include <thread>
#include <mutex>

namespace quan {

    namespace datasequencetree {

        static std::array< const wchar_t *, 3 > extensions = { { L".adfs", L".csv", L".txt" } };

        enum {
            r_rowdata
            , r_processed
        };

        enum {
            c_datafile
            , c_data_type       // RAW | Spectrum | Chromatogram
            , c_sample_type     // standard | unknown | QC
            , c_process         // chromaotgram generation | 
            , c_level
            , c_description
            , number_of_columns
        };

        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                painter->save();
                std::string data_type = index.model()->index( index.row(), c_data_type, index.parent() ).data().toString().toStdString();
                if ( data_type == "raw" ) {
                    painter->fillRect( option.rect, QColor( 0xff, 0x66, 0x44, 0x10 ) );
                } else if ( data_type == "file" ) {
                    painter->setPen( Qt::gray );
                }
                if ( index.column() == c_datafile ) {
                    op.textElideMode = Qt::ElideLeft;
                    QStyledItemDelegate::paint( painter, op, index );
                    // auto align = Qt::AlignRight | Qt::AlignVCenter;
                    // painter->drawText( op.rect, align, index.data().toString() );
                } else
                    QStyledItemDelegate::paint( painter, op, index );
                painter->restore();                
            }
            void setEditorData( QWidget * editor, const QModelIndex& index ) const {
                QStyledItemDelegate::setEditorData( editor, index );
            }
            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const {
                QStyledItemDelegate::setModelData( editor, model, index );
                if ( valueChanged_ )
                    valueChanged_( index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                return QStyledItemDelegate::sizeHint( option, index );
            }

            QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                if ( index.column() == c_sample_type ) {
                    QComboBox * pCombo = new QComboBox( parent );
                    pCombo->addItems( QStringList() << "UNK" << "STD" << "QC" );
                    return pCombo;
                }
                if ( index.column() == c_process ) {
                    const QAbstractItemModel& model = *index.model();
                    QString data_type = model.index( index.row(), c_data_type, index.parent() ).data().toString();
                    if ( data_type == "raw" ) {
                        QComboBox * pCombo = new QComboBox( parent );
                        pCombo->addItems( QStringList() << "Average all" << "Take 1st spc." << "Take 2nd spc." << "Take last spc." );
                        return pCombo;
                    } else if ( data_type == "spc" ) {
                        QComboBox * pCombo = new QComboBox( parent );
                        pCombo->addItems( QStringList() << "AS IS" );
                        return pCombo;
                    }
                }
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
            
            void register_valueChanged( std::function<void( const QModelIndex& )> f ) { 
                valueChanged_ = f;
            }
        private:
            std::function<void( const QModelIndex& )> valueChanged_;
        };

        ////////////////

        class dataSubscriber : public adcontrols::dataSubscriber
                             , public std::enable_shared_from_this< dataSubscriber > {
        public:
            dataSubscriber( int row
                            , const std::wstring& filename
                            , std::function<void( dataSubscriber *)> f ) : row_( row )
                                                                         , filename_( filename )
                                                                         , raw_(0)
                                                                         , callback_(f){
            }
            void open() {
                try {
                    if ( ( datafile_ = std::shared_ptr< adcontrols::datafile >( adcontrols::datafile::open( filename_, true ) )  ) )
                        datafile_->accept( *this );
                    callback_( this );
                } catch ( ... ) {
                    ADERROR() << boost::current_exception_diagnostic_information();
                    QMessageBox::warning(0, "Quan dataSubscriber", boost::current_exception_diagnostic_information().c_str() );
                }
            }
            
            // implementation
            bool subscribe( const adcontrols::LCMSDataset& d ) override {
                raw_ = &d;
                size_t pos = d.make_pos( 0, 0 ); // find first data for  protoId = 0
                ms_ = std::make_shared< adcontrols::MassSpectrum >();
                d.getSpectrum( -1, pos, *ms_ );
                return true;
            }

            bool subscribe( const adcontrols::ProcessedDataset& d ) override {
                portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
                return true;
            }

            // local impl
            const adcontrols::LCMSDataset * raw() {
                return raw_;
            }
            portfolio::Portfolio * processed() {
                return portfolio_.get();
            }
            adcontrols::MassSpectrum * ms() { return ms_.get(); }
            const std::wstring& filename() const { return filename_;  }
            int row() const { return row_; }
        private:
            int row_;
            std::wstring filename_;
            std::shared_ptr< adcontrols::MassSpectrum > ms_;
            std::shared_ptr< adcontrols::datafile > datafile_;
            std::shared_ptr< portfolio::Portfolio > portfolio_;
            const adcontrols::LCMSDataset * raw_;
            std::function< void( dataSubscriber * ) > callback_;
        };

        struct Chromatography {
            static void setRow( QStandardItemModel& model, int row, const QString& data_type, const QModelIndex& parent = QModelIndex()) {
                model.setData( model.index( row, c_data_type, parent ), data_type );
                model.setData( model.index( row, c_sample_type, parent ), "UNK" );
                model.setData( model.index( row, c_process, parent ), "TIC" ); // | Chromatogram Generation
                model.setData( model.index( row, c_level, parent ), 0 ); // level
            }
        };

        struct Infusion {
            static void setRow( QStandardItemModel& model, int row, const QString& data_type, const QModelIndex& parent = QModelIndex()) {
                model.itemFromIndex( model.index( row, c_datafile, parent ) )->setEditable( false ); // not editable
                model.setData( model.index( row, c_data_type, parent ), data_type );
                model.itemFromIndex( model.index( row, c_data_type, parent ) )->setEditable( false );
                model.setData( model.index( row, c_sample_type, parent ), "UNK" );
                if ( data_type == "file" ) {
                    model.itemFromIndex( model.index( row, c_process ) )->setEditable( false );
                } else if ( data_type == "raw" ) {
                    model.setData( model.index( row, c_process, parent ), "Average all" );
                    if ( parent != QModelIndex() )
                        model.itemFromIndex( model.index( row, c_sample_type, parent ) )->setEditable( false );
                } else if ( data_type == "spc" ) {
                    model.setData( model.index( row, c_process, parent ), "AS IS" );
                }
                model.setData( model.index( row, c_level, parent ), 0 ); // level
            }
        };

    }
}

using namespace quan;
using namespace quan::datasequencetree;

DataSequenceTree::DataSequenceTree(QWidget *parent) : QTreeView(parent)
                                                    , model_( new QStandardItemModel )
                                                    , dropCount_( 0 )
{
    auto delegate = new ItemDelegate;
    delegate->register_valueChanged( [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); } );
    setItemDelegate( delegate );
    setModel( model_.get() );
    
    QStandardItemModel& model = *model_;
    model.setColumnCount( number_of_columns );
    model.setHeaderData( c_datafile, Qt::Horizontal, tr("Data name") );  // read only
    model.setHeaderData( c_data_type, Qt::Horizontal, tr("Data type") ); // read only
    model.setHeaderData( c_sample_type, Qt::Horizontal, tr("Sample type") ); // UNK/STD/QC
    model.setHeaderData( c_process, Qt::Horizontal, tr("Spectrum method") ); // Average (start,end)
    model.setHeaderData( c_level, Qt::Horizontal, tr("Level") );             // if standard
    model.setHeaderData( c_description, Qt::Horizontal, tr("Description") );             // if standard

    connect( this, &DataSequenceTree::onJoin, this, &DataSequenceTree::handleJoin );

    setAcceptDrops( true );
}

void
DataSequenceTree::setData( std::shared_ptr< adcontrols::datafile >& )
{
}

void
DataSequenceTree::setData( const QStringList& list )
{
    for ( int i = 0; i < list.size(); ++i ) {
        boost::filesystem::path path( list.at( i ).toStdWString() );
        auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
        if ( it != extensions.end() ) {
            dropIt( path.wstring() );
        }
    }
}

void
DataSequenceTree::handleData( int row )
{
    QStandardItemModel& model = *model_;

    dataSubscriber * data = 0;
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        auto it = std::find_if( dataSubscribers_.begin(), dataSubscribers_.end(), [row]( const std::shared_ptr< dataSubscriber >& d ){
                return d->row() == row;
            });
        if ( it != dataSubscribers_.end() )
            data = it->get();
    } while(0);

    if ( data ) {
        Infusion::setRow( model, row, "file" ); 
        setRaw( data, model.itemFromIndex( model.index( row, 0 ) ) );
        setProcessed( data, model.itemFromIndex( model.index( row, 0 ) ) );
    }
    expandAll();
}

void
DataSequenceTree::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    if ( index.column() == c_sample_type ) {
        if ( index.parent() == QModelIndex() ) {
            auto parent = model.item( index.row() );
            for ( int row = 0; row < parent->rowCount(); ++row ) {
                model.setData( model.index( row, c_sample_type, parent->index() ), index.data() );
            }
        }
    }
    // QStandardItemModel& model = *model_;
}

void
DataSequenceTree::handleIt( dataSubscriber * ptr )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    dataSubscribers_.push_back( ptr->shared_from_this() );
    emit onJoin( ptr->row() );
}

void
DataSequenceTree::handleJoin( int row )
{
    if ( dropCount_ && (--dropCount_ == 0) ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        std::for_each( threads_.begin(), threads_.end(), [] ( std::thread& t ){ t.join(); } );
        threads_.clear();
    }
    handleData( row );
}

void
DataSequenceTree::dropIt( const std::wstring& path )
{
    QStandardItemModel& model = *model_;
    int row = model.rowCount();

    if ( dataSubscribers_.empty() ) {
        // this is the workaround for boost/msvc bug #6320 ref: https://svn.boost.org/trac/boost/ticket/6320
        // so that make sure first call for boost::filesystem::path("") from main thread, and then safe to call from any thread.
        // Simple dummy call for boost::filesystem::path("") does not work due to adcontrols::open() invokes several dynamically 
        // loaded dlls such as Bruker, Agilent etc.
        auto reader = std::make_shared< dataSubscriber >( row, path, [] ( dataSubscriber * ){} );
        model.insertRow( row );
        model.setData( model.index( row, c_datafile ), QString::fromStdWString( path ) );
        reader->open(); 
        dataSubscribers_.push_back( reader );
        handleData( row );
    }
    else {
        std::lock_guard< std::mutex > lock( mutex_ );
        auto reader = std::make_shared< dataSubscriber >( row, path, [this] ( dataSubscriber * p ){ handleIt( p ); } );
        auto it = std::find_if( dataSubscribers_.begin(), dataSubscribers_.end()
                                , [=] ( const std::shared_ptr<dataSubscriber>& d ){ return d->filename() == path; } );
        if ( it == dataSubscribers_.end() ) {
            model.insertRow( row );
            model.setData( model.index( row, c_datafile ), QString::fromStdWString( path ) );
            ++dropCount_;
            threads_.push_back( std::thread( [reader] (){ reader->open(); } ) );
        }
    }
}

void
DataSequenceTree::dragEnterEvent( QDragEnterEvent * event )
{
	if ( const QMimeData * mimeData = event->mimeData() ) {
        if ( mimeData->hasUrls() ) {
            event->acceptProposedAction();            
            return;
            
            QList<QUrl> urlList = mimeData->urls();
            for ( int i = 0; i < urlList.size(); ++i ) {
                boost::filesystem::path path( urlList.at(i).toLocalFile().toStdWString() );
                auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
                if ( it != extensions.end() ) {
                    event->acceptProposedAction();
                    return;
                }
            }
        }
    }
}

void
DataSequenceTree::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
DataSequenceTree::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
DataSequenceTree::dropEvent( QDropEvent * event )
{
	if ( const QMimeData * mimeData = event->mimeData() ) {
        if ( mimeData->hasUrls() ) {
            QList<QUrl> urlList = mimeData->urls();
            for ( int i = 0; i < urlList.size(); ++i ) {
                QString file( urlList.at(i).toLocalFile() );
                boost::filesystem::path path( file.toStdWString() );
                auto it = std::find_if( extensions.begin(), extensions.end(), [path](const wchar_t * ext){ return path.extension() == ext; });
                if ( it != extensions.end() )
                    dropIt( path.wstring() );
            }
        }
    }
}

size_t // row count
DataSequenceTree::setRaw( dataSubscriber * data, QStandardItem * parent )
{
    QStandardItemModel& model = *model_;    
    if ( data ) {
        if ( auto raw = data->raw() ) {
            int n = int( raw->getFunctionCount() );
            parent->setRowCount( n );
            parent->setColumnCount( number_of_columns );

            for ( int fcn = 0; fcn < n; ++fcn ) {
                model.setData( model.index( fcn, c_datafile, parent->index() ),  QString( "Raw trace %1" ).arg( fcn + 1 ), Qt::EditRole );
                Infusion::setRow( model, fcn, "raw", parent->index() );
            }
            
            if ( data->ms() ) {
                adcontrols::segment_wrapper<> segs( *data->ms() );
                int fcn = 0;
                for ( auto& fms : segs ) {
                    std::wstring desc = fms.getDescriptions().toString();
                    model.setData( model.index( fcn++, c_description, parent->index() ), QString::fromStdWString( desc ), Qt::EditRole );
                }
            }
            return n;
        }
    }
    return 0;
}

size_t
DataSequenceTree::setProcessed( dataSubscriber * data, QStandardItem * parent )
{
    QStandardItemModel& model = *model_;

    size_t rowCount = 0;
    if ( data ) {

        if ( auto pf = data->processed() ) {

            for ( auto& folder : pf->folders() ) {

                if ( folder.name() == L"Spectra" ) {

                    int row = parent->rowCount();
                    if ( ( rowCount = folder.folio().size() ) ) {
                        parent->setRowCount( int( rowCount + row ) );
                        parent->setColumnCount( number_of_columns );

                        for ( auto& folium : folder.folio() ) {
                            model.setData( model.index( row, 0, parent->index() ), QString::fromStdWString( folium.name() ), Qt::EditRole );
                            Infusion::setRow( model, row, "spc", parent->index() );
                            ++row;
                        }
                    }
                }
            }
        }
    }
    return rowCount;
}
