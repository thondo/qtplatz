SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   /opt/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /opt/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-g++)

# where is the target environment
get_filename_component (_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
SET( CMAKE_FIND_ROOT_PATH /opt/raspi/arm-linux-gnueabihf /opt/raspi/arm-linux-gnueabihf-rootfs ${_dir} )
set( CMAKE_SYSROOT /opt/raspi/arm-linux-gnueabihf-rootfs )

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

