#!/bin/bash

############
# Version  #
############
echo "Library Installation. Version $RELIABILITY_FRAMEWORK_VERSION"

# SOURCES_FOLDER=/simulation_data/circuit_reliability_framework/installers_last_version/lib_tmp/src/
# BINARY_FOLDER=/simulation_data/circuit_reliability_framework/installers_last_version/lib_tmp/bin
SOURCES_FOLDER=libcircuit_reliability/src/
BINARY_FOLDER=libcircuit_reliability/bin
BASH_VARIABLES_FILES=bash_variables/*
BASH_MAIN_FILE_PATH=~/ownCloud/workspace/bash_environment_conf/bash_fgarcia_workspace
LIBRARY_NAME=libcircuit_reliability.so
BINARY=$LIBRARY_NAME.$RELIABILITY_FRAMEWORK_VERSION
INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY=/usr/include/circuit_reliability
LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY=/usr/lib/circuit_reliability

echo ""
echo ""
echo "*******************************************************************************"
echo "**  In order to install Spectre Handler Library in your system,              **"
echo "**  the binary should be copied to your computer.                            **"
echo "**  Additionally, if you want to develop your program using this libraries,  **"
echo "**  some headers files should be placed and referenced.                      **"
echo "*******************************************************************************"
echo ""


# Headers
if [ ! -d $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY ]; then
	echo "Privileges needed while creating $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY"
	sudo mkdir $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY
	sudo chown $USER $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY
	sudo chmod -R 755 $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY
fi

if [ -d $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY ]; then
	echo "Copying bash_cadence and bash_cadence_develop to home directory"
	cp $BASH_VARIABLES_FILES ~
	echo "Copied"
	if [ ! "$BASH_VERSION" ] ; then
		#echo "Please do not use sh to run this script ($0), just execute it directly" 1>&2
		echo "Importing bash environment files: '. $BASH_MAIN_FILE_PATH'"
		. $BASH_MAIN_FILE_PATH
	else
		echo "Importing bash environment files: 'source $BASH_MAIN_FILE_PATH'"
		source $BASH_MAIN_FILE_PATH
	fi
	echo ""
	echo ""
	echo "Copying header files in $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY"
	echo "Be aware of including this folder in the compilation sentence."
	echo "example: 'g++  -std=c++11 -Wall -O3 -I/usr/include/circuit_reliability   -c -o main.o main.cpp'"
	## copy all the headers, preserving the folders structure
	# rsync -avm --include='*.hpp' -f 'hide,! */' $SOURCES_FOLDER $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY
	# copy all the headers, preserving the folders structure
	sudo rsync -avm --include='*.*' -f 'hide,! */' $SOURCES_FOLDER $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY
else
	echo ""
	echo ""
	echo "Error, $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY does not exist."
fi

# Binary
if [ ! -d $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY ]; then
	echo ""
	echo ""
	echo "Privileges needed while creating $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY"
	sudo mkdir $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY
	sudo chown $USER $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY
	sudo chmod -R 755 $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY
fi

if [ -d $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY ]; then
	echo ""
	echo ""
	echo "Copying binary files in $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY"
	sudo cp $BINARY_FOLDER/$BINARY $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY

#	Creates library link
	echo "Creates library link:    ln -sf $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY/$BINARY $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY/$LIBRARY_NAME "
	cd $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY

	if [ -d $LIBRARY_NAME ]; then
		echo "Removing old library"
		sudo rm $LIBRARY_NAME
	fi
	sudo ln -sf $BINARY $LIBRARY_NAME
	# rsync -avm --include='*.hpp' -f 'hide,! */' $SOURCES $INCLUDE_CIRCUIT_RELIABILITY_DIRECTORY
	echo ""
	echo ""
	echo "All done. Be aware of including this folder in your path."
	echo "example: 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY'"
else
	echo ""
	echo ""
	echo "Error, $LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY does not exist."
fi
