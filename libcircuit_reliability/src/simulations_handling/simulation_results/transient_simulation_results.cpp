/**
* @file basic_simulation_results.cpp
*
* @date Created on: March 27, 2014
*
* @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
*
* @section DESCRIPTION
*
* This TransientSimulationResults Class body.
*
*/

#include <iostream>
// Radiation simulator
#include "transient_simulation_results.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"

TransientSimulationResults::TransientSimulationResults() {
	this->spectre_result = kNotDefinedInt;
	this->reliability_result = kScenarioNotAnalyzed;
	this->full_id = kNotDefinedString;
	this->title = kNotDefinedString;
	this->has_magnitudes_errors = false;
}

TransientSimulationResults::~TransientSimulationResults(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "TransientSimulationResults destructor. direction:" + number2String(this) << "\n";
		std::cout<< "simulation_parameters.clear()\n";
	#endif
	simulation_parameters.clear();
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "deleteContentsOfVectorOfPointers( magnitudes_errors )\n";
	#endif
	deleteContentsOfVectorOfPointers( magnitudes_errors );
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "destructor of TransientSimulationResults ended\n";
	#endif
}

std::string TransientSimulationResults::get_s_reliability_result(){
	return kScenarioResultString[reliability_result];
}

void TransientSimulationResults::RegisterSimulationParameters( std::vector<SimulationParameter*>* simulationParameters ){
	simulation_parameters.clear();
	// it is a std::map, so the structure does not allow to reserve memory
	for( auto const &p : *simulationParameters ){
		simulation_parameters.insert(std::make_pair( p->get_name(), p->get_value()));
	}
}

void TransientSimulationResults::AddMagnitudeErrors( MagnitudeErrors* magnitudeErrors ){
	magnitudes_errors.push_back( magnitudeErrors );
}

void TransientSimulationResults::AddTransientImage(
	const std::string& transientImagePath, const std::string&  transientImageTitle ){
	transient_image_paths.insert(std::make_pair( transientImagePath, transientImageTitle ));
}
