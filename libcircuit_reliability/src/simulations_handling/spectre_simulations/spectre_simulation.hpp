/*
 * spectre_simulation.h
 *
 *  Created on: Feb 06, 2014
 *      Author: fernando
 */

#ifndef SPECTRE_SIMULATION_H
#define SPECTRE_SIMULATION_H

// c++ std libraries
#include <string>
#include <vector>
// boost threads
#include <boost/thread.hpp>
// Radiation simulator
#include "../simulation_results/transient_simulation_results.hpp"
#include "../simulation_results/nd_simulation_results.hpp"
// radiation io simulator includes
#include "../../io_handling/log_io.hpp"
// netlist modeling
#include "../../netlist_modeling/n_d_magnitudes_structure.hpp"
#include "../../netlist_modeling/simulation_parameter.hpp"
#include "../../netlist_modeling/statements/analysis_statement.hpp"

class SpectreSimulation {
public:
	SpectreSimulation();
	virtual ~SpectreSimulation();

	/// called from the simulations handler, it runs the parent_scenario_
	/// RunSimulation and PostSimulations (ManageSpectreFolder)
	void HandleSpectreSimulation();

	/// virtual spectre simulation
	/// called from HandleSpectreSimulation (simulations handler)
	/// or by parent simulation threads
	virtual void RunSimulation() = 0;

