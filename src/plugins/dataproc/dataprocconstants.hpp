#ifndef DATAPROCCONSTANTS_HPP
#define DATAPROCCONSTANTS_HPP

namespace dataproc {
namespace Constants {

    const char ACTION_ID[] = "Dataproc.Action";
    const char MENU_ID[] = "Dataproc.Menu";

    const char * const C_DATAPROCESSOR        = "Dataprocessor";
    const char * const C_DATAPROC_MODE        = "Dataproc.Mode";
    const char * const C_DATAPROC_NAVI        = "Dataproc.Navigator";
    const char * const ABOUT_QTPLATZ          = "Dataproc.AboutQtPlatz";
    const char * const C_DATA_TEXT_MIMETYPE   = "application/txt";
    const char * const C_DATA_NATIVE_MIMETYPE = "application/adfs";

    // common actions
    const char * const METHOD_OPEN          = "dataproc.MethodOpen";
    const char * const METHOD_SAVE          = "dataproc.MethodSave";
    const char * const METHOD_APPLY         = "dataproc.MethodApply";
    const char * const PRINT_CURRENT_VIEW   = "dataproc.PrintCurrentView";
    const char * const CALIBFILE_APPLY      = "dataproc.ApplyCalibration";
    const char * const PROCESS_ALL_CHECKED  = "dataproc.ProcessAllCheckedSpectra";
    const char * const IMPORT_ALL_CHECKED   = "dataproc.ImportAllCheckedSpectra";
    const char * const LISTPEAKS_ON_CHECKED = "dataproc.PeakListAllChecked";

    const char * const CREATE_SPECTROGRAM   = "dataproc.Spectrogram";
    const char * const CLUSTER_SPECTROGRAM  = "dataproc.ClusterSpectrogram";

    // edit
    const char * const CHECK_ALL_SPECTRA   = "dataproc.Edit.CheckAllSpectra";
    const char * const UNCHECK_ALL_SPECTRA = "dataproc.Edit.UncheckAllSpectra";

    // icon
    const char * const ICON_METHOD_SAVE     = ":/dataproc/image/filesave.png";
    const char * const ICON_METHOD_OPEN     = ":/dataproc/image/fileopen.png";
    const char * const ICON_SAVE            = ":/dataproc/image/filesave.png";
    const char * const ICON_OPEN            = ":/dataproc/image/fileopen.png";
    const char * const ICON_METHOD_APPLY    = ":/dataproc/image/apply_small.png";
    const char * const ICON_PDF             = ":/dataproc/image/file_pdf.png"; // http://findicons.com/icon/74870/file_pdf?id=355001
    const char * const ICON_CALIBFILE       = ":/dataproc/image/calibration32.png";
    // freeware license, Designed by Andy Gongea
    // Folium (attachment) name
    const wchar_t * const F_DFT_FILTERD        = L"DFT Low Pass Filtered Spectrum";
    const wchar_t * const F_CENTROID_SPECTRUM  = L"Centroid Spectrum";
    const wchar_t * const F_MSPEAK_INFO        = L"MSPeakInfo";
    const wchar_t * const F_TARGETING          = L"Targeting";

    // settings
    const char * const GRP_DATA_FILES       = "DataFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";

    // shared with Quan
    const wchar_t * const F_QUANSAMPLE         = L"QuanSample";

    // QObject names
    const char * const EDIT_PROCMETHOD    = "dataproc.MainWindow.procmethodname";

} // Constants

    enum ProcessType {
        CentroidProcess
        , TargetingProcess
        , CalibrationProcess
        , PeakFindProcess 
    };

    enum AxisType {
        AxisMZ
        , AxisTime
    };


} // namespace dataproc

#endif // DATAPROCCONSTANTS_HPP

