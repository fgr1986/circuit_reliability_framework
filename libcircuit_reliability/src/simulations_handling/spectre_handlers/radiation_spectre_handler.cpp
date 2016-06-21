/*
 * radiation_spectre_handler.cpp
 *
 *  Created on: Feb 03, 2014
 *      Author: fernando, fgarcia@die.upm.es
 */

// Reliability simulator
#include "radiation_spectre_handler.hpp"
#include "../spectre_simulations/golden_simulation.hpp"
#include "../spectre_simulations/golden_nd_parameters_sweep_simulation.hpp"
#include "../spectre_simulations/ahdl_simulation.hpp"
#include "../spectre_simulations/standard_simulation.hpp"
#include "../spectre_simulations/critical_parameter_nd_parameters_sweep_simulation.hpp"
#include "../spectre_simulations/montecarlo_critical_parameter_nd_parameters_sweep_simulation.hpp"

#include "../../io_handling/results_processor.hpp"
#include "../../io_handling/raw_format_processor.hpp"

#include "../simulation_results/global_results.hpp"
#include "../spectre_simulations/critical_parameter_value_simulation.hpp"
// constants
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/ahdl_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

RadiationSpectreHandler::RadiationSpectreHandler() {
	// spectre
	this->pre_spectre_command = kNotDefinedString;
	this->spectre_command = kNotDefinedString;
	this->post_spectre_command = kNotDefinedString;
	this->golden_scenario_path = kNotDefinedString;
	this->spectre_command_log_arg = kNotDefinedString;
	this->spectre_command_folder_arg = kNotDefinedString;
	this->golden_scenario_folder_path = kNotDefinedString;
	// export_processed_magnitudes
	this->export_processed_magnitudes = false;
	// plot
	this->plot_scatters = false;
	this->plot_transients = false;
	this->plot_last_transients = false;
	this->export_matlab_script = false;
	// files
	this->export_magnitude_errors = false;
	this->delete_spectre_folders = false;
	this->delete_spectre_transients = true;
	this->delete_processed_transients = true;
	this->golden_magnitudes_structure = nullptr;
	// montecarlo iterations
	this->montecarlo_iterations = 1;
	// parallel
	this->max_parallel_profile_instances = 2;
	this->max_parallel_montecarlo_instances = 5;
	this->max_parallel_scenario_instances = 2;
}

RadiationSpectreHandler::~RadiationSpectreHandler() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "RadiationSpectreHandler destructor. direction:" + number2String(this));
		log_io->ReportPlainStandard( "magnitudes_2be_found" );
	#endif
	deleteContentsOfVectorOfPointers( simulation_parameters );
	deleteContentsOfVectorOfPointers( magnitudes_2be_found );
	// ReorderMagnitudes clears unsorted_magnitudes_2be_found
	// deleteContentsOfVectorOfPointers( unsorted_magnitudes_2be_found );
	if( golden_magnitudes_structure ){
		#ifdef DESTRUCTORS_VERBOSE
			log_io->ReportPlainStandard( "golden_magnitudes_structure");
		#endif
		delete golden_magnitudes_structure;
	}
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "simulations");
	#endif
	deleteContentsOfVectorOfPointers( simulations );
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "altered_scenarios");
	#endif
	deleteContentsOfVectorOfPointers( altered_scenarios );
}

void RadiationSpectreHandler::AddSimulationParameter( SimulationParameter* simulationParameter  ){
	simulation_parameters.push_back(simulationParameter);
}

void RadiationSpectreHandler::AddScenarioFolderPath( const int alteredScenarioId,
	const std::string alteredScenarioFolderPath, const std::string alteredStatementPath ){
	altered_scenarios.push_back( new AlteredScenarioSummary(alteredScenarioId,
		alteredScenarioFolderPath, alteredStatementPath) );
}

bool RadiationSpectreHandler::ExportScenariosList(){
	log_io->ReportGreenStandard( "Exporting scenarios list" );
	ResultsProcessor rp;
	return rp.ExportScenariosList(top_folder, altered_scenarios);
}

bool RadiationSpectreHandler::ExportProfilesList(){
	log_io->ReportGreenStandard( "Exporting profiles list" );
	ResultsProcessor rp;
	return rp.ExportProfilesList(top_folder, simulation_parameters);
}

