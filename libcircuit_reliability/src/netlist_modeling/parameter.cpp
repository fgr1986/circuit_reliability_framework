/*
 * Parameter.cpp
 *
 *  Created on: Oct 20, 2011
 *      Author: fernando
 */

/// radiation simulator
#include "parameter.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/name_functions.hpp"


Parameter::Parameter() {
	//this->id = kNotDefinedInt;
	//this->parameter_type = kNotDefinedInt;
	this->name = kNotDefinedString;
	this->value = kNotDefinedString;
	this->title_name = kNotDefinedString;
	this->file_name = kNotDefinedString;
	this->valid_formatted_names = false;
	this->has_value = false;
}

Parameter::Parameter( const std::string& name, const std::string& value ) {
	//this->parameter_type = kNotDefinedInt;
	this->name = name;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
	this->value = value;
	this->has_value = true;
}

Parameter::Parameter( const std::string& name ) {
	this->name = name;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
	this->has_value = false;
}

Parameter::Parameter(const Parameter& orig) {
	//this->parameter_type = orig.parameter_type;
	this->name = orig.name;
	if( !orig.valid_formatted_names ){
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}else{
		this->title_name = orig.title_name;
		this->file_name = orig.file_name;
		this->valid_formatted_names = true;
	}
	this->value = orig.value;
	this->has_value = orig.has_value;
}

Parameter::~Parameter() {
}

std::string Parameter::get_title_name() const{
	if( !valid_formatted_names ){
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return title_name;
}

std::string Parameter::get_file_name() const{
	if( !valid_formatted_names ){
		this->title_name = TitleWithoutIllegalChars( name );
		this->file_name = FilenameWithoutIllegalChars( name );
		this->valid_formatted_names = true;
	}
	return file_name;
}

void Parameter::set_name( const std::string name ){
	this->name = name;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
}

std::string Parameter::ExportParameter(){
	if( has_value ){
		return name + "=" + value;
	}else{
		return name;
	}
}

std::string Parameter::GetInfo(){
	return "Parameter: '" + name + "' Value: '" + value + "'";
}
