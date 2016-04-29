/**
* @file basic_simulation_results.cpp
*
* @date Created on: March 27, 2014
*
* @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
*
* @section DESCRIPTION
*
* This MontecarloSimulationResults Class body.
*
*/

#include <iostream>
// Radiation simulator
#include "montecarlo_simulation_results.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"

MontecarloSimulationResults::MontecarloSimulationResults() {
	this->spectre_result = kNotDefinedInt;
	this->full_id = kNotDefinedString;
	this->upsets_count = kNotDefinedInt;
	this->mean_critical_parameter_value = kNotDefinedInt + 0.0;
}

MontecarloSimulationResults::~MontecarloSimulationResults(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "MontecarloSimulationResults destructor. direction:" + number2String(this) <<std::endl;
		std::cout<< "simulation_parameters.clear()" <<std::endl;
	#endif
	simulation_parameters.clear();
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "deleteContentsOfVectorOfPointers( magnitudes_errors )" <<std::endl;
	#endif
	deleteContentsOfVectorOfPointers( metric_montecarlo_results );
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "destructor of MontecarloSimulationResults ended" <<std::endl;
	#endif
}

void MontecarloSimulationResults::RegisterSimulationParameters( std::vector<SimulationParameter*>* simulationParameters ){
	// it is a std::map, so the structure does not allow to reserve memory
	for( auto const &p : *simulationParameters ){
		simulation_parameters.insert(std::make_pair( p->get_name(), p->get_value()));
		// std::cout << full_id << " " << (*it_p)->get_name() << " " << (*it_p)->get_value();
	}
}

void MontecarloSimulationResults::ReserveMetricMontecarloResults( unsigned int metricMontecarloResultsLength ){
	metric_montecarlo_results.reserve(metricMontecarloResultsLength);
}

void MontecarloSimulationResults::AddMetricMontecarloResults( metric_montecarlo_results_t* mMCResults ){
	metric_montecarlo_results.push_back( mMCResults );
}
