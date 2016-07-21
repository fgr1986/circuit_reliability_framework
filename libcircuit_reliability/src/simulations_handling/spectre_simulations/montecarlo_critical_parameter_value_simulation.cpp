 /**
 * @file standard_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * MontecarloCriticalParameterValueSimulation Class Body
 *
 */

#include <cmath>
// Radiation simulator
#include "montecarlo_critical_parameter_value_simulation.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

MontecarloCriticalParameterValueSimulation::MontecarloCriticalParameterValueSimulation() {
	this->export_processed_metrics = true;
	this->max_parallel_montecarlo_instances = 5;
	// injection mode related
	this->has_additional_injection = false;
}

MontecarloCriticalParameterValueSimulation::~MontecarloCriticalParameterValueSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "MontecarloCriticalParameterValueSimulation destructor. direction:" << this << "\n";
	#endif
}

bool MontecarloCriticalParameterValueSimulation::TestSetUp(){
	if( golden_critical_parameter==nullptr ){
		log_io->ReportError2AllLogs( "nullptr golden_critical_parameter in montecarlo_critical_parameter_simulation");
		return false;
	}
	if( simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr simulation_parameters in montecarlo_critical_parameter_simulation");
		return false;
	}else if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder  in montecarlo_critical_parameter_simulation");
		return false;
	}else if( golden_metrics_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_metrics_structure in montecarlo_critical_parameter_simulation");
		return false;
	}
	return true;
}

void MontecarloCriticalParameterValueSimulation::RunSimulation( ){
	// standard comprobations
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr. ");
		return;
	}
	boost::thread_group mainTG;
	// Register Parameters
	montecarlo_simulation_results.set_full_id( simulation_id );
	montecarlo_simulation_results.RegisterSimulationParameters(simulation_parameters);
	// parallel threads control
	unsigned int runningThreads = 0;
	unsigned int threadsCount = 0;
	critical_parameter_value_simulations_vector.set_group_name("critical_parameter_value_simulations_vector");
	critical_parameter_value_simulations_vector.ReserveSimulationsInMemory( montecarlo_iterations );
	while( threadsCount<montecarlo_iterations ){
		// wait for resources
		WaitForResources( runningThreads, max_parallel_montecarlo_instances, mainTG, threadsCount );
		// CreateProfile sets all parameter values, and after the simulation object
		// is created it can be updated.
		// Thus, it avoids race conditions when updating parameterCountIndexes and parameters2sweep
		CriticalParameterValueSimulation* pCPVS = CreateMonteCarloIteration( threadsCount );
		if( pCPVS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(threadsCount) );
			correctly_simulated = false;
			correctly_processed = false;
			return;
		}
		critical_parameter_value_simulations_vector.AddSpectreSimulation( pCPVS );
		mainTG.add_thread( new boost::thread(boost::bind(&CriticalParameterValueSimulation::RunSimulation, pCPVS)) );
		// update variables
		++threadsCount;
		++runningThreads;
	}
	mainTG.join_all();
	// process data
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "[montecarlo_critical_parameter_simulation] Generating Map files.");
	#endif
	// check if every simulation ended correctly
	correctly_simulated = critical_parameter_value_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = critical_parameter_value_simulations_vector.CheckCorrectlyProcessed();
	montecarlo_simulation_results.set_spectre_result( correctly_simulated );
	if( !AnalyzeMontecarloResults() ){
		 log_io->ReportError2AllLogs( k2Tab + "-> Error in AnalyzeMontecarloResults()" );
	}
	// resources management
	if( correctly_simulated ){
		ManageSpectreFolder();
	}
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "[montecarlo_critical_parameter_simulation] end of RunSimulation.");
	#endif
}

