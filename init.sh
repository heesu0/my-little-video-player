#!/bin/bash

ROOT_PATH="$( cd "$( dirname "$0" )" && pwd -P )"
echo ${ROOT_PATH}

BUILD_PATH=${ROOT_PATH}/Build
if [ -d "$BUILD_PATH" ]; then
  echo "Remove build directory"
  rm -rf "$BUILD_PATH"
fi

mkdir ${BUILD_PATH}
cd ${BUILD_PATH}
  cmake -D CMAKE_BUILD_TYPE=Debug ${ROOT_PATH} 
  make -j8
cd ..
