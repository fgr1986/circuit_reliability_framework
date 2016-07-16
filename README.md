# README
***Circuit Reliability Frameowrk***
version 3.1.2 16/07/2016
https://github.com/fgr1986/circuit_reliability_framework

[Fernando García Redondo](http://www.fernandeando.com)
[fgarcia@die.upm.es](mailto:fgarcia@die.upm.es)

******
#Full changelog in *changelog.md* file.
******

#To do list **Milestone 3.x**

#Major

* Auto tests: transients generation using inputs/outputs information
* Layout Information in scheme: ¿GDSii?

##Minor
* If (obj) delete obj (test if the object exist before the deletion)
* New conditional parsing scheme: REGEX:
	* if () statement
	* if () { statements }
	* if () statement elseif () statement
	* if () { statements } elseif () statement
	* if () { statements } elseif () { statementd }
	* if () { statements } else { statementd }
	* if () { statements } elseif () { statements } else {statements}
	* if () { statements } elseif () { statements } else statement
	* if () { statements } else { statementd }
* New model parsing scheme: Group of models:

```
// A
model model_name model_type {
1:  type= params
...
}

// B
model model_name model_type
{
1:  type= params
...
}

// C
model model_name model_type params
```

******
# Installation
In order to install Spectre Handler Library in your system, the binary should be copied to your computer. Additionally, if you want to develop your program using this libraries, some headers files should be placed and referenced.
You can perform the installation by typing

		# some privileges may be needed
		sh installation.sh

To make use of this library, you need to include the required headers in your code as in the following example

	#include "experiment_environment.hpp"

And reference them during compilation

	# Compyling time :D
	g++ -std=c++11 -Wall -O3 -I/usr/include/circuit_reliability -c -o main.o main.cpp
	# Linking time :D
	g++ myObj1.o myObj2.o ... main.o -o myBin [ other dynamic libraries -L/usr/lib ... ] -L/usr/lib/circuit_reliability [other libraries -lboost_thread ... ] -lcircuit_reliability


The complete list of headers/documentation is placed after installation in

	/usr/include/circuit_reliability

In order to make use of the library, while running a program, the library binary location should be placed in the system path. More specifically, in the **LD_LIBRARY_PATH** variable.The following code can be used placed, for example, in **.bashrc** file

	###########################
	# Circuit Reliability Lib #
	###########################

	# spectre handler library path in system
	LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY=/usr/lib/circuit_reliability
	# add it to LD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LIB_PATH_CIRCUIT_RELIABILITY_DIRECTORY:$LD_LIBRARY_PATH


******
# Requirements
##Ubuntu 16.04 or 14.04 64bits
* Tested with g++ (gcc<4.9 must set **GCC_OLD** definition)
* GnuPlot (version > 4.6)
* pdflatex
* texlive-latex-extra, texlive-formats-extra, texlive-lang-english, texlive-publishers and texlive-generic-extra packages
* Boost libraries (v1.54)
	* lboost_thread
	* lboost_property_tree
	* lboost_regex
	* lboost_lambda
	* lboost_string
	* lboost_filesystem
	* lboost_system

```
sudo apt-get update
sudo apt-get install build-essential
sudo apt-get install gnuplot
sudo apt-get install libboost-all-dev
sudo apt-get install texlive-latex-extra texlive-formats-extra texlive-lang-english texlive-publishers texlive-generic-extra texlive-bibtex-extra texlive-pictures  texlive-latex-base  texlive-latex-extra

```

******
#XML conf files sintax Specified in conf_files folder Readme.md.
Required xml configuration files:

* Technology XML conf file
* Cadence XML conf file
* Radiation XML conf file