CriticalParameterValueSimulation* MontecarloCriticalParameterValueSimulation::CreateMonteCarloIteration(
		unsigned int montecarloCount ){
	// montecarloCount starts in 0,
	std::string s_montecarloCount = number2String(montecarloCount);
	std::string currentFolder = folder + kFolderSeparator
		 + "m_" + s_montecarloCount;
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
	// Simulation
	CriticalParameterValueSimulation* pCPVS = new CriticalParameterValueSimulation();
	pCPVS->set_has_additional_injection( has_additional_injection );
	pCPVS->set_n_dimensional(true);
	pCPVS->set_n_d_profile_index( n_d_profile_index );
	pCPVS->set_is_nested_simulation( true );
	pCPVS->set_is_montecarlo_nested_simulation( true );
	std::stringstream ssMontecarlo;
	// string formating matching Cadence Spectre format
	// as montecarloCount starts in 0, auxMC must start in 1
	auto auxMC = montecarloCount+1;
	if( auxMC<1000 ){
		ssMontecarlo.fill( '0' );
		ssMontecarlo << std::setw(3) << auxMC;
	}else{
		ssMontecarlo << auxMC;
	}
	pCPVS->set_montecarlo_transient_sufix( ssMontecarlo.str() );
	pCPVS->set_simulation_id( simulation_id + "_m_" + s_montecarloCount );
	// pCPVS->set_parameter_index( paramIndex );
	// pCPVS->set_sweep_index( sweepIndex );
	pCPVS->set_log_io( log_io );
	pCPVS->set_altered_scenario_index( altered_scenario_index );
	pCPVS->set_golden_metrics_structure( golden_metrics_structure );
	// Spectre command and args
	pCPVS->set_spectre_command( spectre_command );
	pCPVS->set_pre_spectre_command( pre_spectre_command );
	pCPVS->set_post_spectre_command( post_spectre_command );
	pCPVS->set_spectre_command_log_arg( spectre_command_log_arg );
	pCPVS->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	pCPVS->set_ahdl_simdb_env( ahdl_simdb_env );
	pCPVS->set_ahdl_shipdb_env( ahdl_shipdb_env );
	pCPVS->set_top_folder( top_folder );
	pCPVS->set_folder( currentFolder );
	pCPVS->set_altered_statement_path( altered_statement_path );
	pCPVS->set_delete_spectre_transients( delete_spectre_transients );
	pCPVS->set_delete_processed_transients( delete_processed_transients );
	pCPVS->set_plot_scatters( false );
	pCPVS->set_plot_transients( plot_transients );
	pCPVS->set_plot_last_transients( plot_last_transients );
	pCPVS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pCPVS->set_main_analysis( main_analysis );
	pCPVS->set_main_transient_analysis( main_transient_analysis );
	pCPVS->set_process_metrics( true );
	pCPVS->set_export_processed_metrics( export_processed_metrics );
	pCPVS->set_export_metric_errors( export_metric_errors );
	// copy of simulation_parameters
	pCPVS->CopySimulationParameters( *simulation_parameters );
	/// Update numruns parameter

	/// Update golden parameter
	if( !pCPVS->UpdateGoldenCriticalParameter( *golden_critical_parameter ) ){
		 log_io->ReportError2AllLogs( golden_critical_parameter->get_name()
			  + " not found in CriticalParameterValueSimulation and could not be updated." );
	}
	// add firstRunParameter
	auto firstRunParameter = new SimulationParameter( kFirstRunParamName,
		number2String(montecarloCount+1), true, kNotDefinedInt,
		kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt );
	pCPVS->AddAdditionalSimulationParameter( firstRunParameter );
	// fgarcia
	// update parameter values no needed
	return pCPVS;
}

