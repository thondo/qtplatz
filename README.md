Visit
	http://www.qtplatz.com

BUILD PROCEDURE

 Prerequisite
===============

1. Qt 5.12
2. QWT
3. Boost
4. RDKit
5. OpenCV

Linux platform also require following modules:
libxml2-dev, libxslt1-dev, mesa-common-dev, bc, libncurses5-dev libgstreamer-plugins-base0.10

------------
Check Boost serialization version_type
-----------
Check <boost-source-dir>/libs/serialization/src/basic_archive.cpp
find library_version_type returned by BOOST_ARCHIVE_VERSION()
Current QtPlatz data acquisition is based on 14.

------
 QWT (Qt 5.9 should be installed before compile QWT)
------
// change nmake rather than make on windows

svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1
cd qwt-6.1
<edit qwtconf.pro ...>
qmake qwt.pro
make
make install

--------
 Boost
--------
// On Mac
./bootstrap.sh --prefix=/usr/local/boost-1_62 --with-toolset=clang
sudo ./b2 -j4 toolset=clang cxxflags="-std=c++11 -stdlib=libc++" linkflags="-stdlib=libc++" -s BZIP2_SOURCE=~/src/bzip2-1.0.6 install

// On Linux
./bootstrap.sh
sudo ./b2 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=~/src/bzip2-1.0.6 install

// On Windows (Win64 Visual Studio 14)
.\b2 -j4 --stagedir=./x86_64 --toolset=msvc-14.0 architecture=x86 address-model=64 -s BZIP2_SOURCE=%USERPROFILE%\src\bzip2-1.0.6 link=static stage
<move x86_64 directory under C:/Boost>

------------------------
 RDKit (require Boost) on Windows
------------------------
git clone git://github.com/rdkit/rdkit
mkdir rdkit\build; cd rdkit\build
cmake -DBOOST_LIBRARYDIR=C:\Boost\x86_64\lib -DBOOST_ROOT=C:\Boost\include\boost-1_62 -DBoost_USE_STATIC_LIBS=ON -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF -DCMAKE_DEBUG_POSTFIX="d" -G "Visual Studio 14 Win64" ..
msbuild /m:4 /p:Configuration=Debug INSTALL.vcxproj
msbuild /m:4 /p:Configuration=Release INSTALL.vcxproj

------------------------
 RDKit (require Boost) on Linux & Mac
------------------------
git clone git://github.com/rdkit/rdkit
cd rdkit
export RDBASE=`pwd`
mkdir build; cd build
cmake -DBOOST_ROOT=/usr/local/boost-1_62 -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF ..
cmake -DBOOST_ROOT=/usr/local/boost-1_62 -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF -DRDK_INSTALL_INTREE=OFF -DCMAKE_MACOSX_RPATH=TRUE ..
make -j8
make test
make install
