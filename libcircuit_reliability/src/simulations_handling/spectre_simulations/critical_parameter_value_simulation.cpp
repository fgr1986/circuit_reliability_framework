 /**
 * @file critical_parameter_value_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * CriticalParameterValueSimulation Class Body
 *
 */

// Boost
#include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "critical_parameter_value_simulation.hpp"
// constants
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

CriticalParameterValueSimulation::CriticalParameterValueSimulation() {
	this->critical_parameter_value = kNotDefinedInt;
	this->not_affected_by_max_value= false;
	this->affected_by_min_value= false;
	this->is_montecarlo_nested_simulation = false;
	this->montecarlo_transient_sufix = kNotDefinedString;
	// injection mode related
	this->has_additional_injection = false;
}

CriticalParameterValueSimulation::~CriticalParameterValueSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "CriticalParameterValueSimulation destructor. direction:" << this << "\n";
	#endif
	deleteContentsOfVectorOfPointers( transient_simulations_results );
}

TransientSimulationResults* CriticalParameterValueSimulation::get_last_valid_transient_simulation_results(){
	for( auto it = transient_simulations_results.rbegin(); it != transient_simulations_results.rend(); ++it) {
		if( (*it)->get_spectre_result()==0 ){
			return (*it);
		}
	}
	// no valid lastValidResult found
	log_io->ReportError2AllLogs( "lastValidResult is nullptr. ");
	return nullptr;
}

