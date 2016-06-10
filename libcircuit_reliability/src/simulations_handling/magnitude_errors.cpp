 /**
 * @file magnitude_errors.cpp
 *
 * @date Created on: March 28, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This MagnitudeErrors Class body.
 *
 */

// Radiation simulator
#include "magnitude_errors.hpp"
// constants
#include "../global_functions_and_constants/global_constants.hpp"

MagnitudeErrors::MagnitudeErrors() {
	this->magnitude_name = kNotDefinedString;
	this->has_errors = false;
	this->max_abs_error= 0;
	this->max_abs_error_global = 0;
}

MagnitudeErrors::~MagnitudeErrors(){
	error_timing.clear();
}

void MagnitudeErrors::AddErrorTiming( std::string start, std::string stop ){
	error_timing.insert(std::make_pair(start, stop));
}
