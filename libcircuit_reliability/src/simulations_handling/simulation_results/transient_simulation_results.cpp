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
	this->has_metrics_errors = false;
}

TransientSimulationResults::TransientSimulationResults(
		const TransientSimulationResults& orig, const bool copySimulationParameters ){
	this->spectre_result = orig.spectre_result;
	this->reliability_result = orig.reliability_result;
	this->full_id = orig.full_id;
	this->title = orig.title;
	this->has_metrics_errors = orig.has_metrics_errors;
	this->original_file_path = orig.original_file_path;
	this->processed_file_path = orig.processed_file_path;
	// data structures
	deepCopyVectorOfPointers( orig.metrics_errors, metrics_errors );
	for( const auto& sp : orig.transient_image_paths ){
		transient_image_paths.insert(std::make_pair( sp.first, sp.second));
	}
	if( copySimulationParameters ){
		for( const auto& sp : orig.simulation_parameters ){
			simulation_parameters.insert(std::make_pair( sp.first, sp.second));
		}
	}
}

TransientSimulationResults::~TransientSimulationResults(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "TransientSimulationResults destructor. direction:" + number2String(this) << "\n";
		std::cout<< "simulation_parameters.clear()\n";
	#endif
	simulation_parameters.clear();
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "deleteContentsOfVectorOfPointers( metrics_errors )\n";
	#endif
	deleteContentsOfVectorOfPointers( metrics_errors );
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

void TransientSimulationResults::AddMetricErrors( MetricErrors* metricErrors ){
	metrics_errors.push_back( metricErrors );
}

void TransientSimulationResults::AddTransientImage(
	const std::string& transientImagePath, const std::string&  transientImageTitle ){
	transient_image_paths.insert(std::make_pair( transientImagePath, transientImageTitle ));
}
