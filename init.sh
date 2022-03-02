ROOT_PATH="$( cd "$( dirname "$0" )" && pwd -P )"
echo ${ROOT_PATH}

BUILD_PATH=${ROOT_PATH}/build
if [ -d "$BUILD_PATH" ]; then
  echo "Remove build directory"
  rm -rf "$BUILD_PATH"
fi

mkdir ${BUILD_PATH}
pushd ${BUILD_PATH}
  cmake ${ROOT_PATH}
  make -j8
popd
