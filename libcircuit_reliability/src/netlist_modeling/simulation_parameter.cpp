/*
 * SimulationParameter.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: fernando
 */

// math
#include <cmath>
 #include <iostream>
/// radiation simulator
#include "simulation_parameter.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"
#include "../global_functions_and_constants/name_functions.hpp"

SimulationParameter::SimulationParameter() {
	this->name = kNotDefinedString;
	this->title_name = kNotDefinedString;
	this->file_name = kNotDefinedString;
	this->valid_formatted_names = false;
	this->value = kNotDefinedString;
	this->fixed = true;
	this->golden_fixed = false;
	this->allow_find_critical_value = false;
	this->value_min = kNotDefinedInt;
	this->value_max = kNotDefinedInt;
	this->value_change_mode = kNotDefinedInt;
	this->value_stop_percentage = 5;
	this->sweep_steps_number = 1;
	this->positive = true;
}


SimulationParameter::SimulationParameter( const std::string& name,
	const std::string& value, const bool& fixed,
	const double& value_min, const double& value_max,
	const double& value_stop_percentage, const int& value_change_mode, const unsigned int& sweep_steps_number ) {
	this->name = name;
	this->title_name = TitleWithoutIllegalChars( name );
	this->file_name = FilenameWithoutIllegalChars( name );
	this->valid_formatted_names = true;
	this->value = value;
	this->fixed = fixed;
	this->allow_find_critical_value = false;
	this->value_min = value_min;
	this->value_max = value_max;
	this->value_stop_percentage = value_stop_percentage;
	this->value_change_mode = value_change_mode;
	this->sweep_steps_number = sweep_steps_number;
	this->positive = true;
}


SimulationParameter::SimulationParameter(const SimulationParameter& orig) {
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
	this->fixed = orig.fixed;
	this->golden_fixed = orig.golden_fixed;
	this->allow_find_critical_value = orig.allow_find_critical_value;
	this->value_min = orig.value_min;
	this->value_max = orig.value_max;
	this->value_stop_percentage = orig.value_stop_percentage;
	this->value_change_mode = orig.value_change_mode;
	this->sweep_steps_number = orig.sweep_steps_number;
	this->positive = true;
	this->log_inc = -666;
	this->lineal_inc = -666;
}

SimulationParameter::~SimulationParameter() {
}


bool SimulationParameter::get_allow_sweep() {
	return !fixed && !allow_find_critical_value && sweep_steps_number > 2;
}


std::string SimulationParameter::GetInfo(){
	std::string info = "(Simulation) Parameter: '" + number2String( name ) + "' Default Value: '" + number2String( value );
	info += fixed ? "\n"+ kTab + "fixed: true" : " fixed: false";
	info += allow_find_critical_value ? " allow_find_critical_value: true" : " allow_find_critical_value: false";
	info += "\n"+ kTab + "Min Value: '" + number2String( value_min ) + "'" + "' Max Value: '" + number2String( value_max ) + "' Steps: " + number2String( sweep_steps_number ) ;
	info += "\n"+ kTab + "value_stop_percentage: '" + number2String(value_stop_percentage) + "' value_change_mode: " + number2String( value_change_mode );
	return info;
}

void SimulationParameter::InitSweep(){
	this->positive = value_min>=0 && value_max>=0;
	bool allowLog = std::abs(value_min)>0 && std::abs(value_max)>0;
	this->lineal_inc = (value_max-value_min)/(sweep_steps_number-1);
	this->abs_a_b = std::abs( value_min + value_max );
	if( allowLog ){
		if( positive ){
			// (log(b/a)-log(a/a))/ (N-1)
			this->log_inc = std::log(value_max/value_min)/(sweep_steps_number-1);
		}else{
			this->log_inc = - std::log(std::abs(value_max/value_min))/(sweep_steps_number-1);
		}
	}
	// Debug
	// if( positive ){
	// 	std::cout << "Param " << name << " is POSITIVE li: " << lineal_inc << " lgi:" << log_inc << "\n";
	// }else{
	// 	std::cout << "Param " << name << " is NEGATIVE li: " << lineal_inc << " lgi:" << log_inc << "\n";
	// }
}

double SimulationParameter::GetSweepValue( const int currentStep ){
	double sweepValue;
	switch( value_change_mode ){
		case kSPLineal:{
			sweepValue = currentStep * lineal_inc + value_min;
		}
		break;
		//  X(n) = 10^( (log10(b)-log10(a))/(N-1)*index )
		// a < b in both critical value and sweep modes.
		case  kSPLogSmaller:{
			if( positive ){
				sweepValue =  std::abs(value_min)*std::exp( log_inc*currentStep );
			}else{
				sweepValue = abs_a_b -  std::abs(value_min)*std::exp( log_inc*currentStep );
			}
		}
		break;
		//  X(n) = b - 10^( (log10(b)-log10(a))/(N-1)*index )
		// a < b in both critical value and sweep modes.
		case kSPLogBigger:{
			if( positive ){
				sweepValue = abs_a_b -  std::abs(value_min)*std::exp( log_inc*currentStep );
			}else{
				sweepValue = - std::abs(value_min)*std::exp( log_inc*currentStep );
			}
		}
		break;
		default:{
			sweepValue = currentStep * lineal_inc + value_min;
		}
		break;
	}
	return sweepValue;
}
