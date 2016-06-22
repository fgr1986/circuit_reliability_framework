/*
 * Metric.cpp
 *
 *  Created on: June 21, 2016
 *      Author: fernando
 */

/// reliability simulator
#include "metric.hpp"

#include "../global_functions_and_constants/name_functions.hpp"

Metric::~Metric() {
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "Metric destructor\n";
	#endif
}

std::string Metric::get_title_name() const{
	if( !valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return title_name;
}

std::string Metric::get_enclosed_name() const{
	if( !valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return enclosed_name;
}

std::string Metric::get_file_name() const{
	if( !valid_formatted_names ){
		this->enclosed_name = "\"" + name + "\"";
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return file_name;
}

void Metric::set_name( const std::string name ){
	this->name = name;
	this->enclosed_name = "\"" + name + "\"";
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
}
