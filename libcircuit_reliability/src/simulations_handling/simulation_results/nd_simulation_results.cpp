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

void NDSimulationResults::ReservePlanesInMemory( unsigned int planesLength ){
	// reserve memory
	plane_results_structures->reserve(planesLength);
}

// void NDSimulationResults::AddGeneralMagnitudeDataPath( const std::string& path, const std::string& title ){
// 	general_magnitudes_data_paths.insert( std::make_pair(path, title) );
// }

void NDSimulationResults::AddGeneralMagnitudeImagePath( const std::string& path, const std::string& title ){
	general_magnitudes_image_paths.insert( std::make_pair(path, title) );
}
