 /**
 * @file golden_nd_parameters_sweep_simulation.hpp
 *
 * @date Created on: March 14, 2016
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the simple sweep simulation mode.
 *
 */

#include <atomic>
// math
#include <cmath>
// Boost
#include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "golden_nd_parameters_sweep_simulation.hpp"
// #include "../simulation_results/results_summary_critical_parameter_1d_parameters_sweep.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

GoldenNDParametersSweepSimulation::GoldenNDParametersSweepSimulation() {
	this->max_parallel_profile_instances = 2;
	this->children_correctly_simulated = false;
	// injection mode related
	this->has_additional_injection = false;
}

GoldenNDParametersSweepSimulation::~GoldenNDParametersSweepSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout <<  "GoldenNDParametersSweepSimulation destructor. direction:" << this << "\n";
	#endif
	if( golden_metrics_structure ){
		#ifdef DESTRUCTORS_VERBOSE
			std::cout << "GoldenNDParametersSweepSimulation deleting golden_metrics_structure\n" ;
		#endif
		delete golden_metrics_structure;
	}
	// not required
	// delete golden_simulations_vector;
	// deleteContentsOfVectorOfPointers( golden_simulations_vector );
}

void GoldenNDParametersSweepSimulation::RunSimulation(){
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	// Environment
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	// metrics_structure
	// in 1d-> vector of vectors? ok
	std::vector<std::vector<Metric*>*>* gms = new std::vector<std::vector<Metric*>*>();
	std::vector<std::string>* fs = new std::vector<std::string>();

	golden_metrics_structure = new NDMetricsStructure();
	golden_metrics_structure->set_metrics_structure(gms);
	golden_metrics_structure->set_files_structure(fs);
	// main threads group
	boost::thread_group mainTG;
	// params to be sweeped
	std::vector<SimulationParameter*> parameters2sweep;
	// Threads Creation
	for(auto const &p : *simulation_parameters ){
		if( p->get_allow_sweep() ){
			// Init parameter sweep (increments etc)
			p->InitSweep();
			parameters2sweep.push_back( p );
		}
	}
	 // go through every possible parameter combination
	unsigned int totalThreads = 1;
	for( auto const &p : parameters2sweep ){
		totalThreads = totalThreads*p->get_sweep_steps_number();
	}
	log_io->ReportThread( "Total threads to be simulated: " + number2String(totalThreads) + ". Max number of sweep threads: " + number2String(max_parallel_profile_instances), 1 );
	// parallel threads control
	unsigned int runningThreads = 0;
	// // made atomic
	// std::atomic<unsigned int> threadsCount(0);
	// threadsCount = 0;
	unsigned int threadsCount = 0;
	// current parameter sweep indexes
	std::vector<unsigned int> parameterCountIndexes(parameters2sweep.size(), 0);
	// results
	golden_simulations_vector.ReserveSimulationsInMemory( totalThreads );
	golden_simulations_vector.set_group_name("golden_n-dimensional_analysis");
	// Threads Creation
	while( threadsCount<totalThreads ){
		// wait for resources
		WaitForResources( runningThreads, max_parallel_profile_instances, mainTG, threadsCount );
		// CreateProfile sets all parameter values, and after the simulation object
		// is created it can be updated.
		// Thus, it avoids race conditions when updating parameterCountIndexes and parameters2sweep
		GoldenSimulation* pGS = CreateProfile(parameterCountIndexes, parameters2sweep, threadsCount);
		if( pGS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(threadsCount) );
			children_correctly_simulated = false;
			children_correctly_processed = false;
			return;
		}
		golden_simulations_vector.AddSpectreSimulation( pGS );
		mainTG.add_thread( new boost::thread(boost::bind(&GoldenSimulation::RunSimulation, pGS)) );
		// update variables
		UpdateParameterSweepIndexes( parameterCountIndexes, parameters2sweep);
		++threadsCount;
		++runningThreads;
	}
	mainTG.join_all();
	log_io->ReportPlain2Log( "GoldenNDParametersSweepSimulation: mainTG.join_all()" );
	//
	for( auto const &pSS : *(golden_simulations_vector.get_spectre_simulations()) ){
		// obtain metrics and file references
		auto pGS = dynamic_cast<GoldenSimulation*>(pSS);
		auto pPM = pGS->get_processed_metrics();
		if( pPM==nullptr ){
			log_io->ReportError2AllLogs( "Null processed_metrics for golden_sim " + pGS->get_simulation_id() );
			children_correctly_simulated = false;
			children_correctly_processed = false;
			return;
		}
		gms->push_back( pPM );
		fs->push_back( pGS->get_singular_results_path() );
	}
	// check if every simulation ended correctly
	correctly_simulated = golden_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = golden_simulations_vector.CheckCorrectlyProcessed();
	children_correctly_simulated = correctly_simulated;
	children_correctly_processed = correctly_processed;
	if( !children_correctly_simulated || !children_correctly_processed){
		golden_simulations_vector.ReportChildrenCorrectness();
	}
	log_io->ReportPlain2Log( "END OF GoldenNDParametersSweepSimulation::RunSimulation" );
}

