 /**
 * @file experiment.cpp
 *
 * @date Created on: November 15, 2013
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of HTMLIO Experiment (see experiment.hpp)
 *
 */

// simulator includes
#include "experiment.hpp"

// Spectre Handler Library
// Constants includes
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/files_folders_io_constants.hpp"
// Netlist modeling includes
#include "netlist_modeling/magnitude.hpp"
#include "netlist_modeling/statements/instance_statement.hpp"
#include "netlist_modeling/statements/simple_statement.hpp"

Experiment::Experiment() {
	this->export_matlab_script = false;
	main_circuit.set_name( "main_circuit" );
}

Experiment::~Experiment() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlain2Log( "Experiment Destructor. Direction: " + number2String(this) );
	#endif
}

/// Stage 1, configures environment
bool Experiment::ConfigureEnvironment(){
	bool success = true;
	log_io->ReportStage( "1.0) Configuring simulator...", false );
	log_io->ReportConfigurationOptions(i_xml_file_cadence, i_xml_file_technology,
			i_xml_file_experiment, experiment_folder, number2String( permissive_parsing_mode ) );
	// simulation_modes_handler, xml_io_manager, experiment_environment and post_parsing_statement_handler logger configuration
	simulation_modes_handler.set_log_io( log_io );
	xml_io_manager.set_log_io( log_io );
	experiment_environment.set_log_io( log_io );
	post_parsing_statement_handler.set_log_io( log_io );
	circuit_io_handler.set_log_io( log_io );
	variability_spectre_handler.set_log_io( log_io );
	// Create simulation_modes_handler modes
	simulation_modes_handler.CreateVariabilityProgramModes();
	// Read Configuration from xml
	log_io->ReportPlainStandard( kTab + "1.1) Reading xml configuration files, searching analysis types control statement types and keywords.");
	if( !xml_io_manager.ReadExperimentEnvironmentFromXML( i_xml_file_cadence, i_xml_file_technology,
			i_xml_file_experiment, experiment_environment, variability_spectre_handler, simulation_modes_handler,
			circuit_io_handler, experiment_title ) ){
		log_io->ReportError2AllLogs( kTab + "!!!! Error reading xml !!!! " );
		log_io->ReportError2AllLogs( kTab + "Please, check the xml configuration files." );
		// Restore the previous streambuf (used in log and error files)
		return false;
	}
	log_io->ReportPlainStandard( k2Tab + "-> XML correctly read." );
	// Update experiment_environment regex
	experiment_environment.UpdatekAnalysisStatementRegEx();
	experiment_environment.UpdatekTransistorStatementRegEx();
	experiment_environment.UpdatekControlStatementRegEx();
	log_io->ReportPlainStandard( k2Tab + "ExperimentEnvironment regex updated." );
	// Configure circuit_io_handler
	circuit_io_handler.set_experiment_environment( &experiment_environment );
	circuit_io_handler.set_post_parsing_statement_handler( &post_parsing_statement_handler );
	// No longer needed
	//	circuit_io_handler.set_xml_io_manager( &xml_io_manager );
	//	circuit_io_handler.set_variability_spectre_handler( &variability_spectre_handler );
	// Configure circuit_exporter
	circuit_exporter.set_variability_spectre_handler( &variability_spectre_handler );
	circuit_exporter.set_experiment_environment( &experiment_environment );
	circuit_exporter.set_post_parsing_statement_handler( &post_parsing_statement_handler );
	circuit_exporter.set_circuit_io_handler( &circuit_io_handler );
	circuit_exporter.set_top_folder( experiment_folder );
	circuit_exporter.set_main_circuit( &main_circuit );
	// Configure main circuit
	main_circuit.set_id( 0 );
	main_circuit.set_circuit_id( 0 );
	main_circuit.set_log_io( log_io );
	main_circuit.set_post_parsing_statement_handler( &post_parsing_statement_handler );
	main_circuit.set_master_name( "circuit" );
	main_circuit.set_name( kMainCircuitStringId );
	main_circuit.set_considerate_dependencies( true );
	main_circuit.set_description( i_netlist_file );
	main_circuit.set_experiment_environment( &experiment_environment );
	// spectre handler configuration
	variability_spectre_handler.set_top_folder( experiment_folder );
	variability_spectre_handler.set_export_matlab_script( export_matlab_script );
	// Create folders:
	success = success && CreateFolders();
	// Adding ahdl folders reference
	variability_spectre_handler.set_ahdl_simdb_folder_path( compiled_components_folder + kFolderSeparator + "SIMDB" );
	variability_spectre_handler.set_ahdl_shipdb_folder_path( compiled_components_folder + kFolderSeparator + "SHIPDB" );
	// Adding golden simulation folder reference
	variability_spectre_handler.set_golden_scenario_folder_path( golden_folder );
	variability_spectre_handler.set_golden_ahdl_scenario_folder_path( golden_ahdl_folder );
	variability_spectre_handler.set_variations_AHDL_folder_path(variations_ahdl_folder);
	variability_spectre_handler.set_variation_scenario_folder_path( variation_scenario_folder );

	// Simulation mode selection
	if( !simulation_modes_handler.VariabilitySimulationModeSelection() ){
		log_io->ReportError2AllLogs( k2Tab + "-> Simulation mode has not been selected." );
		return false;
	}
	// Set up simulation mode to other objects
	variability_spectre_handler.set_simulation_mode( simulation_modes_handler.get_selected_mode() );
	experiment_environment.set_simulation_mode( simulation_modes_handler.get_selected_mode() );
	circuit_exporter.set_simulation_mode( simulation_modes_handler.get_selected_mode() );
	circuit_io_handler.set_simulation_mode( simulation_modes_handler.get_selected_mode() );
	post_parsing_statement_handler.set_simulation_modes_handler( &simulation_modes_handler );
	//inject_all_mode in circuit_exporter is not needed because is experiment_environment
	// the responsible of TestCanBeInjected
	log_io->ReportPlainStandard( k2Tab + "-> Main circuit ready to be parsed." );
	return success;
}

