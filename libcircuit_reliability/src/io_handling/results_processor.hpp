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


	bool ProcessResultsFiles( const std::map<std::string, std::string>* paths,
		const std::string outputPath, const std::vector<unsigned int>&& columnIndexes );

	bool ExportScenariosList(
		const std::string& topFolder, const std::vector<AlteredScenarioSummary*>& scenariosList );

	bool ExportProfilesList(
		const std::string& topFolder, const std::vector<SimulationParameter*>& parameterList );

protected:

	// Matrix type
	using Matrix = double**;
	/// Logger
	LogIO* log_io;

	bool PreProcessResultsFiles( std::string&& path,
		unsigned int& totalRows, unsigned int& totalColumns );
	bool ProcessResultFile( const std::string&& path,
		Matrix& matrix, const std::vector<unsigned int>&& columnIndexes );
};

#endif /* RESULTS_PROCESSOR_H */
