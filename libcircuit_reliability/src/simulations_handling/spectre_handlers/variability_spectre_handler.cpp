/*
 * variability_spectre_handler.cpp
 *
 *  Created on: Feb 03, 2014
 *      Author: fernando
 */

// Reliability simulator
#include "variability_spectre_handler.hpp"
#include "../spectre_simulations/golden_simulation.hpp"
#include "../spectre_simulations/golden_nd_parameters_sweep_simulation.hpp"
#include "../spectre_simulations/ahdl_simulation.hpp"
#include "../spectre_simulations/montecarlo_nd_parameters_sweep_simulation.hpp"
#include "../../io_handling/results_processor.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/ahdl_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

VariabilitySpectreHandler::VariabilitySpectreHandler() {
	// spectre
	this->pre_spectre_command = kNotDefinedString;
	this->spectre_command = kNotDefinedString;
	this->post_spectre_command = kNotDefinedString;
	this->golden_scenario_path = kNotDefinedString;
	this->spectre_command_log_arg = kNotDefinedString;
	this->spectre_command_folder_arg = kNotDefinedString;
	this->golden_scenario_folder_path = kNotDefinedString;
	// export_processed_metrics
	this->export_processed_metrics = false;
	// plot
	this->plot_scatters = false;
	this->plot_transients = false;
	this->plot_last_transients = false;
	this->export_matlab_script = false;
	// files
	this->export_metric_errors = false;
	this->delete_spectre_folders = false;
	this->delete_spectre_transients = true;
	this->delete_processed_transients = true;
	this->golden_metrics_structure = nullptr;
	// montecarlo iterations
	this->montecarlo_iterations = 1;
	// parallel
	this->max_parallel_profile_instances = 2;
	this->max_parallel_montecarlo_instances = 5;
}

VariabilitySpectreHandler::~VariabilitySpectreHandler() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "VariabilitySpectreHandler destructor. direction:" + number2String(this));
		log_io->ReportPlainStandard( "metrics_2be_found" );
	#endif
	deleteContentsOfVectorOfPointers( simulation_parameters );
	deleteContentsOfVectorOfPointers( metrics_2be_found );
	// ReorderMetrics clears unsorted_metrics_2be_found
	// deleteContentsOfVectorOfPointers( unsorted_metrics_2be_found );
	if( golden_metrics_structure ){
		#ifdef DESTRUCTORS_VERBOSE
			log_io->ReportPlainStandard( "golden_metrics_structure");
		#endif
		delete golden_metrics_structure;
	}
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "simulations");
	#endif
	deleteContentsOfVectorOfPointers( simulations );

	// #ifdef DESTRUCTORS_VERBOSE
	// 	log_io->ReportPlainStandard( "results_summary_vector");
	// #endif
	// deleteContentsOfVectorOfPointers( results_summary_vector );
}

void VariabilitySpectreHandler::AddSimulationParameter( SimulationParameter* simulationParameter  ){
	simulation_parameters.push_back(simulationParameter);
}


bool VariabilitySpectreHandler::ExportProfilesList(){
	log_io->ReportGreenStandard( "Exporting profiles list" );
	ResultsProcessor rp;
	return rp.ExportProfilesList(top_folder, simulation_parameters);
}

