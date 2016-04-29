 /**
 * @file spectre_simulations_vector.hpp
 *
 * @date Created on: Oct 6, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class acts as simulations container.
 *
 */

#ifndef SPECTRE_SIMULATIONS_VECTOR_H
#define SPECTRE_SIMULATIONS_VECTOR_H

// c++ std libraries
#include <string>
#include <vector>
// Radiation simulator
#include "spectre_simulation.hpp"

class SpectreSimulationsVector {
public:
	SpectreSimulationsVector();
	virtual ~SpectreSimulationsVector();

	/**
	 * @brief gets the lists of spectre_simulations
	 * @return spectre_simulations
	 */
	std::vector<SpectreSimulation*>* get_spectre_simulations(){ return &spectre_simulations; }

	/**
	 * @brief sets group_name
	 *
	 * @param group_name
	 */
	void set_group_name( std::string group_name ){
		this->group_name = group_name;
	}

	/**
	 * @brief gets group_name
	 * @return group_name
	 */
	std::string get_group_name() const{return group_name;}

	/**
	 * @brief Adds a new simulation
	 *
	 * @param singleSpectreSimulation
	 */
	void AddSpectreSimulation( SpectreSimulation* singleSpectreSimulation );

	/**
	 * @brief Returns wether or not spectre has correctly ended in each simulation
	 *
	 */
	bool CheckCorrectlySimulated();

	void ReserveSimulationsInMemory( unsigned int simulationsLength );

private:
	/// Group group_name
	std::string group_name;
	/// Simulation results
	std::vector<SpectreSimulation*> spectre_simulations;
};

#endif /* SPECTRE_SIMULATIONS_VECTOR_H */
