 /**
 * @file simulation_modes_handler.cpp
 *
 * @date Created on: Jan 24, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of SimulationModesHandler Class (see simulation_modes_handler.hpp)
 *
 */

// c++ include
#include <iostream>
// Radiation simulator include
#include "simulation_modes_handler.hpp"
// Constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"

SimulationModesHandler::SimulationModesHandler() {
	// Simulation modes
	this->selected_mode = nullptr;
}

SimulationModesHandler::~SimulationModesHandler() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlain2Log("SimulationModesHandler destructor.Direction: " + number2String(this) );
	#endif
}

void SimulationModesHandler::CreateRadiationProgramModes(){
	log_io->ReportPlain2Log( "Creating Simulation and Radiation Modes" );
	// Simulation modes
	// standard mode
	standard_simulation_mode.set_log_io( log_io );
	standard_simulation_mode.set_id( kStandardMode );
	standard_simulation_mode.set_description( "Standard Mode Simulation: Sensitive nodes detection using default radiation parameters." );
	standard_simulation_mode.set_short_description( "Standard Mode Simulation." );
	// critical parameter value mode
	critical_value_mode.set_log_io( log_io );
	critical_value_mode.set_id( kCriticalValueMode );
	critical_value_mode.set_description( "Critical Parameter Value Simulation Mode. Finds the critical value of a given parameter under certain conditions." );
	critical_value_mode.set_short_description( "Critical Parameter Value Simulation Mode." );
	// n-D parameter value mode
	critical_parameter_nd_parameters_sweep_mode.set_log_io( log_io );
	critical_parameter_nd_parameters_sweep_mode.set_id( kCriticalParameterNDParametersSweepMode );
	critical_parameter_nd_parameters_sweep_mode.set_short_description( "N-Dimensional Parameter Sweep [find critical parameter with every non-fixed parameter] Simulation Mode." );
	critical_parameter_nd_parameters_sweep_mode.set_description( "Parameter Sweep Simulation Mode. Sweeps the chosen parameter, finding the critical parameter value at each parameter value step"
		", running the list of analyses (or multiple analyses) for each value of each paired-parameter." );
	// montecarlo_critical_parameter_nd_parameters_sweep_mode
	montecarlo_critical_parameter_nd_parameters_sweep_mode.set_log_io( log_io );
	montecarlo_critical_parameter_nd_parameters_sweep_mode.set_id( kMontecarloCriticalParameterNDParametersSweepMode );
	montecarlo_critical_parameter_nd_parameters_sweep_mode.set_short_description(
		"Monte Carlo Critical Parameter, ND Parameter Sweep Simulation Mode.");
	montecarlo_critical_parameter_nd_parameters_sweep_mode.set_description(
		"Monte Critical Parameter, ND Parameter Sweep Simulation Mode. Sweeps the pair [critical parameter]-[chosen parameter]"
		", running the list of analyses (or multiple analyses) for each value of each paired-parameter."
		"Each sweep takes into account parameter variations performing several Monte Carlo Analysis" );

	// radiation modes
	// Injection radiation mode
	injection_alteration_mode.set_log_io( log_io );
	injection_alteration_mode.set_name( "injection_mode" );
	injection_alteration_mode.set_injection_mode( true );
	injection_alteration_mode.set_perform_alteration( true );
	// Replacement radiation mode
	replacement_alteration_mode.set_log_io( log_io );
	replacement_alteration_mode.set_name( "replacement_mode" );
	replacement_alteration_mode.set_injection_mode( false );
	replacement_alteration_mode.set_perform_alteration( true );
}