void CriticalParameterValueSimulation::RunSimulation(){
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	// set up local_critical_parameter
	for( auto const & p : *simulation_parameters ){
		if( p->get_name()==golden_critical_parameter->get_name() ){
			local_critical_parameter = p;
			break;
		}
	}
	if(local_critical_parameter==nullptr){
		log_io->ReportError2AllLogs( "local_critical_parameter is nullptr. ");
		return;
	}

	// parameter variations count
	int parameterVariationCount;
	// current parameter value
	double currentCriticalParameterValue;
	std::string s_currentCriticalParameterValue;
	// Environment
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	// only for nested simulations
	TransientSimulationResults* lastResults;
	parameterVariationCount = 0;
	// Test max value
	TransientSimulationResults* maxValueResults = new TransientSimulationResults();
	lastResults = maxValueResults;
	currentCriticalParameterValue = golden_critical_parameter->get_value_max();
	critical_parameter_values_evolution.push_back( currentCriticalParameterValue );
	// Register Parameters
	maxValueResults->RegisterSimulationParameters(simulation_parameters);
	// simulation_id is already "s_x_prof"
	maxValueResults->set_full_id( simulation_id + "_pvc_" + number2String(parameterVariationCount) + "_mc_NaN" );
	maxValueResults->set_title( maxValueResults->get_full_id() );
	// id and folder of the active simulation in the loop
	std::string localSimulationId;
	std::string localSimulationFolder;
	if( !SimulateParameterCriticalValue( *maxValueResults, parameterVariationCount,
		currentCriticalParameterValue, localSimulationFolder, localSimulationId) ){
		if( maxValueResults->get_spectre_result()<=0 ){ //not a spectre problem
			correctly_simulated = maxValueResults->get_spectre_result()==0;
			correctly_processed = false;
			return;
		}else{
			#ifdef SPECTRE_SIMULATIONS_VERBOSE
			log_io->ReportRedStandard("ommiting spectre error in max qcoll " + simulation_id);
			#endif
		}
	}
	transient_simulations_results.push_back( maxValueResults );
	if( maxValueResults->get_reliability_result()==kScenarioNotSensitive ){
		critical_parameter_value = currentCriticalParameterValue;
		not_affected_by_max_value = true;
		// this already plots and deletes (if necesary the files)
		ReportEndOfCriticalParameterValueSimulation( localSimulationFolder, localSimulationId, *lastResults, NOT_SENSITIVE_2_MAX );
		return;
	}
	// handle scenarios (plotting and files deleting)
	HandleIntermediatePlotAndRawResults( *maxValueResults, localSimulationId );
	parameterVariationCount++;
	// Test min value
	TransientSimulationResults* minValueResults = new TransientSimulationResults();
	lastResults = minValueResults;
	currentCriticalParameterValue = golden_critical_parameter->get_value_min();
	critical_parameter_values_evolution.push_back( currentCriticalParameterValue );
	// Register Parameters
	minValueResults->RegisterSimulationParameters(simulation_parameters);
	minValueResults->set_full_id( simulation_id + "_pvc_" + number2String(parameterVariationCount) + "_mc_NaN" );
	minValueResults->set_title( maxValueResults->get_full_id() );
	if( !SimulateParameterCriticalValue( *minValueResults, parameterVariationCount,
		currentCriticalParameterValue, localSimulationFolder, localSimulationId ) ){
		correctly_simulated = minValueResults->get_spectre_result()==0;
		correctly_processed = false;
		return;
	}
	transient_simulations_results.push_back( minValueResults );
	if( minValueResults->get_reliability_result()==kScenarioSensitive ){
		critical_parameter_value = currentCriticalParameterValue;
		affected_by_min_value = true;
		// this already plots and deletes (if necesary the files)
		ReportEndOfCriticalParameterValueSimulation( localSimulationFolder, localSimulationId, *lastResults, SENSITIVE_2_MIN );
		return;
	}
	// handle scenarios (plotting and files deleting)
	HandleIntermediatePlotAndRawResults( *minValueResults, localSimulationId );
	parameterVariationCount++;

	// next parameter value
	double nextCriticalParameterValue;
	// max-min parameter values simulated
	double maxParameterValueWorking = golden_critical_parameter->get_value_min();
	double minParameterValueNotWorking = golden_critical_parameter->get_value_max();
	// current parameter
	switch( golden_critical_parameter->get_value_change_mode() ){
		case kSPLineal:{
			currentCriticalParameterValue = (maxParameterValueWorking+minParameterValueNotWorking) / 2;
		}
		break;
		case kSPLogSmaller:{
			currentCriticalParameterValue = std::sqrt(maxParameterValueWorking*minParameterValueNotWorking);
		}
		break;
		default:{
			currentCriticalParameterValue = (maxParameterValueWorking+minParameterValueNotWorking) / 2;
		}
		break;
	}
	// stop margin
	double stopMargin = 1 + golden_critical_parameter->get_value_stop_percentage() / 100;
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		if(!is_nested_simulation ){
			log_io->ReportPlainStandard( k2Tab + "#" + simulation_id
				+ " scenario, parameter value change mode: "  + number2String(golden_critical_parameter->get_value_change_mode()));
		}
	#endif
	// Find critical value
	// end of sweep condition
	bool endOfSweep = false;
	TransientSimulationResults* previousResults = minValueResults;
	std::string previousLocalSimulationId = localSimulationId;
	double previousCriticalParameterValue = currentCriticalParameterValue;
	while( !endOfSweep ){
		critical_parameter_values_evolution.push_back( currentCriticalParameterValue );
		TransientSimulationResults* variationResults = new TransientSimulationResults();
		variationResults->set_full_id( simulation_id + "_pvc_" + number2String(parameterVariationCount) + "_mc_NaN" );
		variationResults->set_title( maxValueResults->get_full_id() );
		// Register Parameters
		variationResults->RegisterSimulationParameters( simulation_parameters );
		// simulate
		if( !SimulateParameterCriticalValue( *variationResults, parameterVariationCount,
			currentCriticalParameterValue, localSimulationFolder, localSimulationId ) ){
			if( variationResults->get_spectre_result()>0 ){ // spectre error, but previous value
				critical_parameter_value = previousCriticalParameterValue;
				ReportEndOfCriticalParameterValueSimulation( localSimulationFolder, previousLocalSimulationId, *previousResults, SENSITIVE_2_OTHER );
				return;
			}
		}
		// Parameter control
		if( variationResults->get_reliability_result()==kScenarioSensitive ){
			minParameterValueNotWorking = currentCriticalParameterValue;
		}else{
			maxParameterValueWorking = currentCriticalParameterValue;
		}
		// Update next parameter value
		switch( golden_critical_parameter->get_value_change_mode() ){
			case kSPLineal:{
				nextCriticalParameterValue = (maxParameterValueWorking+minParameterValueNotWorking) / 2;
			}
			break;
			case kSPLogSmaller:{
				nextCriticalParameterValue = std::sqrt(maxParameterValueWorking*minParameterValueNotWorking);
			}
			break;
			default:{
				nextCriticalParameterValue = (maxParameterValueWorking+minParameterValueNotWorking) / 2;
			}
			break;
		}
		// Update end condition
		if( minParameterValueNotWorking/maxParameterValueWorking < stopMargin ){
			endOfSweep = true;
			lastResults = variationResults;
			if(!is_nested_simulation){
				#ifdef SPECTRE_SIMULATIONS_VERBOSE
					log_io->ReportPlainStandard( "#"
						+ simulation_id + " scenario. maxW: "
						+ number2String(maxParameterValueWorking)
						+" minNotW: " + number2String(minParameterValueNotWorking)
						+" minNotW/maxW: " + number2String(maxParameterValueWorking/minParameterValueNotWorking)
						+" stopMargin (1+X%): " + number2String(stopMargin));
				#endif
			}
			critical_parameter_value = currentCriticalParameterValue;
		}else{
			HandleIntermediatePlotAndRawResults( *variationResults, localSimulationId );
		}
		// update remaining variables
		previousCriticalParameterValue = currentCriticalParameterValue;
		previousResults = variationResults;
		previousLocalSimulationId = localSimulationId;
		currentCriticalParameterValue = nextCriticalParameterValue;
		parameterVariationCount++;
		transient_simulations_results.push_back( variationResults );
	}
	ReportEndOfCriticalParameterValueSimulation( localSimulationFolder, localSimulationId, *lastResults, SENSITIVE_2_OTHER );
}

