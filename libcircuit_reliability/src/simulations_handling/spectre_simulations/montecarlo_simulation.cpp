 /**
 * @file standard_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * MontecarloSimulation Class Body
 *
 */

#include <cmath>
// Radiation simulator
#include "montecarlo_simulation.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

MontecarloSimulation::MontecarloSimulation() {
	this->export_processed_metrics = true;
	this->max_parallel_montecarlo_instances = 5;
	// injection mode related
	this->has_additional_injection = false;
}

MontecarloSimulation::~MontecarloSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "MontecarloSimulation destructor. direction:" << this << "\n";
	#endif
}

bool MontecarloSimulation::TestSetUp(){
	if( simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr simulation_parameters in montecarlo_simulation");
		return false;
	}else if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder  in montecarlo_simulation");
		return false;
	}else if( golden_metrics_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_metrics_structure in montecarlo_simulation");
		return false;
	}
	return true;
}

void MontecarloSimulation::RunSimulation( ){
	// standard comprobations
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr. ");
		return;
	}
	// Register Parameters
	montecarlo_simulation_results.set_full_id( simulation_id );
	montecarlo_simulation_results.RegisterSimulationParameters(simulation_parameters);
	unsigned int mcIterationCount = 0;
	montecarlo_simulations_vector.set_group_name("montecarlo_simulations_vector");
	montecarlo_simulations_vector.ReserveSimulationsInMemory( montecarlo_iterations );
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportThread( "Total montecarlo to be simulated: " + number2String(montecarlo_iterations) + ". Max number of montecarlo threads: " + number2String(max_parallel_montecarlo_instances), 1 );
	#endif
	while( mcIterationCount<montecarlo_iterations ){
		// not needed to copy 'parameterCountIndexes' since without using boost::ref, arguments are copied
		// to avoid race conditions updating variables
		StandardSimulation* pSS = CreateMonteCarloIteration( mcIterationCount );
		if( pSS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(mcIterationCount) );
			correctly_simulated = false;
			correctly_processed = false;
			return;
		}
		montecarlo_simulations_vector.AddSpectreSimulation( pSS );
		// update variables
		++mcIterationCount;
	}
	// RunSpectreMC
	int auxSpectreResult = RunSpectre( simulation_id );
	montecarlo_simulation_results.set_spectre_result( auxSpectreResult );
	// Transients processing
	for( auto const &ps : *(montecarlo_simulations_vector.get_spectre_simulations()) ){
		StandardSimulation* pSS = dynamic_cast<StandardSimulation*>( ps );
		pSS->ProcessMetricsFromExt( auxSpectreResult );
	}
	// process data
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "[montecarlo_simulation] Generating Map files " + simulation_id);
	#endif
	// check if every simulation ended correctly
	// montecarlo_simulation_results.set_spectre_result( correctly_simulated );
	// correctly_simulated = montecarlo_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = montecarlo_simulations_vector.CheckCorrectlyProcessed();
	if( auxSpectreResult == kNotDefinedInt ){
		 log_io->ReportError2AllLogs( k2Tab + "->[montecarlo_simulation] Error in AnalyzeMontecarloResults()" );
		 return;
	}
	if( !AnalyzeMontecarloResults() ){
		 log_io->ReportError2AllLogs( k2Tab + "->[montecarlo_simulation] Error in AnalyzeMontecarloResults()" );
	}
	// resources management
	if( correctly_simulated ){
		ManageSpectreFolder();
	}
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "[montecarlo_simulation] end of RunSimulation.");
	#endif
}

