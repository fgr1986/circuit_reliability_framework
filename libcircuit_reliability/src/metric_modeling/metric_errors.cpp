 /**
 * @file metric_errors.cpp
 *
 * @date Created on: March 28, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This MetricErrors Class body.
 *
 */

// Radiation simulator
#include "metric_errors.hpp"
// constants
#include "../global_functions_and_constants/global_constants.hpp"

MetricErrors::MetricErrors() {
	this->metric_name = kNotDefinedString;
	this->has_errors = false;
	this->max_abs_error= 0;
	this->max_abs_error_global = 0;
}

MetricErrors::~MetricErrors(){
	error_timing.clear();
}

void MetricErrors::AddErrorTiming( const std::string& start, const std::string& stop ){
	error_timing.insert(std::make_pair(start, stop));
}