bool SimulationModesHandler::RadiationSimulationModeSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes ){
	bool modeSelected = false;
	std::string sBuffer;
	log_io->ReportStage( "Simulation mode selection: available modes ", false );
	// Select mode
	while( !modeSelected ) {
		log_io->ReportUserInformationNeeded( k2Tab + "* Type 'quit' or 'exit' for exit" );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'a' " + standard_simulation_mode.get_description() );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'b' " + critical_value_mode.get_description() );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'c' " + critical_parameter_nd_parameters_sweep_mode.get_description() );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'd' " + montecarlo_critical_parameter_nd_parameters_sweep_mode.get_description() );
		// Capture the characters inserted by the user
		if( getline( std::cin, sBuffer )){
			if (sBuffer.empty()) {
				log_io->ReportStage( "Not a valid selection please, select the radiation mode scheme", true );
				continue;
			}
			if( sBuffer.compare("a")==0  ){
				selected_mode = &standard_simulation_mode;
				modeSelected = true;
			}else if( sBuffer.compare("b")==0 ) {
				selected_mode = &critical_value_mode;
				modeSelected = true;
			}else if( sBuffer.compare("c")==0 ) {
				selected_mode = &critical_parameter_nd_parameters_sweep_mode;
				modeSelected = true;
			}else if( sBuffer.compare("d")==0 ) {
				selected_mode = &montecarlo_critical_parameter_nd_parameters_sweep_mode;
				modeSelected = true;
			}else if( sBuffer.compare("quit")==0 || sBuffer.compare("exit")==0 ) {
				log_io->ReportRedStandard( "Canceling and exiting...");
				return false;
			}else{
				log_io->ReportStage( "Not a valid selection please, select the injection mode scheme", true );
			}
		}
		std::cin.clear();
	}
	modeSelected = modeSelected && RadiationAlterationModeSelection( unalterable_statements, unalterable_nodes);
	modeSelected = modeSelected && ExportRadiationAnalysisSelection( unalterable_statements, unalterable_nodes);
	return modeSelected;
}

bool SimulationModesHandler::RadiationAlterationModeSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes ){
	//mode selection
	std::cin.clear();
	log_io->ReportStage( "Radiation mode selection: please, select the radiation scheme ", false );
	bool modeSelected = false;
	bool injectionSchemeExist = injection_alteration_mode.get_available_radiation_sources()->size() > 0;
	bool substitutionSchemeExist = replacement_alteration_mode.get_available_radiation_sources()->size() > 0;
	std::string selection;
	unsigned int iSelection;
	std::string sBuffer;
	int rsCount;
	std::string auxMessage;
	while( !modeSelected ) {
		rsCount = 1;
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'c' for cancel" );
		// Injection scheme
		if( injectionSchemeExist ){
			log_io->ReportUserInformationNeeded( k2Tab + "* Injection schemes, select the radiation source number:" );
			for( auto const & ars : *injection_alteration_mode.get_available_radiation_sources() ){
					log_io->ReportUserInformationNeeded( k3Tab + "* '" + number2String( rsCount )
						+ "') Radiation source: '" + ars->get_name() + "'" );
					rsCount++;
			}
		}
		if( substitutionSchemeExist ){
			log_io->ReportUserInformationNeeded( k2Tab
				+ "*  Press's' for substitution scheme. The following sources substitutions are available" );
			for( auto const & ars : *replacement_alteration_mode.get_available_radiation_sources() ){
					log_io->ReportUserInformationNeeded( k3Tab + "* Altered subcircuit: '" + ars->get_name() + "'" );
					rsCount++;
			}
		}
		if( getline( std::cin, sBuffer )){
			if (sBuffer.empty()) {
				log_io->ReportStage( "Not a valid selection please, select the radiation mode scheme", true );
				continue;
			}
			iSelection = std::atoi( sBuffer.c_str() );
			if( injectionSchemeExist && iSelection>=1
				&& iSelection <= injection_alteration_mode.get_available_radiation_sources()->size() ){
				selected_mode->set_alteration_mode( &injection_alteration_mode );
				selected_mode->SelectRadiationSource( iSelection-1 );
				modeSelected = true;
				log_io->ReportPurpleStandard( k2Tab + "->Injection scheme '" + sBuffer.c_str() + "' selected " );
			}else if( substitutionSchemeExist && sBuffer.compare("s")==0 ) {
				selected_mode->set_alteration_mode( &replacement_alteration_mode );
				log_io->ReportPurpleStandard( k2Tab + "Substitution scheme selected." );
				modeSelected = true;
			}else if( sBuffer.compare("c")==0 ) {
				return RadiationSimulationModeSelection( unalterable_statements, unalterable_nodes );
			}else{
				log_io->ReportStage( "Not a valid selection please, select the radiation mode scheme", true );
			}
		}
		std::cin.clear() ;
	}
	// if( (selected_mode == &critical_value_mode
	// 	|| selected_mode == &critical_parameter_nd_parameters_sweep_mode
	// 	|| selected_mode == &montecarlo_critical_parameter_nd_parameters_sweep_mode )
	// 	&& selected_mode->get_alteration_mode()->get_critical_parameter()==nullptr ){
	// 	log_io->ReportRedStandard(
	// 		"The selected mode does not have the required critical parameter defined. Please, verify it or select another mode.");
	// 	return RadiationSimulationModeSelection( unalterable_statements, unalterable_nodes );
	// }
	if( selected_mode->get_alteration_mode() == &injection_alteration_mode ){
		modeSelected = modeSelected	&& RadiationInjectionModeSelection( unalterable_statements, unalterable_nodes);
	}
	return modeSelected;
}

