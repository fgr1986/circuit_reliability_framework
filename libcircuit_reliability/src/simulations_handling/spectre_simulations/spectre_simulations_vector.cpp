 /**
 * @file spectre_simulations_vector.cpp
 *
 * @date Created on: Oct 6, 2014
 *
 * @author Author: Fernando GarcÃ­a <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This SpectreSimulationsVector Class body.
 *
 */

#include <iostream>
// Radiation simulator
#include "spectre_simulations_vector.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"

SpectreSimulationsVector::SpectreSimulationsVector() {
	this->group_name = kNotDefinedString;
}

SpectreSimulationsVector::~SpectreSimulationsVector(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "SpectreSimulationsVector destructor\n";
	#endif
	deleteContentsOfVectorOfPointers( spectre_simulations );
}

void SpectreSimulationsVector::AddSpectreSimulation( SpectreSimulation* singleSpectreSimulation ){
	spectre_simulations.push_back( singleSpectreSimulation );
}

void SpectreSimulationsVector::ReserveSimulationsInMemory( unsigned int simulationsLength ){
	spectre_simulations.reserve( simulationsLength );
}

bool SpectreSimulationsVector::CheckCorrectlySimulated(){
	// lambda to compute si there has been any spectre errors
	// is vector of vector of simulations
	bool partial_cs = true;
	// for a vector of vectors
	// for_each( sim_vector->get_spectre_simulations()->begin(), sim_vector->get_spectre_simulations()->end(),
	// 	[&partial_cs] (SpectreSimulationsVector* autoSSV) {
	// 		auto autoSimVec = autoSSV->get_spectre_simulations();
	// 		for_each( autoSimVec->begin(), autoSimVec->end(),
	// 			[&partial_cs] (SpectreSimulation* cps) {
	// 			partial_cs = partial_cs && cps->get_correctly_simulated();
	// 		}  );
	// 	}
	// );
	// for_each( spectre_simulations.begin(), spectre_simulations.end(),
	// 	[&partial_cs] (SpectreSimulation* autoSS) {
	// 		partial_cs = partial_cs && autoSS->get_correctly_simulated();
	// 	}
	// );
	for( auto const &s : spectre_simulations ){
	// for( unsigned int i=0; i< spectre_simulations.size();++i ){
		partial_cs = partial_cs && s->get_correctly_simulated();
	}
	return partial_cs;
}
