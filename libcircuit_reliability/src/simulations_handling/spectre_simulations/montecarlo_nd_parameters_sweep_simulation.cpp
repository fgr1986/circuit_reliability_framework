/**
* @file montecarlor_ND_parameter_sweep_simulation.hpp
*
* @date Created on: March 27, 2014
*
* @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
*
* @section DESCRIPTION
*
* This Class is derived from SpectreSimulation
* and is responsible for the simple sweep simulation mode.
*
*/

// math
#include <cmath>
// Boost
// #include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "montecarlo_nd_parameters_sweep_simulation.hpp"
#include "../../io_handling/results_processor.hpp"
// constants
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

MontecarloNDParametersSweepSimulation::MontecarloNDParametersSweepSimulation() {
	this->max_parallel_profile_instances = 5;
	this->plot_last_transients = false;
	// injection mode related
	this->has_additional_injection = false;
}

MontecarloNDParametersSweepSimulation::~MontecarloNDParametersSweepSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "MontecarloNDParametersSweepSimulation destructor. direction:" << this << "\n";
	#endif
	// deleteContentsOfVectorOfPointers( montecarlo_standard_simulations_vector );
	// altered parameters have already been destroyed in each montecarlo_standard_simulation destructor
	// deleteContentsOfVectorOfPointers( altered_parameters_vector );
}

void MontecarloNDParametersSweepSimulation::RunSimulation( ){
	log_io->ReportPurpleStandard( "Running montecarlo_ND_parameters_sweep_simulation" );
	if (!TestSetUp()){
		log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		return;
	}
	log_io->ReportPurpleStandard( "Updating MC NumRuns:" );
	// Update numruns in MC analyses
	auto numRunParameter = new SimulationParameter( kMCNumRunsParamName, kMCIterationsParamName,
		true, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt, kNotDefinedInt );
	AddAdditionalSimulationParameter( numRunParameter );
	log_io->ReportPurpleStandard( "MC numruns set to:" + numRunParameter->get_value() );

	boost::thread_group mainTG;
	// params to be sweeped
	std::vector<SimulationParameter*> parameters2sweep;
	// Threads Creation
	for( auto const &p : *simulation_parameters ){
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
	// Environment: maybe ConfigureEnvironmentVariables is required. see critical_parameter_ND_....
	log_io->ReportThread( "Total threads to be simulated: " + number2String(totalThreads)
		+ ". Max number of concurrent profile threads: " + number2String(max_parallel_profile_instances)
		+ ". Max number of concurrent montecarlo threads: " + number2String(max_parallel_montecarlo_instances), 1 );
	// parallel threads control
	unsigned int runningThreads = 0;
	unsigned int threadsCount = 0;
	// current parameter sweep indexes
	std::vector<unsigned int> parameterCountIndexes(parameters2sweep.size(), 0);
	montecarlo_standard_simulations_vector.set_group_name("montecarlo_simulations_vector");
	montecarlo_standard_simulations_vector.ReserveSimulationsInMemory( totalThreads );
	while( threadsCount<totalThreads ){
		// wait for resources
		WaitForResources( runningThreads, max_parallel_profile_instances, mainTG, threadsCount );
		// CreateProfile sets all parameter values, and after the simulation object
		// is created it can be updated.
		// Thus, it avoids race conditions when updating parameterCountIndexes and parameters2sweep
		MontecarloSimulation* pMSS = CreateProfile(parameterCountIndexes, parameters2sweep, threadsCount);
		if( pMSS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(threadsCount) );
			correctly_simulated = false;
			correctly_processed = false;
			return;
		}
		montecarlo_standard_simulations_vector.AddSpectreSimulation( pMSS );
		mainTG.add_thread( new boost::thread(boost::bind(&MontecarloSimulation::RunSimulation, pMSS)) );
		// update variables
		UpdateParameterSweepIndexes( parameterCountIndexes, parameters2sweep);
		++threadsCount;
		++runningThreads;
	}
	mainTG.join_all();
	// check if every simulation ended correctly
	correctly_simulated = montecarlo_standard_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = montecarlo_standard_simulations_vector.CheckCorrectlyProcessed();
	// generate map files
	log_io->ReportPlainStandard( k2Tab + "Generating Map files.");
	GenerateAndPlotResults( parameters2sweep );
	// end
	log_io->ReportPlain2Log( "END OF MontecarloNDParametersSweepSimulation::RunSimulation" );
}

bool MontecarloNDParametersSweepSimulation::TestSetUp(){
	if( simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr simulation_parameters in montecarlo_ND_parameters_sweep_simulation");
		return false;
	}else if(folder.compare("")==0){
		log_io->ReportError2AllLogs( "nullptr folder  in montecarlo_ND_parameters_sweep_simulation");
		return false;
	}else if( golden_metrics_structure== nullptr){
		log_io->ReportError2AllLogs( "nullptr golden_metrics_structure in montecarlo_ND_parameters_sweep_simulation");
		return false;
	}
	return true;
}

