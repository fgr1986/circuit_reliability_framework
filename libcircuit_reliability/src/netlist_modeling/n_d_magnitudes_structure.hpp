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

#ifndef N_D_MAGNITUDES_STRUCTURE_H
#define N_D_MAGNITUDES_STRUCTURE_H

// c++ std libraries
// #include <string>
// #include <vector>
// Radiation simulator
#include "magnitude.hpp"

class NDMagnitudesStructure {
public:
	/// default constructor
	NDMagnitudesStructure();
	/// copy constructor
	NDMagnitudesStructure(const NDMagnitudesStructure& orig);

	virtual ~NDMagnitudesStructure();

	/**
	 * @brief gets the lists of magnitudes_vector
	 * @return magnitudes_vector
	 */
	std::vector<Magnitude*>* GetMagnitudesVector( const unsigned int nd_index );

	/**
	 * @brief gets the lists of magnitudes with metric
	 * vector to be removed but NOT its contents
	 * @return magnitudes_vector
	 */
	std::vector<Magnitude*>* GetMetricMagnitudesVector( const unsigned int nd_index );

	/**
	 * @brief gets the path to the golden result file
	 * @return path to the golden result file
	 */
	std::string GetFilePath( const unsigned int nd_index );

	/**
	 * @brief sets group_name
	 *
	 * @param group_name
	 */
	void set_group_name( std::string group_name ){
		this->group_name = group_name;
	}

	/**
	 * @brief sets magnitudes_structure
	 *
	 * @param magnitudes_structure
	 */
	void set_magnitudes_structure( std::vector<std::vector<Magnitude*>*>* magnitudes_structure ){
		this->magnitudes_structure = magnitudes_structure;
	}

	/**
	 * @brief sets files_structure
	 *
	 * @param files_structure
	 */
	void set_files_structure( std::vector<std::string>* files_structure ){
		this->files_structure = files_structure;
	}

	/**
	 * @brief gets group_name
	 * @return group_name
	 */
	std::string get_group_name() const{return group_name;}

	/**
	 * @brief Adds a new Magnitudes
	 *
	 * @param Magnitudes
	 */
	void AddMagnitude( Magnitude* magnitude, const unsigned int nd_index );

	/**
	 * @brief Inits structure
	 *
	 * @param parameterCount
	 * @param sweepCount
	 * @param simpleMagnitudesVector
	 */
	void SimpleInitialization(  const unsigned int totalIndexes,
		std::vector<Magnitude*>& simpleMagnitudesVector, std::string goldenFilePath  );

	/**
	 * @brief returns elements size
	 *
	 */
	unsigned int GetTotalElementsLength();

private:
	/// Group group_name
	std::string group_name;
	/// Magnitudes
	/// [nd_index][MagnitudeCount]
	std::vector<std::vector<Magnitude*>*>* magnitudes_structure;
	/// files_structure where the magnitudes are stored
	/// [nd_index]
	std::vector<std::string>* files_structure;
};

#endif /* N_D_MAGNITUDES_STRUCTURE_H */
