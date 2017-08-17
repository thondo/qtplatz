#!/bin/bash

failed_list=()

#jpeg
list_dependency=('nasm' 'libass-dev' 'libfdk-aac-dev')

#ffmpeg
list_dependency+=('lame' 'libtheora-dev' 'libvorbis-dev'
			'libvpx-dev' 'libopus-dev' 'yasm' 'x264' 'libx264-dev' 'x265' 'libx265-dev'
			'libmp3lame0' 'libmp3lame-dev' )
#opencv
list_dependency+=('python-dev' 'python-numpy' 'libtbb2' 'libtbb-dev' 'libpng-dev' 'libtiff-dev'
		  'libpng12-dev' 'libtiff5-dev' 'libgtk2.0 pkg-config' 'libavcodec-dev'
		  'libavformat-dev' 'libswscale-dev' )

#sudo apt update

for arg in "${list_dependency[@]}"; do
    echo sudo apt-get install -y "$arg"
    sudo apt-get install -y "$arg" || failed_list+="$arg"
done

if [ ${#failed_list[@]} -gt 0 ]; then
   echo "Failed for " "${failed_list[@]}"
fi

#echo "Install dependency for for ffmpeg"
#sudo apt install -y nasm
#sudo apt install libass-dev
#sudo apt install libfdk-aac-dev
##sudo apt install FreeType-2.8, LAME-3.99.5,
#sudo apt install lame
#sudo apt install libtheora-dev
#sudo apt install libvorbis-dev
#sudo apt install libvpx-dev
#sudo apt install libopus-dev
#sudo apt install yasm
#sudo apt install x264
#sudo apt install libx264-dev
#sudo apt install x265
#sudo apt install libx265-dev
#sudo apt install libmp3lame0
#sudo apt install libmp3lame-dev

##dependency for opencv
#echo "Install dependency for OpenCV"
#sudo apt-get install -y python-dev python-numpy libtbb2 libtbb-dev # libjpeg-dev
#sudo apt-get install -y libpng-dev libtiff-dev
#sudo apt-get install -y libpng12-dev libtiff5-dev
#sudo apt-get install -y libgtk2.0 pkg-config libavcodec-dev libavformat-dev libswscale-dev