void CriticalParameterValueSimulation::HandleIntermediatePlotAndRawResults(
	TransientSimulationResults& simulationResults,
	const std::string localSimulationId ){
	// plot previous transient, if needed
	if( !PlotTransient( localSimulationId, simulationResults, false ) ){
		log_io->ReportError2AllLogs( "Error while plotting transients. Scenario #"	+ localSimulationId );
		// return false; // program can continue
	}
	// delete previous transients, if needed
	if( !ManageIndividualResultFiles( simulationResults, false ) ){
		log_io->ReportError2AllLogs( "Error deleting raw data. Scenario #" + localSimulationId );
		// return false; // program can continue
	}
}

void CriticalParameterValueSimulation::ReportEndOfCriticalParameterValueSimulation(
	std::string localSimulationFolder, std::string localSimulationId,
	TransientSimulationResults& lastResults, const int endType ){
	int transGnuplot;
	// log_io report
	#ifdef RESULTS_ANALYSIS_VERBOSE
	switch( endType ){
		case CriticalParameterValueSimulation::NOT_SENSITIVE_2_MAX:{
			log_io->ReportGreenStandard( kTab + "#"
				+ simulation_id + " scenario is not sensitive to the maximum quantity:"
				+ number2String(critical_parameter_value));
		}break;
		case CriticalParameterValueSimulation::SENSITIVE_2_MIN:{
			log_io->ReportRedStandard( kTab + "#"
				+ simulation_id + " scenario is sensitive to the minimum quantity:"
				+ number2String(critical_parameter_value));
		}break;
		case CriticalParameterValueSimulation::SENSITIVE_2_OTHER:{
			log_io->ReportPlainStandard( kTab + "#"
				+ simulation_id + " scenario is sensitive to the critical quantity:"
				+ number2String(critical_parameter_value));
		}break;
		default:{} break;
	}
	#endif
	// Plot critical parameter evolution
	if( endType==SENSITIVE_2_OTHER ){
		if( (transGnuplot=CreateGnuplotCriticalParameterEvolution( )) > 0 ){
			// log_io report
			#ifdef RESULTS_ANALYSIS_VERBOSE
			log_io->ReportError2AllLogs( "Unexpected gnuplot result: " + number2String(transGnuplot) );
			#endif
		}
	}
	// Plot transients
	if( plot_last_transients && (&lastResults)!=nullptr ){
		// transients in nested simulation
		transGnuplot = CreateGnuplotTransientImages( localSimulationId,
			lastResults, n_d_profile_index, true, "last_tran" );
		// log_io report
		#ifdef RESULTS_ANALYSIS_VERBOSE
		if( transGnuplot > 0 ){
			log_io->ReportError2AllLogs( "Unexpected gnuplot result: " + number2String(transGnuplot) );
		}
		#endif
	}
	correctly_simulated = lastResults.get_spectre_result() == 0;
	correctly_processed = true;
	// delete previous transients, if needed
	if(!is_montecarlo_nested_simulation){
		if( !ManageIndividualResultFiles( lastResults, false ) ){
			log_io->ReportError2AllLogs( "Error deleting raw data. Last results of Scenario #" + simulation_id );
			// return false; // program can continue
		}
	}
	// if is_montecarlo_nested_simulation, handled in parent
}

