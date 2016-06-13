/**
 * @file critical_parameter_1d_parameter_sweep_simulation.hpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the simple sweep simulation mode.
 *
 */

#ifndef GOLDEN_ND_PARAMETERS_SWEEP_SIMULATION_H
#define GOLDEN_ND_PARAMETERS_SWEEP_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"
#include "spectre_simulations_vector.hpp"
#include "golden_simulation.hpp"
// netlist modeling
#include "../../netlist_modeling/simulation_parameter.hpp"
#include "../../netlist_modeling/statements/control_statement.hpp"

class GoldenNDParametersSweepSimulation : public SpectreSimulation {
public:
	GoldenNDParametersSweepSimulation();
	virtual ~GoldenNDParametersSweepSimulation();

	/// virtual spectre simulation
	virtual void RunSimulation() override;

	/**
	 * @brief Sets max_parallel_profile_instances
	 *
	 * @param max_parallel_profile_instances
	 */
	void set_max_parallel_profile_instances( int max_parallel_profile_instances ){
		this->max_parallel_profile_instances = max_parallel_profile_instances; }

	/**
	 * @brief Sets magnitudes_2be_found
	 * @parammagnitudes_2be_found
	 */
	void set_magnitudes_2be_found( std::vector<Magnitude*>* magnitudes_2be_found ){
		this->magnitudes_2be_found = magnitudes_2be_found; }

	/**
	 * @brief Retrieve Processed Magnitudes
	 * @details Retrieve Processed Magnitudes
	 * @return processed NDMagnitudesStructure
	 */
	NDMagnitudesStructure* GetGoldenMagnitudes();

	/**
	 * @brief Retrieve children_correctly_simulated
	 * @details Retrieve  children_correctly_simulated
	 * @return processed children_correctly_simulated
	 */
	bool get_children_correctly_simulated(){
		return children_correctly_simulated;
	}

	/**
	 * @brief Retrieve children_correctly_processed
	 * @details Retrieve  children_correctly_processed
	 * @return processed children_correctly_processed
	 */
	bool get_children_correctly_processed(){
		return children_correctly_processed;
	}

private:

	/// Simulation results, organizated by parameter (first vector)
	SpectreSimulationsVector golden_simulations_vector;
	/// Max parallel instances per sweep
	int max_parallel_profile_instances;

	///  all the children suimulated
	bool children_correctly_simulated;
	///  all the children processed
	bool children_correctly_processed;

	/// Golden Simulation saves the magnitudes here
	std::vector<Magnitude*>* processed_magnitudes;
	/// Magnitudes to be found
	std::vector<Magnitude*>* magnitudes_2be_found;

	/**
	 * @brief Creates a new critical_parameter_value_simulation instance
	 *
	 * @param currentFolder
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param threadNumber
	 * @return The new golden_simulation pointer
	 */
	// GoldenSimulation* CreateGoldenSimulation(
	// 		const std::string& currentFolder, const std::vector<unsigned int> & parameterCountIndexes,
	// 		const std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndIndex );

	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;

	/**
	 * @brief Creates profile simulation
	 *
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param threadNumber
	 */
	GoldenSimulation* CreateProfile( const std::vector<unsigned int>& parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndIndex );

	/**
	 * @brief Run sweep simulation
	 * @details Run sweep simulation
	 *
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param threadNumber
	 * @param sweepVectorOfMagnitudesVector
	 * @param sweepVectorOfFiles
	 */
	void RunProfile( GoldenSimulation* pGS );

};

#endif /* GOLDEN_ND_PARAMETERS_SWEEP_SIMULATION_H */
