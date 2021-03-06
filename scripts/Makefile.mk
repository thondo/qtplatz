#!make

default:
	@echo "====================================="
	@echo "make dependency"
	@echo "make cross-tools"
	@echo "make boost"
	@echo "=== Don't forget apply boost-nvcc_hpp.patch if using cuda"
	@echo "make boost-armhf"
	@echo "make cmake"
	@echo "make rdkit"
	@echo "make qwt"
	@echo "make gnuplot"
	@echo "make rtags"
#	@echo "make ffmpeg"
#	@echo "make jpeg"
	@echo "make opencv" || "make opencv.debug"
#	@echo "make arrayfire"
	@echo " -- or make world"

dependency:
	./linux-initial-setup.sh
	./linux-dependency-install.sh

cross-tools:
	./armhf/debian-multiarch-setup.sh

boost:
	./linux-boost-setup.sh

boost-armhf:
	cross_target=armhf ./linux-boost-setup.sh

rdkit:
	./linux-rdkit-setup.sh

qwt:
	./linux-qwt-setup.sh

gnuplot:
	./linux-gnuplot-setup.sh

ffmpeg:
	./linux-ffmpeg-setup.sh

jpeg:
	./linux-jpeg-setup.sh

opencv:
	./linux-opencv-setup.sh

opencv.debug:
	./linux-opencv-setup.sh debug

arrayfire:
	./linux-arrayfire-setup.sh

rtags:
	./linux-rtags-setup.sh

cmake:
	./linux-cmake-setup.sh

world: dependency cross-tools boost boost-armhf cmake rdkit qwt gnuplot opencv rtags
