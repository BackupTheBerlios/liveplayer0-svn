#!/bin/sh

# Variables globales
REP=""
PREFIX_1="/usr/"
PREFIX_2="/usr/local/"
INCLUDE="include/"
LIBS="lib/"
DEPEND_FILE=""
MISSING_FILE="depends_missing.cache"

# Librairies recherchees
LIBASOUND="libasound.so*"
LIBASOUND_DEV="asoundlib.h"
LIBSNDFILE="libsndfile.so*"
FAKELIB="fake.so*"
FAKELIB_DEV="fake.h"

echo ""

# select depends file
if [ -z "$1" ]
then
	echo -n "Try to find distrib name"
	# Ubuntu
	if [ -e '/usr/share/ubuntu-docs' ]
	then
		echo "			Selecting Ubuntu"
		DEPEND_FILE="depends-ubuntu-606"
	# Debian...
	elif [ -e '/etc/debian_version' ]
	then
		echo "			Selecting Debian"
		DEPEND_FILE="depends-debian"
	fi

	# ADD OTHER DISTRIBUTIONS HERE
else
	case "$1" in
		"debian")
		DEPEND_FILE="depends-debian";;
		# ADD OTHER DISTRIBUTIONS HERE
		*)
		echo "Distribution not listed - must find manually missing package names";;
	esac
fi

echo ""

# Cherche si un fichier demande existe
# Argument 1: librairie
# Si l'argument 2 contient la chaine 'DEV'
# on cherche les entetes (header, cad dans /PRFIX/include)
FIND() {
	local TMP

	if [ -z "$1" ]
	then
		echo "Warning, nothing given to search"
		return -1
	fi

	if [ "$2" == "DEV" ]
	then
		echo -n "Searching for $1 headers in $PREFIX_1			"
		TMP=$(find "$PREFIX_1""$INCLUDE" -name "$1" -print)
		if [ ! -z "$TMP" ]
		then
			echo "OK"
			return 0
		else
			echo "Not found"
		fi
		echo -n "Searching for $1 headers in $PREFIX_2			"
		TMP=$(find "$PREFIX_2""$INCLUDE" -name "$1" -print)
		if [ ! -z "$TMP" ]
		then
			echo "OK"
		else
			echo "Not found"
			# Donner ici la corespondance PKG name
			echo "$1 -> "$(FIND_PKG "$1" 'DEV') >> "$MISSING_FILE"
		fi
	else
		echo -n "Searching for $1 in $PREFIX_1				"
		TMP=$(find "$PREFIX_1""$LIBS" -name "$1" -print)
		if [ ! -z "$TMP" ]
		then
			echo "OK"
			return 0
		else
			echo "Not found"
		fi
		echo -n "Searching for $1 in $PREFIX_2				"
		TMP=$(find "$PREFIX_2""$LIBS" -name "$1" -print)
		if [ ! -z "$TMP" ]
		then
			echo "OK"
		else
			echo "Not found"
			# Donner ici la corespondance PKG name
			echo "$1 -> "$(FIND_PKG "$1") >> "$MISSING_FILE"
		fi
	fi

	#find "$PREFIX_1""$2" -name "$1" -print
}

# Cherche une concordance nom-de-packet <-> nom de librairie dans
# Si l'argument 2 recoit la chaine 'DEV', on retourne le nom de
# paquet -dev  (headers)
FIND_PKG() {
	local TMP

	if [ -z "$1" ]
	then
		echo "Warning, nothing given to search"
		return -1
	fi

	if [ -z "$DEPEND_FILE" ]
	then
		echo "Warning, no depends file specified"
		return 1
	fi

	if [ "$2" == "DEV" ]
	then
		grep -v '#' "$DEPEND_FILE" | grep "$1" | cut -d= -f 2 | tr -d \" | tr -d ' '
		grep -v '#' "$DEPEND_FILE" | grep "$1" | cut -d= -f 3 | tr -d \" | tr -d ' '
	else
		grep -v '#' "$DEPEND_FILE" | grep -v 'DEV' | grep "$1" | cut -d= -f 2 | tr -d \" | tr -d ' '
		grep -v '#' "$DEPEND_FILE" | grep -v 'DEV' | grep "$1" | cut -d= -f 3 | tr -d \" | tr -d ' '
	fi
}

# MAIN
#FIND $LIBASOUND
#FIND $LIBASOUND_DEV 'DEV'

#FIND $FAKELIB
#FIND $FAKELIB_DEV

for ENTRY in $(grep -v '#' "$DEPEND_FILE" | cut -d= -f 1)
do
	if [ ! -z $(echo "$ENTRY" | grep '.h$') ]
	then
		FIND "$ENTRY" 'DEV'
	else
		FIND "$ENTRY"
	fi
done


# Show missings if exists
if [ -e "$MISSING_FILE" ]
then
	echo ""
	echo 'Missing dependencies:'
	echo ""
	cat "$MISSING_FILE"
	rm  "$MISSING_FILE"
	echo ""
	exit 1
else
	echo ""
	echo "All dependencies seems OK"
	echo ""
fi

