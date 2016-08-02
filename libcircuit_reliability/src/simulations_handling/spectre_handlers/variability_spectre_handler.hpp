/*
 * spectre_handler.hpp
 *
 *  Created on: Feb 03, 2014
 *      Author: fernando
 */

#ifndef VARIABILITY_SPECTRE_HANDLER_H
#define VARIABILITY_SPECTRE_HANDLER_H

// c++ std libraries
#include <string>
#include <vector>
#include <map>
// Boost
#include <boost/thread.hpp>
// Radiation simulator
// radiation io simulator includes
#include "../spectre_simulations/spectre_simulation.hpp"
#include "../simulation_mode.hpp"
#include "../../io_handling/log_io.hpp"
// Netlist modeling
#include "../../metric_modeling/metric.hpp"
#include "../../metric_modeling/n_d_metrics_structure.hpp"
#include "../../netlist_modeling/statements/radiation_source_subcircuit_statement.hpp"

class VariabilitySpectreHandler {
public:
	VariabilitySpectreHandler();
	virtual ~VariabilitySpectreHandler();

	std::string get_pre_spectre_command() const {return pre_spectre_command;}
	std::string get_spectre_command() const {return spectre_command;}
	std::string get_spectre_command_log_arg() const {return spectre_command_log_arg;}
	std::string get_spectre_command_folder_arg() const {return spectre_command_folder_arg;}
	std::string get_golden_scenario_path() const {return golden_scenario_path;}

	void set_golden_scenario_folder_path(std::string golden_scenario_folder_path) {
		this->golden_scenario_folder_path = golden_scenario_folder_path; }
	void set_golden_ahdl_scenario_folder_path(std::string golden_ahdl_scenario_folder_path) {
		this->golden_ahdl_scenario_folder_path = golden_ahdl_scenario_folder_path; }
	void set_variations_AHDL_folder_path(std::string variations_AHDL_folder_path) {
		this->variations_AHDL_folder_path = variations_AHDL_folder_path; }
	void set_golden_scenario_path(std::string golden_scenario_path) { this->golden_scenario_path = golden_scenario_path; }
	void set_export_matlab_script(bool export_matlab_script) {
		this->export_matlab_script = export_matlab_script; }
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
	// ahdl folders
	void set_ahdl_shipdb_folder_path(std::string ahdl_shipdb_folder_path) { this->ahdl_shipdb_folder_path = ahdl_shipdb_folder_path; }
	void set_ahdl_simdb_folder_path(std::string ahdl_simdb_folder_path) { this->ahdl_simdb_folder_path = ahdl_simdb_folder_path; }
	// Save independent simulations
	void set_delete_spectre_transients(bool delete_spectre_transients){ this->delete_spectre_transients = delete_spectre_transients;}
	bool get_delete_spectre_transients() const{return delete_spectre_transients;}
	// save spectre files and folders
	void set_delete_spectre_folders(bool delete_spectre_folders){ this->delete_spectre_folders = delete_spectre_folders;}
	bool get_delete_spectre_folders() const{return delete_spectre_folders;}
	// save processed files
	void set_delete_processed_transients(bool delete_processed_transients){ this->delete_processed_transients = delete_processed_transients;}
	bool get_delete_processed_transients() const{return delete_processed_transients;}
	//Log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

	/**
	 * @brief Sets montecarlo_iterations
	 *
	 * @param montecarlo_iterations
	 */
	void set_montecarlo_iterations( unsigned int montecarlo_iterations ){
		this->montecarlo_iterations = montecarlo_iterations; }

	/**
 	* @details sets simulation_mode and updates selected_radiation_source
 	* @param simulation_mode <SimulationModesHandler*>
 	*/
	void set_simulation_mode( SimulationMode* simulation_mode) { this->simulation_mode = simulation_mode; }

	/**
	 * @brief Sets experiment top folder
	 *
	 * @param top_folder
	 */
	void set_top_folder(std::string top_folder){
		this->top_folder = top_folder;
	}

	/**
	 * @brief Sets experiment top variation_scenario_folder_path
	 *
	 * @param variation_scenario_folder_path
	 */
	void set_variation_scenario_folder_path(std::string variation_scenario_folder_path){
		this->variation_scenario_folder_path = variation_scenario_folder_path;
	}

	void set_export_processed_metrics( const bool export_processed_metrics ) { this->export_processed_metrics = export_processed_metrics; }
	bool get_export_processed_metrics() const{ return export_processed_metrics; }