	///Log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	void set_altered_scenario_index( const int altered_scenario_index ){
		this->altered_scenario_index = altered_scenario_index;
	}

	void set_folder( const std::string& folder){
		this->folder = folder;
	}

	/**
	 * @brief Sets experiment top folder
	 *
	 * @param top_folder
	 */
	void set_top_folder( const std::string& top_folder){
		this->top_folder = top_folder;
	}
	// save spectre transients
	void set_delete_spectre_transients( const bool delete_spectre_transients){ this->delete_spectre_transients = delete_spectre_transients;}
	// save intermediate results (processed transients)
	void set_delete_processed_transients( const bool delete_processed_transients){ this->delete_processed_transients = delete_processed_transients;}
	// delete_spectre_folders
	void set_delete_spectre_folders( const bool delete_spectre_folders){ this->delete_spectre_folders = delete_spectre_folders;}

	int get_altered_scenario_index() const { return altered_scenario_index; }

	NDMagnitudesStructure* get_golden_magnitudes_structure() const {return golden_magnitudes_structure;}

	std::string get_folder() const{ return folder; }
	std::string get_ahdl_simdb_env() const{ return ahdl_simdb_env; }
	std::string get_ahdl_shipdb_env() const{ return ahdl_shipdb_env; }
	// Spectre command
	void set_pre_spectre_command( std::string pre_spectre_command ){
		this->pre_spectre_command = pre_spectre_command; }
	void set_post_spectre_command( std::string post_spectre_command ){
		this->post_spectre_command = post_spectre_command; }
	void set_spectre_command_log_arg( std::string spectre_command_log_arg ){
		this->spectre_command_log_arg = spectre_command_log_arg; }
	void set_spectre_command_folder_arg( std::string spectre_command_folder_arg ){
		this->spectre_command_folder_arg = spectre_command_folder_arg; }
	void set_spectre_command( std::string spectre_command ){
		this->spectre_command = spectre_command; }
	// ahdl env
	void set_ahdl_shipdb_env(std::string ahdl_shipdb_env) { this->ahdl_shipdb_env = ahdl_shipdb_env; }
	void set_ahdl_simdb_env(std::string ahdl_simdb_env) { this->ahdl_simdb_env = ahdl_simdb_env; }

	// Magnitudes
	void set_golden_magnitudes_structure( NDMagnitudesStructure* golden_magnitudes_structure ){
		this->golden_magnitudes_structure = golden_magnitudes_structure; }
	void set_process_magnitudes( const bool process_magnitudes ){
		this->process_magnitudes = process_magnitudes; }
	void set_export_magnitude_errors( const bool export_magnitude_errors){
		this->export_magnitude_errors = export_magnitude_errors; }
	void set_export_processed_magnitudes( const bool export_processed_magnitudes ){
		this->export_processed_magnitudes = export_processed_magnitudes; }

	void set_main_analysis( AnalysisStatement* main_analysis){ this->main_analysis = main_analysis;}

	void set_main_transient_analysis( AnalysisStatement* main_transient_analysis){ this->main_transient_analysis = main_transient_analysis;}

	std::string get_altered_statement_path() const{return altered_statement_path;}
	void set_altered_statement_path( std::string altered_statement_path ){ this->altered_statement_path = altered_statement_path;}


	bool get_correctly_processed() const{return correctly_processed;}
	bool get_correctly_simulated() const{return correctly_simulated;}
	bool get_export_magnitude_errors() const{return export_magnitude_errors;}


	bool get_plot_scatters(){ return plot_scatters; }
	void set_plot_scatters( const bool plot_scatters) { this->plot_scatters = plot_scatters; }

	bool get_plot_transients(){ return plot_transients; }
	void set_plot_transients( const bool plot_transients) { this->plot_transients = plot_transients; }

	void set_interpolate_plots_ratio( const int interpolate_plots_ratio ){ this->interpolate_plots_ratio = interpolate_plots_ratio; }
	int get_interpolate_plots_ratio() const{ return interpolate_plots_ratio; }

	void AddAdditionalSimulationParameter( SimulationParameter* simulationParameter );

	/**
	 * @brief Set is_nested_simulation
	 *
	 * @param is_nested_simulation
	 */
	void set_is_nested_simulation( const bool is_nested_simulation ){
		this->is_nested_simulation = is_nested_simulation;
	}

	void set_is_montecarlo_nested_simulation( const bool is_montecarlo_nested_simulation ){
		this->is_montecarlo_nested_simulation = is_montecarlo_nested_simulation;
	}

	void set_montecarlo_transient_sufix( const std::string& montecarlo_transient_sufix ){
		this->montecarlo_transient_sufix = montecarlo_transient_sufix;
	}

	/**
	 * @brief Set simulation_id
	 *
	 * @param simulation_id
	 */
	void set_simulation_id( const std::string& simulation_id ){
		this->simulation_id = simulation_id;
	}

	/**
	 * @brief gets simulation_id
	 *
	 * @return simulation_id
	 */
	std::string get_simulation_id() const{ return simulation_id; }


 bool get_export_processed_magnitudes() const{ return export_processed_magnitudes; }

	/**
	 * @brief Set n_dimensional
	 *
	 * @param n_dimensional
	 */
	void set_n_dimensional( const bool n_dimensional ){
		this->n_dimensional = n_dimensional;
	}

	/**
	 * @brief Set n_d_profile_index
	 *
	 * @param n_d_profile_index
	 */
	void set_n_d_profile_index( const int n_d_profile_index ){
		this->n_d_profile_index = n_d_profile_index;
	}

	/**
	 * @brief get n_d_profile_index
	 */
	int get_n_d_profile_index(){ return n_d_profile_index; }

	/**
	 * @brief Updates a Simulation Parameter
	 * @details Searches for the simulation_parameter with the name of updatedParameter
	 * and changes its pointer to updatedParameter
	 *
	 * @param updatedParameter
	 * @return true if ok
	 */
	bool UpdateParameterValue( SimulationParameter& updatedParameter, std::string newValue );


	/**
	 * @brief get golden_critical_parameter
	 * @return golden_critical_parameter
	 */
	SimulationParameter* get_golden_critical_parameter(){ return golden_critical_parameter;}

	/**
	 * @brief set radiation parameters
	 *
	 * @param simulation_parameters
	 */
	void CopySimulationParameters( std::vector<SimulationParameter*>& simulationParameters );

	/**
	 * @brief get radiation parameters
	 *
	 * @return simulation_parameters
	 */
	std::vector<SimulationParameter*>* get_simulation_parameters( ){
		return simulation_parameters;
	}

	/**
	 * @brief Set golden_critical_parameter
	 *
	 * @param golden_critical_parameter
	 */
	bool UpdateGoldenCriticalParameter( SimulationParameter& originalGoldenCriticalParameter );

	NDSimulationResults* get_main_nd_simulation_results(){ return & main_nd_simulation_results; }