bool VariabilitySpectreHandler::RunSimulations(){
	// Report simulation
	log_io->ReportGreenStandard( kTab + simulation_mode->get_description());
	log_io->ReportGreenStandard( "RadiationSpectreHandler Simulation Parameters: " + number2String( simulation_parameters.size()) );
	// Thread group
	boost::thread_group tgScenarios;
	int radiationScenarioCounter = 0;
	// Golden netlist and AHDL netlist
	// Golden results are processed
	if( !SimulateStandardAHDLNetlist() || !SimulateGoldenNetlist() ){
		log_io->ReportError2AllLogs( "Error while simulating or processing the golden or ahdl scenario. Aborted." );
		return false;
	}
	log_io->ReportCyanStandard( k2Tab + "Simulate variability scenario." );
	if( plot_transients ){
		log_io->ReportCyanStandard( k2Tab + "Single transients will be plotted." );
	}
	if( plot_scatters ){
		log_io->ReportCyanStandard( k2Tab + "Scatters will be plotted." );
	}
	// exporting the scenario path (id) and altered instance
	log_io->ReportCyanStandard( k2Tab + "Exporting scenarios and profiles." );
	if( !ExportProfilesList() ){
		log_io->ReportError2AllLogs( kTab + "Error exporting scenarios and profiles." );
	}
	// reserve memory, only one altered scenarios
	simulations.reserve( 1 );
	// retrieve numruns and set it to '1', for manual handling in kMontecarloNDParametersSweepMode
	if( simulation_mode->get_id()!=kMontecarloNDParametersSweepMode){
		log_io->ReportError2AllLogs( "Simulation mode is not kMontecarloNDParametersSweepMode. Aborted." );
		return false;
	}
	SimulationParameter* pMontecarloIterations = nullptr;
	for( auto & p : simulation_parameters ){
		if( p->get_name() == kMontecarloIterationsParameterWord){
			pMontecarloIterations = p;
			break; // break for
		}
	}
	if( pMontecarloIterations==nullptr ){
		log_io->ReportError2AllLogs( "param " + kMontecarloIterationsParameterWord +" is null. Required for this simulation mode");
		return false;
	}
	montecarlo_iterations = std::stoi( pMontecarloIterations->get_value() );
	log_io->ReportPurpleStandard( "Montecarlo iterations: " + number2String(montecarlo_iterations) );

	// Simulate variability
	log_io->ReportInfo2AllLogs("[performance] Reliability Simulations started: " + GetCurrentDateTime("%d-%m-%Y.%X"));
	MontecarloNDParametersSweepSimulation* sss = new MontecarloNDParametersSweepSimulation();
	sss->set_n_d_profile_index( 0 );
	sss->set_has_additional_injection( simulation_mode->get_alteration_mode()->get_injection_mode() );
	sss->set_simulation_id("parent_scenario_" + number2String(radiationScenarioCounter));
	sss->set_is_nested_simulation(false);
	sss->set_altered_scenario_index( radiationScenarioCounter );
	// simulation parameters
	sss->CopySimulationParameters( simulation_parameters );
	log_io->ReportPurpleStandard( "sss simulation_parameters: " + number2String(sss->get_simulation_parameters()->size()));
	// montecarlo iterations is NOT set from main analysis
	sss->set_montecarlo_iterations( montecarlo_iterations );
	sss->set_max_parallel_montecarlo_instances( max_parallel_montecarlo_instances );
	// paralell instances
	// max_parallel_montecarlo_instances controlled by spectre
	sss->set_max_parallel_profile_instances( max_parallel_profile_instances );
	sss->set_log_io( log_io );
	sss->set_golden_metrics_structure( golden_metrics_structure );
	// Spectre command and args
	sss->set_spectre_command( spectre_command );
	sss->set_pre_spectre_command( pre_spectre_command );
	sss->set_post_spectre_command( post_spectre_command );
	sss->set_spectre_command_log_arg( spectre_command_log_arg );
	sss->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	sss->set_ahdl_simdb_env( kEnableAHDLFolderSIMDB + ahdl_simdb_folder_path );
	sss->set_ahdl_shipdb_env( kEnableAHDLFolderSHIPDB + ahdl_shipdb_folder_path );
	sss->set_top_folder( top_folder );
	sss->set_folder( variation_scenario_folder_path );
	sss->set_altered_statement_path( "none" );
	// result files
	sss->set_delete_spectre_folders( delete_spectre_folders );
	sss->set_delete_spectre_transients( delete_spectre_transients );
	// export_processed_metrics true, because of scatter plots,
	// and instead of delete_processed_transients || plot_transients || plot_last_transients
	sss->set_export_processed_metrics( export_processed_metrics ||
		plot_scatters || plot_transients || plot_last_transients );
	sss->set_delete_processed_transients( sss->get_export_processed_metrics() && delete_processed_transients );
	sss->set_plot_scatters( plot_scatters );
	sss->set_plot_transients( plot_transients );
	sss->set_export_metric_errors( export_metric_errors );
	// plotting variables
	sss->set_interpolate_plots_ratio( interpolate_plots_ratio );
	// analysis
	sss->set_main_analysis( simulation_mode->get_analysis_statement() );
	sss->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	sss->set_process_metrics( true );
	sss->set_export_metric_errors( export_metric_errors );
	// add simulation to list
	simulations.push_back(sss);
	// Run the threads
	log_io->ReportPlainStandard( kTab + "->Simulating variability netlist #" + number2String(radiationScenarioCounter) );
	log_io->ReportThread( "Variability Scenario Simulation #" + number2String(radiationScenarioCounter), 1 );
	// fgarcia, this reference can be a problem¿?
	// boost::thread radiation_t(boost::bind(&SpectreSimulation::HandleSpectreSimulation, boost::ref(sss)));
	tgScenarios.create_thread(boost::bind(&SpectreSimulation::HandleSpectreSimulation, sss));
	++radiationScenarioCounter;
	//wait each thread
	tgScenarios.join_all();
	log_io->ReportInfo2AllLogs("[performance] Reliability Simulations ended: " + GetCurrentDateTime("%d-%m-%Y.%X"));
	log_io->ReportPlainStandard( "->All Spectre instances have ended." );
	log_io->ReportPlainStandard( kTab + "->All simulations have ended." );
	return true;
}

