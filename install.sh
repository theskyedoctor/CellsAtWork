#!/bin/bash

SOURCE_DIR=$(pwd)
BUILD_DIR=${SOURCE_DIR}/build


mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}" || exit

cmake "${SOURCE_DIR}"

cmake --build "${BUILD_DIR}"