bool CriticalParameterValueSimulation::SimulateParameterCriticalValue(
		TransientSimulationResults& simulationResults,
		int parameterVariationCount, double currentCriticalParameterValue,
		std::string& localSimulationFolder, std::string& localSimulationId ){
	// Current simulation folder
	localSimulationFolder = folder;
	std::string s_parameterVariationCount = number2String( parameterVariationCount );
	std::string s_currentCriticalParameterValue = number2String( currentCriticalParameterValue );
	// copy only files to folder
	// find . -maxdepth 1 -type f -exec cp {} destination_path \;
	std::string copyNetlists0 = "find ";
	std::string copyNetlists1 = " -maxdepth 1 -type f -exec cp {} ";
	std::string copyNetlists2 = " \\;";
	std::string copyNetlists ;
	// fgarcia
	// Create the folder structure
	localSimulationFolder = folder + kFolderSeparator + kIntermediateSimulationsFolder + "_pvc_" + s_parameterVariationCount;
	if( !CreateFolder(localSimulationFolder, true ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folder '" + localSimulationFolder + "'." );
		return false;
	}
	copyNetlists = copyNetlists0 + folder + copyNetlists1 + localSimulationFolder + copyNetlists2;
	if( std::system( copyNetlists.c_str() ) > 0){
		log_io->ReportError2AllLogs( k2Tab + "-> Error while copying netlist to '" + localSimulationFolder + "'." );
		return false;
	}
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario, currentCriticalParameterValue:" + s_currentCriticalParameterValue);
	#endif
	// Changing critical parameter value
	local_critical_parameter->set_value( number2String(currentCriticalParameterValue) );
	// Export current set of parameters
	if( !ExportParametersCircuit( localSimulationFolder, parameterVariationCount )){
		log_io->ReportError2AllLogs( "Error creating parameters Circuit ");
		return false;
	}
	localSimulationId = simulation_id + "_pvc_" + s_parameterVariationCount;
	// Run Spectre
	simulationResults.set_spectre_result( RunSpectre( localSimulationFolder, localSimulationId, s_parameterVariationCount ));
	if( simulationResults.get_spectre_result() > 0 ){
		log_io->ReportError2AllLogs( "Spectre ended with an unexpected value: "
			+ number2String(simulationResults.get_spectre_result()) + ". Scenario #"  + localSimulationId );
		return false;
	}
	// Set up metrics
	std::vector<Metric*>* analyzedMetrics = CreateMetricsVectorFromGoldenMetrics( n_d_profile_index );
	// Reading metrics
	if( !ProcessSpectreResults( localSimulationFolder, localSimulationId, simulationResults, false, *analyzedMetrics, false, is_montecarlo_nested_simulation ) ){
		log_io->ReportError2AllLogs( "Error while processing the critical value simulation spectre_results. Scenario #"
			+ simulation_id );
		return false;
	}
	// Interpolating and analyzing metrics
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportPlain2Log( kTab + "#" + simulation_id + " -> Interpolating spectre_results");
	#endif
	// Interpolate results
	if( !InterpolateAndAnalyzeMetrics( simulationResults, *analyzedMetrics, n_d_profile_index, localSimulationId ) ){
		log_io->ReportError2AllLogs( "Error while interpolating the critical value metrics. Scenario #" + simulation_id );
		return false;
	}
	// delete analyzed metrics
	deleteContentsOfVectorOfPointers( *analyzedMetrics);
	delete analyzedMetrics;
	return true;
}

bool CriticalParameterValueSimulation::TestSetUp(){
	if( golden_critical_parameter==nullptr
		|| simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr parameter_critical_value");
		return false;
	}else if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder ");
		return false;
	}else if( golden_metrics_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_metrics_structure");
		return false;
	}
	return true;
}


