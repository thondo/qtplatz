//#include <QtCore/QCoreApplication>
#include <adcontrols/descriptions.h>
#include <adcontrols/massspectrum.h>
#include <fstream>
//#include <boost/serialization/string.hpp>
//#include <boost/serialization/nvp.hpp>
//#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/xml_woarchive.hpp>
//#include <boost/archive/xml_wiarchive.hpp>

int main()
{
    using namespace adcontrols;

    MassSpectrum ms;

    /*    
    desc.add( Description( L"key1", L"KANJI") );
    desc.add( Description( L"key2", L"KANJI") );
    desc.add( Description( L"key3", L"KANJI") );
    desc.add( Description( L"key4", L"KANJI 漢字") );
    desc.add( Description( L"key5", L"KANJI") );
*/
    double mass[10];
    double ints[10];
    for ( int i = 0; i < 10; ++i ) {
        mass[i] = 100 + 1.0 / 3;
        if ( i == 0 )
            ints[i] = 10;
        else
            ints[i] = ints[i-1] * ints[i-1] + mass[i];
    }


    std::wofstream of("output.xml");
    ms.resize( 10 );
    ms.setMassArray( mass );
    ms.setIntensityArray(  ints );
    of << ms.saveXml();
    of.close();
/*
    std::wstring xml = desc.saveXml();
    of << desc.saveXml();

    Descriptions copy;
    copy.loadXml( xml );
    size_t n = copy.size();

    std::wstring xml2 = desc.saveXml();

    of.close();
*/
}
