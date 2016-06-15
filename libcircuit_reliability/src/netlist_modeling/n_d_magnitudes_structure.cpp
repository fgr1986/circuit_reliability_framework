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
	this->valid_useful_magnitude_vectors = false;
	// useful magnitude_vectors
	metrics_magnitudes_vector = new std::vector<Magnitude*>();
	plottable_magnitudes_vector = new std::vector<Magnitude*>();
}

NDMagnitudesStructure::NDMagnitudesStructure(const NDMagnitudesStructure& orig) {
	this->group_name = orig.group_name;
	/// copying the 2D structure
	/// [nd_index][MagnitudeCount]
	magnitudes_structure = new std::vector<std::vector<Magnitude*>*>();
	// useful magnitude_vectors
	metrics_magnitudes_vector = new std::vector<Magnitude*>();
	plottable_magnitudes_vector = new std::vector<Magnitude*>();
	// reserve memory
	magnitudes_structure->reserve( orig.magnitudes_structure->size() );
	unsigned int magCount = orig.magnitudes_structure->front()->size();
	//debug
	unsigned int msCount = 0;
	for( auto const& ms : *(orig.magnitudes_structure) ){
		if( ms==nullptr ){
			std::cout << "[Copying NDMagnitudesStructure] ERROR, null ms for index" << msCount << "\n";
			std::clog << "[Copying NDMagnitudesStructure] ERROR, null ms for index" << msCount << "\n";
			std::cerr << "[Copying NDMagnitudesStructure] ERROR, null ms for index" << msCount << "\n";
		}
		std::vector<Magnitude*>* mv = new std::vector<Magnitude*>();
		// reserve memory
		mv->reserve( magCount );
		deepCopyVectorOfPointers( *ms, *mv );
		magnitudes_structure->push_back( mv );
		// debug
		++msCount;
	}
	/// copying the 2D structure
	files_structure = new std::vector<std::string>();
	// reserve memory
	files_structure->reserve( orig.files_structure->size() );
	for( auto const& f : *(orig.files_structure) ){
		files_structure->push_back( std::string( f ) );
	}
	// subvectors
	this->valid_useful_magnitude_vectors = CreateUsefulMagnitudeVectors();
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
	// subvectors
	this->valid_useful_magnitude_vectors = CreateUsefulMagnitudeVectors();
}

unsigned int NDMagnitudesStructure::GetTotalElementsLength(){
	return magnitudes_structure->size();
}

void NDMagnitudesStructure::AddMagnitude( Magnitude* magnitude, const unsigned int index ){
	if( index>= magnitudes_structure->size() ){
		std::cout << "Accessing NDMagnitudesStructure magnitudes_structure, index: " << index << " size: " <<  magnitudes_structure->size() << "\n";
		std::cerr << "Accessing NDMagnitudesStructure magnitudes_structure, index: " << index << " size: " <<  magnitudes_structure->size() << "\n";
	}
	magnitudes_structure->at(index)->push_back( magnitude );
	this->valid_useful_magnitude_vectors = false;
}

std::vector<Magnitude*>* NDMagnitudesStructure::GetMagnitudesVector( const unsigned int index ){
	if( index>= magnitudes_structure->size() ){
		std::cout << "Accessing NDMagnitudesStructure magnitudes_structure, index: " << index << " size: " <<  magnitudes_structure->size() << "\n";
		std::cerr << "Accessing NDMagnitudesStructure magnitudes_structure, index: " << index << " size: " <<  magnitudes_structure->size() << "\n";
		return nullptr;
	}
	if( magnitudes_structure->at(index)==nullptr ){
		std::cout << "Accessing NDMagnitudesStructure magnitudes_structure, index (profile): " << index << " returns nullptr\n";
		std::cerr << "Accessing NDMagnitudesStructure magnitudes_structure, index (profile): " << index << " returns nullptr\n";
		return nullptr;
	}
	return magnitudes_structure->at(index);
}

std::string NDMagnitudesStructure::GetFilePath( const unsigned int index ){
	if( index>= magnitudes_structure->size() ){
		std::cout << "Accessing NDMagnitudesStructure files_structure, index: " << index << " size: " <<  files_structure->size() << "\n";
		std::cerr << "Accessing NDMagnitudesStructure files_structure, index: " << index << " size: " <<  files_structure->size() << "\n";
		return "error";
	}
	return files_structure->at(index);
}

bool NDMagnitudesStructure::CreateUsefulMagnitudeVectors() const{
	// clear, do not delete contents
	if( metrics_magnitudes_vector!=nullptr ){
		metrics_magnitudes_vector->clear();
	}
	if( plottable_magnitudes_vector!=nullptr ){
		plottable_magnitudes_vector->clear();
	}
	metrics_magnitudes_vector = new std::vector<Magnitude*>();
	plottable_magnitudes_vector = new std::vector<Magnitude*>();
	if( magnitudes_structure->size()<=0 ){
		std::cout << "[ERROR] NDMagnitudesStructure magnitudes_structure is null or empty\n";
		std::cerr << "[ERROR] NDMagnitudesStructure magnitudes_structure is null or empty\n";
		return false;
	}
	// accessing magnitudes_structure->at(0) (basic usage, no data other than names will be used)
	for(auto const &m : *magnitudes_structure->at(0)){
		if( m->get_analyzable() ){
			metrics_magnitudes_vector->push_back(m);
		}
		if( m->get_plottable() ){
			plottable_magnitudes_vector->push_back(m);
		}
	}
	return true;
}

std::vector<Magnitude*>* NDMagnitudesStructure::GetBasicMetricMagnitudesVector() const{
	if( !valid_useful_magnitude_vectors ){
		if( !CreateUsefulMagnitudeVectors() ){
			std::cout << "[ERROR] Called from NDMagnitudesStructure::GetBasicMetricMagnitudesVector\n";
			std::cerr << "[ERROR] Called from NDMagnitudesStructure::GetBasicMetricMagnitudesVector\n";
			return nullptr;
		}
		valid_useful_magnitude_vectors = true;
	}
	return metrics_magnitudes_vector;
}

std::vector<Magnitude*>* NDMagnitudesStructure::GetBasicPlottableMagnitudesVector() const{
	if( !valid_useful_magnitude_vectors ){
		if( !CreateUsefulMagnitudeVectors() ){
			std::cout << "[ERROR] Called from NDMagnitudesStructure::GetBasicPlottableMagnitudesVector\n";
			std::cerr << "[ERROR] Called from NDMagnitudesStructure::GetBasicPlottableMagnitudesVector\n";
			return nullptr;
		}
		valid_useful_magnitude_vectors = true;
	}
	return plottable_magnitudes_vector;
}
