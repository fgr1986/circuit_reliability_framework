/*
 * magnitude.cpp
 *
 *  Created on: December 16, 2013
 *      Author: fernando
 */

 #include <iostream>
/// radiation simulator
#include "magnitude.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/name_functions.hpp"

Magnitude::Magnitude( const std::string& name) {
  this->transient_magnitude = true;
	this->name = name;
	this->enclosed_name = "\"" + name + "\"";
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
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

Magnitude::Magnitude(const Magnitude& orig) {
  this->transient_magnitude = true;
	this->name = orig.name;
	if( !orig.valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}else{
		this->enclosed_name = orig.enclosed_name;
		this->title_name = orig.title_name;
		this->file_name = orig.file_name;
		this->valid_formatted_names = true;
	}
	this->found_in_results = orig.found_in_results;
	this->plottable = orig.plottable;
	this->plottable_in_golden = orig.plottable_in_golden;
	// Analysis attributes
	this->analyzable = orig.analyzable;
	this->analyze_error_in_time = orig.analyze_error_in_time;
	this->analyze_error_in_time_window = orig.analyze_error_in_time_window;
	if( this->analyze_error_in_time_window ){
		this->analyzable_time_window_t0 = orig.analyzable_time_window_t0;
		this->analyzable_time_window_tf = orig.analyzable_time_window_tf;
	}
	/// ones
	this->abs_error_margin_ones = orig.abs_error_margin_ones;
	this->error_time_span_ones = orig.error_time_span_ones;
	this->error_threshold_ones = orig.error_threshold_ones;
	/// zeros
	this->abs_error_margin_zeros = orig.abs_error_margin_zeros;
	this->error_time_span_zeros = orig.error_time_span_zeros;
	this->error_threshold_zeros = orig.error_threshold_zeros;
	/// default
	this->abs_error_margin_default = orig.abs_error_margin_default;
	this->error_time_span_default = orig.error_time_span_default;
	this->ommit_upper_threshold = orig.ommit_upper_threshold;
	this->ommit_lower_threshold = orig.ommit_lower_threshold;
	// copy
	for( auto const& v : orig.values ){
		values.push_back(v);
	}
}

Magnitude::Magnitude(const Magnitude& orig, bool copyValues) {
  this->transient_magnitude = true;
	this->name = orig.name;
	if( !orig.valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}else{
		this->enclosed_name = orig.enclosed_name;
		this->title_name = orig.title_name;
		this->file_name = orig.file_name;
		this->valid_formatted_names = true;
	}
	this->found_in_results = orig.found_in_results;
	this->plottable = orig.plottable;
	this->plottable_in_golden = orig.plottable_in_golden;
	// Analysis attributes
	this->analyzable = orig.analyzable;
	this->analyze_error_in_time = orig.analyze_error_in_time;
	this->analyze_error_in_time_window = orig.analyze_error_in_time_window;
	if( this->analyze_error_in_time_window ){
		this->analyzable_time_window_t0 = orig.analyzable_time_window_t0;
		this->analyzable_time_window_tf = orig.analyzable_time_window_tf;
	}
	/// ones
	this->abs_error_margin_ones = orig.abs_error_margin_ones;
	this->error_time_span_ones = orig.error_time_span_ones;
	this->error_threshold_ones = orig.error_threshold_ones;
	/// zeros
	this->abs_error_margin_zeros = orig.abs_error_margin_zeros;
	this->error_time_span_zeros = orig.error_time_span_zeros;
	this->error_threshold_zeros = orig.error_threshold_zeros;
	/// default
	this->abs_error_margin_default = orig.abs_error_margin_default;
	this->error_time_span_default = orig.error_time_span_default;
	this->ommit_upper_threshold = orig.ommit_upper_threshold;
	this->ommit_lower_threshold = orig.ommit_lower_threshold;
	if( copyValues ){
		// copy
		for( auto const& v : orig.values ){
			values.push_back(v);
		}
	}
}

Magnitude::~Magnitude() {
}

std::string Magnitude::get_title_name() const{
	if( !valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return title_name;
}

std::string Magnitude::get_enclosed_name() const{
	if( !valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return enclosed_name;
}

std::string Magnitude::get_file_name() const{
	if( !valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return file_name;
}

void Magnitude::set_name( const std::string name ){
	this->name = name;
	this->enclosed_name = "\"" + name + "\"";
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
}

void Magnitude::AddValue( double value ){
	values.push_back( value );
}

double Magnitude::GetValue( int position ){
	return values.at( position );
}