int MontecarloSimulation::RunSpectre( const std::string& scenarioId ){
	// previous setup
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return kNotDefinedInt;
	}
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr.");
		return kNotDefinedInt;
	}
	// MC setup
	// add firstRunParameter and seedParameter
	auto firstRunParameter = new SimulationParameter( kMCFirstRunParamName, "1",
		true, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt );
	auto seedParameter = new SimulationParameter( kMCSeedParamName, number2String(n_d_profile_index),
		true, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt );
	AddAdditionalSimulationParameter( firstRunParameter );
	AddAdditionalSimulationParameter( seedParameter );

	// Parameters file
	if( !ExportParametersCircuit( folder, 0 )){
		log_io->ReportError2AllLogs( "Error creating parameters Circuit ");
		return kNotDefinedInt;
	}
	// Environment variables
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	// standard runspectre
	std::string execCommand = spectre_command + " "
		+ spectre_command_log_arg + " " + folder + kFolderSeparator + kSpectreLogFile + " "
		+ spectre_command_folder_arg + " " + folder + kFolderSeparator + kSpectreResultsFolder + " "
		+ folder + kFolderSeparator + kMainNetlistFile
		+ " " + post_spectre_command + " " + folder + kFolderSeparator + kSpectreStandardLogsFile ;
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "#" + scenarioId + " scenario: Simulating scenario." );
	#endif
	int spectre_result = std::system( execCommand.c_str() ) ;
	if(spectre_result>0){
		correctly_simulated = false;
		log_io->ReportError2AllLogs( "Unexpected Spectre spectre_result for scenario #"
			+ scenarioId + ": spectre output = " + number2String(spectre_result) );
		log_io->ReportError2AllLogs( "Spectre Log Folder: " + folder );
		return spectre_result;
	}
	correctly_simulated = true;
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportGreenStandard( k2Tab + "#" + scenarioId + " scenario: Simulating ENDED."
		+ " spectre_result=" + number2String(spectre_result) );
	#endif
	return spectre_result;
}

StandardSimulation* MontecarloSimulation::CreateMonteCarloIteration( unsigned int montecarloCount ){
	// montecarloCount starts in 0,
	std::string s_montecarloCount = number2String(montecarloCount);
	// Simulation
	StandardSimulation* pSS = new StandardSimulation();
	pSS->set_has_additional_injection( has_additional_injection );
	pSS->set_n_dimensional( true );
	pSS->set_n_d_profile_index( n_d_profile_index );
	pSS->set_is_nested_simulation( true );
	pSS->set_is_montecarlo_nested_simulation( true );
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
	pSS->set_montecarlo_transient_sufix( ssMontecarlo.str() );
	pSS->set_simulation_id( simulation_id + "_m_" + s_montecarloCount );
	// pSS->set_parameter_index( paramIndex );
	// pSS->set_sweep_index( sweepIndex );
	pSS->set_log_io( log_io );
	pSS->set_altered_scenario_index( altered_scenario_index );
	pSS->set_golden_metrics_structure( golden_metrics_structure );
	// Spectre command and args
	pSS->set_spectre_command( spectre_command );
	pSS->set_pre_spectre_command( pre_spectre_command );
	pSS->set_post_spectre_command( post_spectre_command );
	pSS->set_spectre_command_log_arg( spectre_command_log_arg );
	pSS->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	pSS->set_ahdl_simdb_env( ahdl_simdb_env );
	pSS->set_ahdl_shipdb_env( ahdl_shipdb_env );
	pSS->set_top_folder( top_folder );
	pSS->set_folder( folder );
	pSS->set_altered_statement_path( altered_statement_path );
	pSS->set_delete_spectre_transients( delete_spectre_transients );
	pSS->set_delete_processed_transients( delete_processed_transients );
	pSS->set_plot_scatters( false );
	pSS->set_plot_transients( plot_transients );
	pSS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pSS->set_main_analysis( main_analysis );
	pSS->set_main_transient_analysis( main_transient_analysis );
	pSS->set_process_metrics( true );
	pSS->set_export_processed_metrics( export_processed_metrics );
	pSS->set_export_metric_errors( export_metric_errors );
	// copy of simulation_parameters
	pSS->CopySimulationParameters( *simulation_parameters );
	// firstrun and seed in main simulation
	return pSS;
}

