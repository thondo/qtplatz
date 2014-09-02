/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <utils/fancymainwindow.h>
#include "dataprocconstants.hpp"
#include <memory>
#include <array>

namespace adportable { class Configuration; }
namespace adcontrols { class datafile; class ProcessMethod; class MSAssignedMasses; class MSPeaks; class MassSpectrum; }
namespace adprot { class digestedPeptides; }
namespace portfolio { class Folium; }
namespace Core { class IMode; }
namespace Utils { class StyledBar; }
class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;
class QStackedWidget;
class QLineEdit;
class QComboBox;

namespace dataproc {

    class Dataprocessor;
    class iSequenceImpl;

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        ~MainWindow();
        explicit MainWindow(QWidget *parent = 0);

        static MainWindow * instance();
        
        QWidget * createContents( Core::IMode *, const std::wstring& apppath );
        
        void activateLayout();
        void setSimpleDockWidgetArrangement();
        
        static QToolButton * toolButton( const char * );
        static QToolButton * toolButton( QAction * );
        
        void getProcessMethod( adcontrols::ProcessMethod& );
		void setProcessMethod( const adcontrols::ProcessMethod& );
        void OnInitialUpdate();
        void OnFinalClose();

        void applyCalibration( const adcontrols::MSAssignedMasses& );
        void applyCalibration( const adcontrols::MSAssignedMasses&, portfolio::Folium& );
        
        void processMethodSaved( const QString& );
        void processMethodLoaded( const QString&, const adcontrols::ProcessMethod& );
        int currentProcessView( std::string& ) const;
        void printCurrentView( const QString& ) const;

        void saveDefaultMSCalibrateResult( portfolio::Folium& );
        void lockMassHandled( const std::shared_ptr< adcontrols::MassSpectrum >& );
        void dataMayChanged();
        void zoomedOnSpectrum( const QRectF& );

        void proteinSelected( const adprot::digestedPeptides& );

        static QString makePrintFilename( const std::wstring& id, const std::wstring& insertor = L"__", const char * extension = ".svg" );
        static QString makeDisplayName( const std::wstring& id, const char * insertor = "::", int nbsp = 0 );
        static std::wstring foliumName( const std::wstring& id );
        static std::wstring portfolioFilename( const std::wstring& id );
        static QString currentDir();

        void handleProcessChecked();
        void handleImportChecked();
        void handleExportPeakList();

        enum idPage { idSelMSProcess, idSelElementalComp, idSelMSCalibration, idSelMSCalibSpectra
                      , idSelChromatogram, idSelMSPeaks, idSelSpectrogram, idSelSpectra, idNum };

        void selPage( idPage );
        enum idPage curPage() const;

		bool editor_factories( iSequenceImpl& );

    private:
        void handleProcessed( Dataprocessor *, portfolio::Folium& );

    public slots:
        void handleSessionAdded( Dataprocessor * );
        void handleSelectionChanged( Dataprocessor *, portfolio::Folium& );
        // void onMethodApply( adcontrols::ProcessMethod& );
        void actionApply();
        void handle_add_mspeaks( const adcontrols::MSPeaks& );
        void actCreateSpectrogram();
		void actClusterSpectrogram();

    private slots:
        void handleApplyMethod();
        void handleFeatureSelected( int );
        void handleFeatureActivated( int );
        void handlePeptideTarget( const QVector<QPair<QString, QString> >& );

        friend class MSPeaksWnd;

    private:
        QWidget * toolBar_;
        QHBoxLayout * toolBarLayout_;
        QComboBox * axisChoice_;
        
        QAction * actionSearch_;
        QAction * actionApply_;

        std::array< QAction *, idNum > selPages_;
        QStackedWidget * stack_;

        std::unique_ptr< QLineEdit > processMethodNameEdit_;
        enum ProcessType currentFeature_;

        void setToolBarDockWidget( QDockWidget * dock );
        void createToolbar();
        QDockWidget * createDockWidget( QWidget *, const QString& title = QString(), const QString& pageName = QString() );
        void createDockWidgets();
        Utils::StyledBar * createStyledBarTop();
        Utils::StyledBar * createStyledBarMiddle();
        void currentPageChanged( int );

    signals:
        void onPrintCurrentView( const QString& ) const;
        void onAddMSPeaks( const adcontrols::MSPeaks& ) const;
        void onDataMayCanged() const;
        void onZoomedOnSpectrum( const QRectF& ) const;
    };

}
    
#endif // MAINWINDOW_HPP