MontecarloSimulation* MontecarloNDParametersSweepSimulation::CreateProfile(
	const std::vector<unsigned int>& parameterCountIndexes,
	std::vector<SimulationParameter*>& parameters2sweep, const unsigned int threadNumber ){
	// Create folder
	std::string s_threadNumber = number2String(threadNumber);
	std::string currentFolder = folder + kFolderSeparator
		 + "param_profile_" + s_threadNumber;
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
	MontecarloSimulation* pMSS = CreateMontecarloSimulation( currentFolder, parameterCountIndexes, parameters2sweep, threadNumber );
	if( pMSS==nullptr ){
		log_io->ReportError2AllLogs( "pMSS is nullptr" );
		return nullptr;
	}
	return pMSS;
}

MontecarloSimulation* MontecarloNDParametersSweepSimulation::CreateMontecarloSimulation(
		const std::string currentFolder, const std::vector<unsigned int>& parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int threadNumber  ){
	std::string s_threadNumber = number2String(threadNumber);
	// Simulation
	MontecarloSimulation* pMSS = new MontecarloSimulation();
	pMSS->set_has_additional_injection( has_additional_injection );
	pMSS->set_n_dimensional(true);
	pMSS->set_n_d_profile_index(threadNumber);
	pMSS->set_is_nested_simulation( true );
	pMSS->set_montecarlo_iterations( montecarlo_iterations );
	pMSS->set_max_parallel_montecarlo_instances( max_parallel_montecarlo_instances );
	pMSS->set_simulation_id(  simulation_id + "_child_prof_" + s_threadNumber );
	// pMSS->set_parameter_index( paramIndex );
	// pMSS->set_sweep_index( sweepIndex );
	pMSS->set_log_io( log_io );
	pMSS->set_altered_scenario_index( altered_scenario_index );
	pMSS->set_golden_metrics_structure( golden_metrics_structure );
	// Spectre command and args
	pMSS->set_spectre_command( spectre_command );
	pMSS->set_pre_spectre_command( pre_spectre_command );
	pMSS->set_post_spectre_command( post_spectre_command );
	pMSS->set_spectre_command_log_arg( spectre_command_log_arg );
	pMSS->set_spectre_command_folder_arg( spectre_command_folder_arg );
	// Spectre environment variables
	pMSS->set_ahdl_simdb_env( ahdl_simdb_env );
	pMSS->set_ahdl_shipdb_env( ahdl_shipdb_env );
	pMSS->set_top_folder( top_folder );
	pMSS->set_folder( currentFolder );
	pMSS->set_altered_statement_path( altered_statement_path );
	pMSS->set_delete_spectre_folders( delete_spectre_folders );
	pMSS->set_delete_spectre_transients( delete_spectre_transients );
	pMSS->set_delete_processed_transients( delete_processed_transients );
	pMSS->set_plot_scatters( plot_scatters );
	pMSS->set_plot_transients( plot_transients );
	// pMSS->set_plot_last_transients( plot_last_transients );
	pMSS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pMSS->set_main_analysis( main_analysis );
	pMSS->set_main_transient_analysis( main_transient_analysis );
	// fgarcia: false?, so we analyze later s_xxx_001.tran, s_xxx_002..tran... instead s_xxx.tran
	pMSS->set_process_metrics( false );
	pMSS->set_export_processed_metrics( export_processed_metrics );
	pMSS->set_export_metric_errors( export_metric_errors );
	// copy of simulation_parameters
	pMSS->CopySimulationParameters( *simulation_parameters );
	// no need to update golden parameter
	// update parameter values
	unsigned int sweepedParamIndex = 0;
	for( auto const &p : parameters2sweep ){
		if( !pMSS->UpdateParameterValue( *p, number2String( p->GetSweepValue( parameterCountIndexes.at(sweepedParamIndex)))) ){
			log_io->ReportError2AllLogs( p->get_name() + " not found in MontecarloSimulation and could not be updated." );
		}
		++sweepedParamIndex;
	}
	return pMSS;
}

