#!/bin/bash

VERSION="7.0.9.1"
RELEASE=ssv1
TAG=7.0.9.1

# script require 'sudo rpm' for install RPM packages
# and access to mirror.yandex.ru repository

pushd `dirname $0` >/dev/null
SCRIPT_DIR=`pwd`
popd >/dev/null

PATH=$PATH:$SCRIPT_DIR

# create build/RPMS folder - all built packages will be duplicated here
RES_TMP=build/TMP/
RES_RPMS=build/RPMS/
rm -rf "$RES_TMP"

mkdir -p $RES_TMP
mkdir -p $RES_RPMS

# download and install packages required for build
sudo yum -y install spectool yum-utils rpmdevtools redhat-rpm-config rpm-build libtool \
  glib2-devel gcc-c++ epel-rpm-macros cmake3 \
  || \
  { echo "can't install base packages" >&2 ; exit 1 ; }

# create folders for RPM build environment
mkdir -vp  `rpm -E '%_tmppath %_rpmdir %_builddir %_sourcedir %_specdir %_srcrpmdir %_rpmdir/%_arch'`
RPM_SOURCES_DIR=`rpm -E %_sourcedir`

rm -f "$RPM_SOURCES_DIR/ace-tao-$VERSION-$RELEASE.tar.gz"

# prepare archive
pushd "$RES_TMP"

rm -rf ACE_TAO
git clone https://github.com/yoori/ACE_TAO.git ACE_TAO

pushd ACE_TAO
git checkout tags/$TAG

git clone https://github.com/yoori/MPC.git MPC
cd MPC
git checkout tags/$TAG
cd ..

RPM/git-archive-all.sh --format tar.gz "$RPM_SOURCES_DIR/ace-tao-$VERSION-$RELEASE.tar.gz" \
  || \
  { echo "can't archive source" >&2 ; exit 1 ; }

popd

echo "archive created in $RPM_SOURCES_DIR/ace-tao-$VERSION.tar.gz"

BIN_RPM_FOLDER=`rpm -E '%_rpmdir/%_arch'`

SPEC_FILE=`rpm -E %_specdir`/ace-tao.spec
cp RPM/SPECS/ace-tao.spec "$SPEC_FILE"

$SUDO_PREFIX yum-builddep -y --define "_version $VERSION" --define "_release $RELEASE" "$SPEC_FILE" || \
  { echo "can't install build requirements" >&2 ; exit 1 ; }

echo "to download source"

spectool --force -g -R --define "_version $VERSION" --define "_release $RELEASE" "$SPEC_FILE" || \
  { echo "can't download source RPM" >&2 ; exit 1 ; }

echo "to build rpm"

rpmbuild --force -ba --define "_version $VERSION" --define "_release $RELEASE" "$SPEC_FILE" || \
  { echo "can't build RPM" >&2 ; exit 1 ; }

# install
cp $BIN_RPM_FOLDER/xgboost*.rpm $RES_RPMS/