protected:

	/// LogIO
	LogIO* log_io;

	/// some simulations can be nested in ParameterSweep Simulations
	bool is_nested_simulation;
	/// and also nested in montecarlo analysis
	bool is_montecarlo_nested_simulation;
	std::string montecarlo_transient_sufix;
	/// simulation_id
	std::string simulation_id;
	/// Scenario Index
	int altered_scenario_index;
	/// For N-Dimensional sweep Simulations
	bool n_dimensional;
	/// n_d_profile_index profile_index
	int n_d_profile_index;

	/// Simulation parameters
	/// thought it is a pointer to a vector,
	/// each simulation has its own vector of objects,
	std::vector<SimulationParameter*>* simulation_parameters;
	/// Critical parameter
	SimulationParameter* golden_critical_parameter;

	/// Altered Statement path (from instance to circuit)
	std::string altered_statement_path;
	/// Folder path
	std::string folder;
	/// Experiment Top folder
	std::string top_folder;
	/// delete_spectre_folders
	bool delete_spectre_folders;
	/// Save spectre transients
	bool delete_spectre_transients;
	/// Save intermediate simulation results
	bool delete_processed_transients;
	/// plot scatters
	bool plot_scatters;
	/// plot all transients
	bool plot_transients;
	// gnuplot interpolation ratio
	int interpolate_plots_ratio;

	/// Analysis
	/// Analysis statement
	AnalysisStatement* main_analysis;
	/// Analysis statement
	AnalysisStatement* main_transient_analysis;

	/// Spectre
	/// Pre Spectre command
	std::string pre_spectre_command;
	/// Spectre command
	std::string spectre_command;
	/// Pos Spectre command
	std::string post_spectre_command;
	/// Spectre log args command
	std::string spectre_command_log_arg;
	/// Spectre folder args command
	std::string spectre_command_folder_arg;
	/// environment ahdl simdb property
	std::string ahdl_simdb_env;
	/// environment ahdl shipdb property
	std::string ahdl_shipdb_env;

	/// Magnitudes
	/// Process Magnitudes
	bool process_magnitudes;
	/// export magnitudes error
	bool export_magnitude_errors;
	/// Export processed magnitudes
	bool export_processed_magnitudes;
	/// Golden magnitudes
	NDMagnitudesStructure* golden_magnitudes_structure;

	/// process feedback
	bool correctly_processed;
	/// spectre simulation feedback
	bool correctly_simulated;

	/// simulation results structure
	NDSimulationResults main_nd_simulation_results;
	// if apply, contains info/image results of each metric

	/**
	 * @brief Exports the parameters circuit refered in the radiation circuit.
	 * @details Exports the parameters circuit refered in the radiation circuit.
	 * The radiation parameters have the default value
	 * and the critical parameter the set up value.
	 *
	 * @param currentFolder
	 * @param auxCount
	 * @param currentValue
	 * @return true if the method correctly ends.
	 */
	bool ExportParametersCircuit( std::string currentFolder, int auxCount );

	/**
	 * @brief Shows the ahdl environment variables
	 */
	void ShowEnvironmentVariables();

	/**
	 * @brief Configures the ahdl environment variables
	 */
	void ConfigureEnvironmentVariables();

	/**
	 * @brief waits till resources are free
	 *
	 * @param threadsCount
	 * @param maxThreads
	 * @param thread_group
	 */
	void WaitForResources( unsigned int & threadsCount,
		const unsigned int maxThreads, boost::thread_group & tg, const unsigned int thread2BeWaited );

	/**
	 * @brief [brief description]
	 * @details [long description]
	 *
	 * @return [description]
	 */
	std::vector<Magnitude*>* CreateMagnitudesVectorFromGoldenMagnitudes(const unsigned int& index );

	/**
	 * @brief Interpolates and analyzes the simulation results
	 *
	 * @param TransientSimulationResults
	 * @return true if the method correctly ends.
	 */
	bool InterpolateAndAnalyzeMagnitudes(
		TransientSimulationResults& transientSimulationResults,
		std::vector<Magnitude*>& simulatedMagnitudes, const unsigned int index, const std::string partialId );

	/**
	 * @brief Interpolates and analyzes the selected magnitude
	 *
	  * @param TransientSimulationResults
	  * @param  radiationError
	  * @param goldenMagnitude
	  * @param simulatedMagnitude
	  * @param goldenTime
	  * @param simulatedTime
	 * @return true if the method correctly ends.
	  */
	bool InterpolateAndAnalyzeMagnitude( TransientSimulationResults& transientSimulationResults,
		bool& radiationError, Magnitude& goldenMagnitude, Magnitude& simulatedMagnitude,
		Magnitude& goldenTime, Magnitude& simulatedTime, const std::string& partialId );

	/**
	 * @brief Interpolates a value
	 *
	 * @param x1 signal 1 first value
	 * @param x2 signal 1 second value
	 * @param y1 signal 2 first value
	 * @param y2 signal 2 second value
	 * @param a desired point of interpolation
	 * @return interpolated value
	 */
	double InterpolateValue( bool& interpolationError,
		const double& x1, const double& x2,
		const double& y1, const double& y2, const double& a);

	/**
	 * @brief Process Spectre results
	 *
	 * @param currentFolder
	 * @param localSimulationId
	 * @param TransientSimulationResults
	 * @param processMainTransient Process main transient or main analysis
	 * @return true if the method ends correctly
	 *
	 */
	bool ProcessSpectreResults( const std::string& currentFolder, const std::string& localSimulationId,
		TransientSimulationResults& transientSimulationResults, const bool& processMainTransient,
		std::vector<Magnitude*>& myParameterMagnitudes, const bool& isGolden  );

	/**
	 * @brief Plot the transient results
	 *
	 * @param currentFolder
	 * @param TransientSimulationResults
	 * @param isGolden if the simulation refers the golden scenario
	 * @return true if the method ends correctly
	 *
	 */
	bool PlotTransient( const std::string& localSimulationId,
		TransientSimulationResults& transientSimulationResults, const bool& isGolden  );

	/**
		* @brief deletes, if desired, the transient raw results together with the pr
		* processed magnitudes results.
		*
		* @param TransientSimulationResults
		* @param isGolden if the simulation refers the golden scenario
		* @return true if the method ends correctly
		*
		*/
	bool ManageIndividualResultFiles( TransientSimulationResults& transientSimulationResults, bool isGolden );

	/**
		* @brief deletes, if desired, the transient raw results together
		*
		* @param TransientSimulationResults
		*
		*/
	bool ManageIndividualSpectreFiles( TransientSimulationResults& transientSimulationResults );

	/**
		* @brief deletes, if desired, the processed magnitudes results.
		*
		* @param TransientSimulationResults
		*
		*/
	bool ManageIndividualProcessedFiles( TransientSimulationResults& transientSimulationResults );

	/**
		* @brief deletes, if desired, spectre folders
		*
		*
		*/
	bool ManageSpectreFolder();

	/**
	 * @brief Tests the set up
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() = 0;

	std::string GetSpectreLogFilePath( const std::string& currentFolder );

	virtual std::string GetSpectreResultsFilePath(const std::string& currentFolder,
		const bool& processMainTransient);

	virtual std::string GetProcessedResultsFilePath(const std::string& currentFolder,
		const std::string& localSimulationId, const bool& processMainTransient);

	/**
	 * @brief Creates a gnuplot of the transient
	 *
	 * @param currentFolder
	 * @param  processedResultsFilePath
	 * @param TransientSimulationResults
	 * @return gnuplot output
	 */
	int CreateGnuplotTransientImages(
		const std::string& localSimulationId, TransientSimulationResults& transientSimulationResults,
		const unsigned int& index, const bool& hasOtherInfo, const std::string& otherInfo );

	/**
	 * @brief Checks if an error has occurred
	 * @details [long description]
	 *
	 * @param magnitude
	 * @param currentMagnitudeValue
	 * @param currentGoldenValue
	 * @param currentMagnitudeError
	 * @param absErrorMargin
	 * @return if an error has occurred
	 */
	bool CheckError( const Magnitude& magnitude, const double& currentSimulatedValue,
		const double& currentGoldenValue, const double& currentMagnitudeError, const double& absErrorMargin  );

	bool Get2AnalyzableTimeWindow(
		std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itGoldenTimeEnd,
		std::vector<double>::iterator& itSimulatedTime, std::vector<double>::iterator& itSimulatedTimeEnd,
		std::vector<double>::iterator& itGoldenMagnitude, std::vector<double>::iterator& itSimulatedMagnitude,
		double analyzableTimeWindowT0 );

	bool CheckEndOfWindow(
		const bool magnitudeAnalizableInTWindow, const double& analyzableTimeWindowTF,
		std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itSimulatedTime );

	void ReportSimulationsLengthError( Magnitude& goldenTime,
		Magnitude& simulatedTime, const std::string& partialId );

	void VerboseStartMagnitudeAnalysis(
		std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itSimulatedTime,
		std::vector<double>::iterator& itGoldenMagnitude, std::vector<double>::iterator& itSimulatedMagnitude,
		const Magnitude& simulatedMagnitude );

	void VerboseReliabilityError( const std::string& errorType,
		TransientSimulationResults& transientSimulationResults, const std::string& partialId,
		const std::string& magName, const double& currentTime, const double& currentMagnitudeError,
		const double& currentSimulatedValue, const double& currentGoldenValue, const double& backSimulatedValue, const double& backGoldenValue,
		std::vector<double>::iterator& itSimulatedMagnitude, std::vector<double>::iterator& itGoldenMagnitude );
};

#endif /* SPECTRE_SIMULATION_H */
