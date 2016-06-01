 /**
 * @file golden_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * AHDLSimulation Class Body
 *
 */

// Radiation simulator
#include "ahdl_simulation.hpp"
// constants
#include "../../io_handling/raw_format_processor.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

AHDLSimulation::AHDLSimulation() {
	this->singular_results_path = kNotDefinedString;
}

AHDLSimulation::~AHDLSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
	   std::cout<< "AHDLSimulation destructor. direction:" + number2String(this) << "\n";
	#endif
}

void AHDLSimulation::RunSpectreSimulation( ){
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSpectreSimulation had not been previously set up. ");
		return;
	}
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr. ");
		return;
	}
	if( !ExportParametersCircuit( folder, n_d_profile_index )){
		log_io->ReportError2AllLogs( "Error creating parameters Circuit ");
		return;
	}
	basic_simulation_results.set_spectre_result( RunSpectre() );
}

bool AHDLSimulation::TestSetUp(){
	if( folder.compare("")==0 ){
		log_io->ReportError2AllLogs( "nullptr folder ");
		return false;
	}
	return true;
}

int AHDLSimulation::RunSpectre(){
	std::string execCommand = spectre_command + " "
		+ spectre_command_log_arg + " " + folder + kFolderSeparator + kSpectreLogFile + " "
		+ spectre_command_folder_arg + " " + folder + kFolderSeparator + kSpectreResultsFolder + " "
		+ folder + kFolderSeparator + kMainNetlistFile
		+ " " + post_spectre_command + " " + folder + kFolderSeparator + kSpectreStandardLogsFile ;

	log_io->ReportPlainStandard( k2Tab + "#" + simulation_id + " scenario: Simulating singular scenario."  );
	log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: Simulating singular scenario." );

	int spectre_result = std::system( execCommand.c_str() ) ;
	if(spectre_result>0){
		correctly_simulated = false;
		log_io->ReportError2AllLogs( "Unexpected Spectre spectre_result for singular scenario #"
			+ simulation_id + ": spectre output = " + number2String(spectre_result) );
		log_io->ReportError2AllLogs( "Spectre Log Folder " + folder );
		return spectre_result;
	}
	singular_results_path = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ simulation_id + "_" + kProcessedTransientFile;

	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportGreenStandard( k2Tab + "#" + simulation_id + " scenario: ENDED."
		+ " spectre_result=" + number2String(spectre_result) );
	#endif
	return spectre_result;
}