bool RadiationSpectreHandler::ProcessScenarioStatistics(){
	// Novel results handler
	GlobalResults globalResults;
	globalResults.set_log_io( log_io );
	globalResults.set_top_folder( top_folder );
	globalResults.set_simulation_mode( simulation_mode );
	globalResults.set_simulations( &simulations );
	return globalResults.ProcessScenarioStatistics();
}

bool RadiationSpectreHandler::RunSimulations(){
	// Report simulation
	log_io->ReportGreenStandard( kTab + simulation_mode->get_description());
	log_io->ReportGreenStandard( "RadiationSpectreHandler Simulation Parameters: " + number2String( simulation_parameters.size()) );
	// Thread group
	boost::thread_group tgScenarios;
	int radiationScenarioCounter = 0;
	// Golden netlist and Radiation subcircuit AHDL netlist
	// Golden results are processed
	if( !SimulateGoldenNetlist() || !SimulateStandardAHDLNetlist() ){
		log_io->ReportError2AllLogs( "Error while simulating or processing the golden (or ahdl) scenario. Aborted." );
		return false;
	}
	log_io->ReportCyanStandard( k2Tab + "Simulate pool of altered scenarios." );
	if( plot_scatters ){
		log_io->ReportCyanStandard( k2Tab + "Scatters will be plotted." );
	}
	if( plot_transients ){
		log_io->ReportCyanStandard( k2Tab + "Single transients will be plotted." );
	}
	// exporting the scenario path (id) and altered instance
	log_io->ReportCyanStandard( k2Tab + "Exporting scenarios and profiles." );
	if( !ExportScenariosList() || !ExportProfilesList() ){
		log_io->ReportError2AllLogs( kTab + "Error exporting scenarios and profiles." );
	}
	// reserve memory
	simulations.reserve( altered_scenarios.size() );
	// retrieve numruns and set it to '1', for manual handling in kMontecarloCriticalParameterNDParametersSweepMode
	if( simulation_mode->get_id()==kMontecarloCriticalParameterNDParametersSweepMode){
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
	}
	// run
	log_io->ReportInfo2AllLogs("[performance] Reliability Simulations started: " + GetCurrentDateTime("%d-%m-%Y.%X"));
	unsigned int concurrentScenarioThreads = 0;
	for( auto const & as : altered_scenarios ){
		// limit concurrent parallel scenarios
		if( concurrentScenarioThreads == max_parallel_scenario_instances ){
			// Wait for threads
			#ifdef SPECTRE_SIMULATIONS_VERBOSE
			log_io->ReportPlainStandard( "Waiting for some Scenario threads to end." );
			#endif
			tgScenarios.join_all();
			concurrentScenarioThreads = 0;
		}
		SpectreSimulation* sss;
		switch( simulation_mode->get_id() ){
			case kStandardMode: {
					sss = new StandardSimulation();
					// StandardSimulation* pStandardS =  dynamic_cast<StandardSimulation*>(sss);
				}
			break;
			case kCriticalValueMode: {
					if( critical_parameter==nullptr ){
						log_io->ReportError2AllLogs( "This mode does not allow a nullptr critical parameter" );
						return false;
					}
					sss = new CriticalParameterValueSimulation();
				}
			break;
			case kCriticalParameterNDParametersSweepMode: {
					if( critical_parameter==nullptr ){
						log_io->ReportError2AllLogs( "This mode does not allow a nullptr critical parameter" );
						return false;
					}
					sss = new CriticalParameterNDParameterSweepSimulation();
					CriticalParameterNDParameterSweepSimulation* pParameterSweep =  dynamic_cast<CriticalParameterNDParameterSweepSimulation*>(sss);
					pParameterSweep->set_max_parallel_profile_instances( max_parallel_profile_instances );
					pParameterSweep->set_plot_last_transients( plot_last_transients );
				}
			break;
			case kMontecarloCriticalParameterNDParametersSweepMode: {
					if( critical_parameter==nullptr ){
						log_io->ReportError2AllLogs( "This mode does not allow a nullptr critical parameter" );
						return false;
					}
					sss = new MontecarloCriticalParameterNDParametersSweepSimulation();
					MontecarloCriticalParameterNDParametersSweepSimulation* pMCPNDPSS =
						dynamic_cast<MontecarloCriticalParameterNDParametersSweepSimulation*>(sss);
					// get numruns and set it to montecarlo_iterations
					pMCPNDPSS->set_montecarlo_iterations( montecarlo_iterations );
					pMCPNDPSS->set_max_parallel_profile_instances( max_parallel_profile_instances );
					pMCPNDPSS->set_plot_last_transients( plot_last_transients );
					pMCPNDPSS->set_max_parallel_montecarlo_instances( max_parallel_montecarlo_instances );
				}
			break;
			default: {
				log_io->ReportRedStandard("default mode:" + number2String(simulation_mode->get_id()) );
				return false;
			}
			break;
		}
		sss->set_n_d_profile_index( 0 );
		sss->set_simulation_id("parent_scenario_" + number2String( as->get_altered_scenario_id() ));
		sss->set_is_nested_simulation(false);
		sss->set_altered_scenario_index( radiationScenarioCounter );
		// simulation parameters
		sss->CopySimulationParameters( simulation_parameters );
		if( sss->get_simulation_parameters()==nullptr ){
			log_io->ReportError2AllLogs( "nullptr sss get_simulation_parameters");
			return false;
		}
		// Critical Parameter
		if( simulation_mode->get_id()== kCriticalValueMode
			|| simulation_mode->get_id()== kCriticalParameterNDParametersSweepMode
			|| simulation_mode->get_id()== kMontecarloCriticalParameterNDParametersSweepMode ){
			if( !sss->UpdateGoldenCriticalParameter( *critical_parameter ) ){
				log_io->ReportError2AllLogs( critical_parameter->get_name()
					+ " not found in simulation and could not be updated." );
			}else{
				log_io->ReportPurpleStandard( critical_parameter->get_name()
					+ " found and updated." );
			}
		}
		sss->set_log_io( log_io );
		sss->set_golden_magnitudes_structure( golden_magnitudes_structure );
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
		sss->set_folder( as->get_altered_scenario_folder_path() );
		sss->set_altered_statement_path( as->get_altered_scenario_altered_element_path() );
		// result files
		sss->set_delete_spectre_folders( delete_spectre_folders );
		sss->set_delete_spectre_transients( delete_spectre_transients );
		sss->set_export_processed_magnitudes( export_processed_magnitudes ||
			plot_scatters || plot_transients || plot_last_transients );
		sss->set_delete_processed_transients( sss->get_export_processed_magnitudes() && delete_processed_transients );
		sss->set_plot_scatters( plot_scatters );
		sss->set_plot_transients( plot_transients );
		sss->set_export_magnitude_errors( export_magnitude_errors );
		// plotting variables
		sss->set_interpolate_plots_ratio( interpolate_plots_ratio );
		// analysis
		sss->set_main_analysis( simulation_mode->get_analysis_statement() );
		sss->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
		sss->set_process_magnitudes( true );
		// add simulation to list
		simulations.push_back(sss);
		// Run the threads
		log_io->ReportPlainStandard( kTab + "->Simulating altered netlist #" + number2String(radiationScenarioCounter) );
		// fgarcia, this reference can be a problem¿?
		// boost::thread radiation_t(boost::bind(&SpectreSimulation::HandleSpectreSimulation, boost::ref(sss)));
		tgScenarios.create_thread(boost::bind(&SpectreSimulation::HandleSpectreSimulation, sss));
		log_io->ReportThread( "Altered Scenario Simulation #" + number2String(radiationScenarioCounter), 1 );
		// Update counters
		++radiationScenarioCounter;
		++concurrentScenarioThreads;
	}
	//wait each thread
	tgScenarios.join_all();
	log_io->ReportInfo2AllLogs("[performance] Reliability Simulations ended: " + GetCurrentDateTime("%d-%m-%Y.%X"));
	log_io->ReportPlainStandard( "->All Spectre instances have ended." );
	log_io->ReportPlainStandard( "->Processing scenario statistics." );
	bool partialResult = ProcessScenarioStatistics();
	log_io->ReportInfo2AllLogs("[performance] ProcessScenarioStatistics ended: " + GetCurrentDateTime("%d-%m-%Y.%X"));
	log_io->ReportPlainStandard( kTab + "->All simulations have ended." );
	return partialResult;
}