bool MontecarloNDParametersSweepSimulation::GenerateAndPlotResults(
	const std::vector<SimulationParameter*>& parameters2sweep  ){
	// a) General Results
	// 1 file: upsets ratio at each profile
	// N files, one per metric/metric, including meanMaxError, maxError, etc
	std::string mapsFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
	std::string gnuplotScriptFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
	std::string imagesFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator + kMontecarloNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
	if( !CreateFolder(mapsFolder, true ) || !CreateFolder(imagesFolder, true ) || !CreateFolder(gnuplotScriptFolder, true ) ){
		 log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
		 + mapsFolder + " and " + imagesFolder + "'." );
		 log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
		 return false;
	}
	// aux mags
	auto auxMetrics = golden_metrics_structure->GetMetricsVector( 0 );
	if( auxMetrics==nullptr ){
		log_io->ReportError2AllLogs("auxMetrics is null");
		return false;
	}
	unsigned int totalAnalizableMetrics = 0;
	for( auto const &m : *auxMetrics ){
		if( m->get_analyzable() ){
			++totalAnalizableMetrics;
		}
	}
	bool partialResults = true;
	partialResults = partialResults && GenerateAndPlotGeneralResults( *auxMetrics,
		parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder );
	if( !partialResults ){
		log_io->ReportError2AllLogs( "Unexpected error in GenerateAndPlotGeneralResults" );
		return false;
	}
	// b) Profile Scatters
	// scatter of each profile already plotted in MontecarloSimulation
	// c) 3d plots
	// p1 vs p2 3d plots
	// std::vector<std::tuple<int,int>>
	std::set<std::pair<unsigned int,unsigned int>> exportedParamTuples;
	// reserve memory
	main_nd_simulation_results.ReservePlanesInMemory( CountInvolvedPlanes(parameters2sweep) );
	// init all metric column_indexes
	if( !InitMetricColumnIndexes(*auxMetrics) ){
		log_io->ReportError2AllLogs( "Unexpected error in InitMetricColumnIndexes" );
		return false;
	}
	unsigned int p1Index = 0;
	std::string planesMapsFolder;
	std::string planesGnuplotScriptFolder;
	std::string planesImagesFolder;
	for( auto const &p1 : parameters2sweep ){
		unsigned int p2Index = 0;
		for( auto const &p2 : parameters2sweep ){
			auto auxPair1 = std::make_pair( p1Index, p2Index);
			auto auxPair2 = std::make_pair( p2Index, p1Index);
			if( p1Index!=p2Index &&
				exportedParamTuples.end()==exportedParamTuples.find( auxPair1 ) &&
				exportedParamTuples.end()==exportedParamTuples.find( auxPair2 ) ){
				// generate partial map and results
				#ifdef RESULTS_POST_PROCESSING_VERBOSE
				log_io->ReportPlainStandard("Exporting map: " + p1->get_name() + "-" + p2->get_name() );
				#endif
				planesMapsFolder.clear();
				planesGnuplotScriptFolder.clear();
				planesImagesFolder.clear();
				planesMapsFolder =  mapsFolder + kFolderSeparator + p1->get_file_name() + "_" + p2->get_file_name();
				planesGnuplotScriptFolder =  gnuplotScriptFolder + kFolderSeparator + p1->get_file_name() + "_" + p2->get_file_name();
				planesImagesFolder = imagesFolder + kFolderSeparator + p1->get_file_name() + "_" + p2->get_file_name();
				if( !CreateFolder(planesMapsFolder, true ) || !CreateFolder(planesImagesFolder, true ) || !CreateFolder(planesGnuplotScriptFolder, true ) ){
					log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '" + planesMapsFolder + " and " + planesImagesFolder + "'." );
					log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
					return false;
				}
				partialResults = partialResults && GenerateAndPlotParameterPairResults(
					*auxMetrics, totalAnalizableMetrics,
					p1Index, p2Index, parameters2sweep, planesMapsFolder, planesGnuplotScriptFolder, planesImagesFolder );
				// add parameters to set
				exportedParamTuples.insert(auxPair1);
			}
			++p2Index;
		}
		++p1Index;
	} // end of p1 vs p2 3d plot
	return partialResults;
}

bool MontecarloNDParametersSweepSimulation::InitMetricColumnIndexes(
		const std::vector<Metric*>& auxMetrics ){
	// partial plane inputs

	// (0) p1 p2 Upset_Ratio
	// (3 + 0) metric_name  max_error_metric min_error_metric mean_max_error_metric"
	// (3 + 4) median_max_error_metric  q12_max_error_metric q34_max_error_metric "
	// (3 + 7) max_max_error_global min_max_error_global mean_max_error_global"
	// if oceanEvalMetric
	// (3 + 10) METRIC_MAX_VAL METRIC_MIN_VAL METRIC_MEAN_VAL\n";
	p_p_c_i_statistic_2be_processed = {2}; // upsets
	p_p_c_i_max = {};
	p_p_c_i_min = {};
	p_p_c_i_mean = {2};
	unsigned int metricCount = 3; // MAG_i_name in input
	for( auto const &m: auxMetrics ){
		if( m->get_analyzable() ){
			// for use inside MontecarloCriticalParameterNDParametersSweepSimulation
			p_p_c_i_max.push_back( metricCount+1 );
			p_p_c_i_min.push_back( metricCount+2 );
			p_p_c_i_mean.push_back( metricCount+3 ); // metric
			p_p_c_i_max.push_back( metricCount+7 );
			p_p_c_i_min.push_back( metricCount+8 );
			p_p_c_i_mean.push_back( metricCount+9 ); // global
			if( !m->is_transient_magnitude() ){
				p_p_c_i_max.push_back( metricCount+10 );
				p_p_c_i_min.push_back( metricCount+11 );
				p_p_c_i_mean.push_back( metricCount+12 ); // oceanEvalMetric
				metricCount += 3;
			}
			// update counters
			metricCount += 10; // next MAG_i_name in input
		}
	}
	return true;
}

