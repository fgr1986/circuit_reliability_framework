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
	this->export_processed_magnitudes = true;
	this->max_parallel_montecarlo_instances = 5;
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
	}else if( golden_magnitudes_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_magnitudes_structure in montecarlo_simulation");
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
	boost::thread_group mainTG;
	// Register Parameters
	montecarlo_simulation_results.set_full_id( simulation_id );
	montecarlo_simulation_results.RegisterSimulationParameters(simulation_parameters);
	// parallel threads control
	unsigned int runningThreads = 0;
	unsigned int threadsCount = 0;
	montecarlo_simulations_vector.set_group_name("montecarlo_simulations_vector");
	montecarlo_simulations_vector.ReserveSimulationsInMemory( montecarlo_iterations );
	while( threadsCount<montecarlo_iterations ){
		// wait for resources
		WaitForResources( runningThreads, max_parallel_montecarlo_instances, mainTG, threadsCount );
		// not needed to copy 'parameterCountIndexes' since without using boost::ref, arguments are copied
		// to avoid race conditions updating variables
		StandardSimulation* pSS = CreateMonteCarloIteration( threadsCount );
		if( pSS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(threadsCount) );
			correctly_simulated = false;
			correctly_processed = false;
			return;
		}
		montecarlo_simulations_vector.AddSpectreSimulation( pSS );
		mainTG.add_thread( new boost::thread(boost::bind(&StandardSimulation::RunSimulation, pSS)) );
		// update variables
		++threadsCount;
		++runningThreads;
	}
	mainTG.join_all();
	// process data
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "[montecarlo_simulation] Generating Map files " + simulation_id);
	#endif
	// check if every simulation ended correctly
	correctly_simulated = montecarlo_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = montecarlo_simulations_vector.CheckCorrectlyProcessed();
	montecarlo_simulation_results.set_spectre_result( correctly_simulated );
	if( !AnalyzeMontecarloResults() ){
		 log_io->ReportError2AllLogs( k2Tab + "->[montecarlo_simulation] Error in AnalyzeMontecarloResults()" );
	}
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "[montecarlo_simulation] end of RunSimulation.");
	#endif
}