bool SimulationModesHandler::RadiationInjectionModeSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes ){
	//mode selection
	log_io->ReportStage( "Injection mode selection", false );
	bool modeSelected = false;
	std::string selection;
	std::string sBuffer;
	std::string auxMessage;
	while( !modeSelected ) {
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'c' for cancel" );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'a' if you want to inject every instance statement");
		log_io->ReportUserInformationNeeded( k3Tab + " BUT the unalterable statements (declared in experiment_conf.xml) and its children:" );
		std::string unalterableStatements = "";
		bool firstUnalterable = true;
		for( auto const & us : *unalterable_statements ){
			if(firstUnalterable){
				firstUnalterable = false;
				unalterableStatements += us;
			}else{
				unalterableStatements += ", " + us;
			}
		}
		log_io->ReportPurpleStandard( k4Tab + "-> " + unalterableStatements );
		std::string unalterableSNodes = "";
		firstUnalterable = true;
		log_io->ReportUserInformationNeeded( k3Tab + " Neither the following nodes will be injected:" );
		for( auto const &un : *unalterable_nodes ){
			if(firstUnalterable){
				firstUnalterable = false;
				unalterableSNodes += un;
			}else{
				unalterableSNodes += ", " + un;
			}
		}
		log_io->ReportPurpleStandard( k4Tab + "-> " + unalterableSNodes );

		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'b' if you want to inject only the following statements (extracted from experiment_conf.xml):" );
		for( auto const &as : *(injection_alteration_mode.get_alterable_statements()) ) {
			log_io->ReportPurpleStandard( k3Tab + "-> " + as.first + " " + as.second );
		}
		log_io->ReportUserInformationNeeded( k3Tab + " BUT the unalterable statements (declared in experiment_conf.xml) and its children:" );
		log_io->ReportPurpleStandard( k4Tab + "-> " + unalterableStatements );
		log_io->ReportUserInformationNeeded( k3Tab + " Neither the following nodes will be injected:" );
		log_io->ReportPurpleStandard( k4Tab + "-> " + unalterableSNodes );
		// Capture the characters inserted by the user
		if( getline( std::cin, sBuffer )){
			if (sBuffer.empty()) {
				log_io->ReportStage( "Not a valid selection please, select the radiation mode scheme", true );
				continue;
			}
			if( sBuffer.compare("a")==0  ){
				injection_alteration_mode.set_inject_all_mode( true );
				modeSelected = true;
				log_io->ReportPurpleStandard( k2Tab + "->Inject all perceptible statements scheme selected " );
			}else if( sBuffer.compare("b")==0 ) {
				injection_alteration_mode.set_inject_all_mode( false );
				modeSelected = true;
				log_io->ReportPurpleStandard( k2Tab + "->Inject only pre-defined statements scheme selected " );
			}else if( sBuffer.compare("c")==0 ) {
				return RadiationSimulationModeSelection( unalterable_statements, unalterable_nodes );
			}else{
				log_io->ReportStage( "Not a valid selection please, select the injection mode scheme", true );
			}
		}
		std::cin.clear() ;
	}
	return modeSelected;
}

