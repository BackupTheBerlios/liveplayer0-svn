#!/bin/bash

# Install liveplayer...
./depends.sh

if [ "$?" != "0" ]
then
	echo "Sorry - a problem occured while installing the dependencies - abort"
	exit 1
fi

# Install qwt5
echo "Installing the qwt5 library..."
cd lib/qwt5
./install_qwt5.sh
cd ../..

if [ "$?" != "0" ]
then
	echo "Sorry - a problem occured while installing the dependencies (qwt5) - abort"
	exit 1
fi

# Compiling liveplayer
qmake-qt4 && make

if [ "$?" != "0" ]
then
	echo "Sorry - a problem occured while compiling liveplayer"
	exit 1
fi

echo "liveplayer seems to be compiled."
echo "You find the binary in the bin directory"