bool MontecarloCriticalParameterValueSimulation::AnalyzeMontecarloResults(){

	bool partialResult = true;
	// create metric metrics structure
	unsigned int upsetsCount = 0;
	auto metricMetrics = golden_metrics_structure->GetBasicMetricMetricsVector();
	// metric errors
	double maxErrorMetric [metricMetrics->size()];
	double minErrorMetric [metricMetrics->size()];
	double meanMaxErrorMetric [metricMetrics->size()];
	double medianMaxErrorMetric [metricMetrics->size()];
	double q12MaxErrorMetric [metricMetrics->size()];
	double q34MaxErrorMetric [metricMetrics->size()];
	// global errors
	double maxMaxErrorGlobal [metricMetrics->size()];
	double minMaxErrorGlobal [metricMetrics->size()];
	double meanMaxErrorGlobal [metricMetrics->size()];
	// data
	std::vector<std::vector<double>> errorData( metricMetrics->size(), std::vector<double>(montecarlo_iterations, 0));
	for (unsigned int i=0;i<metricMetrics->size();++i){
		// metric
		maxErrorMetric[i] = 0.0;
		minErrorMetric[i] = std::numeric_limits<double>::max();	// min
		meanMaxErrorMetric[i] = 0.0;
		medianMaxErrorMetric[i] = 0.0;
		q12MaxErrorMetric[i] = 0.0;
		q34MaxErrorMetric[i] = 0.0;
		//global
		maxMaxErrorGlobal[i] = 0.0;
		minMaxErrorGlobal[i] = std::numeric_limits<double>::max();	// min
		meanMaxErrorGlobal[i] = 0.0;
		// errorData[i] = new double[montecarlo_iterations];
		for(unsigned int j=0; j<montecarlo_iterations;++j){
			errorData[i][j] = 0.0;
		}
	}
	double criticalParameterValueMean = 0;
	double criticalParameterValueMax = 0;
	double criticalParameterValueMin = std::numeric_limits<double>::max();	// min

	std::string gnuplotMapFilePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator
		+ kMontecarloCriticalParameterNDParametersSweepResultsFolderSubProfiles + kFolderSeparator
		+ "sim_" + simulation_id + "_profile_" + number2String( n_d_profile_index ) + kDataSufix;
	// export file
	std::ofstream gnuplotMapFile;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotMapFile << "#montecarlo_iteration #" << golden_critical_parameter->get_name() << " #spectre_error\n";
		// compute statistics
		unsigned int mcCount = 0;
		correctly_simulated_count = 0;
		for( const auto& gS : *(critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
			CriticalParameterValueSimulation* pCPVS = dynamic_cast<CriticalParameterValueSimulation*>(gS);
			if( pCPVS->get_correctly_simulated() ){
				gnuplotMapFile  << std::defaultfloat <<  mcCount << " " << pCPVS->get_critical_parameter_value() << " 0\n";
				criticalParameterValueMean += pCPVS->get_critical_parameter_value();
				if( criticalParameterValueMax<pCPVS->get_critical_parameter_value() ){
					criticalParameterValueMax = pCPVS->get_critical_parameter_value();
				}
				if( criticalParameterValueMin>pCPVS->get_critical_parameter_value() ){
					criticalParameterValueMin = pCPVS->get_critical_parameter_value();
				}
				// last transient results
				auto tr = pCPVS->get_last_valid_transient_simulation_results();
				if( tr==nullptr ){ // double check
					log_io->ReportError2AllLogs( "[montecarlo_critical_parameter_value_simulation] pCPVS->get_last_valid_transient_simulation_results() is null: " + pCPVS->get_simulation_id() );
					// update mcCount
					++mcCount;
					break; // break for
				}
				// compute statistics
				if( tr->get_reliability_result()!=kScenarioNotSensitive ){
					++upsetsCount;
				}
				++correctly_simulated_count;
				// metrics
				unsigned int magCount = 0;
				// fgarcia
				if( tr->get_metrics_errors()->size() != metricMetrics->size() ){
					log_io->ReportError2AllLogs( "[fgarcia-debug] tr->get_metrics_errors()->size() != metricMetrics->size():" );
					log_io->ReportError2AllLogs( "[fgarcia-debug] tr->get_metrics_errors()->size(): " + number2String(tr->get_metrics_errors()->size()) );
					log_io->ReportError2AllLogs( "[fgarcia-debug] metricMetrics->size(): " + number2String(metricMetrics->size()) );
				}
				for( auto const &me : *(tr->get_metrics_errors()) ){
					// global
					if( me->get_max_abs_error_global()>maxMaxErrorGlobal[magCount] ){
						maxMaxErrorGlobal[magCount] = me->get_max_abs_error_global();
					}
					if( me->get_max_abs_error_global()<minMaxErrorGlobal[magCount] ){
						minMaxErrorGlobal[magCount] = me->get_max_abs_error_global();
					}
					meanMaxErrorGlobal[magCount] += me->get_max_abs_error_global();
					// metric mean
					meanMaxErrorMetric[magCount] += me->get_max_abs_error();
					// all remaining statistics for metric
					errorData[magCount][mcCount] = me->get_max_abs_error();
					++magCount;
				}
			}else{
				gnuplotMapFile  << std::defaultfloat << mcCount << " -666 1\n";
			}
			// update counter, everything was alright
			++mcCount;
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while exporting the file: gnuplotMapFile ex-> " + ex_what );
		partialResult = false;
	}
	// close file
	gnuplotMapFile.close();
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		log_io->ReportPlainStandard( "sim " + simulation_id + " correctly_simulated_count: " + number2String(correctly_simulated_count) );
	#endif
	if( correctly_simulated_count!= montecarlo_iterations ){
		log_io->ReportRedStandard( "There where spectre errors in sim " + simulation_id + ", correctly_simulated_count: " + number2String(correctly_simulated_count) );
	}
	// report results
	montecarlo_simulation_results.set_upsets_count(upsetsCount);
	// set file
	montecarlo_simulation_results.set_critical_parameter_value_data_path( gnuplotMapFilePath );
	// compute mean
	for( unsigned int m=0; m<metricMetrics->size(); ++m){
		meanMaxErrorMetric[m] = meanMaxErrorMetric[m]/((double) correctly_simulated_count);
		meanMaxErrorGlobal[m] = meanMaxErrorGlobal[m]/((double) correctly_simulated_count);
		// compute q12, q34 and median
		// short
		std::sort(std::begin( errorData[m] ), std::end( errorData[m] ));
		// min error is the first
		minErrorMetric[m] = errorData[m][0];
		maxErrorMetric[m] = errorData[m][errorData[m].size()-1];
		// obtain quantiles
		q12MaxErrorMetric[m] = errorData[m][errorData[m].size()*1/4];
		// double q23 = data[magCount][data[magCount].size()*2/4];
		q34MaxErrorMetric[m] = errorData[m][errorData[m].size()*3/4];
		medianMaxErrorMetric[m] = errorData[m][errorData[m].size()/2];
	}
	// report results
	criticalParameterValueMean = (double)( criticalParameterValueMean/((double)correctly_simulated_count) );
	montecarlo_simulation_results.set_max_critical_parameter_value( criticalParameterValueMax );
	montecarlo_simulation_results.set_min_critical_parameter_value( criticalParameterValueMin );
	montecarlo_simulation_results.set_mean_critical_parameter_value( criticalParameterValueMean );
	// reserve memory
	montecarlo_simulation_results.ReserveMetricMontecarloResults( metricMetrics->size() );
	unsigned int magCount = 0;
	for( auto const &m : *metricMetrics ){
		// deleted in MontecarloSimulationResults destructur
		auto mMCR = new metric_montecarlo_results_t();
		mMCR->metric_name = m->get_name();
		// errors in metric
		mMCR->max_error_metric = maxErrorMetric[magCount];
		mMCR->min_error_metric = minErrorMetric[magCount];
		mMCR->mean_max_error_metric = meanMaxErrorMetric[magCount];
		mMCR->median_max_error_metric = medianMaxErrorMetric[magCount];
		mMCR->q12_max_error_metric = q12MaxErrorMetric[magCount];
		mMCR->q34_max_error_metric = q34MaxErrorMetric[magCount];
		// global errors
		mMCR->max_max_error_global = maxMaxErrorGlobal[magCount];
		mMCR->min_max_error_global = minMaxErrorGlobal[magCount];
		mMCR->mean_max_error_global = meanMaxErrorGlobal[magCount];
		montecarlo_simulation_results.AddMetricMontecarloResults( mMCR );
		// #ifdef RESULTS_ANALYSIS_VERBOSE
		// log_io->ReportGreenStandard( "[debug]" simulation_id +  "-> meanMaxErrorGlobal:" + number2String(meanMaxErrorGlobal[magCount])
		// 	+ " mean_max_error_metric:" + number2String(meanMaxErrorMetric[magCount])
		// 	+ " b max_error_global:" + number2String(mMCR->max_max_error_global)
		// 	+ " b mean_max_error_metric:" + number2String(mMCR->mean_max_error_metric)  );
		// #endif
		++magCount;
	}
	// plot scatters
	if( plot_scatters ){
		partialResult = partialResult && PlotScatters();
	}
	// handle individual results
	for( const auto& gS : *(critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
		CriticalParameterValueSimulation* pCPVS = dynamic_cast<CriticalParameterValueSimulation*>(gS);
		auto tr = pCPVS->get_last_valid_transient_simulation_results();
		if( !ManageIndividualResultFiles( *tr, false ) ){
			log_io->ReportError2AllLogs( "Error deleting raw data. Last results of Scenario #" + pCPVS->get_simulation_id() );
			// return false; // program can continue
		}
	}
	return partialResult;
}

bool MontecarloCriticalParameterValueSimulation::PlotScatters(){
	bool partialResults = true;
	// Create folders
	std::string gnuplotScriptFolder =  top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator
		+ kMontecarloCriticalParameterNDParametersSweepResultsFolderSubProfiles + kFolderSeparator
		+ "profile_" + number2String( n_d_profile_index );
	std::string imagesFolder =  top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator
		+ kMontecarloCriticalParameterNDParametersSweepResultsFolderSubProfiles + kFolderSeparator
		+ "profile_" + number2String( n_d_profile_index );
	if( !CreateFolder(imagesFolder, true ) || !CreateFolder(gnuplotScriptFolder, true ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
			+ gnuplotScriptFolder + " and " + imagesFolder + "'." );
		log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
		return false;
	}
	// plot metric scatters
	if( plot_scatters ){
		partialResults = partialResults && PlotMetricScatters(gnuplotScriptFolder, imagesFolder);
	}
	// title
	std::string title = golden_critical_parameter->get_title_name() + ", " +  simulation_id;
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + simulation_id + "_cpv_" + golden_critical_parameter->get_title_name() + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + simulation_id + "_cpv_" + golden_critical_parameter->get_title_name() + kSvgSufix;
		std::ofstream gnuplotScriptFile;
	try {
		// for each transient
		// Generate scripts
		gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
		// Svg
		gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
			 << " fname " << kSvgFont << "\n";
		gnuplotScriptFile << "set output \"" << outputImagePath << "\"" << "\n";
		gnuplotScriptFile << "set title \" " << title << " \"" << "\n";
		gnuplotScriptFile << "set grid" << "\n";
		// Axis
		// 	 gnuplotScriptFile << "set logscale y" << "\n";
		gnuplotScriptFile << "set format x \"%g\"" << "\n";
		gnuplotScriptFile << "set format y \"%g\"" << "\n";
		// gnuplotScriptFile << "set xlabel \""  << pairedParameter.get_title_name() << "\"" << "\n";
		gnuplotScriptFile << "set xlabel \""  << "Montecarlo Iteration" << "\"" << "\n";
		gnuplotScriptFile << "set y2label \"Spectre error\"\n";
		gnuplotScriptFile << "set ylabel \" " << golden_critical_parameter->get_title_name() << " \"" << "\n";
		// # remove border on top and right and set color to gray
		gnuplotScriptFile << kCustomBorders << "\n";
		// line style
		gnuplotScriptFile << kTransientSimilarLinesPalette;
		// legend
		gnuplotScriptFile << "set key bottom right" << "\n";
		gnuplotScriptFile << "g(x,y) = y>0 ? 0 : x" << "\n";
		// Plot
		gnuplotScriptFile << "plot '" << montecarlo_simulation_results.get_critical_parameter_value_data_path()
			<< "' u 1:(g($2, $3))" << " axes x1y1 w lp ls 1 title '" << golden_critical_parameter->get_title_name() << "', \\" << "\n";
		gnuplotScriptFile << " '" << montecarlo_simulation_results.get_critical_parameter_value_data_path()
			<< "' u 1:3" << " axes x1y2 with boxes ls 2 title 'Spectre Errors'" << "\n";
		gnuplotScriptFile << "ntics = 15" << "\n";
		gnuplotScriptFile << "set xtics ntics" << "\n";
		gnuplotScriptFile << "unset output" << "\n";
		// close file
		gnuplotScriptFile << "quit" << "\n";
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		partialResults = -1;
	}
	// close files
	gnuplotScriptFile.close();
	if( partialResults<0 ){
		return partialResults;
	}
	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	// Image paths
	montecarlo_simulation_results.set_critical_parameter_value_image_path(outputImagePath);
	// plot
	partialResults += std::system( execCommand.c_str() );

	return partialResults;
}

bool MontecarloCriticalParameterValueSimulation::PlotMetricScatters(
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	bool partialResults = true;
	std::string goldenFilePath  = golden_metrics_structure->GetFilePath( n_d_profile_index );
	// for each plotable and analizable metric
	unsigned int magCount = 0;
	unsigned int magResultIndex = 2;
	// for each plotable
	for( auto const &mag : *(golden_metrics_structure->GetBasicPlottableMetricsVector()) ){
		// title
		std::string title = mag->get_title_name() +  ", " +  simulation_id +  " montecarlo scatter";
		// Files
		std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
			 + simulation_id + "_mag_" + number2String(magCount) + "_scatter" + kGnuPlotScriptSufix;
		std::string outputImagePath = imagesFolder + kFolderSeparator
			 + simulation_id + "_mag_" + number2String(magCount) + "_scatter" + kSvgSufix;
 		std::ofstream gnuplotScriptFile;
		try {
			// for each transient
			// Generate scripts
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			// Svg
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
				 << " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath << "\"" << "\n";
			gnuplotScriptFile << "set title \" " << title << " \"" << "\n";
			gnuplotScriptFile << "set grid" << "\n";
			// Axis
			// 	 gnuplotScriptFile << "set logscale y" << "\n";
			gnuplotScriptFile << "set format x \"%g\"" << "\n";
			gnuplotScriptFile << "set format y \"%g\"" << "\n";
			// gnuplotScriptFile << "set xlabel \""  << pairedParameter.get_title_name() << "\"" << "\n";
			gnuplotScriptFile << "set xlabel \""  << "Profile" << "\"" << "\n";
			gnuplotScriptFile << "set ylabel \" " << mag->get_title_name() << " \"" << "\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << kCustomBorders;
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette;
			// line style
			gnuplotScriptFile << kTransientSimilarLinesPalette;
			gnuplotScriptFile << "set style fill solid" << "\n";
			// legend
			gnuplotScriptFile << "set key bottom right" << "\n";
			// Plot
			gnuplotScriptFile << "plot '" << goldenFilePath << "' u 1:" << number2String(magResultIndex) << " w l ls 1 title 'Golden', \\" << "\n";
			unsigned int irCount = 2;
			for( const auto& gS : *(critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
				CriticalParameterValueSimulation* pCPVS = dynamic_cast<CriticalParameterValueSimulation*>(gS);
				auto ir = pCPVS->get_last_valid_transient_simulation_results();
				gnuplotScriptFile << " '" << ir->get_processed_file_path() << "' u 1:" << number2String(magResultIndex)
					<< " w p ls " << number2String(irCount++) << " notitle, \\" << "\n";
			}
			gnuplotScriptFile << "\n";
			gnuplotScriptFile << "ntics = 15" << "\n";
			gnuplotScriptFile << "set xtics ntics" << "\n";
			gnuplotScriptFile << "unset output" << "\n";
			// close file
			gnuplotScriptFile << "quit" << "\n";
		}catch (std::exception const& ex) {
			std::string ex_what = ex.what();
			log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
			partialResults = -1;
		}
		// close files
		gnuplotScriptFile.close();
		if( partialResults<0 ){
			return partialResults;
		}
		// Exec comand
		std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
		// Image paths for montecarlo results
		if( mag->get_analyzable() ){
			auto mcsr = montecarlo_simulation_results.get_metric_montecarlo_results();
			std::for_each( mcsr->begin(), mcsr->end(), [&mag, &outputImagePath]( metric_montecarlo_results_t*& mmr) {  //arg taken by reference
				if(mmr->metric_name==mag->get_name()){ mmr->mc_scatter_image_path = outputImagePath; };
			});
		}
		// plot
		partialResults += std::system( execCommand.c_str() );
		// update counters
		++magCount;
		++magResultIndex;
	} // for each mag
	return partialResults;
}
