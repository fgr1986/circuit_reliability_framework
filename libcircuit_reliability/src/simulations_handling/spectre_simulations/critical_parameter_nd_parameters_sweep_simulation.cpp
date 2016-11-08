/**
* @file critical_parameter_nD_parameter_sweep_simulation.hpp
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
#include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "critical_parameter_nd_parameters_sweep_simulation.hpp"
#include "../../io_handling/results_processor.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
// in header file: #include "global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"
// #include "../../global_functions_and_constants/name_functions.hpp"

CriticalParameterNDParameterSweepSimulation::CriticalParameterNDParameterSweepSimulation() {
	this->critical_parameter_value = kNotDefinedInt;
	this->not_affected_by_max_value= false;
	this->affected_by_min_value= false;
	this->max_parallel_profile_instances = 15;
	this->plot_last_transients = false;
	this->plot_critical_parameter_value_evolution = false;
	// injection mode related
	this->has_additional_injection = false;
}

CriticalParameterNDParameterSweepSimulation::~CriticalParameterNDParameterSweepSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "CriticalParameterNDParameterSweepSimulation destructor. direction:" << this << "\n";
	#endif
	// deleteContentsOfVectorOfPointers( critical_parameter_value_simulations_vector );
	// altered parameters have already been destroyed in each critical_parameter_value_simulation destructor
	// deleteContentsOfVectorOfPointers( altered_parameters_vector );
}

void CriticalParameterNDParameterSweepSimulation::RunSimulation( ){
	if (!TestSetUp()){
		 log_io->ReportError2AllLogs( "RunSimulation had not been previously set up. ");
		 return;
	}
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
	if( totalThreads<3 ){
		log_io->ReportError2AllLogs( "No sweept parameter, threads:" + number2String(totalThreads) );
		correctly_simulated = false;
		correctly_processed = false;
		return;
	}
	// Environment: maybe ConfigureEnvironmentVariables is required. see critical_parameter_1d_....
	log_io->ReportThread( "Total threads to be simulated: " + number2String(totalThreads) + ". Max number of sweep threads: " + number2String(max_parallel_profile_instances), 1 );
	// parallel threads control
	unsigned int runningThreads = 0;
	unsigned int threadsCount = 0;
	// current parameter sweep indexes
	std::vector<unsigned int> parameterCountIndexes(parameters2sweep.size(), 0);
	critical_parameter_value_simulations_vector.set_group_name("critical_parameter_value_simulations_vector");
	critical_parameter_value_simulations_vector.ReserveSimulationsInMemory( totalThreads );
	while( threadsCount<totalThreads ){
		// wait for resources
		WaitForResources( runningThreads, max_parallel_profile_instances, mainTG, threadsCount );
		// CreateProfile sets all parameter values, and after the simulation object
		// is created it can be updated.
		// Thus, it avoids race conditions when updating parameterCountIndexes and parameters2sweep
		CriticalParameterValueSimulation* pCPVS = CreateProfile(parameterCountIndexes, parameters2sweep, threadsCount);
		if( pCPVS==nullptr ){
			log_io->ReportError2AllLogs( "Null CreateProfile " + number2String(threadsCount) );
			correctly_simulated = false;
			correctly_processed = false;
			return;
		}
		critical_parameter_value_simulations_vector.AddSpectreSimulation( pCPVS );
		mainTG.add_thread( new boost::thread(boost::bind(&CriticalParameterValueSimulation::RunSimulation, pCPVS)) );
		// update variables
		UpdateParameterSweepIndexes( parameterCountIndexes, parameters2sweep);
		++threadsCount;
		++runningThreads;
	}
	mainTG.join_all();
	// check if every simulation ended correctly
	correctly_simulated = critical_parameter_value_simulations_vector.CheckCorrectlySimulated();
	correctly_processed = critical_parameter_value_simulations_vector.CheckCorrectlyProcessed();
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlainStandard( k2Tab + "Generating Map files.");
	#endif
	// generate map files
	GenerateAndPlotResults( parameters2sweep );
	// end
	log_io->ReportPlain2Log( "END OF CriticalParameterNDParameterSweepSimulation::RunSimulation" );
}

CriticalParameterValueSimulation* CriticalParameterNDParameterSweepSimulation::CreateProfile(
	const std::vector<unsigned int>& parameterCountIndexes,
	std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndProfileIndex ){
	// Create folder
	std::string s_ndProfileIndex = number2String(ndProfileIndex);
	std::string currentFolder = folder + kFolderSeparator
		 + "param_profile_" + s_ndProfileIndex;
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
	CriticalParameterValueSimulation* pCPVS = CreateCriticalParameterValueSimulation(
		currentFolder, parameterCountIndexes, parameters2sweep, ndProfileIndex);
	if( pCPVS==nullptr ){
		log_io->ReportError2AllLogs( "pCPVS is nullptr" );
		return nullptr;
	}
	return pCPVS;
}

CriticalParameterValueSimulation* CriticalParameterNDParameterSweepSimulation::CreateCriticalParameterValueSimulation(
		const std::string& currentFolder, const std::vector<unsigned int>& parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int ndProfileIndex  ){
	std::string s_ndProfileIndex = number2String(ndProfileIndex);
	// Simulation
	CriticalParameterValueSimulation* pCPVS = new CriticalParameterValueSimulation();
	pCPVS->set_has_additional_injection( has_additional_injection );
	pCPVS->set_n_dimensional(true);
	pCPVS->set_n_d_profile_index(ndProfileIndex);
	pCPVS->set_is_nested_simulation( true );
	pCPVS->set_simulation_id(  "s_" + number2String(altered_scenario_index) + "_prof_" + s_ndProfileIndex );
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
	pCPVS->set_plot_scatters( plot_scatters );
	pCPVS->set_plot_transients( plot_transients );
	pCPVS->set_plot_last_transients( plot_last_transients );
	pCPVS->set_plot_critical_parameter_value_evolution( plot_critical_parameter_value_evolution );
	pCPVS->set_interpolate_plots_ratio( interpolate_plots_ratio );
	pCPVS->set_main_analysis( main_analysis );
	pCPVS->set_main_transient_analysis( main_transient_analysis );
	pCPVS->set_process_metrics( true );
	pCPVS->set_export_processed_metrics( export_processed_metrics );
	pCPVS->set_export_metric_errors( export_metric_errors );
	// copy of simulation_parameters
	pCPVS->CopySimulationParameters( *simulation_parameters );
	/// Update golden parameter
	if( !pCPVS->UpdateGoldenCriticalParameter( *golden_critical_parameter ) ){
		 log_io->ReportError2AllLogs( golden_critical_parameter->get_name()
			  + " not found in CriticalParameterValueSimulation and could not be updated." );
	}
	// update parameter values
	unsigned int sweepedParamIndex = 0;
	for( auto const &p : parameters2sweep ){
		if( !pCPVS->UpdateParameterValue( *p, number2String( p->GetSweepValue( parameterCountIndexes.at(sweepedParamIndex)))) ){
			log_io->ReportError2AllLogs( p->get_name() + " not found in CriticalParameterValueSimulation and could not be updated." );
		}
		++sweepedParamIndex;
	}
	return pCPVS;
}

bool CriticalParameterNDParameterSweepSimulation::TestSetUp(){
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

bool CriticalParameterNDParameterSweepSimulation::InitMetricColumnIndexes(
		const std::vector<Metric*>& auxMetrics ){
	// profile outputs
	// 0  1  2      3          4                    5
	// #profCount #Profile #Qcoll #MAG_i_name #MAG_i_maxErrorMetric #MAG_i_maxErrorGlobal
	out_profile_c_i_mean = {2};
	// partial plane inputs
	// 0  1  2      3          4                    5
	// p1 p2 crit 	MAG_i_name MAG_i_maxErrorGlobal MAG_i_maxErrorMetric
	p_p_c_i_statistic_2be_processed = {2}; // critical param
	// partial plane outputs
	// 0  1  2      	3       	4         5				6						7						8							9						10					11						12
	// p1 p2 critMax  critMin 	critMean 	m0Name	m0MetricMax	m0MetricMin	m0MetricMean  m0GlobalMax	m0GlobalMin	m0GlobalMean	m1Name
	out_p_c_i_max = {2};
	out_p_c_i_min = {3};
	out_p_c_i_mean = {4};
	unsigned int inMagCount = 3; // MAG_i_name in input
	unsigned int outMagCount = 5; // MAG_i_name in output
	for( auto const &m: auxMetrics ){
		if( m->get_analyzable() ){
			// for use inside CriticalParameterNDParameterSweepSimulation
			p_p_c_i_statistic_2be_processed.push_back( inMagCount+1 ); // MAG_i_maxErrorMetric
			p_p_c_i_statistic_2be_processed.push_back( inMagCount+2 ); // MAG_i_maxErrorGlobal
			// for use outside CriticalParameterNDParameterSweepSimulation
			out_profile_c_i_mean.push_back( inMagCount+1 );// MAG_i_maxErrorMetric
			out_profile_c_i_mean.push_back( inMagCount+2 );// MAG_i_maxErrorGlobal
			out_p_c_i_max.push_back( outMagCount + 1 );
			out_p_c_i_min.push_back( outMagCount + 2 );
			out_p_c_i_mean.push_back( outMagCount + 3 );
			out_p_c_i_max.push_back( outMagCount + 4 );
			out_p_c_i_min.push_back( outMagCount + 5 );
			out_p_c_i_mean.push_back( outMagCount + 6 );
			// update counters
			inMagCount += 3; // next MAG_i_name in input
			outMagCount += 7; // next MAG_i_name in output
		}
	}
	return true;
}

bool CriticalParameterNDParameterSweepSimulation::GenerateAndPlotResults(
		const std::vector<SimulationParameter*>& parameters2sweep ){
	std::string mapsFolder =  top_folder + kFolderSeparator
		 + kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kCriticalParameterNDParametersSweepResultsFolder + kFolderSeparator
		 + "scenario_" + simulation_id;
 	std::string gnuplotScriptFolder =  top_folder + kFolderSeparator
 		 + kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator + kCriticalParameterNDParametersSweepResultsFolder + kFolderSeparator
 		 + "scenario_" + simulation_id;
 	std::string imagesFolder =  top_folder + kFolderSeparator
 		 + kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator + kCriticalParameterNDParametersSweepResultsFolder + kFolderSeparator
 		 + "scenario_" + simulation_id;
	if( !CreateFolder(mapsFolder, true) || !CreateFolder(imagesFolder, true ) || !CreateFolder(gnuplotScriptFolder, true ) ){
		 log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
		 + mapsFolder + " and " + imagesFolder + "'." );
		 log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
		 return false;
	}
	// aux mags
	auto auxMetrics = golden_metrics_structure->GetMetricsVector( 0 );
	unsigned int totalAnalizableMetrics = 0;
	for( auto const &m : *auxMetrics ){
		if( m->get_analyzable() ){
			++totalAnalizableMetrics;
		}
	}
	bool partialResults = true;
	// general map
	partialResults = partialResults && GenerateAndPlotGeneralResults(
		*auxMetrics, parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder );
	if( !partialResults ){
		log_io->ReportError2AllLogs( "Unexpected error in GenerateAndPlotGeneralResults" );
		return false;
	}
	// p1 vs p2 3d plots
	// std::vector<std::tuple<int,int>>
	std::set<std::pair<unsigned int,unsigned int>> exportedParamTuples;
	// init all column indexes
	if( !InitMetricColumnIndexes(*auxMetrics) ){
		log_io->ReportError2AllLogs( "Unexpected error in InitMetricColumnIndexes" );
		return false;
	}
	// reserve memory
	main_nd_simulation_results.ReservePlanesInMemory( CountInvolvedPlanes(parameters2sweep) );
	// create planes
	std::string planesMapsFolder;
	std::string planesGnuplotScriptFolder;
	std::string planesImagesFolder;
	unsigned int p1Index = 0;
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
					log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '"
						+ planesMapsFolder + " and " + planesImagesFolder + "'." );
					log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
					return false;
				}
				partialResults = partialResults && GenerateAndPlotParameterPairResults(
					*auxMetrics, totalAnalizableMetrics,
					p1Index, p2Index, parameters2sweep,
					planesMapsFolder, planesGnuplotScriptFolder, planesImagesFolder );
				// add parameters to set
				exportedParamTuples.insert(auxPair1);
			}
			++p2Index;
		}
		++p1Index;
	} // end of p1 vs p2 3d plot
	return partialResults;
}

bool CriticalParameterNDParameterSweepSimulation::GenerateAndPlotGeneralResults(
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
	double maxCritCharge = 0;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotSpectreErrorMapFile.open( gnuplotSpectreErrorMapFilePath.c_str() );
		gnuplotMapFile << "#profCount #Profile" << " #"  << golden_critical_parameter->get_name()
			<< " #MAG_i_name #MAG_i_maxErrorMetric #MAG_i_maxErrorGlobal\n";
		gnuplotSpectreErrorMapFile << "#profCount #Profile #SpectreError \n";
		unsigned int profileCount = 0;
		bool severalSweepParameter = parameters2sweep.size()>1;
		auto sweepParameter = parameters2sweep.at(0);
		for( auto const &simulation : *(critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
			CriticalParameterValueSimulation* convSim = dynamic_cast<CriticalParameterValueSimulation*>(simulation);
			std::string auxIndexes = getIndexCode( auxiliarIndexes );
			std::string auxSpectreError = convSim->get_correctly_simulated() ? "0" : "1";
			gnuplotMapFile << std::defaultfloat << ( severalSweepParameter ? profileCount : sweepParameter->GetSweepValue(profileCount) )
				<< " " << auxIndexes << " " << convSim->get_critical_parameter_value();
			maxCritCharge = convSim->get_critical_parameter_value()>maxCritCharge ? convSim->get_critical_parameter_value() : maxCritCharge;
			// mag errors
			auto magErrors = convSim->get_last_valid_transient_simulation_results()->get_metrics_errors();
			for( auto const &m : *magErrors ){
				gnuplotMapFile << " " << m->get_metric_name() << " " << m->get_max_abs_error() << " " << m->get_max_abs_error_global();
			}
			gnuplotMapFile << "\n";
			// spectre errors
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
	if( partialResults ){
		// create gnuplot scatter map graphs
		int gnuplotResult = GnuplotGeneralCritParamValueResults( gnuplotSpectreErrorMapFilePath, gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotGeneralMetricResults( auxMetrics,
			maxCritCharge, mapsFolder, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ". Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

bool CriticalParameterNDParameterSweepSimulation::GenerateAndPlotParameterPairResults(
		const std::vector<Metric*>& auxMetrics, const unsigned int& totalAnalizableMetrics,
		const unsigned int& p1Index, const unsigned int& p2Index,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
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
	for( auto const &planeIndexes : *planes ){
		partialResults = partialResults && GenerateAndPlotItemizedPlane(
			auxMetrics, p1Index, p2Index, itemizedCount++,
			parameters2sweep, mapsFolder, gnuplotScriptFolder, imagesFolder, *planeIndexes, *planeStructure);
	}
	// add plane
	log_io->ReportPlainStandard( "#" + simulation_id + ", Processing itemized results with ResultsProcessor" );
	main_nd_simulation_results.AddPlaneResultsStructure( planeStructure );
	// Process itemized planes to extract the p1-p2 general plane results
	ResultsProcessor rp;
	std::string generalParameterResultsFile = mapsFolder + kFolderSeparator
		+ planeStructure->get_plane_id() + "_general_" + kDataSufix;
	// statistic process metric and global max errors
	partialResults = partialResults && rp.StatisticProcessResultsFiles(
		planeStructure->get_itemized_data_paths(), generalParameterResultsFile, std::move(p_p_c_i_statistic_2be_processed));
	if( !partialResults ){
		log_io->ReportError2AllLogs( "[ERROR ResultsProcessor] Error processing " + generalParameterResultsFile );
		return partialResults;
	}
	planeStructure->set_general_data_path(generalParameterResultsFile);
	// plot upsets and metric errors
	SimulationParameter* p1 = parameters2sweep.at(p1Index);
	SimulationParameter* p2 = parameters2sweep.at(p2Index);
	int gnuplotResult = GnuplotPlane( *planeStructure, false, *p1, *p2,
		planeStructure->get_plane_id() + "general", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	gnuplotResult += GnuplotPlaneMetricResults( auxMetrics, *planeStructure,
		false, *p1, *p2, 0, "general", generalParameterResultsFile,
		gnuplotScriptFolder, imagesFolder );
	if( gnuplotResult > 0 ){
		log_io->ReportError2AllLogs( "Sim " + simulation_id + ".Unexpected gnuplot result: " + number2String(gnuplotResult) );
	}
	// clean
	deletePlaneProfileIndexesStructure( *planes );
	return partialResults;
}

bool CriticalParameterNDParameterSweepSimulation::GenerateAndPlotItemizedPlane(
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
		 + "s_" + simulation_id + "_" + partialPlaneId + kDataSufix;
	// export file
	std::ofstream gnuplotMapFile;
	try {
		gnuplotMapFile.open( gnuplotMapFilePath.c_str() );
		gnuplotMapFile.setf(std::ios::scientific);
		gnuplotMapFile << "#" << p1->get_name() << " " << p2->get_name()
			<< " "  << golden_critical_parameter->get_name()
			<< " #MAG_i_name #MAG_i_maxErrorMetric #MAG_i_maxErrorGlobal\n";
		unsigned int profileCount = 0;
		unsigned int p1SweepCount = 0;
		unsigned int p2SweepCount = 0;
		for( auto const &simulation : *(critical_parameter_value_simulations_vector.get_spectre_simulations()) ){
			if( vectorContains(profileIndexesInPlane, profileCount) ){
				CriticalParameterValueSimulation* convSim = dynamic_cast<CriticalParameterValueSimulation*>(simulation);
				if( p2SweepCount==p2->get_sweep_steps_number() ){
					p2SweepCount = 0;
					++p1SweepCount;
					gnuplotMapFile << "\n";
				}
				gnuplotMapFile << p1->GetSweepValue(p1SweepCount) << " " << p2->GetSweepValue(p2SweepCount++)
					<< " " << convSim->get_critical_parameter_value();
				// mag errors
				auto magErrors = convSim->get_last_valid_transient_simulation_results()->get_metrics_errors();
				for( auto const &m : *magErrors ){
					gnuplotMapFile << " " << m->get_metric_name() << " " << m->get_max_abs_error() << " " << m->get_max_abs_error_global();
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
	// fgarcia
	if( partialResults ){
		int gnuplotResult = GnuplotPlane( plane, true, *p1, *p2,
			"partialPlaneId_" + partialPlaneId, gnuplotMapFilePath,
			gnuplotScriptFolder, imagesFolder );
		gnuplotResult += GnuplotPlaneMetricResults( auxMetrics, plane, true,
			*p1, *p2, itemizedCount, "partialPlaneId_" + partialPlaneId,
			gnuplotMapFilePath, gnuplotScriptFolder, imagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Sim " + simulation_id + ".Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}
	}
	return partialResults;
}

int CriticalParameterNDParameterSweepSimulation::GnuplotGeneralCritParamValueResults(
	const std::string& gnuplotSpectreErrorMapFilePath,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	// title
	std::string title = simulation_id +  " general_profiles";
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + "s_" + simulation_id + "_general" + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + "s_" + simulation_id + "_general" + kSvgSufix;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	// Svg
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
		 << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
	gnuplotScriptFile << "set title \" " << title
		 << " vs " << golden_critical_parameter->get_title_name() << " \"\n";
	gnuplotScriptFile << "set grid\n";
	// Axis
	if( golden_critical_parameter->get_value_change_mode()!=kSPLineal ){
		 gnuplotScriptFile << "set logscale y\n";
	}
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";
	gnuplotScriptFile << "set xlabel \""  << "Profile" << "\"\n";
	gnuplotScriptFile << "set ylabel \""  << golden_critical_parameter->get_title_name() << "\"\n";
	gnuplotScriptFile << "set y2label \"Spectre error\"\n";
	// # remove border on top and right and set color to gray
	gnuplotScriptFile << kCustomBorders;
	// line style
	gnuplotScriptFile <<  kTransientSimilarLinesPalette;
	gnuplotScriptFile <<  "set style fill solid\n";
	// legend
	gnuplotScriptFile <<  "set key bottom right\n";
	gnuplotScriptFile <<  "ntics = 10\n";
	// Plot
	// add xtics with xtic(2), u 1:3:xtic(2)
	if( correctly_simulated ){
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:3 axes x1y1 w lp ls 2 title '"
			<< golden_critical_parameter->get_title_name() << "'\n";
	}else{
		gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' u 1:3 axes x1y1 w lp ls 2 title '"
			<< golden_critical_parameter->get_title_name() << "', \\\n";
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

int CriticalParameterNDParameterSweepSimulation::GnuplotGeneralMetricResults(
		const std::vector<Metric*>& analyzedMetrics, double& maxCritCharge,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	int partialResult = 0;
	unsigned int magCount = 0;
	std::string gnuplotDataFile = main_nd_simulation_results.get_general_data_path();
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				+ simulation_id + "_general_mag_" + number2String(magCount) + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				+ simulation_id + "_general_mag_" + number2String(magCount) + kSvgSufix;
			std::string title = "[General] " + m->get_title_name() + " errors & " + golden_critical_parameter->get_title_name() + " Statistics";
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			// Svg
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
				<< " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
			gnuplotScriptFile << "set title \" " << title << " \"\n";
			gnuplotScriptFile << "set grid\n";
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set format y2 \"%g\"\n";
			gnuplotScriptFile << "set xlabel \"Profile\"\n";
			gnuplotScriptFile << "set y2label \"" << golden_critical_parameter->get_title_name() << " \"\n";
			gnuplotScriptFile << "set ylabel \"Error in metric "  << m->get_title_name() << "\"\n";
			// # remove border on top and right and set color to gray
			gnuplotScriptFile << kCustomBorders;
			gnuplotScriptFile << "set y2tics\n";
			// palete range
			if( maxCritCharge < 0 ){
				maxCritCharge = 1;
				// Palete
				gnuplotScriptFile << kMinimalPalette << "\n";
				gnuplotScriptFile << "set y2range [0:1]\n";
				gnuplotScriptFile << "set cbrange [0:1]\n";
			}else{
				// Color Paletes
				gnuplotScriptFile << kUpsetsPalette << "\n";
				gnuplotScriptFile << "set y2range [0:"<< maxCritCharge << "]\n";
				gnuplotScriptFile << "set cbrange [0:"<< maxCritCharge << "]\n";
			}
			// line style
			gnuplotScriptFile <<  kProfilesPalette;
			// Plot
			// # 1              2            3         4                      5				6
			// # profCount profile %crit_param #MAG_i_name #AG_i_maxErrorMetric #MMAG_i_maxErrorGlobal
			int magDataMetricIndex = in_profile_gnuplot_first_mag_metric_offset + p_data_per_metric_per_line*magCount; // metric
			int magDataGlobalIndex = in_profile_gnuplot_first_mag_global_offset + p_data_per_metric_per_line*magCount; // global
			gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:3 axis x1y2 with filledcurve x1 ls 6 title ' " << golden_critical_parameter->get_title_name() << "', \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << magDataMetricIndex << " axis x1y1  w lp ls 1 title '" << m->get_title_name() << "  (max_error_metric)', \\\n";
			gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << magDataGlobalIndex << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << "  (max_error_global)'\n";
			// legend
			gnuplotScriptFile <<  "set key top left\n";

			gnuplotScriptFile << "unset output\n";
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResult += std::system( execCommand.c_str() );
			// Image paths
			main_nd_simulation_results.AddGeneralMetricImagePath( outputImagePath , title );
			++magCount;
		}
	}
	return partialResult;
}

int CriticalParameterNDParameterSweepSimulation::GnuplotPlane(
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder  ){
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
		 + "s_" + simulation_id + "_" + partialPlaneId + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator
		 + "s_" + simulation_id + "_" + partialPlaneId + "_3D" + kSvgSufix;
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";

	gnuplotScriptFile << "set grid\n";
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
	gnuplotScriptFile << "set zlabel \"" << golden_critical_parameter->get_title_name() << "\" rotate by 90\n";
	// Color Paletes
	gnuplotScriptFile << kUpsetsPalette;
	gnuplotScriptFile << "set title \"" << golden_critical_parameter->get_title_name() << ", " << partialPlaneId << " \" \n";
	gnuplotScriptFile << k3DProperties;
	if( interpolate_plots_ratio >= 0 ){
		gnuplotScriptFile << "set pm3d interpolate " << interpolate_plots_ratio << "," << interpolate_plots_ratio << "\n";
	}
	if( isPartialPlane ){
		// p1 p2 critParamValue
		gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:3 notitle " << kElegantLine3D << " w pm3d\n";
	}else{
		// p1 p2 MaxCritParamValue MinCritParamValue MeanCritParamValue
		gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:5 notitle " << kElegantLine3D << " w pm3d\n";
		gnuplotScriptFile << "# additional available plots: \n";
		gnuplotScriptFile << "# Max qcrit (considering profiles): \n";
		gnuplotScriptFile << "# splot '" << gnuplotDataFile << "' u 1:2:3 notitle " << kElegantLine3D << " w pm3d\n";
		gnuplotScriptFile << "# Min qcrit (considering profiles): \n";
		gnuplotScriptFile << "# splot '" << gnuplotDataFile << "' u 1:2:5 notitle " << kElegantLine3D << " w pm3d\n";
	}
	gnuplotScriptFile << "unset output\n";
	// close file
	gnuplotScriptFile << "quit\n";
	gnuplotScriptFile.close();

	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	// Image paths
	if( isPartialPlane ){
		plane.AddItemizedImagePath( outputImagePath, partialPlaneId );
	}else{
		plane.set_general_image_path( outputImagePath );
	}
	return std::system( execCommand.c_str() );
}

int CriticalParameterNDParameterSweepSimulation::GnuplotPlaneMetricResults(
	const std::vector<Metric*>& analyzedMetrics,
	PlaneResultsStructure& plane, const bool isPartialPlane,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const unsigned int& partialPlaneCount,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	unsigned int magCount = 0;
	int partialResults = 0;
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				 + simulation_id + "_" + partialPlaneId + "_mag_" + number2String(magCount) + "_" + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				 + simulation_id + "_" + partialPlaneId + "_mag_" + number2String(magCount) + "_3D" + kSvgSufix;
			std::string title = m->get_title_name() + ", " + partialPlaneId;
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath  << "\"\n";
			gnuplotScriptFile << "set grid\n";
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
			// Offset for xtics
			gnuplotScriptFile << "set ytics left offset 0,-0.5\n";
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
			if( isPartialPlane ){
				int magDataIndex =  plane_gnuplot_first_mag_offset + p_data_per_metric_per_line*magCount; // title
				// p1 p2 critParamValue magName magMetricError magGlobalError
				gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataIndex+1) << " title 'metric_max_err_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataIndex+2) << " title 'global_max_err_" << m->get_title_name() << "' w lp ls 1\n";
			}else{
				int magDataMetricIndex = out_plane_gnuplot_first_mag_metric_offset + out_data_per_metric_per_line*magCount; // max
				int magDataGlobalIndex = out_plane_gnuplot_first_mag_global_offset + out_data_per_metric_per_line*magCount; // max
				// p1 p2 MaxCritParamValue MinCritParamValue MeanCritParamValue
				// magName magMaxMetricError magMinMetricError magMeanMetricError
				// magMaxGlobalError magMinGlobalError magMeanGlobalError

				gnuplotScriptFile << "set multiplot layout 2, 1 \n";

				gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataMetricIndex+1) << " notitle w lp ls 2, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << magDataMetricIndex << " notitle w lp ls 1, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataMetricIndex+2) << " title 'max_err_metric_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";
				gnuplotScriptFile << "# Uncomment for global error \n";
				gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex+1) << " notitle w lp ls 2, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex) << " notitle w lp ls 1, \\\n";
				gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magDataGlobalIndex+2) << " title 'max_err_global_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";

				gnuplotScriptFile << "unset multiplot\n";
			}
			gnuplotScriptFile << "unset output\n";
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();

			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResults += std::system( execCommand.c_str() );
			// Image paths
			if( isPartialPlane ){
				plane.AddItemizedMetricImagePath( partialPlaneCount, outputImagePath, title );
			}else{
				plane.AddGeneralMetricImagePath( outputImagePath, title );
			}
			++magCount;
		}
	}
	return partialResults;
}
