#!/bin/bash

set -e

CURRENTVER=
PLATFORM=$1
LIBTYPE=STATIC
INITPATH=${PATH}

# set env
if [[ "${PLATFORM}" == "hisi_3518e" ]];then
	export PATH=/opt/hisi-linux-3518e/x86-arm/arm-hisiv300-linux/target/bin/:${INITPATH}
fi

# cmake and build program
pushd .

rm ./cmake -fr
mkdir -p  cmake
cd cmake
cmake -DCMAKE_TOOLCHAIN_FILE=../${PLATFORM}.toolchain.cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPLATFORM=${PLATFORM} -DCURVER="${CURRENTVER}" ..
make
echo "-- make done"

cp -af ./mbedtls-2.3.0/library/*.a ../


popd