bool MontecarloNDParametersSweepSimulation::GenerateAndPlotParameterPairResults(
		const std::vector<Metric*>& auxMetrics, const unsigned int& totalAnalizableMetrics,
		const unsigned int& p1Index, const unsigned int& p2Index,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	#ifdef RESULTS_ANALYSIS_VERBOSE
	log_io->ReportPlainStandard("Creating maps for params: " + parameters2sweep.at(p1Index)->get_name()
		+ "-" + parameters2sweep.at(p2Index)->get_name() );
	#endif
	bool partialResults = true;
	auto planes = GetPlanesForParams( p1Index, p2Index, parameters2sweep);
	unsigned int itemizedCount = 0;
	PlaneResultsStructure* planeStructure = new PlaneResultsStructure();
	// resize and reserve memory
	planeStructure->ResizeItemizedPlanesMemory( planes->size() );
	planeStructure->set_plane_id( number2String(p1Index) + "_" + number2String(p2Index) );
	for( auto& planeIndexes : *planes ){
		partialResults = partialResults && GenerateAndPlotItemizedPlane(
			auxMetrics, p1Index, p2Index, itemizedCount++,
			parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder, *planeIndexes, *planeStructure);
	}
	// add plane
	main_nd_simulation_results.AddPlaneResultsStructure( planeStructure );
	log_io->ReportPlainStandard( "#" + simulation_id + ", Processing itemized results with ResultsProcessor" );
	// Process itemized planes to extract the p1-p2 general plane results
	ResultsProcessor rp;
	std::string generalParameterResultsFile = mapsFolder + kFolderSeparator
		+ planeStructure->get_plane_id() + "_general" + kDataSufix;
	// process only mean
	// fgarcia
	partialResults = partialResults && rp.StatisticProcessStatisticsFiles(
		planeStructure->get_itemized_data_paths(), generalParameterResultsFile,
		std::move(p_p_c_i_max), std::move(p_p_c_i_min), std::move(p_p_c_i_mean));
	if( !partialResults ){
		log_io->ReportError2AllLogs( "[ERROR ResultsProcessor] Error processing " + generalParameterResultsFile );
		return partialResults;
	}
	planeStructure->set_general_data_path(generalParameterResultsFile);
	// plot upsets and metric errors
	SimulationParameter* p1 = parameters2sweep.at(p1Index);
	SimulationParameter* p2 = parameters2sweep.at(p2Index);
	log_io->ReportPlainStandard("Processed GnuplotUpsetsPlane" );
	int gnuplotResult = GnuplotUpsetsPlane( *planeStructure, false, *p1, *p2,
		planeStructure->get_plane_id() + "_general_", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	gnuplotResult += GnuplotPlaneMetricResults( auxMetrics, *planeStructure,
		false, *p1, *p2, 0, "_general_", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	if( gnuplotResult > 0 ){
		log_io->ReportError2AllLogs( "Sim " + simulation_id + ".Unexpected gnuplot result: " + number2String(gnuplotResult) );
	}
	// clean
	deletePlaneProfileIndexesStructure( *planes );
	return partialResults;
}

bool MontecarloNDParametersSweepSimulation::GenerateAndPlotItemizedPlane(
	const std::vector<Metric*>& auxMetrics,
	const unsigned int& p1Index, const unsigned int& p2Index, const unsigned int& itemizedCount,
	const std::vector<SimulationParameter*>& parameters2sweep,
	const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder,
	const std::vector<unsigned int>& profileIndexesInPlane, PlaneResultsStructure& plane ){
	bool partialResults = true;
	// auxiliar indexes
	std::vector<unsigned int> auxiliarIndexes(parameters2sweep.size(), 0);
	SimulationParameter* p1 = parameters2sweep.at(p1Index);
	SimulationParameter* p2 = parameters2sweep.at(p2Index);
	std::string partialPlaneId = p1->get_file_name() + "_" + p2->get_file_name() + "_" + number2String(itemizedCount);
	// General profiles one
	std::string gnuplotMapFilePath = mapsFolder + kFolderSeparator
		 + simulation_id + "_" + partialPlaneId + kDataSufix;
	// export file
	std::ofstream gnuplotMapFile;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotMapFile << "#" << p1->get_name() << " " << p2->get_name() << " Upset_Ratio"
			<< "\n# metric_name  max_error_metric min_error_metric mean_max_error_metric"
			<< "\n# median_max_error_metric  q12_max_error_metric q34_max_error_metric "
			<< "\n# max_max_error_global min_max_error_global mean_max_error_global";
		gnuplotMapFile << "\n# if oceanEvalMetric METRIC_MAX_VAL METRIC_MIN_VAL METRIC_MEAN_VAL\n";
		unsigned int profileCount = 0;
		unsigned int p1SweepCount = 0;
		unsigned int p2SweepCount = 0;
		for( auto& simulation : *(montecarlo_standard_simulations_vector.get_spectre_simulations()) ){
			if( vectorContains(profileIndexesInPlane, profileCount) ){
				MontecarloSimulation* mcSSim = dynamic_cast<MontecarloSimulation*>(simulation);
				if( p2SweepCount==p2->get_sweep_steps_number() ){
					p2SweepCount = 0;
					++p1SweepCount;
					gnuplotMapFile << "\n";
				}
				gnuplotMapFile << p1->GetSweepValue(p1SweepCount) << " " << p2->GetSweepValue(p2SweepCount++)
					<< " " << (double)(mcSSim->get_montecarlo_simulation_results()->get_upsets_count()/((double)montecarlo_iterations));
				// metrics
				for( auto& mMCr : *(mcSSim->get_montecarlo_simulation_results()->get_metric_montecarlo_results()) ){
					gnuplotMapFile << " " << mMCr->metric_name << " " << mMCr->max_error_metric << " " << mMCr->min_error_metric
						<< " " << mMCr->mean_max_error_metric << " " << mMCr->median_max_error_metric << " " << mMCr->q12_max_error_metric
						<< " " << mMCr->q34_max_error_metric << " " << mMCr->max_max_error_global
						<< " " << mMCr->min_max_error_global<< " " << mMCr->mean_max_error_global;
						// ocean_eval_metric
						if( !mMCr->transient_magnitude ){
							gnuplotMapFile << " " << mMCr->ocean_eval_metric_max_val
								<< " " << mMCr->ocean_eval_metric_min_val << " " << mMCr->ocean_eval_metric_mean_val;
						}
				}
				gnuplotMapFile << "\n";
			}// update counters
			++profileCount;
			UpdateParameterSweepIndexes( auxiliarIndexes, parameters2sweep );
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		partialResults = false;
	}
	// close file
	gnuplotMapFile.close();
	// add data path
	plane.AddItemizedDataPath( gnuplotMapFilePath, partialPlaneId );
	// create gnuplot scatter map graphs
	if( partialResults ){
		// create gnuplot scatter map graphs
		int gnuplotResult = GnuplotUpsetsPlane( plane, true, *p1, *p2,
			"partialPlaneId_" + partialPlaneId, gnuplotMapFilePath,
			gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotPlaneMetricResults( auxMetrics, plane, true,
			*p1, *p2, itemizedCount, "partialPlaneId_" + partialPlaneId,
			gnuplotMapFilePath, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

bool MontecarloNDParametersSweepSimulation::GenerateAndPlotGeneralResults(
		const std::vector<Metric*>& auxMetrics,
		const std::vector< SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	bool partialResults = true;
	// auxiliar indexes
	std::vector<unsigned int> auxiliarIndexes(parameters2sweep.size(), 0);
	// General profiles one
	std::string gnuplotMapFilePath = mapsFolder + kFolderSeparator + "general_s_" + simulation_id + kDataSufix;
	std::string gnuplotSpectreErrorMapFilePath = mapsFolder + kFolderSeparator + "general_spectre_error_s_" + simulation_id + kDataSufix;
	std::ofstream gnuplotMapFile;
	std::ofstream gnuplotSpectreErrorMapFile;
	double maxUpsetRatio = 0;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotSpectreErrorMapFile.open( gnuplotSpectreErrorMapFilePath.c_str() );
		gnuplotMapFile << "#profileCount #Profile Upset_Ratio"
			<< "\n# metric_name  max_error_metric min_error_metric mean_max_error_metric"
			<< "\n# median_max_error_metric  q12_max_error_metric q34_max_error_metric "
			<< "\n# max_max_error_global min_max_error_global mean_max_error_global";
		gnuplotMapFile << "\n# if oceanEvalMetric METRIC_MAX_VAL METRIC_MIN_VAL METRIC_MEAN_VAL\n";
		gnuplotSpectreErrorMapFile << "#profileCount #Profile SpectreError \n";
		// scientific format
		gnuplotMapFile << std::scientific;
		unsigned int profileCount = 0;
		bool severalSweepParameter = parameters2sweep.size()>1;
		auto sweepParameter = parameters2sweep.at(0);
		for( auto const &simulation : *(montecarlo_standard_simulations_vector.get_spectre_simulations()) ){
			MontecarloSimulation* mcSSim = dynamic_cast<MontecarloSimulation*>(simulation);
			std::string auxIndexes = getIndexCode( auxiliarIndexes );
			std::string auxSpectreError = mcSSim->get_correctly_simulated() ? "0" : "1";
			double upsetsRatio = (double) 100*(mcSSim->get_montecarlo_simulation_results()->get_upsets_count())/((double)montecarlo_iterations);
			gnuplotMapFile << std::defaultfloat << ( severalSweepParameter ? profileCount : sweepParameter->GetSweepValue(profileCount) )
				<< " " << auxIndexes << " " << upsetsRatio;
			// update maxUpsetRatio
			maxUpsetRatio = upsetsRatio>maxUpsetRatio ? upsetsRatio : maxUpsetRatio;
			// metrics
			for( auto const &mMCr : *(mcSSim->get_montecarlo_simulation_results()->get_metric_montecarlo_results()) ){
				gnuplotMapFile << " " << mMCr->metric_name << " " << mMCr->max_error_metric << " " << mMCr->min_error_metric
					<< " " << mMCr->mean_max_error_metric << " " << mMCr->median_max_error_metric << " " << mMCr->q12_max_error_metric
					<< " " << mMCr->q34_max_error_metric << " " << mMCr->max_max_error_global
					<< " " << mMCr->min_max_error_global<< " " << mMCr->mean_max_error_global;
					// ocean_eval_metric
					if( !mMCr->transient_magnitude ){
						gnuplotMapFile << " " << mMCr->ocean_eval_metric_max_val
							<< " " << mMCr->ocean_eval_metric_min_val << " " << mMCr->ocean_eval_metric_mean_val;
					}
			}
			gnuplotMapFile << "\n";
			gnuplotSpectreErrorMapFile << profileCount++ << " " << auxIndexes << " " << auxSpectreError << "\n";
			// update counters
			UpdateParameterSweepIndexes( auxiliarIndexes, parameters2sweep );
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		partialResults = false;
	}
	// close files
	gnuplotMapFile.close();
	gnuplotSpectreErrorMapFile.close();
	main_nd_simulation_results.set_general_data_path(gnuplotMapFilePath);
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
	log_io->ReportPlainStandard( k2Tab + "gnuplotMapFile Created" );
	#endif
	if( partialResults ){
		// create gnuplot scatter map graphs
		int gnuplotResult = GnuplotUpsetsGeneralResults( gnuplotSpectreErrorMapFilePath, gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotGeneralMetricResults( auxMetrics,
			maxUpsetRatio, mapsFolder, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

int MontecarloNDParametersSweepSimulation::GnuplotUpsetsGeneralResults(
	const std::string& gnuplotSpectreErrorMapFilePath,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
	log_io->ReportPlainStandard( k2Tab + "Plot with GnuplotUpsetsGeneralResults" );
	#endif
	// title
	std::string title = "[General] Upset Ratio, " +  simulation_id +  " general_profiles";
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + simulation_id + "_general" + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + simulation_id + "_general" + kSvgSufix;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	// Svg
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
		 << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
	gnuplotScriptFile << "set title \" " << title << " \"\n";
	gnuplotScriptFile << "set grid\n";
	// Axis
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";
	gnuplotScriptFile << "set xlabel \""  << "Profile" << "\"\n";
	gnuplotScriptFile << "set ylabel \" Upset Ratio \"\n";
	gnuplotScriptFile << "set y2label \"Spectre error\"\n";

	// # remove border on top and right and set color to gray
	gnuplotScriptFile << kCustomBorders;
	// line style
	gnuplotScriptFile <<  kTransientSimilarLinesPalette;
	// legend
	gnuplotScriptFile <<  "set key bottom right\n";
	gnuplotScriptFile <<  "ntics = 10\n";
	// Plot
	// add xtics with xtic(2), u 1:3:xtic(2)
	if( correctly_simulated ){
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:3 axes x1y1 w lp ls 2 title 'Upset Ratio'\n";

	}else{
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:3 axes x1y1 w lp ls 2 title 'Upset Ratio', \\"
		<< "\n";
		gnuplotScriptFile <<  " '" << gnuplotSpectreErrorMapFilePath << "' u 1:3 axes x1y2 with boxes ls 1 notitle\n";
	}
	gnuplotScriptFile <<  "set xtics ntics\n";

	gnuplotScriptFile << "unset output\n";

	// close file
	gnuplotScriptFile << "quit\n";
	gnuplotScriptFile.close();
	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	// Image paths
	main_nd_simulation_results.set_general_image_path( outputImagePath );
	return std::system( execCommand.c_str() );
}

int MontecarloNDParametersSweepSimulation::GnuplotGeneralMetricResults(
	const std::vector<Metric*>& analyzedMetrics, double& maxUpsetRatio,
	const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){

	int partialResult = 0;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	// mag count indexes
	unsigned int metricCount = 0;
	int magDataMetricIndex = in_profile_gnuplot_first_mag_metric_offset; // max
	int magDataGlobalIndex = 0; // max
	int magDataOveanEvalValIndex = 0; // max
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// update indexes
			magDataGlobalIndex = magDataMetricIndex + in_profile_gnuplot_partial_mag_global_offset; // max
			magDataOveanEvalValIndex = magDataMetricIndex + in_profile_gnuplot_partial_mag_ocean_eval_val_offset; // max
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				+ simulation_id + "_general_mag_" + number2String(metricCount) + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				+ simulation_id + "_general_mag_error_" + number2String(metricCount) + kSvgSufix;
			std::string title = "[General] " + m->get_title_name() + " errors & Upset Statistics";
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			// Svg
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
				<< " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
			gnuplotScriptFile << "set title \" " << title << " \"\n";
			gnuplotScriptFile << "set grid\n";
			// Axis
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set format y2 \"%g\"\n";
			gnuplotScriptFile << "set xlabel \"Profile\"\n";
			gnuplotScriptFile << "set y2label \"Upset \%\"\n";
			gnuplotScriptFile << "set ylabel \"Error in metric "  << m->get_title_name() << "\"\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << kCustomBorders << "\n";
			gnuplotScriptFile << "set y2tics\n";
			// palete range
			if( maxUpsetRatio < 1 ){
				maxUpsetRatio = 1;
				// Palete
				gnuplotScriptFile << kMinimalPalette;
				gnuplotScriptFile << "set y2range [0:1]\n";
				gnuplotScriptFile << "set cbrange [0:1]\n";
			}else{
				// Color Paletes
				gnuplotScriptFile << kUpsetsPalette;
				gnuplotScriptFile << "set y2range [0:"<< maxUpsetRatio << "]\n";
				gnuplotScriptFile << "set cbrange [0:"<< maxUpsetRatio << "]\n";
			}
			// line style
			gnuplotScriptFile <<  kProfilesPalette;
			// Plot upsets
			// # 1              2            3      4             5                6              7             8   9   10  11
			// # profCount profile %upsets MAG_i_name MAG_i_maxError MAG_i_minError MAG_i_mean MAG_i_median q12 q34 MAG_i_maxErrorGlobal
			gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:3 axis x1y2 with filledcurve x1 ls 6 title '\% Upsets', \\\n";
			// metric
			// old candlesticks  # Data columns: X Min 1stQuartile Median 3rdQuartile Max
			// gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+1)  <<  ":" << (magDataMetricIndex+4) << ":" << (magDataMetricIndex+3) << ":" << (magDataMetricIndex+5) << ":" << magDataMetricIndex << " axis x1y1  w candlesticks ls 1 notitle whiskerbars, \\\n";
			// new candlesticks  # Data columns: X BoxMin WMin WMax BoxMax
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+4)  <<  ":" << (magDataMetricIndex+1) << ":" << magDataMetricIndex << ":" << (magDataMetricIndex+5) << " axis x1y1  w candlesticks ls 1 notitle whiskerbars, \\\n";
			// mean in candlesticks
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+3)  <<  ":" << (magDataMetricIndex+3) << ":" << (magDataMetricIndex+3) << ":" << (magDataMetricIndex+3) << " axis x1y1  w candlesticks lt -1 lw 2 notitle, \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+2) << " axis x1y1  w lp ls 2 title '" << m->get_title_name() << " (max_error_metric)', \\\n";
			// global
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2)  <<  ":" << (magDataGlobalIndex+1) << ":" << magDataGlobalIndex << " axis x1y1  w errorbars ls 3 notitle, \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2) << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << " (max_error_global)'\n";
			//
			gnuplotScriptFile << " # Uncomment the following for ploting the median\n";
			gnuplotScriptFile <<  "#     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+3) << " axis x1y1  w lp ls 5 title 'median max_err_" << m->get_title_name() << "'\n";
			// legend
			gnuplotScriptFile <<  "set key top left\n";
			gnuplotScriptFile << "unset output\n";
			if( !m->is_transient_magnitude() ){
				title = "[General] " + m->get_title_name() + " errors & Upset Statistics";
				outputImagePath = imagesFolder + kFolderSeparator + simulation_id + "_general_mag_values_" + number2String(metricCount) + kSvgSufix;
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
				gnuplotScriptFile << "set title \" " << title << " \"\n";
				gnuplotScriptFile << "plot '" << gnuplotDataFile << "' using 1:3 axis x1y2 with filledcurve x1 ls 6 title '\% Upsets', \\\n";
				gnuplotScriptFile << "     '" << gnuplotDataFile << "' u 1:" << (magDataOveanEvalValIndex+2)  <<  ":" << (magDataOveanEvalValIndex+1) << ":" << magDataOveanEvalValIndex << " axis x1y1  w errorbars ls 3 notitle, \\\n";
				gnuplotScriptFile << "     '" << gnuplotDataFile << "' u 1:" << (magDataOveanEvalValIndex+2) << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << " '\n";
				// legend
				gnuplotScriptFile << "set key top left\n";
				gnuplotScriptFile << "unset output\n";
			}
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();
			// update counters
			if( m->is_transient_magnitude() ){
				magDataMetricIndex += data_per_magnitude_per_line; // max
			}else{
				magDataMetricIndex += data_per_ocean_eval_metric_per_line; // max
			}
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResult += std::system( execCommand.c_str() );
			// Image paths
			main_nd_simulation_results.AddGeneralMetricImagePath( outputImagePath , title );

			// Image paths
			// criticalParameterValueSimulationsVector.set_group_processed_image_path(  outputImagePath + kSvgSufix  );
			// criticalParameterValueSimulationsVector.set_group_processed_latex_image_path( outputImagePath+ kLatexOutputImagesSufix  );
			metricCount++;
		}
	}
	return partialResult;
}

int MontecarloNDParametersSweepSimulation::GnuplotUpsetsPlane(
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + simulation_id + "_" + partialPlaneId + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + simulation_id + "_" + partialPlaneId + "_3D" + kSvgSufix;
	std::string title = "Upset Ratio, " + partialPlaneId;
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath  << "\"\n";
	gnuplotScriptFile << "set grid\n";
	// Axix border
	gnuplotScriptFile << "# set border 4095\n";
	// Axis
	if( p1.get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale x\n";
	}
	if( p2.get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale y\n";
	}
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";
	gnuplotScriptFile << "set xlabel \""  << p1.get_title_name() << "\"\n";
	gnuplotScriptFile << "set ylabel \""  << p2.get_title_name() << "\"\n";
	gnuplotScriptFile << "set zlabel \"Upset Ratio\" rotate by 90\n";
	gnuplotScriptFile << "set title \"" << title << "\"\n";

	gnuplotScriptFile << kUpsetsPalette ;
	gnuplotScriptFile << k3DProperties;
	if( interpolate_plots_ratio >= 0 ){
		gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
	}
	gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:3 notitle " << kElegantLine3D << " w pm3d\n";
	gnuplotScriptFile << "unset output\n";
	// close file
	gnuplotScriptFile << "quit\n";
	gnuplotScriptFile.close();

	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	// Image paths
	if( isPartialPlane ){
		plane.AddItemizedImagePath( outputImagePath, title );
	}else{
		plane.set_general_image_path( outputImagePath );
	}
	return std::system( execCommand.c_str() );
}

int MontecarloNDParametersSweepSimulation::GnuplotPlaneMetricResults(
	const std::vector<Metric*>& analyzedMetrics,
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const unsigned int& partialPlaneCount,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	int partialResults = 0;
	// metrics counters
	unsigned int metricCount = 0;
	int magDataMetricIndex = in_profile_gnuplot_first_mag_metric_offset; // max
	int magDataGlobalIndex = 0; // max
	int magDataOveanEvalValIndex = 0; // max
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// update indexes
			magDataGlobalIndex = magDataMetricIndex + in_profile_gnuplot_partial_mag_global_offset; // max
			magDataOveanEvalValIndex = magDataMetricIndex + in_profile_gnuplot_partial_mag_ocean_eval_val_offset; // max
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				 + simulation_id + "_" + partialPlaneId + "_mag_" + number2String(metricCount) + "_" + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				 + simulation_id + "_" + partialPlaneId + "_mag_error_" + number2String(metricCount) + "_3D" + kSvgSufix;
			std::string title = m->get_title_name() + " (error), " + partialPlaneId;
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath  << "\"\n";
			gnuplotScriptFile << "set grid\n";
			// Axix border
			gnuplotScriptFile << "# set border 4095\n";
			// Axis
			if( p1.get_value_change_mode()!=kSPLineal ){
				gnuplotScriptFile << "set logscale x\n";
			}
			if( p2.get_value_change_mode()!=kSPLineal ){
				gnuplotScriptFile << "set logscale y\n";
			}
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set xlabel \"" << p1.get_title_name() << "\"\n";
			gnuplotScriptFile << "set ylabel \"" << p2.get_title_name() << "\"\n";
			gnuplotScriptFile << "set zlabel \"" << m->get_title_name() << "\" rotate by 90\n";
			// Format
			gnuplotScriptFile << "set format cb \"%g\"\n";
			gnuplotScriptFile << "set title \"" << title << " \"\n";
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette;
			gnuplotScriptFile << kTransientSimilarLinesPalette;
			gnuplotScriptFile << k3DProperties;
			if( interpolate_plots_ratio >= 0 ){
				gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
			}
			if( m->is_transient_magnitude() ){
				gnuplotScriptFile << "set multiplot layout 2, 1 \n";
			}
			gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataMetricIndex+1) << " notitle w lp ls 2, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << magDataMetricIndex << " notitle w lp ls 1, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataMetricIndex+2) << " title 'max_err_metric_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";
			if( m->is_transient_magnitude() ){
				gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex+1) << " notitle w lp ls 2, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex) << " notitle w lp ls 1, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex+2) << " title 'max_err_global_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";
				gnuplotScriptFile << "unset multiplot\n";
			}
			gnuplotScriptFile << "unset output\n";

			if( !m->is_transient_magnitude() ){
				title = "[General] " + m->get_title_name() + ", " + partialPlaneId;
				outputImagePath = imagesFolder + kFolderSeparator + simulation_id + "_" + partialPlaneId + "_mag_values_" + number2String(metricCount) + "_3D" + kSvgSufix;
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
				gnuplotScriptFile << "set title \" " << title << " \"\n";
				gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataOveanEvalValIndex+1) << " notitle w lp ls 2, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataOveanEvalValIndex) << " notitle w lp ls 1, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataOveanEvalValIndex+2) << " title 'max_err_global_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";
				// legend
				gnuplotScriptFile <<  "set key top left\n";
				gnuplotScriptFile << "unset output\n";
			}
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();
			// update counters
			if( m->is_transient_magnitude() ){
				magDataMetricIndex += data_per_magnitude_per_line; // max
			}else{
				magDataMetricIndex += data_per_ocean_eval_metric_per_line; // max
			}
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResults += std::system( execCommand.c_str() );
			// Image paths
			if( isPartialPlane ){
				plane.AddItemizedMetricImagePath( partialPlaneCount, outputImagePath, title );
			}else{
				plane.AddGeneralMetricImagePath( outputImagePath, title );
			}
			metricCount++;
		}
	}
	return partialResults;
}