StandardSimulation* MontecarloSimulation::CreateMonteCarloIteration( unsigned int montecarloCount ){
	// montecarloCount starts in 0,
	std::string s_montecarloCount = number2String(montecarloCount);
	std::string currentFolder = folder + kFolderSeparator
		 + "m_" + s_montecarloCount;
	if( !CreateFolder(currentFolder, true) ){
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
	StandardSimulation* pSS = new StandardSimulation();
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
	pSS->set_golden_magnitudes_structure( golden_magnitudes_structure );
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
	pSS->set_folder( currentFolder );
	pSS->set_altered_statement_path( altered_statement_path );
	pSS->set_save_spectre_transients( save_spectre_transients );
	pSS->set_save_processed_transients( save_processed_transients );
	pSS->set_plot_scatters( false );
	pSS->set_plot_transients( plot_transients );
	pSS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pSS->set_main_analysis( main_analysis );
	pSS->set_main_transient_analysis( main_transient_analysis );
	pSS->set_process_magnitudes( true );
	pSS->set_export_processed_magnitudes( true );
	pSS->set_export_magnitude_errors( export_magnitude_errors );
	// copy of simulation_parameters
	pSS->CopySimulationParameters( *simulation_parameters );
	/// Update numruns parameter
	/// No need to update golden parameter
	// add firstRunParameter
	auto firstRunParameter = new SimulationParameter( kFirstRunParamName,
		number2String(montecarloCount+1), true, kNotDefinedInt,
		kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt );
	pSS->AddAdditionalSimulationParameter( firstRunParameter );
	// fgarcia
	// update parameter values no needed
	return pSS;
}

bool MontecarloSimulation::AnalyzeMontecarloResults(){
	// fgarcia
	// no critical parameter analysis is needed
	bool partialResult = true;
	// create metric magnitudes structure
	auto metricMagnitudes = golden_magnitudes_structure->GetMetricMagnitudesVector(0);
	double maxErrorGlobal [metricMagnitudes->size()];
	double maxErrorMetric [metricMagnitudes->size()];
	double minErrorMetric [metricMagnitudes->size()];
	double meanMaxErrorMetric [metricMagnitudes->size()];
	double medianMaxErrorMetric [metricMagnitudes->size()];
	double q12MaxErrorMetric [metricMagnitudes->size()];
	double q34MaxErrorMetric [metricMagnitudes->size()];
	// double** errorData = new double*[metricMagnitudes->size()];
	// std::vector<std::vector<double>> errorData( metricMagnitudes->size(), std::vector<double>(montecarlo_iterations) );
	std::vector<std::vector<double>> errorData( metricMagnitudes->size(), std::vector<double>(montecarlo_iterations, 0));
	for (unsigned int i=0;i<metricMagnitudes->size();++i){
		maxErrorGlobal[i] = 0.0;
		maxErrorMetric[i] = 0.0;
		minErrorMetric[i] = 0.0;
		meanMaxErrorMetric[i] = 0.0;
		medianMaxErrorMetric[i] = 0.0;
		q12MaxErrorMetric[i] = 0.0;
		q34MaxErrorMetric[i] = 0.0;
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
				log_io->ReportError2AllLogs( "[fgarcia-debug] pSS->get_last_valid_transient_simulation_results() is null: " + pSS->get_simulation_id() );
				break; // break for
			}
			// statistics
			++correctly_simulated_count;
			// magnitudes
			unsigned int magCount = 0;
			// fgarcia
			if( tr->get_magnitudes_errors()->size() != metricMagnitudes->size() ){
				log_io->ReportError2AllLogs( "[fgarcia-debug] tr->get_magnitudes_errors()->size() != metricMagnitudes->size(), sim" + pSS->get_simulation_id());
				log_io->ReportError2AllLogs( "[fgarcia-debug] tr->get_magnitudes_errors()->size(): " + number2String(tr->get_magnitudes_errors()->size()) );
				log_io->ReportError2AllLogs( "[fgarcia-debug] metricMagnitudes->size(): " + number2String(metricMagnitudes->size()) );
			}
			for( auto const &me : *(tr->get_magnitudes_errors()) ){
				if( me->get_max_abs_error_global()>maxErrorGlobal[magCount] ){
					maxErrorGlobal[magCount] = me->get_max_abs_error_global();
				}
				meanMaxErrorMetric[magCount] += me->get_max_abs_error();
				errorData[magCount][mcCount] = me->get_max_abs_error();
				++magCount;
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
	// set file
	montecarlo_simulation_results.set_critical_parameter_value_data_path("~/no_file_required_in_this_mode");
	montecarlo_simulation_results.set_mean_critical_parameter_value( kNotDefinedInt );
	// compute mean
	for( unsigned int m=0; m<metricMagnitudes->size(); ++m){
		log_io->ReportPlainStandard( "[debug] " + simulation_id + " [m]: " + number2String(m) + " meanMaxErrorMetric:"  + number2String(meanMaxErrorMetric[m]) );
		meanMaxErrorMetric[m] = meanMaxErrorMetric[m]/((double) correctly_simulated_count);
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
	// reserve memory
	montecarlo_simulation_results.ReserveMetricMontecarloResults( metricMagnitudes->size() );
	unsigned int magCount = 0;
	for( auto const &m : *metricMagnitudes ){
		// deleted in MontecarloSimulationResults destructur
		auto mMCR = new metric_montecarlo_results_t();
		mMCR->metric_magnitude_name = m->get_name();
		mMCR->max_error_global = maxErrorGlobal[magCount];
		mMCR->max_error_metric = maxErrorMetric[magCount];
		mMCR->min_error_metric = minErrorMetric[magCount];
		mMCR->mean_max_error_metric = meanMaxErrorMetric[magCount];
		mMCR->median_max_error_metric = medianMaxErrorMetric[magCount];
		mMCR->q12_max_error_metric = q12MaxErrorMetric[magCount];
		mMCR->q34_max_error_metric = q34MaxErrorMetric[magCount];
		montecarlo_simulation_results.AddMetricMontecarloResults( mMCR );
		#ifdef RESULTS_POST_PROCESSING_VERBOSE
		log_io->ReportGreenStandard( simulation_id +  "-> max_error_global:" + number2String(maxErrorGlobal[magCount])
			+ " mean_max_error_metric:" + number2String(meanMaxErrorMetric[magCount])
			+ " b max_error_global:" + number2String(mMCR->max_error_global)
			+ " b mean_max_error_metric:" + number2String(mMCR->mean_max_error_metric)  );
		#endif
		++magCount;
	}
	// plot scatters
	if( plot_scatters ){
		partialResult = partialResult && PlotProfileResults();
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

bool MontecarloSimulation::PlotProfileResults(){
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
	if( !CreateFolder(imagesFolder, true) || !CreateFolder(gnuplotScriptFolder, true) ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
			+ gnuplotScriptFolder + " and " + imagesFolder + "'." );
		log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
		return false;
	}
	montecarlo_simulation_results.set_critical_parameter_value_image_path("~/no_file_required_in_this_mode");
	// plot magnitude scatters
	if( plot_scatters ){
		partialResults = partialResults && PlotProfileResultsMagnitudes(gnuplotScriptFolder, imagesFolder);
	}
	return partialResults;
}

bool MontecarloSimulation::PlotProfileResultsMagnitudes(
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	bool partialResults = true;
	std::string goldenFilePath  = golden_magnitudes_structure->GetFilePath( n_d_profile_index );
	// for each plotable and analizable magnitude
	unsigned int magCount = 0;
	unsigned int magResultIndex = 2;
	for( auto const &mmr : *(montecarlo_simulation_results.get_metric_montecarlo_results()) ){
		// title
		std::string title = mmr->metric_magnitude_name +  ", " +  simulation_id +  " montecarlo scatter";
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
			gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight
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
			gnuplotScriptFile << "set ylabel \" " << mmr->metric_magnitude_name << " \"" << "\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1" << "\n";
			gnuplotScriptFile << "set border 3 back ls 11" << "\n";
			gnuplotScriptFile << "set tics nomirror" << "\n";
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette << "\n";
			// line style
			gnuplotScriptFile << "set style line 1 lc rgb '#5e9c36' pt 6 ps 1 lt 1 lw 2 # --- green" << "\n";
			gnuplotScriptFile << "set style fill solid" << "\n";
			// // Background
			gnuplotScriptFile << kWholeBackground << "\n";
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
		// Image paths
		mmr->mc_scatter_image_path = outputImagePath;
		// plot
		partialResults += std::system( execCommand.c_str() );
		// update counters
		++magCount;
		++magResultIndex;
	} // for each mag
	return partialResults;
}
