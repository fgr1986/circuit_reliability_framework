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

#ifndef N_D_METRICS_STRUCTURE_H
#define N_D_METRICS_STRUCTURE_H

// c++ std libraries
// #include <string>
// #include <vector>
// Radiation simulator
#include "metric.hpp"
#include "magnitude.hpp"

class NDMetricsStructure {
public:
	/// default constructor
	NDMetricsStructure();
	/// copy constructor
	NDMetricsStructure(const NDMetricsStructure& orig);

	virtual ~NDMetricsStructure();

	/**
	 * @brief gets the lists of metrics_vector
	 * @return metrics_vector
	 */
	std::vector<Metric*>* GetMetricsVector( const unsigned int nd_index );

	/**
	 * @brief gets the lists of metrics with metric
	 * vector to be removed but NOT its contents
	 * @return metrics_vector
	 */
	std::vector<Metric*>* GetBasicMetricMetricsVector() const;


	/**
	 * @brief gets the lists of plotable metrics
	 * vector to be removed but NOT its contents
	 * @return metrics_vector
	 */
	std::vector<Magnitude*>* GetBasicPlottableMetricsVector() const;

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
	 * @brief sets metrics_structure
	 *
	 * @param metrics_structure
	 */
	void set_metrics_structure( std::vector<std::vector<Metric*>*>* metrics_structure ){
		this->metrics_structure = metrics_structure;
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
	 * @brief Adds a new Metrics
	 *
	 * @param Metrics
	 */
	void AddMetric( Metric* metric, const unsigned int nd_index );

	/**
	 * @brief Inits structure
	 *
	 * @param parameterCount
	 * @param sweepCount
	 * @param simpleMetricsVector
	 */
	void SimpleInitialization(  const unsigned int totalIndexes,
		std::vector<Metric*>& simpleMetricsVector, std::string goldenFilePath  );

	/**
	 * @brief returns elements size
	 *
	 */
	unsigned int GetTotalElementsLength();

private:
	/// Group group_name
	std::string group_name;
	/// Metrics
	/// [nd_index][MetricCount]
	std::vector<std::vector<Metric*>*>* metrics_structure;
	/// files_structure where the metrics are stored
	/// [nd_index]
	std::vector<std::string>* files_structure;
	// to increase performance
	mutable std::vector<Metric*>* metrics_metrics_vector;
	mutable std::vector<Magnitude*>* plottable_metrics_vector;
	mutable bool valid_useful_metric_vectors{false};

	bool CreateUsefulMetricVectors() const;
};

#endif /* N_D_METRICS_STRUCTURE_H */
