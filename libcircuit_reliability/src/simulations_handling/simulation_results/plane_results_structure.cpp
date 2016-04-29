/**
 * results_summary.cpp
 *
 * Created on: April 1, 2016
 *   Author: fernando
 */

// Radiation simulator
#include "plane_results_structure.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"

PlaneResultsStructure::PlaneResultsStructure() {
	// this->simulation_id = kNotDefinedString;
	this->plane_id = kNotDefinedString;
	this->general_data_path = kNotDefinedString;
	this->general_image_path = kNotDefinedString;
}

PlaneResultsStructure::~PlaneResultsStructure(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "PlaneResultsStructure destructor. direction:" + number2String(this) <<std::endl;
	#endif
}

void PlaneResultsStructure::AddGeneralMagnitudeImagePath( const std::string& path, const std::string& title ){
	general_magnitudes_image_paths.insert( std::make_pair(path, title) );
}

void PlaneResultsStructure::ResizeItemizedPlanesMemory( unsigned int planesCount ){
	itemized_magnitudes_image_paths.resize(planesCount);
}

void PlaneResultsStructure::AddItemizedDataPath( const std::string& path, const std::string& title ){
	itemized_data_paths.insert( std::make_pair(path, title) );
}

void PlaneResultsStructure::AddItemizedImagePath( const std::string& path, const std::string& title ){
	itemized_image_paths.insert( std::make_pair(path, title) );
}

void PlaneResultsStructure::AddItemizedMagnitudeImagePath( const unsigned int& planeIndex,
		const std::string& path, const std::string& title ){
	itemized_magnitudes_image_paths.at(planeIndex).insert(
		std::make_pair(path, title) );
}
