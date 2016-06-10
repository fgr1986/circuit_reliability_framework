 /**
 * @file standard_simulation.cpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * MontecarloStandardSimulation Class Body
 *
 */

#include <cmath>
// Radiation simulator
#include "montecarlo_standard_simulation.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

MontecarloStandardSimulation::MontecarloStandardSimulation() {
	this->export_processed_magnitudes = true;
}

MontecarloStandardSimulation::~MontecarloStandardSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "MontecarloStandardSimulation destructor. direction:" << this << "\n";
		std::cout << "Deleting montecarlo_transient_simulation_results\n";
	#endif
	deleteContentsOfVectorOfPointers( montecarlo_transient_simulation_results );
}

void MontecarloStandardSimulation::RunSpectreSimulation( ){
	// standard comprobations
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSpectreSimulation had not been previously set up. ");
		return;
	}
	if (simulation_parameters==nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters is nullptr. ");
		return;
	}
	// Register Parameters
	montecarlo_simulation_results.set_full_id( simulation_id );
	montecarlo_simulation_results.RegisterSimulationParameters(simulation_parameters);
	// Environment variables
	ConfigureEnvironmentVariables();
	ShowEnvironmentVariables();
	// Parameters file
	if( !ExportParametersCircuit( folder, 0 )){
		log_io->ReportError2AllLogs( "Error creating parameters Circuit ");
		return;
	}
	montecarlo_simulation_results.set_spectre_result( RunSpectre( simulation_id ) );
	// magnitudes are not processed.
	// fgarcia: important, to be handled later:
	// 	InterpolateAndAnalyzeMagnitudes
	// 	PlotTransient
	// 	ManageIndividualResultFiles
	// 	deleteContentsOfVectorOfPointers( *analyzedMagnitudes);
}

bool MontecarloStandardSimulation::ProcessAndAnalyzeMontecarloTransients(){
	bool partialResult = true;
	// create metric magnitudes structure
	auto metricMagnitudes = golden_magnitudes_structure->GetMetricMagnitudesVector(0);
	unsigned int upsetsCount = 0;
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
	// reserve memory
	montecarlo_transient_simulation_results.reserve(montecarlo_iterations);
	// analyze transients
	unsigned int montecarloCountEnd = kMontecarloCountStart + montecarlo_iterations;
	// mcCount starts at 0, montecarloCount may not
	unsigned int mcCount = 0;
	for( unsigned int montecarloCount=kMontecarloCountStart; montecarloCount<montecarloCountEnd; ++montecarloCount ){
		// TransientSimulationResults is NOT stored for later use
		TransientSimulationResults* individualTransientResults = CreateTransientSimulationResults( montecarloCount );
		// Set up magnitudes, each Rfp needs an individual vector
		std::vector<Magnitude*>* analyzedMagnitudes = CreateMagnitudesVectorFromGoldenMagnitudes( n_d_profile_index );
		partialResult = partialResult && ProcessInterpolateAndAnalyzeIndividualSpectreTransientResults(
			*individualTransientResults, *analyzedMagnitudes);
		// compute statistics
		if( individualTransientResults->get_reliability_result()!=kScenarioNotSensitive ){
			++upsetsCount;
		}
		unsigned int magCount = 0;
		for( auto const &me : *(individualTransientResults->get_magnitudes_errors()) ){
			if( me->get_max_abs_error_global()>maxErrorGlobal[magCount] ){
				maxErrorGlobal[magCount] = me->get_max_abs_error_global();
			}
			meanMaxErrorMetric[magCount] += me->get_max_abs_error();
			errorData[magCount][mcCount] = me->get_max_abs_error();
			++magCount;
		}
		montecarlo_transient_simulation_results.push_back( individualTransientResults );
		// delete  analyzedMagnitudes;
		delete analyzedMagnitudes;
		// update counter
		++mcCount;
	}
	// compute mean
	for( unsigned int m=0; m<metricMagnitudes->size(); ++m){
		meanMaxErrorMetric[m] = meanMaxErrorMetric[m]/((double) metricMagnitudes->size());
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
	montecarlo_simulation_results.set_upsets_count(upsetsCount);
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

		#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportGreenStandard( simulation_id +  "-> max_error_global:" + number2String(maxErrorGlobal[magCount])
			+ " mean_max_error_metric:" + number2String(meanMaxErrorMetric[magCount])
			+ " b max_error_global:" + number2String(mMCR->max_error_global)
			+ " b mean_max_error_metric:" + number2String(mMCR->mean_max_error_metric)  );
		#endif
		++magCount;
	}
	// not needed
	// delete aux variables errorData
	// for(unsigned int i = 0; i<metricMagnitudes->size();++i){
	// 	delete[] errorData[i];
	// }
	// not reaquired
	// delete[] maxErrorGlobal;
	// delete[] maxErrorMetric;
	// delete[] meanMaxErrorMetric;
	// delete[] errorData;
	// return

	// plot scatters
	if( plot_scatters ){
		partialResult = partialResult && PlotProfileResults();
	}
	// manage individual results file
	for( auto const &itr : montecarlo_transient_simulation_results ){
		if( !ManageIndividualResultFiles( *itr, false ) ){
			log_io->ReportError2AllLogs( "Error deleting raw data. Scenario #" + itr->get_full_id() );
		}
	}
	return partialResult;
}