GoldenSimulation* GoldenNDParametersSweepSimulation::CreateProfile(
	const std::vector<unsigned int>& parameterCountIndexes,
	std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndIndex ){
	// Create folder
	std::string s_ndIndex = number2String(ndIndex);
	std::string currentFolder = folder + kFolderSeparator + "param_profile_" + s_ndIndex;
	if( !CreateFolder(currentFolder, true ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + currentFolder + "'." );
		log_io->ReportError2AllLogs( "Error running profile" );
		return nullptr;
	}
	// copy only files to folder
	// find . -maxdepth 1 -type f -exec cp {} destination_path \;
	std::string copyNetlists0 = "find ";
	std::string copyNetlists1 = " -maxdepth 1 -type f -exec cp {} ";
	std::string copyNetlists2 = " \\;";
	std::string copyNetlists;
	copyNetlists = copyNetlists0 + folder + copyNetlists1 + currentFolder + copyNetlists2;
	if( std::system( copyNetlists.c_str() ) > 0){
		log_io->ReportError2AllLogs( k2Tab + "-> Error while copying netlist to '" + currentFolder + "'." );
		log_io->ReportError2AllLogs( "Error running sweep" );
		return nullptr;
	}
	// create thread
	// GoldenSimulation* pGS = CreateGoldenSimulation( currentFolder, parameterCountIndexes,
	// 	parameters2sweep, ndIndex );
	GoldenSimulation* pGS = new GoldenSimulation();
	pGS->set_has_additional_injection( false );
	pGS->set_is_nested_simulation( true );
	pGS->set_simulation_id(  "golden_param_profile_" + s_ndIndex );
	pGS->set_n_dimensional(true);
	pGS->set_n_d_profile_index(ndIndex);
	pGS->set_log_io( log_io );
	pGS->set_altered_scenario_index( altered_scenario_index );
	// pGS->set_golden_metrics_structure( golden_metrics_structure );
	// Spectre command and args
	pGS->set_spectre_command( spectre_command );
	pGS->set_pre_spectre_command( pre_spectre_command );
	pGS->set_post_spectre_command( post_spectre_command );
	pGS->set_spectre_command_log_arg( spectre_command_log_arg );
	pGS->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	pGS->set_metrics_2be_found( metrics_2be_found );
	pGS->set_ahdl_simdb_env( ahdl_simdb_env );
	pGS->set_ahdl_shipdb_env( ahdl_shipdb_env );
	pGS->set_top_folder( top_folder );
	pGS->set_folder( currentFolder );
	pGS->set_altered_statement_path( altered_statement_path );
	pGS->set_delete_spectre_transients( delete_spectre_transients );
	pGS->set_delete_processed_transients( delete_processed_transients );
	pGS->set_plot_transients( plot_transients );
	pGS->set_main_analysis( main_analysis );
	pGS->set_main_transient_analysis( main_transient_analysis );
	pGS->set_process_metrics( true );
	// golden always true
	pGS->set_export_processed_metrics( true );
	pGS->set_export_metric_errors( export_metric_errors );
	// copy of simulation_parameters
	pGS->CopySimulationParameters( *simulation_parameters );
	// update parameter values
	unsigned int sweepedParamIndex = 0;
	for( auto const &p : parameters2sweep ){
		// if parameter value is fixed for golden scenario, whe use the defualt value
		if( !p->get_golden_fixed() ){
			// update val only when required,
			if( !pGS->UpdateParameterValue( *p, number2String( p->GetSweepValue( parameterCountIndexes.at(sweepedParamIndex)) ) ) ){
				log_io->ReportError2AllLogs( p->get_name() + " not found in GoldenSimulation and could not be updated." );
			}
		}
		++sweepedParamIndex;
	}
	return pGS;
}

bool GoldenNDParametersSweepSimulation::TestSetUp(){
	if( simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr parameter_critical_value");
		return false;
	}else if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder ");
		return false;
	}
	return true;
}

NDMetricsStructure* GoldenNDParametersSweepSimulation::GetGoldenMetrics(){
	NDMetricsStructure* gms = new NDMetricsStructure(*golden_metrics_structure);
	return gms;
}