bool RadiationSpectreHandler::SimulateStandardAHDLNetlist( ){
	// Radiation subcircuit AHDL netlist
	AHDLSimulation* radiation_AHDL_s = new AHDLSimulation();
	radiation_AHDL_s->set_simulation_id("ahdl_scenario");
	// not needed
	// radiation_AHDL_s->set_is_nested_simulation(false);
	radiation_AHDL_s->CopySimulationParameters( simulation_parameters );
	log_io->ReportPurpleStandard( "radiation_AHDL_s simulation_parameters: " + number2String(radiation_AHDL_s->get_simulation_parameters()->size()));
	radiation_AHDL_s->set_main_analysis( simulation_mode->get_analysis_statement() );
	radiation_AHDL_s->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	radiation_AHDL_s->set_log_io( log_io );
	// not needed
	// radiation_AHDL_s->set_plot_transients( false );
	// Not required cause magnitudes are not processed
	// golden_ss->set_golden_magnitudes( &magnitudes_2be_found );
	radiation_AHDL_s->set_top_folder( top_folder );
	radiation_AHDL_s->set_folder( radiation_subcircuit_AHDL_folder_path );
	radiation_AHDL_s->set_process_magnitudes( false );
	radiation_AHDL_s->set_export_magnitude_errors( false );
	// Spectre command and args
	radiation_AHDL_s->set_spectre_command( spectre_command );
	radiation_AHDL_s->set_pre_spectre_command( pre_spectre_command );
	radiation_AHDL_s->set_post_spectre_command( post_spectre_command );
	radiation_AHDL_s->set_spectre_command_log_arg( spectre_command_log_arg );
	radiation_AHDL_s->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	radiation_AHDL_s->set_ahdl_simdb_env( kEnableAHDLFolderSIMDB + ahdl_simdb_folder_path  );
	radiation_AHDL_s->set_ahdl_shipdb_env( kEnableAHDLFolderSHIPDB + ahdl_shipdb_folder_path  );
	log_io->ReportPurpleStandard( "Simulating radiation_AHDL netlist.");
	log_io->ReportThread( "AHDL scenario.", 1);
	boost::thread radiation_AHDL_t(boost::bind(&SpectreSimulation::HandleSpectreSimulation, radiation_AHDL_s));
	radiation_AHDL_t.join();
	log_io->ReportGreenStandard( "radiation_AHDL netlist simulated.");
	if(radiation_AHDL_s->get_simulation_results()->get_spectre_result() > 0){
		log_io->ReportError2AllLogs( "WARNING: while simulation the ahdl scenario." );
	}
	// Reorder magnitudes
	bool partialResult = ReorderMagnitudes( radiation_AHDL_s->GetSpectreResultsFilePath() );
	delete radiation_AHDL_s;
	log_io->ReportPurpleStandard( "radiation_AHDL_s deleted");
	return partialResult;
}

