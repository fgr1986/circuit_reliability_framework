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
#include "n_d_metrics_structure.hpp"
// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"

NDMetricsStructure::NDMetricsStructure() {
	this->group_name = kNotDefinedString;
	this->metrics_structure = nullptr;
	this->files_structure = nullptr;
	this->valid_useful_metric_vectors = false;
	// useful metric_vectors
	analyzable_metrics_vector = new std::vector<Metric*>();
	ocean_eval_metrics_vector = new std::vector<OceanEvalMetric*>();
	plottable_metrics_vector = new std::vector<Magnitude*>();
}

NDMetricsStructure::NDMetricsStructure(const NDMetricsStructure& orig) {
	this->group_name = orig.group_name;
	/// copying the 2D structure
	/// [nd_index][MetricCount]
std::cout << "[debug] A\n";
	metrics_structure = new std::vector<std::vector<Metric*>*>();
	// useful metric_vectors
	analyzable_metrics_vector = new std::vector<Metric*>();
	ocean_eval_metrics_vector = new std::vector<OceanEvalMetric*>();
	plottable_metrics_vector = new std::vector<Magnitude*>();
std::cout << "[debug] B\n";
	// reserve memory
	metrics_structure->reserve( orig.metrics_structure->size() );
	unsigned int magCount = orig.metrics_structure->front()->size();
	//debug
	unsigned int msCount = 0;
	for( auto const& ms : *(orig.metrics_structure) ){
std::cout << "[debug] C\n";
		if( ms==nullptr ){
			std::cout << "[Copying NDMetricsStructure] ERROR, null ms for index" << msCount << "\n";
			std::clog << "[Copying NDMetricsStructure] ERROR, null ms for index" << msCount << "\n";
			std::cerr << "[Copying NDMetricsStructure] ERROR, null ms for index" << msCount << "\n";
		}
		std::vector<Metric*>* mv = new std::vector<Metric*>();
		// reserve memory
		mv->reserve( magCount );
		deepCopyVectorOfInheritancePointers( *ms, *mv );
		metrics_structure->push_back( mv );
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
	this->valid_useful_metric_vectors = CreateUsefulMetricVectors();
}

NDMetricsStructure::~NDMetricsStructure(){
	#ifdef DESTRUCTORS_VERBOSE
	std::cout << "Deleting NDMetricsStructure metrics\n";
	#endif
	/// deleting the 2D structure
	for( auto const& ms : *metrics_structure ){
		deleteContentsOfVectorOfPointers( *ms );
	}
	#ifdef DESTRUCTORS_VERBOSE
	std::cout << "Deleting NDMetricsStructure files_structure\n";
	#endif
	/// deleting the 1D structure
	files_structure->clear();
}

void NDMetricsStructure::SimpleInitialization( const unsigned int totalIndexes,
	std::vector<Metric*>& simpleMetricsVector, std::string goldenFilePath ) {
	metrics_structure = new std::vector<std::vector<Metric*>*>();
	files_structure = new std::vector<std::string>();
	// reserve memory
	metrics_structure->reserve( totalIndexes );
	files_structure->reserve( totalIndexes );
	for(unsigned i=0; i<totalIndexes; ++i){
		std::vector<Metric*>* mv = new std::vector<Metric*>();
		// reserve memory
		mv->reserve( simpleMetricsVector.size() );
		deepCopyVectorOfInheritancePointers( simpleMetricsVector, *mv );
		metrics_structure->push_back( mv );
		// file set_files_structure
		files_structure->push_back( goldenFilePath );
	}
	// subvectors
	this->valid_useful_metric_vectors = CreateUsefulMetricVectors();
}

unsigned int NDMetricsStructure::GetTotalElementsLength(){
	return metrics_structure->size();
}

void NDMetricsStructure::AddMetric( Metric* metric, const unsigned int index ){
	if( index>= metrics_structure->size() ){
		std::cout << "Accessing NDMetricsStructure metrics_structure, index: " << index << " size: " <<  metrics_structure->size() << "\n";
		std::cerr << "Accessing NDMetricsStructure metrics_structure, index: " << index << " size: " <<  metrics_structure->size() << "\n";
	}
	metrics_structure->at(index)->push_back( metric );
	this->valid_useful_metric_vectors = false;
}

std::vector<Metric*>* NDMetricsStructure::GetMetricsVector( const unsigned int index ){
	if( index>= metrics_structure->size() ){
		std::cout << "Accessing NDMetricsStructure metrics_structure, index: " << index << " size: " <<  metrics_structure->size() << "\n";
		std::cerr << "Accessing NDMetricsStructure metrics_structure, index: " << index << " size: " <<  metrics_structure->size() << "\n";
		return nullptr;
	}
	if( metrics_structure->at(index)==nullptr ){
		std::cout << "Accessing NDMetricsStructure metrics_structure, index (profile): " << index << " returns nullptr\n";
		std::cerr << "Accessing NDMetricsStructure metrics_structure, index (profile): " << index << " returns nullptr\n";
		return nullptr;
	}
	return metrics_structure->at(index);
}

std::string NDMetricsStructure::GetFilePath( const unsigned int index ){
	if( index>= metrics_structure->size() ){
		std::cout << "Accessing NDMetricsStructure files_structure, index: " << index << " size: " <<  files_structure->size() << "\n";
		std::cerr << "Accessing NDMetricsStructure files_structure, index: " << index << " size: " <<  files_structure->size() << "\n";
		return "error";
	}
	return files_structure->at(index);
}

bool NDMetricsStructure::CreateUsefulMetricVectors() const{
	// clear, do not delete contents
	if( analyzable_metrics_vector!=nullptr ){
		analyzable_metrics_vector->clear();
	}
	if( plottable_metrics_vector!=nullptr ){
		plottable_metrics_vector->clear();
	}
	if( ocean_eval_metrics_vector!=nullptr ){
		ocean_eval_metrics_vector->clear();
	}
	analyzable_metrics_vector = new std::vector<Metric*>();
	plottable_metrics_vector = new std::vector<Magnitude*>();
	ocean_eval_metrics_vector = new std::vector<OceanEvalMetric*>();
	if( metrics_structure->size()<=0 ){
		std::cout << "[ERROR] NDMetricsStructure metrics_structure is null or empty\n";
		std::cerr << "[ERROR] NDMetricsStructure metrics_structure is null or empty\n";
		return false;
	}
	// accessing metrics_structure->at(0) (basic usage, no data other than names will be used)
	for(auto const &m : *metrics_structure->at(0)){
		if( m->is_transient_magnitude() ){
			auto pOceanEvMe = static_cast<OceanEvalMetric*>(m);
			ocean_eval_metrics_vector->push_back( pOceanEvMe );
		}
		if( m->get_analyzable() ){
			analyzable_metrics_vector->push_back(m);
		}
		if( m->is_transient_magnitude() ){
			auto pMag = static_cast<Magnitude*>(m);
			if( pMag->get_plottable() ){
				plottable_metrics_vector->push_back(pMag);
			}
		}
	}
	return true;
}

std::vector<Metric*>* NDMetricsStructure::GetBasicAnalyzableMetricsVector() const{
	if( !valid_useful_metric_vectors ){
		if( !CreateUsefulMetricVectors() ){
			std::cout << "[ERROR] Called from NDMetricsStructure::GetBasicAnalyzableMetricsVector\n";
			std::cerr << "[ERROR] Called from NDMetricsStructure::GetBasicAnalyzableMetricsVector\n";
			return nullptr;
		}
		valid_useful_metric_vectors = true;
	}
	return analyzable_metrics_vector;
}

std::vector<OceanEvalMetric*>* NDMetricsStructure::GetBasicOceanEvalMetricsVector() const{
	if( !valid_useful_metric_vectors ){
		if( !CreateUsefulMetricVectors() ){
			std::cout << "[ERROR] Called from NDMetricsStructure::GetBasicOceanEvalMetricsVector\n";
			std::cerr << "[ERROR] Called from NDMetricsStructure::GetBasicOceanEvalMetricsVector\n";
			return nullptr;
		}
		valid_useful_metric_vectors = true;
	}
	return ocean_eval_metrics_vector;
}

std::vector<Magnitude*>* NDMetricsStructure::GetBasicPlottableMetricsVector() const{
	if( !valid_useful_metric_vectors ){
		if( !CreateUsefulMetricVectors() ){
			std::cout << "[ERROR] Called from NDMetricsStructure::GetBasicPlottableMetricsVector\n";
			std::cerr << "[ERROR] Called from NDMetricsStructure::GetBasicPlottableMetricsVector\n";
			return nullptr;
		}
		valid_useful_metric_vectors = true;
	}
	return plottable_metrics_vector;
}