bool MontecarloSimulation::AnalyzeMontecarloResults(){
	// fgarcia
	// no critical parameter analysis is needed
	bool partialResult = true;
	// create metric metrics structure
	unsigned int upsetsCount = 0;
	auto analyzedMetrics = golden_metrics_structure->GetBasicAnalyzableMetricsVector();
	auto oceanEvalMetrics = golden_metrics_structure->GetBasicOceanEvalMetricsVector();
	// oceanEvalVals
	double maxValEvalMetric [oceanEvalMetrics->size()];
	double minValEvalMetric [oceanEvalMetrics->size()];
	double meanValEvalMetric [oceanEvalMetrics->size()];
	// metric errors
	double maxErrorMetric [analyzedMetrics->size()];
	double minErrorMetric [analyzedMetrics->size()];
	double meanMaxErrorMetric [analyzedMetrics->size()];
	double medianMaxErrorMetric [analyzedMetrics->size()];
	double q12MaxErrorMetric [analyzedMetrics->size()];
	double q34MaxErrorMetric [analyzedMetrics->size()];
	// global errors
	double maxMaxErrorGlobal [analyzedMetrics->size()];
	double minMaxErrorGlobal [analyzedMetrics->size()];
	double meanMaxErrorGlobal [analyzedMetrics->size()];
	// data
	std::vector<std::vector<double>> errorData( analyzedMetrics->size(), std::vector<double>(montecarlo_iterations, 0));
	for (unsigned int i=0;i<analyzedMetrics->size();++i){
		// oceanEvalMetrics
		maxValEvalMetric[i] = 0.0;
		minValEvalMetric[i] = std::numeric_limits<double>::max();	// min
		meanValEvalMetric[i] = 0.0;
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
	// process data
	// compute statistics
	unsigned int mcCount = 0;
	correctly_simulated_count = 0;
	for( const auto& gS : *(montecarlo_simulations_vector.get_spectre_simulations()) ){
		StandardSimulation* pSS = dynamic_cast<StandardSimulation*>(gS);
		if( pSS->get_correctly_simulated() ){
			// last transient results
			auto tr = pSS->get_basic_simulation_results();
			if(tr==nullptr){ // double check
				log_io->ReportError2AllLogs( "pSS->get_last_valid_transient_simulation_results() is null: " + pSS->get_simulation_id() );
				break; // break for
			}
			// compute statistics
			if( tr->get_reliability_result()!=kScenarioNotSensitive ){
				++upsetsCount;
			}
			++correctly_simulated_count;
			// metrics
			unsigned int metricCount = 0;
			unsigned int oceanEvalCount = 0;
			// fgarcia
			if( tr->get_metrics_errors()->size() != analyzedMetrics->size() ){
				log_io->ReportError2AllLogs( "tr->get_metrics_errors()->size() != analyzedMetrics->size(), sim" + pSS->get_simulation_id());
				log_io->ReportError2AllLogs( "tr->get_metrics_errors()->size(): " + number2String(tr->get_metrics_errors()->size()) );
				log_io->ReportError2AllLogs( "analyzedMetrics->size(): " + number2String(analyzedMetrics->size()) );
			}
			for( auto const &me : *(tr->get_metrics_errors()) ){
				// ocean_eval_metrics
				if( !me->is_transient_magnitude() ){
					if( maxValEvalMetric[oceanEvalCount] < me->get_metric_value() ){
						maxValEvalMetric[oceanEvalCount] = me->get_metric_value();
					}
					if( minValEvalMetric[oceanEvalCount] > me->get_metric_value() ){
						minValEvalMetric[oceanEvalCount] = me->get_metric_value();
					}
					// mean and post increment
					meanValEvalMetric[oceanEvalCount++] += me->get_metric_value();
				}
				// global
				if( me->get_max_abs_error_global()>maxMaxErrorGlobal[metricCount] ){
					maxMaxErrorGlobal[metricCount] = me->get_max_abs_error_global();
				}
				if( me->get_max_abs_error_global()<minMaxErrorGlobal[metricCount] ){
					minMaxErrorGlobal[metricCount] = me->get_max_abs_error_global();
				}
				meanMaxErrorGlobal[metricCount] += me->get_max_abs_error_global();
				// metric mean
				meanMaxErrorMetric[metricCount] += me->get_max_abs_error();
				// other metric statistics
				errorData[metricCount][mcCount] = me->get_max_abs_error();
				++metricCount;
			}
		} // end of correctly simulated
		// update counter
		++mcCount;
	} // end foreach
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		log_io->ReportPlainStandard( "sim " + simulation_id + " correctly_simulated_count: " + number2String(correctly_simulated_count) );
	#endif
	if( correctly_simulated_count!= montecarlo_iterations ){
		log_io->ReportRedStandard( "There where spectre errors in sim " + simulation_id + ", correctly_simulated_count: " + number2String(correctly_simulated_count) );
	}
	// report results
	montecarlo_simulation_results.set_upsets_count(upsetsCount);
	// set file
	montecarlo_simulation_results.set_critical_parameter_value_data_path("~/no_file_required_in_this_mode");
	// not needed
	// montecarlo_simulation_results.set_mean_critical_parameter_value( kNotDefinedInt );
	// compute mean of oceanMetric vals
	for( unsigned int m=0; m<oceanEvalMetrics->size(); ++m){
		meanValEvalMetric[m] = meanValEvalMetric[m]/((double) correctly_simulated_count);
	}
	// compute mean of all metrics
	for( unsigned int m=0; m<analyzedMetrics->size(); ++m){
		meanMaxErrorMetric[m] = meanMaxErrorMetric[m]/((double) correctly_simulated_count);
		meanMaxErrorGlobal[m] = meanMaxErrorGlobal[m]/((double) correctly_simulated_count);
		// compute q12, q34 and median
		// short
		std::sort( std::begin( errorData[m] ), std::end( errorData[m] ));
		// min error is the first
		minErrorMetric[m] = errorData[m][0];
		maxErrorMetric[m] = errorData[m][errorData[m].size()-1];
		// obtain quantiles
		q12MaxErrorMetric[m] = errorData[m][errorData[m].size()*1/4];
		// double q23 = data[metricCount][data[metricCount].size()*2/4];
		q34MaxErrorMetric[m] = errorData[m][errorData[m].size()*3/4];
		medianMaxErrorMetric[m] = errorData[m][errorData[m].size()/2];
	}
	// reserve memory
	montecarlo_simulation_results.ReserveMetricMontecarloResults( analyzedMetrics->size() );
	unsigned int mCount = 0;
	unsigned int oemCount = 0;
	for( auto const &m : *analyzedMetrics ){
		// deleted in MontecarloSimulationResults destructur
		auto mMCR = new metric_montecarlo_results_t();
		mMCR->metric_name = m->get_name();
		// metric
		mMCR->max_error_metric = maxErrorMetric[mCount];
		mMCR->min_error_metric = minErrorMetric[mCount];
		mMCR->mean_max_error_metric = meanMaxErrorMetric[mCount];
		mMCR->median_max_error_metric = medianMaxErrorMetric[mCount];
		mMCR->q12_max_error_metric = q12MaxErrorMetric[mCount];
		mMCR->q34_max_error_metric = q34MaxErrorMetric[mCount];
		// global errors
		mMCR->max_max_error_global = maxMaxErrorGlobal[mCount];
		mMCR->min_max_error_global = minMaxErrorGlobal[mCount];
		mMCR->mean_max_error_global = meanMaxErrorGlobal[mCount];
		montecarlo_simulation_results.AddMetricMontecarloResults( mMCR );
		// oceanEvalMetrics
		if( !m->is_transient_magnitude() ){
			mMCR->transient_magnitude = false;
			mMCR->ocean_eval_metric_max_val = maxValEvalMetric[oemCount];;
			mMCR->ocean_eval_metric_min_val = minValEvalMetric[oemCount];
			// mean and postincrement
			mMCR->ocean_eval_metric_mean_val = meanValEvalMetric[oemCount++];
		}
		++mCount;
	}
	// plot scatters
	if( plot_scatters ){
		partialResult = partialResult && PlotScatters();
	}
	// handle individual results
	for( const auto& gS : *(montecarlo_simulations_vector.get_spectre_simulations()) ){
		StandardSimulation* pSS = dynamic_cast<StandardSimulation*>(gS);
		auto tr = pSS->get_basic_simulation_results();
		if( !ManageIndividualResultFiles( *tr, false ) ){
			log_io->ReportError2AllLogs( "Error deleting raw data. Last results of Scenario #" + pSS->get_simulation_id() );
			// return false; // program can continue
		}
	}
	return partialResult;
}

bool MontecarloSimulation::PlotScatters(){
	bool partialResults = true;
	// Create folders
	std::string gnuplotScriptFolder =  top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator
		+ kMontecarloNDParametersSweepResultsFolderSubProfiles + kFolderSeparator
		+ "profile_" + number2String( n_d_profile_index );
	std::string imagesFolder =  top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator
		+ kMontecarloNDParametersSweepResultsFolderSubProfiles + kFolderSeparator
		+ "profile_" + number2String( n_d_profile_index );
	if( !CreateFolder(imagesFolder, true ) || !CreateFolder(gnuplotScriptFolder, true ) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
			+ gnuplotScriptFolder + " and " + imagesFolder + "'." );
		log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
		return false;
	}
	montecarlo_simulation_results.set_critical_parameter_value_image_path("~/no_file_required_in_this_mode");
	// plot metric scatters
	if( plot_scatters ){
		partialResults = partialResults && PlotMetricScatters(gnuplotScriptFolder, imagesFolder);
	}
	return partialResults;
}

bool MontecarloSimulation::PlotMetricScatters(
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
			for( const auto& gS : *(montecarlo_simulations_vector.get_spectre_simulations()) ){
				StandardSimulation* pSS = dynamic_cast<StandardSimulation*>(gS);
				auto ir = pSS->get_basic_simulation_results();
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