bool SimulationModesHandler::ExportRadiationAnalysisSelection(
	std::vector<std::string>* unalterable_statements, std::vector<std::string>* unalterable_nodes ){
	//mode selection
	log_io->ReportStage( "Select if the analysis present in the netlists should be preserved.", false );
	bool modeSelected = false;
	std::string selection;
	std::string sBuffer;
	std::string auxMessage;
	while( !modeSelected ) {
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'c' for cancel" );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'a' to simulate only the selected radiation analysis:" );
		log_io->ReportUserInformationNeeded( k3Tab + "(" + selected_mode->get_description() + ")" );
		log_io->ReportUserInformationNeeded( k2Tab + "* Press 'b' to preserve the analysis present in the netlists." );
		log_io->ReportUserInformationNeeded( k3Tab + "[Note] This selection can affect the viability of the simulation." );
		// Capture the characters inserted by the user
		if( getline( std::cin, sBuffer )){
			if (sBuffer.empty()) {
				log_io->ReportStage( "Not a valid selection please, select the radiation mode scheme", true );
				continue;
			}
			if( sBuffer.compare("a")==0  ){
				export_other_analysis = false;
				modeSelected = true;
				log_io->ReportPurpleStandard( k2Tab + "Simulate only the radiation analysis." );
			}else if( sBuffer.compare("b")==0 ) {
				export_other_analysis = true;
				modeSelected = true;
				log_io->ReportPurpleStandard( k2Tab + "Preserve the analysis present in the netlists." );
			}else if( sBuffer.compare("c")==0 ) {
				return RadiationSimulationModeSelection( unalterable_statements, unalterable_nodes);
			}else{
				log_io->ReportStage( "Not a valid selection please, select the injection mode scheme", true );
			}
		}
		std::cin.clear() ;
	}
	return modeSelected;
}

void SimulationModesHandler::CreateVariabilityProgramModes(){
	log_io->ReportPlain2Log( "Creating Simulation Modes" );
	// Simulation modes
	/// montecarlo_critical_parameter_nd_parameters_sweep_mode
	montecarlo_nd_parameters_sweep_mode.set_log_io( log_io );
	montecarlo_nd_parameters_sweep_mode.set_id( kMontecarloNDParametersSweepMode );
	montecarlo_nd_parameters_sweep_mode.set_short_description( "Monte Carlo-Sweep Simulation Mode.");
	montecarlo_nd_parameters_sweep_mode.set_description( "Monte Carlo-Sweep Simulation Mode. Sweeps the chosen parameters"
		", running the list of analyses (or multiple analyses) for each value of each paired-parameter."
		"Each sweep takes into account parameter variations performing several Monte Carlo Analysis" );

	// radiation modes
	// Non alteration mode
	non_alteration_mode.set_log_io( log_io );
	non_alteration_mode.set_name( "non_alteration_mode" );
	non_alteration_mode.set_injection_mode( false );
	non_alteration_mode.set_perform_alteration( false );

}

bool SimulationModesHandler::VariabilitySimulationModeSelection(){

	selected_mode = &montecarlo_nd_parameters_sweep_mode;
	selected_mode->set_alteration_mode( &non_alteration_mode );
	return true;
}