	void set_plot_scatters( bool plot_scatters ) { this->plot_scatters = plot_scatters; }
	bool get_plot_scatters(){ return plot_scatters; }
	bool get_plot_transients(){ return plot_transients; }
	void set_plot_transients( bool plot_transients) { this->plot_transients = plot_transients; }
	bool get_export_metric_errors(){ return export_metric_errors; }
	void set_export_metric_errors( bool export_metric_errors) { this->export_metric_errors = export_metric_errors; }
	bool get_plot_last_transients(){ return plot_last_transients; }
	void set_plot_last_transients( bool plot_last_transients) { this->plot_last_transients = plot_last_transients; }

	void set_interpolate_plots_ratio( int interpolate_plots_ratio ){ this->interpolate_plots_ratio = interpolate_plots_ratio; }
	int get_interpolate_plots_ratio() const{ return interpolate_plots_ratio; }

	/**
	 * @brief Sets max_parallel_profile_instances
	 *
	 * @param max_parallel_profile_instances
	 */
	void set_max_parallel_profile_instances( int max_parallel_profile_instances ){
		this->max_parallel_profile_instances = max_parallel_profile_instances; }

	/**
	 * @brief Sets max_parallel_montecarlo_instances
	 *
	 * @param max_parallel_montecarlo_instances
	 */
	void set_max_parallel_montecarlo_instances( unsigned int max_parallel_montecarlo_instances ){
		this->max_parallel_montecarlo_instances = max_parallel_montecarlo_instances; }

	void AddMetric( Metric* metric );

	std::vector<SpectreSimulation*>* get_simulations(){ return &simulations; }

	bool RunSimulations();


	std::vector<SimulationParameter*>* get_simulation_parameters(){
		return &simulation_parameters; }
	/**
	 * @details adds a simulation_parameters
	 *
	 * @param simulation_parameters
	 */
	void AddSimulationParameter(
		SimulationParameter* simulationParameter );


	// Save individual transients
	void set_save_injection_sources( const bool save_injection_sources ){ this->save_injection_sources = save_injection_sources;}
	bool get_save_injection_sources() const{return save_injection_sources;}

private:

	/// Simulation parameters
	std::vector<SimulationParameter*> simulation_parameters;
	/// Critical parameter
	// SimulationParameter* critical_parameter;
	/// Experiment top folder
	std::string top_folder;
	/// Simulations
	std::vector<SpectreSimulation*> simulations;
	/// Log manager
	LogIO* log_io;


	/// max parallel instances per parameter
	unsigned int montecarlo_iterations;
	/// save (if applies the injected magnitude (like radiation current))
	bool save_injection_sources;
	/// saves individually spectre transients
	bool delete_spectre_transients;
	/// saves individually processed transients
	bool delete_processed_transients;
	/// deletes spectre folders
	bool delete_spectre_folders;
	/// export explicit
	bool export_processed_metrics;
	/// plot scatter plots
	bool plot_scatters;
	/// plot transients
	bool plot_transients;
	/// only for critical_parameter_nd_parameter_sweep_simulation
	/// (and others involving critical_parameter_simulation arrays)
	bool plot_last_transients;
	/// exports metric errors
	bool export_metric_errors;

	/// ESimulationsMode
	SimulationMode* simulation_mode;
	// spectre pre, command, post and args
	std::string pre_spectre_command;
	std::string post_spectre_command;
	std::string spectre_command_log_arg;
	std::string spectre_command_folder_arg;
	std::string spectre_command;
	// golden scenario path
	std::string golden_scenario_path;
	std::string golden_scenario_folder_path;
	std::string golden_ahdl_scenario_folder_path;
	std::string variations_AHDL_folder_path;
	// simulation folder - altered statement
	std::string variation_scenario_folder_path;
	// ahdl ddbb
	std::string ahdl_shipdb_folder_path;
	std::string ahdl_simdb_folder_path;
	// Unsorted Metrics which are going to be processed
	std::vector<Metric*> unsorted_metrics_2be_found;
	// Metrics which are going to be processed
	std::vector<Metric*> metrics_2be_found;
	// Golden Metrics
	NDMetricsStructure* golden_metrics_structure;
	bool export_matlab_script;
	int interpolate_plots_ratio;
	/// max parallel instances per parameter_sweep
	int max_parallel_profile_instances;
	/// max parallel instances per parameter
	unsigned int max_parallel_montecarlo_instances;

	bool SimulateGoldenNetlist( );
	bool SimulateStandardAHDLNetlist( );
	bool SimulateGoldenAHDLNetlist();
	bool ExportProfilesList();

	bool ReorderMetrics( const std::string& spectreResultTrans, const std::string& spectreLog );

};

#endif /* VARIABILITY_SPECTRE_HANDLER_H */