bool RadiationSpectreHandler::ReorderMagnitudes( const std::string& spectreResultTrans ){
	RAWFormatProcessor rfp;
	bool partialResult = rfp.PrepProcessTransientMagnitudes( &unsorted_magnitudes_2be_found, &magnitudes_2be_found, spectreResultTrans );
	// debug
	log_io->ReportCyanStandard( "Sorted Magnitudes to be found" );
	for( auto const& m : magnitudes_2be_found){
		log_io->ReportCyanStandard( m->get_name() );
	}
	// free memory
	deleteContentsOfVectorOfPointers( unsorted_magnitudes_2be_found );
	return partialResult;
}

bool RadiationSpectreHandler::SimulateGoldenAHDLNetlist( ){
	// Golden netlist
	GoldenSimulation* ahdl_golden_ss = new GoldenSimulation();
	ahdl_golden_ss->set_n_d_profile_index( 0 );
	ahdl_golden_ss->set_is_nested_simulation(false);
	ahdl_golden_ss->set_simulation_id("AHDL_golden_scenario");
	// simulation parameters
	ahdl_golden_ss->CopySimulationParameters( simulation_parameters );
	log_io->ReportPurpleStandard( "ahdl_golden_ss simulation_parameters: " + number2String(ahdl_golden_ss->get_simulation_parameters()->size()));
	ahdl_golden_ss->set_main_analysis( simulation_mode->get_analysis_statement() );
	ahdl_golden_ss->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	ahdl_golden_ss->set_log_io( log_io );
	ahdl_golden_ss->set_plot_transients( false );
	ahdl_golden_ss->set_plot_scatters( false );
	ahdl_golden_ss->set_top_folder( top_folder );
	ahdl_golden_ss->set_folder( golden_ahdl_scenario_folder_path );
	// result files
	ahdl_golden_ss->set_process_magnitudes( false );
	ahdl_golden_ss->set_export_magnitude_errors( false );
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

	// We copy the golden magnitudes, because ahdl_golden_ss object is going to be destroyed
	// analysis/radiation parameters are pointers, and they are not destroyed in the
	// ahdl_golden_ss radiation_AHDL_s destruction
	// Does not need to be copied because is not a GoldenSimulation member
	if( ahdl_golden_ss->get_simulation_results()->get_spectre_result() > 0 ){
		log_io->ReportError2AllLogs( "Error while simulating AHDL golden scenario. Aborted." );
		delete ahdl_golden_ss;
		return false;
	}
	//delete all simulations
	delete ahdl_golden_ss;
	log_io->ReportPurpleStandard( "ahdl_golden_ss deleted");
	return true;
}

