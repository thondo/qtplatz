#!/bin/sh

SRC=~/src
BOOST_VERSION=1_63_0
BOOST_INSTALL_DIR=boost-1_63
BOOST_BUILD_DIR=$SRC/boost_${BOOST_VERSION}

if [ ! -d $SRC ]; then
    mkdir $SRC
fi

cd $SRC;

if [ ! -d bzip2-1.0.6 ]; then
    wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
    tar xvf bzip2-1.0.6.tar.gz
fi

if [ ! -d $BOOST_BUILD_DIR ]; then
    VERSION=$(echo $BOOST_VERSION | tr _ .)
    wget https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
    tar xvf download
    mv download ~/Downloads/${BOOST_VERSION}.tar.bz2
fi

cd $BOOST_BUILD_DIR

./bootstrap.sh --prefix=/usr/local/$BOOST_INSTALL_DIR
./b2 -j4 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=${SRC}/bzip2-1.0.6

sudo ./b2 -j4 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=${SRC}/bzip2-1.0.6 install
echo "Setting up ld.so.conf.d"
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/boost.conf
sudo ldconfig

echo "=============================="
echo "   BOOST $BOOST_VERSION Installed "
echo "=============================="
