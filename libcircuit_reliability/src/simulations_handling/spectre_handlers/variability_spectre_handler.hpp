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
#include "../../netlist_modeling/magnitude.hpp"
#include "../../netlist_modeling/n_d_magnitudes_structure.hpp"
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
	void set_save_spectre_transients(bool save_spectre_transients){ this->save_spectre_transients = save_spectre_transients;}
	bool get_save_spectre_transients() const{return save_spectre_transients;}
	// save spectre files and folders
	void set_delete_spectre_folders(bool delete_spectre_folders){ this->delete_spectre_folders = delete_spectre_folders;}
	bool get_delete_spectre_folders() const{return delete_spectre_folders;}
	// save processed files
	void set_save_processed_transients(bool save_processed_transients){ this->save_processed_transients = save_processed_transients;}
	bool get_save_processed_transients() const{return save_processed_transients;}
	//Log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

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

	void set_plot_scatters( bool plot_scatters ) { this->plot_scatters = plot_scatters; }
	bool get_plot_scatters(){ return plot_scatters; }
	bool get_plot_transients(){ return plot_transients; }
	void set_plot_transients( bool plot_transients) { this->plot_transients = plot_transients; }
	void set_export_magnitude_errors( bool export_magnitude_errors) { this->export_magnitude_errors = export_magnitude_errors; }
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

	void AddMagnitude( Magnitude* magnitude );

	std::vector<SpectreSimulation*>* get_simulations(){ return &simulations; }

	bool RunSpectreSimulations();


	std::vector<SimulationParameter*>* get_simulation_parameters(){
		return &simulation_parameters; }
	/**
	 * @details adds a simulation_parameters
	 *
	 * @param simulation_parameters
	 */
	void AddSimulationParameter(
		SimulationParameter* simulationParameter );

	/**
	 * @brief setscritical_parameter
	 * @details critical_parameter
	 *
	 * @param critical_parameter
	 */
	void set_critical_parameter( SimulationParameter* critical_parameter){
		this->critical_parameter = critical_parameter;
	}

private:

	/// Simulation parameters
	std::vector<SimulationParameter*> simulation_parameters;
	/// Critical parameter
	SimulationParameter* critical_parameter;
	/// Experiment top folder
	std::string top_folder;
	/// Simulations
	std::vector<SpectreSimulation*> simulations;
	/// Log manager
	LogIO* log_io;

	/// saves individually spectre transients
	bool save_spectre_transients;
	/// saves individually processed transients
	bool save_processed_transients;
	/// deletes spectre folders
	bool delete_spectre_folders;
	/// plot scatter plots
	bool plot_scatters;
	/// plot transients
	bool plot_transients;
	/// only for critical_parameter_nd_parameter_sweep_simulation
	/// (and others involving critical_parameter_simulation arrays)
	bool plot_last_transients;
	/// exports magnitude errors
	bool export_magnitude_errors;

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
	// Magnitudes which are going to be processed
	std::vector<Magnitude*> magnitudes_2be_found;
	// Golden Magnitudes
	NDMagnitudesStructure* golden_magnitudes_structure;
	bool export_matlab_script;
	int interpolate_plots_ratio;
	/// max parallel instances per parameter_sweep
	int max_parallel_profile_instances;

	bool SimulateGoldenNetlist( );
	bool SimulateStandardAHDLNetlist( );
	bool SimulateGoldenAHDLNetlist();

};

#endif /* VARIABILITY_SPECTRE_HANDLER_H */