bool Experiment::CreateFolders(){
	log_io->ReportPlainStandard( k2Tab + "Creating output folders." );
	// Create results folder
	if( circuit_io_handler.CreateFolder(experiment_folder, true) ) {
		log_io->ReportPlainStandard( k2Tab + "-> Results stored in '" + experiment_folder + "'." );
	} else {
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + experiment_folder + "'." );
		return false;
	}
	results_folder = experiment_folder + kFolderSeparator + kResultsFolder;
	compiled_components_folder = experiment_folder + kFolderSeparator + kCompiledComponentsFolder;
	variation_scenario_folder = experiment_folder + kFolderSeparator + kAlterationScenariosFolder;
	variations_ahdl_folder = experiment_folder + kFolderSeparator + kAlterationSubcircuitAHDLFolder ;
	golden_folder = experiment_folder + kFolderSeparator + kGoldenFolder ;
	golden_ahdl_folder = experiment_folder + kFolderSeparator + kGoldenFolder + "_ahdl" ;
	// Resultsfolder
	//	->generated data (sweep and montecarlo)
	//	->generated gnu_plot_scripts
	//	->generated images
	//	->generated pdf

	// results folders
	if( !circuit_io_handler.CreateFolder( results_folder , false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + results_folder + "'." );
		return false;
	}
	// data
	std::string dataFolder = results_folder + kFolderSeparator + kResultsDataFolder;
	if( !circuit_io_handler.CreateFolder( dataFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + dataFolder+ "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( dataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ dataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( dataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolderSubProfiles, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ dataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolderSubProfiles + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( dataFolder + kFolderSeparator + kTransientResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ dataFolder + kFolderSeparator + kTransientResultsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( dataFolder + kFolderSeparator + kSummaryResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ dataFolder + kFolderSeparator + kSummaryResultsFolder + "'." );
		return false;
	}
	// gnuplot_scripts
	std::string gnuplotScriptsFolder = results_folder + kFolderSeparator + kResultsGnuplotScriptsFolder;
	if( !circuit_io_handler.CreateFolder( gnuplotScriptsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+gnuplotScriptsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( gnuplotScriptsFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ gnuplotScriptsFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( gnuplotScriptsFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolderSubProfiles, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ dataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolderSubProfiles + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( gnuplotScriptsFolder + kFolderSeparator + kTransientResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ gnuplotScriptsFolder + kFolderSeparator + kTransientResultsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( gnuplotScriptsFolder + kFolderSeparator + kSummaryResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ gnuplotScriptsFolder + kFolderSeparator + kSummaryResultsFolder + "'." );
		return false;
	}
	// images
	std::string imagesFolder = results_folder + kFolderSeparator + kResultsImagesFolder;
	if( !circuit_io_handler.CreateFolder( imagesFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ imagesFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( imagesFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ imagesFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( imagesFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolderSubProfiles, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ dataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolderSubProfiles + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( imagesFolder + kFolderSeparator + kTransientResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ imagesFolder + kFolderSeparator + kTransientResultsFolder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( imagesFolder + kFolderSeparator + kSummaryResultsFolder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '"
			+ imagesFolder + kFolderSeparator + kSummaryResultsFolder + "'." );
		return false;
	}
	// Create compiled_components_folder folder
	if( !circuit_io_handler.CreateFolder( compiled_components_folder , false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + compiled_components_folder + "'." );
		return false;
	}
	circuit_io_handler.CreateFolder( compiled_components_folder + kFolderSeparator + "SHIPDB", false );
	circuit_io_handler.CreateFolder( compiled_components_folder + kFolderSeparator + "SIMDB", false );
	circuit_io_handler.CreateFolder( compiled_components_folder + kFolderSeparator + "SHIPDB_golden", false );
	circuit_io_handler.CreateFolder( compiled_components_folder + kFolderSeparator + "SIMDB_golden", false );
	// Create golden folder
	if( !circuit_io_handler.CreateFolder( golden_folder , false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + golden_folder + "'." );
		return false;
	}
	if( !circuit_io_handler.CreateFolder( golden_ahdl_folder , false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + golden_ahdl_folder + "'." );
		return false;
	}
	// Create variations_ahdl_folder folder
	if( !circuit_io_handler.CreateFolder( variations_ahdl_folder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + variations_ahdl_folder + "'." );
		return false;
	}
	// Create the scenarios folder
	if( !circuit_io_handler.CreateFolder( variation_scenario_folder, false ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + variation_scenario_folder + "'." );
		return false;
	}
	log_io->ReportPlainStandard( k2Tab + "output folders created." );
	return true;
}

// Stage 2
bool Experiment::ReadMainNetlist( int stageNumber ){
	return circuit_io_handler.ReadMainNetlist( stageNumber, i_netlist_file, main_circuit, permissive_parsing_mode);
}

// Stage 3
bool Experiment::AnalyzeStatementDependency( int stageNumber ){
	bool success;
	log_io->ReportStage( number2String(stageNumber) + ") Analyzing Instances Dependency...", false );
	success = post_parsing_statement_handler.SetUpAnalysisExportation();
	success = post_parsing_statement_handler.AnalyzeStatementDependency();
	if( success ){
		log_io->ReportInfo2AllLogs( "" );
		log_io->ReportInfo2AllLogs( k2Tab + "-> All dependencies analyzed." );
	}else{
		log_io->ReportError2AllLogs( k2Tab + "Error analyzing dependencies !!!." );
	}
	return success;
}

// Stage 4
bool Experiment::CreateVariationScenarios( int stageNumber ){
	bool success = true;
	log_io->ReportStage( number2String(stageNumber) + ") Radiating main netlist nodes...", false );
	success = success && circuit_exporter.SetUpScenarios(
		golden_folder, golden_ahdl_folder, variations_ahdl_folder, variation_scenario_folder );
	if( success ){
		log_io->ReportInfo2AllLogs( "" );
		log_io->ReportInfo2AllLogs( k2Tab + "-> Main Circuit analyzed." );
	}else{
		log_io->ReportError2AllLogs( k2Tab + "Error radiatying main circuit !!!." );
	}
	return success;
}

// Stage 5
bool Experiment::SimulateVariationScenarios( int stageNumber ){
	bool success;
	log_io->ReportStage( number2String(stageNumber) + ") Simulating the altered scenarios...", false );
	success = variability_spectre_handler.RunSimulations();
	if( success ){
		log_io->ReportInfo2AllLogs( "" );
		log_io->ReportInfo2AllLogs( k2Tab + "-> Scenarios Simulated." );
	}else{
		log_io->ReportError2AllLogs( k2Tab + "Error simulating the altered scenarios !!!." );
	}
	return success;
}
