/**
 * results_summary.cpp
 *
 * Created on: Jun 16, 2014
 *   Author: fernando
 */

// Radiation simulator
#include "nd_simulation_results.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"

NDSimulationResults::NDSimulationResults() {
	this->simulation_id = kNotDefinedString;
	this->n_dimensional = false;
	this->title = kNotDefinedString;
	this->general_image_path = kNotDefinedString;
	this->general_data_path = kNotDefinedString;
	plane_results_structures = new std::vector<PlaneResultsStructure*>();
}

NDSimulationResults::~NDSimulationResults() {
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "NDSimulationResults destructor. direction:" + number2String(this) << "\n";
	#endif
	if( plane_results_structures!=nullptr ){
		#ifdef DESTRUCTORS_VERBOSE
			std::cout<< "NDSimulationResults destructor. deleteContentsOfVectorOfPointers( *plane_results_structures )";
		#endif
		deleteContentsOfVectorOfPointers( *plane_results_structures );
	}
}

void NDSimulationResults::AddPlaneResultsStructure( PlaneResultsStructure* plane ){
	plane_results_structures->push_back( plane );
}

PlaneResultsStructure* NDSimulationResults::GetPlaneResultsStructure( const unsigned int planeIndex ){
	if( planeIndex<plane_results_structures->size() ){
		return plane_results_structures->at(planeIndex);
	}
	return nullptr;
}

void NDSimulationResults::ReservePlanesInMemory( const unsigned int planesLength ){
	// reserve memory
	plane_results_structures->reserve(planesLength);
}

// void NDSimulationResults::AddGeneralMetricDataPath( const std::string& path, const std::string& title ){
// 	general_metrics_data_paths.insert( std::make_pair(path, title) );
// }

void NDSimulationResults::AddGeneralMetricImagePath( const std::string& path, const std::string& title ){
	general_metrics_image_paths.insert( std::make_pair(path, title) );
}
