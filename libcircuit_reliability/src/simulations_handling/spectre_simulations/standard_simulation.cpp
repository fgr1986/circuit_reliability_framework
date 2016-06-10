 /**
 * @file standard_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * StandardSimulation Class Body
 *
 */

#include <cmath>
// Radiation simulator
#include "standard_simulation.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

StandardSimulation::StandardSimulation() {
	this->export_processed_magnitudes = true;
	this->montecarlo_transient_sufix = kNotDefinedString;
}

StandardSimulation::~StandardSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "StandardSimulation destructor" << "\n";
	#endif
}

void StandardSimulation::RunSpectreSimulation( ){
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSpectreSimulation had not been previously set up. ");
		return;
	}
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr. ");
		return;
	}
	// Register Parameters
	basic_simulation_results.RegisterSimulationParameters(simulation_parameters);
	basic_simulation_results.set_full_id( simulation_id );
	basic_simulation_results.set_title( simulation_id );
	// Environment variables
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	// Parameters file
	if( !ExportParametersCircuit( folder, 0 )){
		log_io->ReportError2AllLogs( "Error creating parameters Circuit ");
		return;
	}
	basic_simulation_results.set_spectre_result( RunSpectre( simulation_id ) );
	if( process_magnitudes ){
		log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: processing results.");
		// Set up magnitudes
		std::vector<Magnitude*>* analyzedMagnitudes = CreateMagnitudesVectorFromGoldenMagnitudes( n_d_profile_index );
		// process magnitudes
		if( !ProcessSpectreResults( folder, simulation_id, basic_simulation_results, false, *analyzedMagnitudes, false ) ){
			log_io->ReportError2AllLogs( "Error while processing the critical value simulation spectre_results. Scenario #"
				+ simulation_id );
			return;
		}
		// Interpolating and analyzing magnitudes
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlain2Log( kTab + "#" + simulation_id + " -> Interpolating spectre_results");
		#endif
		if( !InterpolateAndAnalyzeMagnitudes( basic_simulation_results, *analyzedMagnitudes, n_d_profile_index, simulation_id ) ){
			log_io->ReportError2AllLogs( "Error while interpolating the critical value magnitudes. Scenario #"
				+ simulation_id );
			return;
		}
		// plot previous transient, if needed
		if( !PlotTransient( simulation_id, basic_simulation_results, false ) ){
			log_io->ReportError2AllLogs( "Error while ploting transients. Scenario #"	+ simulation_id );
			// return false; // program can continue
		}
		// delete previous transients, if needed
		if( !ManageIndividualResultFiles( basic_simulation_results, false ) ){
			log_io->ReportError2AllLogs( "Error deleting raw data. Scenario #" + simulation_id );
			// return false; // program can continue
		}
		// delete analyzed magnitudes
		deleteContentsOfVectorOfPointers( *analyzedMagnitudes);
		delete analyzedMagnitudes;
	}
}

bool StandardSimulation::TestSetUp(){
	if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder ");
		return false;
	}else if( golden_magnitudes_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_magnitudes_structure");
		return false;
	}
	return true;
}


std::string StandardSimulation::GetSpectreResultsFilePath(const std::string& currentFolder,
	const bool& processMainTransient){
	// for critical_parameter_value_simulations that are nested
	if( is_montecarlo_nested_simulation ){
		return currentFolder + kFolderSeparator + kSpectreResultsFolder
			+ kFolderSeparator + main_analysis->get_name() + "-" + montecarlo_transient_sufix + "_"
			+ main_transient_analysis->get_name() + kTransientSufix;
	}else{
		std::string analysisFinalName = main_analysis->get_name();
		if( processMainTransient ){
			analysisFinalName = main_transient_analysis->get_name();
		}
		return currentFolder + kFolderSeparator + kSpectreResultsFolder
			+ kFolderSeparator + analysisFinalName + kTransientSufix;
	}
}

std::string StandardSimulation::GetProcessedResultsFilePath(const std::string& currentFolder,
		const std::string& localSimulationId, const bool& processMainTransient){
	// for critical_parameter_value_simulations that are nested
	if( is_montecarlo_nested_simulation ){
		return top_folder + kFolderSeparator + kResultsFolder + kFolderSeparator
		+ kResultsDataFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ kProcessedPrefix + simulation_id + "_m_" + montecarlo_transient_sufix + kDataSufix;
	}else{
		std::string analysisFinalName = main_analysis->get_name();
		if( processMainTransient ){
			analysisFinalName = main_transient_analysis->get_name();
		}
		return top_folder + kFolderSeparator + kResultsFolder + kFolderSeparator + kResultsDataFolder
			+ kFolderSeparator + kTransientResultsFolder + kFolderSeparator
			+ localSimulationId + "_" + kProcessedTransientFile;
	}
}

int StandardSimulation::RunSpectre( std::string scenarioId ){
	std::string execCommand = spectre_command + " "
		+ spectre_command_log_arg + " " + folder + kFolderSeparator + kSpectreLogFile + " "
		+ spectre_command_folder_arg + " " + folder + kFolderSeparator + kSpectreResultsFolder + " "
		+ folder + kFolderSeparator + kMainNetlistFile
		+ " " + post_spectre_command + " " + folder + kFolderSeparator + kSpectreStandardLogsFile ;

	log_io->ReportPlainStandard( k2Tab + "#" + scenarioId + " scenario: Simulating scenario."  );
	log_io->ReportPlain2Log( k2Tab + "#" + scenarioId + " scenario: Simulating scenario." );

	int spectre_result = std::system( execCommand.c_str() ) ;
	if(spectre_result>0){
		correctly_simulated = false;
		log_io->ReportError2AllLogs( "Unexpected Spectre spectre_result for scenario #"
			+ scenarioId + ": spectre output = " + number2String(spectre_result) );
		log_io->ReportError2AllLogs( "Spectre Log Folder: " + folder );
		return spectre_result;
	}
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportGreenStandard( k2Tab + "#" + scenarioId + " scenario: Simulating ENDED."
		+ " spectre_result=" + number2String(spectre_result) );
	#endif
	return spectre_result;
}
