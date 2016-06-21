/*
 * OceanEvalMagnitude.cpp
 *
 *  Created on: April 20, 2013
 *      Author: fernando
 */

/// radiation simulator
#include "ocean_eval_magnitude.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/name_functions.hpp"

OceanEvalMagnitude::OceanEvalMagnitude( const std::string& name ) {
  this->transient_magnitude = false;
	this->name = name;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->value = kNotDefinedInt;
	this->abs_error_margin = kNotDefinedInt;
	// standard from mag
	this->found_in_results = false;
	this->plottable = false;
	this->plottable_in_golden = false;
	this->analyzable = false;
	this->analyze_error_in_time = false;
	this->analyze_error_in_time_window = false;
	/// ones
	this->abs_error_margin_ones = kNotDefinedInt;
	this->error_time_span_ones = kNotDefinedInt;
	this->error_threshold_ones = kNotDefinedInt;
	/// zeros
	this->abs_error_margin_zeros = kNotDefinedInt;
	this->error_time_span_zeros = kNotDefinedInt;
	this->error_threshold_zeros = kNotDefinedInt;
	/// default
	this->abs_error_margin_default = kNotDefinedInt;
	this->error_time_span_default = kNotDefinedInt;
	///
	this->ommit_upper_threshold = false;
	this->ommit_lower_threshold = false;
}

OceanEvalMagnitude::OceanEvalMagnitude(const OceanEvalMagnitude& orig) {
	this->name = orig.name;
	this->transient_magnitude = false;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->value = orig.value;
	this->abs_error_margin = orig.abs_error_margin;
	// standard
	this->found_in_results = false;
	this->plottable = false;
	this->plottable_in_golden = false;
	this->analyzable = false;
	this->analyze_error_in_time = false;
	this->analyze_error_in_time_window = false;
	/// ones
	this->abs_error_margin_ones = kNotDefinedInt;
	this->error_time_span_ones = kNotDefinedInt;
	this->error_threshold_ones = kNotDefinedInt;
	/// zeros
	this->abs_error_margin_zeros = kNotDefinedInt;
	this->error_time_span_zeros = kNotDefinedInt;
	this->error_threshold_zeros = kNotDefinedInt;
	/// default
	this->abs_error_margin_default = kNotDefinedInt;
	this->error_time_span_default = kNotDefinedInt;
	///
	this->ommit_upper_threshold = false;
	this->ommit_lower_threshold = false;
}

OceanEvalMagnitude::~OceanEvalMagnitude() {
}