bool MontecarloStandardSimulation::ProcessInterpolateAndAnalyzeIndividualSpectreTransientResults(
	TransientSimulationResults& individualTransientResults, std::vector<Magnitude*>& analyzedMagnitudes ){
	// Raw Format Processor stores each sweep analysis results in the sweep_simulation magnitudes
	RAWFormatProcessor rfp;
	rfp.set_format( kGnuPlot );
	rfp.set_magnitudes( &analyzedMagnitudes );
	rfp.set_transient_file_path( individualTransientResults.get_original_file_path() );
	rfp.set_processed_file_path( individualTransientResults.get_processed_file_path() ) ;
	rfp.set_export_processed_magnitudes( export_processed_magnitudes );
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlain2Log( k3Tab + "#" + simulation_id + " scenario: processing spectre output data. Scenario #"
			+ individualTransientResults.get_original_file_path() );
		log_io->ReportPlain2Log( k3Tab + "#" + simulation_id + " scenario: processed spectre output data. Scenario #"
			+ individualTransientResults.get_processed_file_path() );
	#endif
	correctly_processed = rfp.ProcessPSFASCIIUnSorted();
	if( !correctly_processed ){
		log_io->ReportError2AllLogs("ProcessPSFASCIIUnSorted failed.");
		return false;
	}
	// Register parameters
	individualTransientResults.RegisterSimulationParameters( simulation_parameters );
	// plot transients
	if( export_processed_magnitudes && plot_transients ){
		int transGnuplot = CreateGnuplotTransientImages( individualTransientResults.get_full_id(),
			individualTransientResults, n_d_profile_index, true, individualTransientResults.get_full_id() );
		if( transGnuplot > 0 ){
			log_io->ReportError2AllLogs( "Unexpected gnuplot result: " + number2String(transGnuplot) );
			return false;
		}
	}
	// interpolate and analyze magnitudes
	if( !InterpolateAndAnalyzeMagnitudes( individualTransientResults, analyzedMagnitudes,
			n_d_profile_index, individualTransientResults.get_full_id() ) ){
		log_io->ReportError2AllLogs( "Error while interpolating the magnitudes. Scenario #"  + simulation_id );
		return false;
	}
	// delete files if needed in a later stage,
	// after PlotProfileResults has been called
	return true;
}

TransientSimulationResults* MontecarloStandardSimulation::CreateTransientSimulationResults(
	const unsigned int montecarloCount ){
	// Creates transient results for each motnecarlo thread
	TransientSimulationResults* individualTransientResults = new TransientSimulationResults();
	individualTransientResults->set_full_id( simulation_id + "_m_" + number2String(montecarloCount) );
	std::stringstream ssMontecarlo;
	// string formating matching Cadence Spectre format
	if( montecarloCount<1000 ){
		ssMontecarlo.fill( '0' );
		ssMontecarlo << std::setw(3) << montecarloCount;
	}else{
		ssMontecarlo << montecarloCount;
	}
	// spectre results file
	std::string spectreResultsFilePath = folder + kFolderSeparator + kSpectreResultsFolder
		+ kFolderSeparator + main_analysis->get_name() + "-" + ssMontecarlo.str() + "_"
		+ main_transient_analysis->get_name() + kTransientSufix;
	// processed file
	std::string	processedResultsFilePath = top_folder + kFolderSeparator + kResultsFolder + kFolderSeparator
		+ kResultsDataFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ kProcessedPrefix + simulation_id + "_m_" + ssMontecarlo.str() + kDataSufix;
	individualTransientResults->set_original_file_path( spectreResultsFilePath );
	individualTransientResults->set_processed_file_path( processedResultsFilePath );
	return individualTransientResults;
}

bool MontecarloStandardSimulation::TestSetUp(){
	if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder ");
		return false;
	}else if( golden_magnitudes_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_magnitudes_structure");
		return false;
	}
	return true;
}

int MontecarloStandardSimulation::RunSpectre( std::string scenarioId ){
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

bool MontecarloStandardSimulation::PlotProfileResults(){
	bool partialResults = true;
	std::string goldenFilePath  = golden_magnitudes_structure->GetFilePath( n_d_profile_index );
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
			gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
			gnuplotScriptFile << "set title \" " << title << " \"\n";
			gnuplotScriptFile << "set grid\n";
			// Axis
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set xlabel \""  << "Profile" << "\"\n";
			gnuplotScriptFile << "set ylabel \" " << mmr->metric_magnitude_name << " \"\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1\n";
			gnuplotScriptFile << "set border 3 back ls 11\n";
			gnuplotScriptFile << "set tics nomirror\n";
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette << "\n";
			// line style
			gnuplotScriptFile << "set style line 1 lc rgb '#5e9c36' pt 6 ps 1 lt 1 lw 2 # --- green\n";
			gnuplotScriptFile << "set style fill solid\n";
			// // Background
			gnuplotScriptFile << kWholeBackground << "\n";
			// legend
			gnuplotScriptFile << "set key bottom right\n";
			// Plot
			gnuplotScriptFile << "plot '" << goldenFilePath << "' u 1:" << number2String(magResultIndex) << " w l ls 1 title 'Golden', \\\n";
			unsigned int irCount = 2;
			for( auto const &ir : montecarlo_transient_simulation_results ){
				gnuplotScriptFile << " '" << ir->get_processed_file_path() << "' u 1:" << number2String(magResultIndex)
					<< " w p ls " << number2String(irCount++) << " notitle, \\\n";
			}
			gnuplotScriptFile << "\n";
			gnuplotScriptFile << "ntics = 15\n";
			gnuplotScriptFile << "set xtics ntics\n";
			gnuplotScriptFile << "unset output\n";
			// close file
			gnuplotScriptFile << "quit\n";
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