bool VariabilitySpectreHandler::SimulateStandardAHDLNetlist( ){
	// Radiation subcircuit AHDL netlist
	AHDLSimulation* var_AHDL_s = new AHDLSimulation();
	var_AHDL_s->set_simulation_id("ahdl_scenario");
	// not needed
	// var_AHDL_s->set_parameter_index( 0 );
	// var_AHDL_s->set_sweep_index( 0 );
	// var_AHDL_s->set_is_nested_simulation(false);
	var_AHDL_s->set_main_analysis( simulation_mode->get_analysis_statement() );
	var_AHDL_s->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	var_AHDL_s->set_log_io( log_io );
	var_AHDL_s->CopySimulationParameters( simulation_parameters );
	log_io->ReportPurpleStandard( "var_AHDL_s simulation_parameters: " + number2String(var_AHDL_s->get_simulation_parameters()->size()));
	// not needed
	// var_AHDL_s->set_plot_transients( false );
	// Not required cause metrics are not processed
	// golden_ss->set_golden_metrics( &metrics_2be_found );
	var_AHDL_s->set_top_folder( top_folder );
	var_AHDL_s->set_folder( variations_AHDL_folder_path );
	var_AHDL_s->set_process_metrics( false );
	var_AHDL_s->set_export_metric_errors( false );
	// Spectre command and args
	var_AHDL_s->set_spectre_command( spectre_command );
	var_AHDL_s->set_pre_spectre_command( pre_spectre_command );
	var_AHDL_s->set_post_spectre_command( post_spectre_command );
	var_AHDL_s->set_spectre_command_log_arg( spectre_command_log_arg );
	var_AHDL_s->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	var_AHDL_s->set_ahdl_simdb_env( kEnableAHDLFolderSIMDB + ahdl_simdb_folder_path  );
	var_AHDL_s->set_ahdl_shipdb_env( kEnableAHDLFolderSHIPDB + ahdl_shipdb_folder_path  );
	log_io->ReportPurpleStandard( "Simulating var_AHDL netlist.");
	log_io->ReportThread( "AHDL scenario.", 1);
	// check others
	boost::thread var_AHDL_t(boost::bind(&SpectreSimulation::HandleSpectreSimulation, var_AHDL_s));
	var_AHDL_t.join();
	log_io->ReportGreenStandard( "var_AHDL netlist simulated.");
	if(var_AHDL_s->get_simulation_results()->get_spectre_result() > 0){
		log_io->ReportError2AllLogs( "WARNING: while simulation the ahdl scenario." );
	}
	// Reorder metrics
	bool partialResult = ReorderMetrics( var_AHDL_s->GetSpectreResultsFilePath(),
		var_AHDL_s->GetSpectreLogFilePath() );
	delete var_AHDL_s;
	return partialResult;
}

