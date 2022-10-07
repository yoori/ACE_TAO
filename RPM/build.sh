#!/bin/bash

VERSION=$1
RELEASE=$2
TAG=$VERSION-$RELEASE

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
sudo yum -y install spectool yum-utils rpmdevtools redhat-rpm-config rpm-build epel-rpm-macros \
  || \
  { echo "can't install base packages" >&2 ; exit 1 ; }

# create folders for RPM build environment
mkdir -vp  `rpm -E '%_tmppath %_rpmdir %_builddir %_sourcedir %_specdir %_srcrpmdir %_rpmdir/%_arch'`
RPM_SOURCES_DIR=`rpm -E %_sourcedir`

rm -f "$RPM_SOURCES_DIR/ace-tao-$VERSION-$RELEASE.tar.gz"

# prepare archive
pushd "$RES_TMP"

rm -rf "ace-tao-$VERSION-$RELEASE.orig"
mkdir "ace-tao-$VERSION-$RELEASE.orig"
cd "ace-tao-$VERSION-$RELEASE.orig"

echo ">>>> git clone --branch $TAG https://github.com/yoori/ACE_TAO.git ACE_TAO"
git clone -c advice.detachedHead=false --branch $TAG https://github.com/yoori/ACE_TAO.git ACE_TAO
git clone -c advice.detachedHead=false --branch $TAG https://github.com/yoori/MPC.git MPC
cd ..

rm -rf "ace-tao-$VERSION-$RELEASE/"
echo ">>> Before rsync pwd : $(pwd)"
ls -l
echo ">>> Rsync : rsync -av --exclude '/build' --exclude "\"".git"\"" --exclude "\"".github"\"" --exclude "\"".svn"\"" ace-tao-$VERSION-$RELEASE.orig/ ace-tao-$VERSION-$RELEASE/"
rsync -av --exclude '/build' --exclude ".git" --exclude ".github" --exclude ".svn" "ace-tao-$VERSION-$RELEASE.orig/" "ace-tao-$VERSION-$RELEASE/" >/dev/null

echo ">>> tar : tar -czvf $RPM_SOURCES_DIR/ace-tao-$VERSION-$RELEASE.tar.gz ace-tao-$VERSION-$RELEASE"
tar -czvf "$RPM_SOURCES_DIR/ace-tao-$VERSION-$RELEASE.tar.gz" "ace-tao-$VERSION-$RELEASE" \
  || \
  { echo "can't archive source" >&2 ; exit 1 ; }

#ace-tao-$VERSION-$RELEASE/ACE_TAO/RPM/git-archive-all.sh --format tar.gz "$RPM_SOURCES_DIR/ace-tao-$VERSION-$RELEASE.tar.gz" \
#  || \
#  { echo "can't archive source" >&2 ; exit 1 ; }

echo "Before spec cp: $(pwd)"
ls -l

SPEC_FILE=$(rpm -E %_specdir)/ace-tao-$VERSION-$RELEASE.spec
cp "ace-tao-$VERSION-$RELEASE/ACE_TAO/RPM/SPECS/ace-tao.spec" "$SPEC_FILE"

popd

BIN_RPM_FOLDER=$(rpm -E '%_rpmdir/%_arch')

echo ">>>> build dep"

$SUDO_PREFIX yum-builddep -y --define "_version $VERSION" --define "_release $RELEASE" "$SPEC_FILE" || \
  { echo "can't install build requirements" >&2 ; exit 1 ; }

echo ">>>> to build rpm: rpmbuild --force -ba --define "\""_version $VERSION"\"" --define "\""_release $RELEASE"\"" "\""$SPEC_FILE"\"

rpmbuild --force -ba --define "_version $VERSION" --define "_release $RELEASE" "$SPEC_FILE" || \
  { echo "can't build RPM" >&2 ; exit 1 ; }

# install
cp $BIN_RPM_FOLDER/ace-tao-*.rpm $RES_RPMS/
