/**
 * RESULTS_PROCESSOR_H.hpp
 *
 *  Created on: April 2, 2016
 *	  Author: fernando
 */

#ifndef RESULTS_PROCESSOR_H
#define RESULTS_PROCESSOR_H

// c++ std libraries
#include <string>
#include <vector>
#include <map>

#include "../io_handling/log_io.hpp"
#include "../netlist_modeling/altered_scenario_summary.hpp"
#include "../netlist_modeling/simulation_parameter.hpp"

class ResultsProcessor  {
public:

	/**
	 * @brief Default constructor
	 */
	ResultsProcessor();
	virtual ~ResultsProcessor();


	/**
	* @details sets log_io
	* @param log_io <LogIO*>
	*/
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

	// creates a file with the shape of each in paths,
	// whose colums, if specified in columnIndexes, are the mean values
	// of those in paths
	bool MeanProcessResultsFiles( const std::map<std::string, std::string>* paths,
		const std::string outputPath, const std::vector<unsigned int>&& columnIndexes );

	// creates a file extending those in paths,
	// For each column in columnIndexes, we compute mean, max val and min val
	bool StatisticProcessResultsFiles( const std::map<std::string, std::string>* paths,
		const std::string outputPath, const std::vector<unsigned int>&& columnIndexes );

	// creates a file extending those in paths,
	// For each column in columnIndexes, we compute mean, max val and min val
	bool StatisticProcessStatisticsFiles( const std::map<std::string, std::string>* paths,
		const std::string outputPath, const std::vector<unsigned int>&& maxFieldsColumnIndexes,
		const std::vector<unsigned int>&& minFieldsColumnIndexes, const std::vector<unsigned int>&& meanFieldsColumnIndexes );

	bool ExportScenariosList(
		const std::string& topFolder, const std::vector<AlteredScenarioSummary*>& scenariosList );

	bool ExportProfilesList(
		const std::string& topFolder, const std::vector<SimulationParameter*>& parameterList );

	static const unsigned int kMeanProcessResultsFiles = 0;
	static const unsigned int kStatisticProcessResultsFiles = 1;
	static const unsigned int kStatisticProcessStatisticsFiles = 2;

protected:

	// Matrix type
	using Matrix = double**;
	/// Logger
	LogIO* log_io;

	bool PreProcessResultsFiles( const std::string&& path, unsigned int& totalRows, unsigned int& totalColumns );
	bool MeanProcessResultFile( const std::string&& path,
		Matrix& matrix, bool* computedColumns ); //(int (&myArray)[100])
	bool StatisticProcessResultFile( const std::string&& path,
		Matrix& matrix, bool* computedColumns );
	bool StatisticProcessStatisticsFile( const std::string&& path,
		Matrix& matrixMax, Matrix& matrixMin, Matrix& matrixMean,
		bool* computedColumnsMax, bool* computedColumnsMin, bool* computedColumnsMean );

};

#endif /* RESULTS_PROCESSOR_H */