bool VariabilitySpectreHandler::ReorderMetrics( const std::string& spectreResultTrans,
		const std::string& spectreLog ){

	RAWFormatProcessor rfp;
	bool partialResult = rfp.PrepProcessTransientMetrics( &unsorted_metrics_2be_found,
		&metrics_2be_found, spectreResultTrans, spectreLog );
	// debug
	log_io->ReportCyanStandard( "Sorted Metrics to be found" );
	for( auto const& m : metrics_2be_found){
		if( m->is_transient_magnitude() ){
			log_io->ReportPlainStandard( m->get_name() + " is a transient metric" );
		}else{
			log_io->ReportPlainStandard( m->get_name() + " is an oceanEval metric" );
		}
	}
	if( !partialResult ){
		for( auto const& m : unsorted_metrics_2be_found){
			if( m->is_transient_magnitude() ){
				log_io->ReportRedStandard( "[debug] original unsorted metrics: " + m->get_name() + " is a transient metric" );
			}else{
				log_io->ReportRedStandard( "[debug] original unsorted metrics: " + m->get_name() + " is an oceanEval metric" );
			}
		}
	}
	// free memory
	deleteContentsOfVectorOfPointers( unsorted_metrics_2be_found );
	return partialResult;
}

bool VariabilitySpectreHandler::SimulateGoldenAHDLNetlist( ){
	// Golden netlist
	GoldenSimulation* ahdl_golden_ss = new GoldenSimulation();
	ahdl_golden_ss->set_n_d_profile_index( 0 );
	ahdl_golden_ss->set_is_nested_simulation(false);
	ahdl_golden_ss->set_simulation_id("AHDL_golden_scenario");
	// simulation parameters
	ahdl_golden_ss->CopySimulationParameters( simulation_parameters );
	ahdl_golden_ss->set_main_analysis( simulation_mode->get_analysis_statement() );
	ahdl_golden_ss->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	ahdl_golden_ss->set_log_io( log_io );
	ahdl_golden_ss->set_plot_transients( false );
	ahdl_golden_ss->set_plot_scatters( false );
	ahdl_golden_ss->set_top_folder( top_folder );
	ahdl_golden_ss->set_folder( golden_ahdl_scenario_folder_path );
	// result files
	ahdl_golden_ss->set_process_metrics( false );
	ahdl_golden_ss->set_export_metric_errors( false );
	ahdl_golden_ss->set_delete_spectre_folders( delete_spectre_folders );
	ahdl_golden_ss->set_delete_spectre_transients( false );
	ahdl_golden_ss->set_delete_processed_transients( false );
	// Spectre command and args
	ahdl_golden_ss->set_spectre_command( spectre_command );
	ahdl_golden_ss->set_pre_spectre_command( pre_spectre_command );
	ahdl_golden_ss->set_post_spectre_command( post_spectre_command );
	ahdl_golden_ss->set_spectre_command_log_arg( spectre_command_log_arg );
	ahdl_golden_ss->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	ahdl_golden_ss->set_ahdl_simdb_env( kEnableAHDLFolderSIMDB + ahdl_simdb_folder_path + "_golden" );
	ahdl_golden_ss->set_ahdl_shipdb_env( kEnableAHDLFolderSHIPDB + ahdl_shipdb_folder_path + "_golden" );
	log_io->ReportPurpleStandard( "Simulating AHDL golden netlist.");
	log_io->ReportThread( "AHDL Golden scenario.", 1);
	boost::thread ahdl_golden_t(boost::bind(&SpectreSimulation::HandleSpectreSimulation, ahdl_golden_ss));
	ahdl_golden_t.join();
	log_io->ReportGreenStandard( "AHDL Golden netlist simulated and processed.");
	if( ahdl_golden_ss->get_simulation_results()->get_spectre_result() > 0 ){
		log_io->ReportError2AllLogs( "Error while simulating or processing the ahdl_golden_ss scenario. Aborted." );
		delete ahdl_golden_ss;
		return false;
	}
	//delete all simulations
	delete ahdl_golden_ss;
	return true;
}

