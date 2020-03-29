#!/bin/bash

#
#   Global variables
#
SCRIPT_FOLDER="$( cd "$( dirname "$0" )" && pwd )"
CRYPTO_C_DIR="${SCRIPT_FOLDER}/../ext/virgil-crypto-c"
BUILD_DIR_BASE="${CRYPTO_C_DIR}"

#***************************************************************************************
check_error() {
   RETRES=$?
   if [ $RETRES != 0 ]; then
        echo "----------------------------------------------------------------------"
        echo "############# !!! PROCESS ERROR ERRORCODE=[$RETRES]  #################"
        echo "----------------------------------------------------------------------"
        [ "$1" == "0" ] || exit $RETRES
   else
        echo "-----# Process OK. ---------------------------------------------------"
   fi
   return $RETRES
}

#
#   Arguments
#
PLATFORM="host"

#
#   Pack libraries to one
#
function pack_libs() {
    LIBS_DIR=${1}
    FINAL_LIB="libvscryptoc.a"

    pushd ${LIBS_DIR}

      local LIBS=( "libed25519.a" "libmbedcrypto.a" "libprotobuf-nanopb.a" "libvsc_common.a" "libvsc_foundation.a" "libvsc_foundation_pb.a")

      # Split static lib to object files
      for LIB in "${LIBS[@]}"; do
          ar x ${LIB}
          check_error
          rm ${LIB}
      done

			# Combine all object files to a static lib
			ar rcs ${FINAL_LIB} *.o
			check_error

      # Clean up object files
      rm *.o

    popd
}

#
#   Build
#
function build() {
    local BUILD_TYPE=$1
    local CMAKE_ARGUMENTS=$2
    local CORES=10

    local BUILD_DIR=${BUILD_DIR_BASE}/cmake-build-${PLATFORM}/${BUILD_TYPE}
    local INSTALL_DIR=${BUILD_DIR_BASE}/cmake-build-${PLATFORM}/${BUILD_TYPE}/installed
    [ "$(arch)" == "x86_64" ] && LIB_ARCH="64" || LIB_ARCH=""
    local LIBS_DIR=${INSTALL_DIR}/usr/local/lib${LIB_ARCH}

    echo
    echo "===================================="
    echo "=== ${PLATFORM} ${BUILD_TYPE} build"
    echo "=== Output directory: ${BUILD_DIR}"
    echo "===================================="
    echo

    rm -rf ${BUILD_DIR}
    mkdir -p ${BUILD_DIR}
    mkdir -p ${INSTALL_DIR}

    pushd ${BUILD_DIR}
      # prepare to build
      cmake ${BUILD_DIR_BASE} ${CMAKE_ARGUMENTS} -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -G "Unix Makefiles"
      check_error

      # build all targets
      make -j ${CORES}
      check_error

      # install all targets
      make DESTDIR=${INSTALL_DIR} install
      check_error

      pack_libs ${LIBS_DIR}

    popd
}

# Common CMake arguments for the project
CMAKE_ARGUMENTS="-DCMAKE_CXX_FLAGS='-fvisibility=hidden' -DCMAKE_C_FLAGS='-fvisibility=hidden' \
-DENABLE_CLANGFORMAT=OFF \
-DENABLE_CLANGFORMAT=OFF \
-DVIRGIL_PHP_TESTING=OFF \
-DVIRGIL_LIB_PYTHIA=OFF \
-DVIRGIL_LIB_RATCHET=OFF \
-DVIRGIL_LIB_PHE=OFF \
-DVIRGIL_POST_QUANTUM=OFF"

#
#   Build both Debug and Release
#
build "release" "${CMAKE_ARGUMENTS}"