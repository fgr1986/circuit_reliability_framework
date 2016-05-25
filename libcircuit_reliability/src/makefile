##########################################
###  Spectre Handler Library makefile  ###
##########################################

# # of cores used during compiling
export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"
# export MAKEFLAGS="-j 2"
# export MAKEFLAGS="-j 1"
############
# Version  #
############
VERSION=$(LIBCIRCUIT_RELIABILITY_VERSION)

#################
# Compiler def  #
#################
CC=g++

#################
# Gcc flags     #
#################
#CPPFLAGS+= -Wall -g // Debugg it turns on (almost) all the warnings that g++ can tell you about
#CPPFLAGS+= -Wall -O3 -fdump-tree-cfg-graph -fdump-rtl-expand-graph  // rtl graphs, O3
#CPPFLAGS+= -Wall -O3 -g -fdump-tree-original-raw -fdump-tree-all-all -finstrument-functions // rtl graphs, O3
#CPPFLAGS+= -std=c++11 -Wall -O3 -I $(MATLAB_ROOT)extern/include // Matlab, c++11
# CPPFLAGS+= -g -std=c++11 -Wall -O3 -v // verbose
# CPPFLAGS+= -std=c++11 -Wall -O3 -fPIC	// needed for dynamic shared library

# fPIC: Position Independent Code means that the generated machine code
# is not dependent on being located at a specific address in order to work.
CPPFLAGS+= -std=c++11 -Wall -O3 -fPIC

#################
# Library flags #
#################
# // Matlab, c++11, boost
LDFLAGS= -L/usr/lib -lboost_thread -lboost_regex -lboost_filesystem -lboost_system -shared

######################
# Sources definition #
######################
SRC= $(wildcard *.cpp)
SRC+= $(wildcard io_handling/*.cpp)
SRC+= $(wildcard netlist_modeling/*.cpp)
SRC+= $(wildcard netlist_modeling/statements/*.cpp)
SRC+= $(wildcard simulations_handling/*.cpp)
SRC+= $(wildcard simulations_handling/spectre_handlers/*.cpp)
SRC+= $(wildcard simulations_handling/spectre_simulations/*.cpp)
SRC+= $(wildcard simulations_handling/simulation_results/*.cpp)
SRC+= $(wildcard global_functions_and_constants/*.cpp)

######################
# Objects definition #
######################
OBJECTS= $(SRC:.cpp=.o)

######################
# Headers Definition #
######################
HEADERS= $(wildcard *.hpp)
HEADERS+= $(wildcard io_handling/*.hpp)
HEADERS+= $(wildcard netlist_modeling/*.hpp)
HEADERS+= $(wildcard netlist_modeling/statements/*.hpp)
HEADERS+= $(wildcard simulations_handling/*.hpp)
HEADERS+= $(wildcard simulations_handling/spectre_handlers/*.hpp)
HEADERS+= $(wildcard simulations_handling/spectre_simulations/*.hpp)
HEADERS+= $(wildcard simulations_handling/simulation_results/*.hpp)
HEADERS+= $(wildcard global_functions_and_constants/*.hpp)
# Headers to be included in dist tar
DIST_HEADERS=  $(addprefix "../src/", $(HEADERS))
# SRC cpp to be included in dist tar
DIST_CPP=  $(addprefix "../src/", $(SRC))
# makefile to be included in dist tar
DIST_MAKE= $(addprefix "../src/", makefile)


################
# target files #
################
# binary file
LIBRARY_NAME=libcircuit_reliability.so
BINARY=$(LIBRARY_NAME).$(VERSION)
# release file
RELEASE_FILE=$(LIBRARY_NAME)_release_${VERSION}.tar.gz

###############
## Folders   ##
###############
# binary folder
EXECDIR=../bin/
# conf folder
CONFDIR=../conf_files/
# dist dir
DIST_DIR=../dist/

###################
## Dist elements ##
###################
# binary and link
# DIST= $(EXECDIR)$(BINARY)
DIST= $(EXECDIR)
# headers
DIST+=$(DIST_HEADERS)
# src cpp
DIST+=$(DIST_CPP)
# src makefile
DIST+=$(DIST_MAKE)
# DIST+= $(EXECDIR)$(LIBRARY_NAME)
# installation script
DIST+= ../../library_installation.sh
# documentation
# DIST+= ../doc/*
# conf examples
DIST+= ../../aux_resources/example_*
# DIST+= $(CONFDIR)xml/*.xml
# DIST+= $(CONFDIR)netlists/*
# latex/html example resources
# DIST+= ../html_resources/*
# DIST+= ../latex_resources/*
# md files
DIST+= ../../aux_resources/*.md
# bash environment files
##########################
# Bash files Definition  #
##########################
# BASH_VARIABLES_FILES=../bash_variables/*
DIST+= ../../aux_resources/bash_variables/*

all: ${BINARY}

$(BINARY): $(OBJECTS) $(HEADERS)
#	@echo $(HEADERS)
#	g++  -Wall -O3 -g -fdump-tree-original  -c -o parameter.o parameter.cpp
	$(CC) $(OBJECTS) -o $(EXECDIR)$(BINARY) $(LDFLAGS)
#	@echo "creating $(LIBRARY_NAME) link to $(BINARY)"
#	Creates library link
#	ln -sf $(EXECDIR)$(BINARY) $(EXECDIR)$(LIBRARY_NAME)
#
# Cleanning up
# .p .cfg .expand .dot
#
clean:
	@echo "Cleaning up..."
	rm -f *.o *~
	@echo "Cleaning up subdirectories..."
	rm -f io_handling/*.o io_handling/*~
	rm -f netlist_modeling/*.o netlist_modeling/*~
	rm -f netlist_modeling/statements/*.o netlist_modeling/statements/*~
	rm -f simulations_handling/*.o simulations_handling/*~
	rm -f simulations_handling/spectre_handlers/*.o simulations_handling/spectre_handlers/*~
	rm -f simulations_handling/spectre_simulations/*.o simulations_handling/spectre_simulations/*~
	rm -f simulations_handling/simulation_results/*.o simulations_handling/simulation_results/*~
	rm -f global_functions_and_constants/*.o global_functions_and_constants/*~

#
# DIST
#

dist:
	tar cvzf $(DIST_DIR)$(RELEASE_FILE) $(DIST)
