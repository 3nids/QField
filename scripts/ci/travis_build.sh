#!/bin/bash

if [ "${TRAVIS_PULL_REQUEST}" = "false" ]; then
  APP_NAME="QField"
  PKG_NAME="qfield"
  APP_ICON="qfield-logo.svg"
elif [ "${TRAVIS_TAG}x" = "x" ]; then
  APP_NAME="QField Nightly"
  PKG_NAME="qfield_nightly"
  APP_ICON="qfield-testlogo.svg"
else
  APP_NAME="QField Beta ${TRAVIS_PULL_REQUEST}"
  PKG_NAME="qfield_beta"
  APP_ICON="qfield-testlogo.svg"
fi
docker run -v $(pwd):/usr/src/qfield -e "BUILD_FOLDER=build-${ARCH}" -e "ARCH=${ARCH}" -e "STOREPASS=${STOREPASS}" -e "KEYNAME=${KEYNAME}" -e "KEYPASS=${KEYPASS}" -e "VERSION=${TRAVIS_TAG}" -e "PKG_NAME=${PKG_NAME}" -e "APP_NAME=${APP_NAME}" -e "APP_ICON=${APP_ICON}" opengisch/qfield-sdk:${QFIELD_SDK_VERSION} /usr/src/qfield/scripts/docker-build.sh
