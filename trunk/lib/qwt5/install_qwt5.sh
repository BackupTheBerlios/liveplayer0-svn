#!/bin/bash

# This script try to install qwt5 witch is needed for Qt4
# We assume that all dependencies are installed
# To make it uninstallable when this version comes integrated 
# to the modern distributions, I choose to place the sources in
# /usr/src/qwt5 (please change the variable if needed)
SRC_ROOT="/usr/src/qwt5"
QWT5="qwt-5.0.1"
QWT5_TARBALL="$QWT5.tar.bz2"

# containers, don't edit.
ERRORS=""
COMPILE_DES_PLUG=""

# Unsinstall if requierd
if [ "$1" == "--uninstall" ]
then
	echo "Uninstalling the qwt5 library..."
	cd "$SRC_ROOT/$QWT5"
	sudo make uninstall
	sudo rm /usr/lib/libqwt.so
	sudo rm /usr/lib/libqwt.so.5
	exit 0
fi

# create a directory in SRC_ROOT
if [ ! -e "$SRC_ROOT/$QWT5" ]
then
	echo "creating $SRC_ROOT/$QWT5..."
	sudo mkdir -p "$SRC_ROOT/$QWT5"
fi

# Compile designer plugins
echo -n "Install the plugins for QT Designer [y/n] ? "
read ANS
if [ "$ANS" == "y" ]
then
	COMPILE_DES_PLUG="true"
fi

if [ ! -e "$QWT5_TARBALL" ]
then
	echo "sorry - cannot find $QWT5_TARBALL"
	echo "please place this installer in the directory in witch the tarball exists,"
	echo "or specify another path in this script"
	exit 1
fi

# extract and copy
tar -xjf "$QWT5_TARBALL"


# Compile library
cd "$QWT5"
qmake-qt4 qwt.pro
make
if [ "$?" != "0" ]
then
	echo "Sorry - an error occured while compiling qwt"
	exit 1
fi

# Compile designer plugins
if [ "$COMPILE_DES_PLUG" == "y" ]
then
	cd designer
	qmake qwtplugin.pro
	make
	if [ "$?" != "0" ]
	then
		ERRORS="$ERRORS"" - Error while compiling the qwt5 QT Designer plugins"
	fi
	cd ..
fi

# Copy to SRC_ROOT/QWT5
sudo cp -rp * "$SRC_ROOT/$QWT5"
cd "$SRC_ROOT/$QWT5"
sudo make install
if [ "$COMPILE_DES_PLUG" == "y" ]
then
	cd designer
	sudo make install
fi

# create the links
sudo ln -s /usr/local/qwt/lib/libqwt.so /usr/lib/libqwt.so
sudo ln -s /usr/local/qwt/lib/libqwt.so.5 /usr/lib/libqwt.so.5
echo "Execute ldconfig..."
sudo ldconfig

echo "Install of libqwt5 seems to bo OK"
