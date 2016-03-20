// This is a -*- C++ -*- header.
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

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <coreplugin/idocument.h>
#include <adcontrols/datasubscriber.hpp>
#include "constants.hpp"
#include <memory>
#include <vector>
#include <tuple>

namespace adcontrols {
    class datafile;
    class LCMSDataset;
    class ProcessMethod;
    class MassSpectrum;
    class MassSpectra;
    class Chromatogram;
    class MSAssignedMasses;
    class MSCalibrateMethod;
	class MSCalibrateResult;
    class CentroidMethod;
    class SpectrogramClusters;
    namespace lockmass { class mslock; }
}

namespace adprot { class digestedPeptides; }

namespace portfolio {
    class Portfolio;
    class Folium;
    class Folder;
}

namespace SignalObserver { class Observer; }

namespace dataproc {

    class IFileImpl;

    class Dataprocessor : public Core::IDocument
                        , public adcontrols::dataSubscriber
                        , public std::enable_shared_from_this< Dataprocessor > {
        Q_OBJECT
    public:
        ~Dataprocessor();
        Dataprocessor();

        inline Core::IDocument * document() { return this; };
        
        // Core::IDocument
        bool save( QString* errorString, const QString& filename = QString(), bool autoSave = false ) override;
        bool reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType ) override;

        QString defaultPath() const override;
        QString suggestedFileName() const override;
        bool isModified() const override;
        bool isSaveAsAllowed() const override;
        bool isFileReadOnly() const override;
        IDocument::ReloadBehavior reloadBehavior( ChangeTrigger state, ChangeType type ) const override;

        // Dataprocessor
        void setModified( bool );
        bool create( const QString& token );
        bool open( const QString&, QString& errmsg );
        // Core::IDocument * ifile();

        void exportXML() const;
        
		const std::wstring& filename() const;
        adcontrols::datafile& file();
        const adcontrols::LCMSDataset* getLCMSDataset();
        portfolio::Portfolio getPortfolio();
        bool load( const std::wstring& path, const std::wstring& id );
		bool fetch( portfolio::Folium& );
        void setCurrentSelection( portfolio::Folium& );
        void setCurrentSelection( portfolio::Folder& );
        void applyProcess( const adcontrols::ProcessMethod&, enum ProcessType );
        void applyProcess( portfolio::Folium&, const adcontrols::ProcessMethod&, enum ProcessType );
        void applyCalibration( const adcontrols::ProcessMethod& );
        void applyCalibration( const adcontrols::ProcessMethod&, const adcontrols::MSAssignedMasses&  );
        void applyCalibration( const adcontrols::ProcessMethod&, const adcontrols::MSAssignedMasses&, portfolio::Folium&  );
        portfolio::Folium currentSelection() const;

        // apply calibration to entire dataset
        void applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& );
		void lockMassHandled( const std::wstring& foliumId, const std::shared_ptr< adcontrols::MassSpectrum >&, const adcontrols::lockmass::mslock& );
        void formulaChanged();

        void sendCheckedSpectraToCalibration( Dataprocessor * );
        void removeCheckedItems();

        void createSpectrogram();
        void clusterSpectrogram();
        void findPeptide( const adprot::digestedPeptides& );
        
        portfolio::Folium addSpectrum( const adcontrols::MassSpectrum&, const adcontrols::ProcessMethod& );
        portfolio::Folium addSpectrum( std::shared_ptr< const adcontrols::MassSpectrum >&, const adcontrols::ProcessMethod& );
        portfolio::Folium addChromatogram( const adcontrols::Chromatogram&, const adcontrols::ProcessMethod&, bool checked = false );
        portfolio::Folium addSpectrogram( std::shared_ptr< adcontrols::MassSpectra >& );
        portfolio::Folium addSpectrogramClusters( std::shared_ptr< adcontrols::SpectrogramClusters >& );

        void subtract( portfolio::Folium& base, portfolio::Folium& target );

        portfolio::Portfolio& portfolio() { return *portfolio_; }

        static const std::shared_ptr< adcontrols::ProcessMethod > findProcessMethod( const portfolio::Folium& );
        static bool saveMSCalibration( portfolio::Folium& );
        static bool saveMSCalibration( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        static bool loadMSCalibration( const std::wstring&, adcontrols::MSCalibrateResult&, adcontrols::MassSpectrum& );

        // implement adcontrols::dataSubscriber
        bool subscribe( const adcontrols::LCMSDataset& ) override;
        bool subscribe( const adcontrols::ProcessedDataset& ) override;
        void notify( adcontrols::dataSubscriber::idError, const wchar_t * ) override;
        //
		bool onFileAdded( const std::wstring& path, adfs::file& ) override;
        // <------------------------
    private:
        void addCalibration( const adcontrols::MassSpectrum&, const adcontrols::ProcessMethod& );
        void addCalibration( const adcontrols::MassSpectrum& profile
                             , const adcontrols::MassSpectrum& centroid
                             , const adcontrols::MSCalibrateMethod&, const adcontrols::MSAssignedMasses& );

    private slots:

    signals :
        void onNotify( const QString& );

    private:
        std::shared_ptr< adcontrols::datafile > file_;
        std::unique_ptr< portfolio::Portfolio > portfolio_;
        std::wstring idActiveFolium_;
        const adcontrols::LCMSDataset * rawDataset_;
        bool modified_;

        void setDisplayName( const QString& );
    };

} // dataproc

#endif // DATAPROCESSOR_H