bool VariabilitySpectreHandler::SimulateGoldenNetlist( ){
	// We need for a first AHDL compilation for the golden
	if(!SimulateGoldenAHDLNetlist()){
		log_io->ReportError2AllLogs( "Error in AHDL Golden");
		return false;
	}
	// preorder metrics

	// Golden netlist
	GoldenNDParametersSweepSimulation* golden_ss
		= new GoldenNDParametersSweepSimulation();
	golden_ss->set_metrics_2be_found( &metrics_2be_found );
	golden_ss->set_max_parallel_profile_instances( max_parallel_profile_instances );
	// Golden netlist
	golden_ss->set_n_d_profile_index( 0 );
	golden_ss->set_is_nested_simulation(false);
	golden_ss->set_simulation_id("golden_scenario");
	// simulation parameters
	golden_ss->CopySimulationParameters( simulation_parameters );
	golden_ss->set_main_analysis( simulation_mode->get_analysis_statement() );
	golden_ss->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	golden_ss->set_log_io( log_io );
	golden_ss->set_plot_transients( plot_transients );
	golden_ss->set_plot_scatters( false );
	golden_ss->set_top_folder( top_folder );
	golden_ss->set_folder( golden_scenario_folder_path );
	// Files
	golden_ss->set_process_metrics( true );
	golden_ss->set_export_metric_errors( false );
	golden_ss->set_delete_spectre_folders( delete_spectre_folders );
	golden_ss->set_delete_spectre_transients( delete_spectre_transients );
	golden_ss->set_delete_processed_transients( true );
	// Spectre command and args
	golden_ss->set_spectre_command( spectre_command );
	golden_ss->set_pre_spectre_command( pre_spectre_command );
	golden_ss->set_post_spectre_command( post_spectre_command );
	golden_ss->set_spectre_command_log_arg( spectre_command_log_arg );
	golden_ss->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	golden_ss->set_ahdl_simdb_env( kEnableAHDLFolderSIMDB + ahdl_simdb_folder_path + "_golden" );
	golden_ss->set_ahdl_shipdb_env( kEnableAHDLFolderSHIPDB + ahdl_shipdb_folder_path + "_golden" );
	log_io->ReportPurpleStandard( "Simulating golden netlist.");
	log_io->ReportThread( "Golden scenario.", 1);
	boost::thread golden_t(boost::bind(&SpectreSimulation::HandleSpectreSimulation, golden_ss));
	golden_t.join();
	log_io->ReportGreenStandard( "Golden netlist simulated and processed.");

	// We copy the golden metrics, because golden_ss object is going to be destroyed
	// analysis/radiation parameters are pointers, and they are not destroyed in the
	// golden_ss var_AHDL_s destruction
	// Does not need to be copied because is not a GoldenSimulation member

	// Golden netlist
	if( !golden_ss->get_children_correctly_simulated() || !golden_ss->get_children_correctly_processed() ){
		log_io->ReportError2AllLogs( "Error while simulating or processing the golden scenario. Aborted." );
		delete golden_ss;
		return false;
	}
	log_io->ReportGreenStandard( "copying the golden metrics.");
	golden_metrics_structure = golden_ss->GetGoldenMetrics();
	log_io->ReportGreenStandard( "golden metrics copied. Deleting golden_ss");
	//delete all simulations
	delete golden_ss;
	log_io->ReportGreenStandard( "golden_ss deleted");
	return true;
}

void VariabilitySpectreHandler::AddMetric( Metric* metric ){
	this->unsorted_metrics_2be_found.push_back( metric );
}