std::string CriticalParameterValueSimulation::GetSpectreResultsFilePath(const std::string& currentFolder,
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

std::string CriticalParameterValueSimulation::GetProcessedResultsFilePath(const std::string& currentFolder,
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

int CriticalParameterValueSimulation::RunSpectre(
	std::string currentFolder, std::string localSimulationId, std::string parameterChangeCount ){
	std::string execCommand = spectre_command + " "
		+ spectre_command_log_arg + " " + currentFolder + kFolderSeparator + kSpectreLogFile + " "
		+ spectre_command_folder_arg + " " + currentFolder + kFolderSeparator + kSpectreResultsFolder + " "
		+ currentFolder + kFolderSeparator + kMainNetlistFile
		+ " " + post_spectre_command + " " + currentFolder + kFolderSeparator + kSpectreStandardLogsFile ;
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportPlainStandard( k2Tab + "#" + localSimulationId + " scenario: Simulating scenario."
		" Parameter change #" + parameterChangeCount );
	log_io->ReportPlain2Log( k2Tab + "#" + localSimulationId + " scenario: Simulating scenario."
		" Parameter change #" + parameterChangeCount + "exec:'" + execCommand + "'" );
	#endif
	int spectre_result = std::system( execCommand.c_str() ) ;
	if(spectre_result>0){
		correctly_simulated = false;
		log_io->ReportError2AllLogs( "Unexpected Spectre spectre_result for scenario #"
			+ localSimulationId + ": spectre output = " + number2String(spectre_result) );
		log_io->ReportError2AllLogs( "Spectre Log Folder " + currentFolder );
		return spectre_result;
	}
	correctly_simulated = true;
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportGreenStandard( k2Tab + "#" + localSimulationId + " scenario: ENDED."
		" Parameter change #" + parameterChangeCount + ", spectre_result=" + number2String(spectre_result) );
	#endif
	return spectre_result;
}

int CriticalParameterValueSimulation::CreateGnuplotCriticalParameterEvolution(){

	std::string s_param_profile_suf = is_nested_simulation ?  "_param_prof_" + number2String(n_d_profile_index) : "";

	std::string gnuplotScriptFilePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator + kCriticalParameterEvolutionFolder + kFolderSeparator
		+ "critical_param_evolution_scenario_" + simulation_id
		+ s_param_profile_suf + "_" + kGnuPlotScriptSufix;

	std::string outputImagePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator
		+ kCriticalParameterEvolutionFolder + kFolderSeparator
		+ "critical_param_evolution_scenario_" + simulation_id
		+ s_param_profile_suf + kGnuplotTransientSVGSufix;

	// static data
	std::ofstream critParameterEvolutionDataFile;
	std::string critParameterEvolutionDataPath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator
		+ kCriticalParameterEvolutionFolder + kFolderSeparator
		+ "critical_param_evolution_scenario_" + simulation_id
		+ s_param_profile_suf;
	// static script
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set grid\n";
	// Axis
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";

	// # remove border on top and right and set color to gray
	gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1\n";
	gnuplotScriptFile << "set border 3 back ls 11\n";
	gnuplotScriptFile << "set tics nomirror\n";
	// line style
	gnuplotScriptFile <<  "set style line 1 lc rgb '#8b1a0e' pt 6 ps 1 lt 1 lw 2 # --- red\n";
	// Create file
	critParameterEvolutionDataFile.open( critParameterEvolutionDataPath.c_str() );
	for( auto const & cv : critical_parameter_values_evolution ){
		critParameterEvolutionDataFile << cv << "\n";
	}
	critParameterEvolutionDataFile.close();
	// plots
	// Svg
	gnuplotScriptFile <<  "set term svg noenhanced size "<< kSvgImageWidth << ","
		<< kSvgImageHeight << " fname " << kSvgFont << "\n";
	gnuplotScriptFile <<  "set output \"" << outputImagePath  << "\"\n";

	// critical parameter
	gnuplotScriptFile <<  "set xlabel \"Iteration\"\n";
	gnuplotScriptFile <<  "set ylabel \"Critical Parameter Value\"\n";
	gnuplotScriptFile <<  "set title \"Critical Parameter Value Evolution \"\n";
	gnuplotScriptFile <<  "set key bottom right\n";
	if( golden_critical_parameter->get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale y\n";
	}
	// Plot
	gnuplotScriptFile <<  "plot '" << critParameterEvolutionDataPath
		<< "' w lp ls 1 title 'Critical Parameter Value Evolution' \n";
	gnuplotScriptFile << "unset output\n";
	// Add images path
	critical_parameter_values_evolution_image_path = outputImagePath;

	gnuplotScriptFile << "\nquit\n";
	gnuplotScriptFile.close();

	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	return std::system( execCommand.c_str() );
}
