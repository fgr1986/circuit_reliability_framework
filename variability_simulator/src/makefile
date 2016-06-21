######################################
### Variability Simulator makefile ###
######################################

# # of cores used during compiling
export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"
#export MAKEFLAGS="-j 2"

############
# Version  #
############
VERSION=$(VARIABILITY_SIMULATOR_VERSION)

#################
# Compiler def  #
#################
CC=g++

################
# target files #
################
# binary file
EXECUTABLE=variability_simulator_${VERSION}
# release file
RELEASE_FILE=variability_simulator_release_${VERSION}.tar.gz

###############
## Folders   ##
###############
# binary folder
EXECDIR=../bin/
# include folder
LIBSPECTRE=/usr/lib/circuit_reliability
# headers include
INCLIB=/usr/include/circuit_reliability
# dist dir
DIST_DIR=../dist/

# Timing
# conf folder
# CONFDIR=../experiments/timing/conf_files
# SCRIPTSDIR=../experiments/timing/scripts
# SRAM
# CONFDIR=../experiments/sram/conf_files
# SCRIPTSDIR=../experiments/sram/scripts
# RUNEXAMPLE=../scripts/run_cadence_test.sh

###################
## Dist elements ##
###################
# Headers to be included in dist tar
DIST_HEADERS=  $(addprefix "../src/", $(HEADERS))
# SRC cpp to be included in dist tar
DIST_CPP=  $(addprefix "../src/", $(SRC))
# makefile to be included in dist tar
DIST_MAKE= $(addprefix "../src/", makefile)
# headers
# DIST= $(EXECDIR)$(EXECUTABLE)
DIST= $(EXECDIR)
DIST+=$(DIST_HEADERS)
# src cpp
DIST+=$(DIST_CPP)
# src makefile
DIST+=$(DIST_MAKE)
# default run script
# DIST+= $(RUNEXAMPLE)
# documentation
# DIST+= ../doc/*
# conf files
# DIST+= $(CONFDIR)/*
# script examples
# DIST+= $(SCRIPTSDIR)/*
# latex/html example resources
# md files
# DIST+= ../*.md

#################
# Gcc flags     #
#################
#CPPFLAGS+= -Wall -g // Debugg it turns on (almost) all the warnings that g++ can tell you about
#CPPFLAGS+= -Wall -O3 -fdump-tree-cfg-graph -fdump-rtl-expand-graph  // rtl graphs, O3
#CPPFLAGS+= -Wall -O3 -g -fdump-tree-original-raw -fdump-tree-all-all -finstrument-functions // rtl graphs, O3
#CPPFLAGS+= -std=c++11 -Wall -O3 -I $(MATLAB_ROOT)extern/include // Matlab, c++11
# CPPFLAGS+= -g -std=c++11 -Wall -O3

# CPPFLAGS= -std=c++11 -Wall -O3 -I$(INCLIB)
CPPFLAGS= -g -std=c++11 -Wall -O3 -I$(INCLIB)

#################
# Library flags #
#################
# c++11, boost
LDFLAGS= -L/usr/lib -L$(LIBSPECTRE) -lboost_thread -lboost_regex -lboost_filesystem -lboost_system -lcircuit_reliability

######################
# Sources definition #
######################
SRC= $(wildcard *.cpp)

######################
# Objects definition #
######################

OBJECTS= $(SRC:.cpp=.o)

######################
# Headers Definition #
######################
HEADERS= $(wildcard *.hpp)

all: ${EXECUTABLE}

$(EXECUTABLE): $(OBJECTS) $(HEADERS)
#	@echo $(HEADERS)
#g++  -Wall -O3 -g -fdump-tree-original  -c -o parameter.o parameter.cpp
	$(CC) $(OBJECTS) -o $(EXECDIR)$(EXECUTABLE) $(LDFLAGS)

#
# Cleanning up
# .p .cfg .expand .dot
#
clean:
	@echo "Cleaning up..."
	rm -f *.o *~

#
# DIST
#
dist:
	tar cvzf $(DIST_DIR)$(RELEASE_FILE) $(DIST)

#
# RUN
#
run: $(EXE)
	@echo "Running" $(EXE) "..."
	@./$(EXECDIR)/$< $(EXE_OPTS)
