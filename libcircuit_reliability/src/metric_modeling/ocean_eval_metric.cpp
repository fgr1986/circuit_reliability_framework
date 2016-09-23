/*
 * OceanEvalMetric.cpp
 *
 *	Created on: April 20, 2013
 *			Author: fernando
 */

/// reliability simulator
#include "ocean_eval_metric.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/name_functions.hpp"

OceanEvalMetric::OceanEvalMetric( const std::string& name ) {
	this->transient_magnitude = false;
	this->name = name;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->enclosed_name = "\"" + name + "\"";
	this->valid_formatted_names = true;
	this->value = kNotDefinedInt;
	this->ocean_eval_expression = kNotDefinedString;
	this->error_margin_up = kNotDefinedInt;
	this->error_margin_down = kNotDefinedInt;
	// standard from metric
	this->found_in_results = false;
	this->analyzable = false;
}

OceanEvalMetric::OceanEvalMetric(const OceanEvalMetric& orig) {
	this->transient_magnitude = false;
	this->name = orig.name;
	this->title_name = TitleWithoutIllegalChars( orig.name );
	this->file_name = FilenameWithoutIllegalChars( orig.name );
	this->enclosed_name = "\"" + orig.name + "\"";
	this->valid_formatted_names = true;
	this->value = orig.value;
	this->ocean_eval_expression = orig.ocean_eval_expression;
	this->error_margin_up = orig.error_margin_up;
	this->error_margin_down = orig.error_margin_down;
	// standard from metric
	this->found_in_results = orig.found_in_results;
	this->analyzable = orig.analyzable;
}

OceanEvalMetric::OceanEvalMetric(const OceanEvalMetric& orig, const bool copyValue ) {
	this->transient_magnitude = false;
	if( copyValue ){
		this->value = orig.value;
	}else{
		this->value = kNotDefinedInt;
	}
	this->name = orig.name;
	this->title_name = TitleWithoutIllegalChars( orig.name );
	this->file_name = FilenameWithoutIllegalChars( orig.name );
	this->enclosed_name = "\"" + orig.name + "\"";
	this->valid_formatted_names = true;
	this->ocean_eval_expression = orig.ocean_eval_expression;
	this->error_margin_up = orig.error_margin_up;
	this->error_margin_down = orig.error_margin_down;
	// standard from metric
	this->found_in_results = orig.found_in_results;
	this->analyzable = orig.analyzable;
}

OceanEvalMetric* OceanEvalMetric::GetCopy() {
	return new OceanEvalMetric(*this);
}

OceanEvalMetric::~OceanEvalMetric() {
}

double OceanEvalMetric::get_value_at( const int index ){
	return value;
}
