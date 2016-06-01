 /**
 * @file spectre_simulations_vector.cpp
 *
 * @date Created on: Oct 6, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This SpectreSimulationsVector Class body.
 *
 */

#include <iostream>
// Radiation simulator
#include "n_d_magnitudes_structure.hpp"
// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"

NDMagnitudesStructure::NDMagnitudesStructure() {
	this->group_name = kNotDefinedString;
	this->magnitudes_structure = nullptr;
	this->files_structure = nullptr;
}

NDMagnitudesStructure::NDMagnitudesStructure(const NDMagnitudesStructure& orig) {
	this->group_name = orig.group_name;
	/// copying the 2D structure
	magnitudes_structure = new std::vector<std::vector<Magnitude*>*>();
	// reserve memory
	magnitudes_structure->reserve( orig.magnitudes_structure->size() );
	for( auto const& ms : *(orig.magnitudes_structure) ){
		std::vector<Magnitude*>* mv = new std::vector<Magnitude*>();
		// reserve memory
		mv->reserve( ms->size() );
		deepCopyVectorOfPointers( *ms, *mv );
		magnitudes_structure->push_back( mv );
	}
	/// copying the 2D structure
	files_structure = new std::vector<std::string>();
	// reserve memory
	files_structure->reserve( orig.files_structure->size() );
	for( auto const& f : *(orig.files_structure) ){
		files_structure->push_back( std::string( f ) );
	}
}

NDMagnitudesStructure::~NDMagnitudesStructure(){

	#ifdef DESTRUCTORS_VERBOSE
	std::cout << "Deleting NDMagnitudesStructure magnitudes\n";
	#endif
	/// deleting the 2D structure
	for( auto const& ms : *magnitudes_structure ){
		deleteContentsOfVectorOfPointers( *ms );
	}
	#ifdef DESTRUCTORS_VERBOSE
	std::cout << "Deleting NDMagnitudesStructure files_structure\n";
	#endif
	/// deleting the 1D structure
	files_structure->clear();
}

void NDMagnitudesStructure::SimpleInitialization( const unsigned int totalIndexes,
	std::vector<Magnitude*>& simpleMagnitudesVector, std::string goldenFilePath ) {
	magnitudes_structure = new std::vector<std::vector<Magnitude*>*>();
	files_structure = new std::vector<std::string>();
	// reserve memory
	magnitudes_structure->reserve( totalIndexes );
	files_structure->reserve( totalIndexes );
	for(unsigned i=0; i<totalIndexes; ++i){
		std::vector<Magnitude*>* mv = new std::vector<Magnitude*>();
		// reserve memory
		mv->reserve( simpleMagnitudesVector.size() );
		deepCopyVectorOfPointers( simpleMagnitudesVector, *mv );
		magnitudes_structure->push_back( mv );
		// file set_files_structure
		files_structure->push_back( goldenFilePath );
	}
}

unsigned int NDMagnitudesStructure::GetTotalElementsLength(){
	return magnitudes_structure->size();
}

void NDMagnitudesStructure::AddMagnitude( Magnitude* magnitude, const unsigned int index ){
	magnitudes_structure->at(index)->push_back( magnitude );
}

std::vector<Magnitude*>* NDMagnitudesStructure::GetMagnitudesVector( const unsigned int index ){
	return magnitudes_structure->at(index);
}

std::vector<Magnitude*>* NDMagnitudesStructure::GetMetricMagnitudesVector( const unsigned int index ){
	auto metricMagnitudes = new std::vector<Magnitude*>();
	for(auto const &m : *magnitudes_structure->at(index)){
		if(m->get_analyzable()){
			metricMagnitudes->push_back(m);
		}
	}
	return metricMagnitudes;
}

std::string NDMagnitudesStructure::GetFilePath( const unsigned int index ){
	return files_structure->at(index);
}