bool RadiationSpectreHandler::SimulateGoldenNetlist( ){
	SpectreSimulation* golden_ss;
	if( simulation_mode->get_id()== kCriticalParameterNDParametersSweepMode
		|| simulation_mode->get_id()== kMontecarloCriticalParameterNDParametersSweepMode ){
		// We need for a first AHDL compilation for the golden
		if(!SimulateGoldenAHDLNetlist()){
			log_io->ReportError2AllLogs( "Error in AHDL Golden");
			return false;
		}
		// Golden netlist
		golden_ss = new GoldenNDParametersSweepSimulation();
		GoldenNDParametersSweepSimulation* pGPSS =  dynamic_cast<GoldenNDParametersSweepSimulation*>(golden_ss);
		pGPSS->set_magnitudes_2be_found( &magnitudes_2be_found );
		pGPSS->set_max_parallel_profile_instances( max_parallel_profile_instances );
	}else{
		// Golden netlist
		golden_ss = new GoldenSimulation();
		GoldenSimulation* pGS =  dynamic_cast<GoldenSimulation*>(golden_ss);
		pGS->set_magnitudes_2be_found( &magnitudes_2be_found );
	}
	// Golden netlist
	golden_ss->set_n_d_profile_index( 0 );
	golden_ss->set_is_nested_simulation(false);
	golden_ss->set_simulation_id("golden_scenario");
	// simulation parameters
	golden_ss->CopySimulationParameters( simulation_parameters );
	log_io->ReportPurpleStandard( "golden_ss simulation_parameters: " + number2String(golden_ss->get_simulation_parameters()->size()));
	golden_ss->set_main_analysis( simulation_mode->get_analysis_statement() );
	golden_ss->set_main_transient_analysis( simulation_mode->get_main_transient_analysis() );
	golden_ss->set_log_io( log_io );
	golden_ss->set_plot_scatters( plot_scatters );
	golden_ss->set_plot_transients( plot_transients );
	golden_ss->set_top_folder( top_folder );
	golden_ss->set_folder( golden_scenario_folder_path );
	// Files
	golden_ss->set_process_magnitudes( true );
	golden_ss->set_export_magnitude_errors( false );
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

	// We copy the golden magnitudes, because golden_ss object is going to be destroyed
	// analysis/radiation parameters are pointers, and they are not destroyed in the
	// golden_ss radiation_AHDL_s destruction
	// Does not need to be copied because is not a GoldenSimulation member
	if( simulation_mode->get_id()== kCriticalParameterNDParametersSweepMode
		|| simulation_mode->get_id()== kMontecarloCriticalParameterNDParametersSweepMode ){
		// Golden netlist
		GoldenNDParametersSweepSimulation* pGPSS =  dynamic_cast<GoldenNDParametersSweepSimulation*>(golden_ss);
		if( !pGPSS->get_children_correctly_simulated() || !pGPSS->get_children_correctly_processed() ){
			log_io->ReportError2AllLogs( "Error while simulating or processing the golden scenario. Aborted." );
			delete golden_ss;
			return false;
		}
		golden_magnitudes_structure = pGPSS->GetGoldenMagnitudes();
	}else{
		GoldenSimulation* pGS =  dynamic_cast<GoldenSimulation*>(golden_ss);
		if( pGS->get_simulation_results()->get_spectre_result() > 0
			|| !pGS->get_correctly_processed() ){
			log_io->ReportError2AllLogs( "Error while simulating or processing the golden scenario. Aborted." );
			delete golden_ss;
			return false;
		}
		golden_magnitudes_structure = pGS->GetGoldenMagnitudes();
	}
	//delete all simulations
	delete golden_ss;
	log_io->ReportPurpleStandard( "golden_ss deleted");
	return true;
}

void RadiationSpectreHandler::AddMagnitude( Magnitude* magnitude ){
	this->unsorted_magnitudes_2be_found.push_back( magnitude );
}
