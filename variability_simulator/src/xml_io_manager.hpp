/**
 * @file xml_io_manager.hpp
 *
 * @date Created on: February 2, 2013
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is responsible of the simulations results files as a web page.
 * Several html web pages are created (hierarchical analysis,
 * dependency analysis, simulations results analysis).
 *
 */

#ifndef XMLIOMANAGER_H
#define XMLIOMANAGER_H

///XMLIOManager Class
// c++ std required libraries includes
#include <string>
#include <vector>
// Boost ptree includes
#include <boost/property_tree/ptree.hpp>

// Spectre Handler Library
// Radiation simulator includes
#include "experiment_environment.hpp"
// io includes
#include "io_handling/circuit_io_handler.hpp"
#include "io_handling/log_io.hpp"
// Simulations handling includes
#include "simulations_handling/spectre_handlers/variability_spectre_handler.hpp"
#include "simulations_handling/simulation_modes_handler.hpp"
// Netlist modeling includes
#include "netlist_modeling/statements/statement.hpp"

class XMLIOManager {
public:
	/// Default constructor
	XMLIOManager();
	/// Default destructor
	virtual ~XMLIOManager();

	/**
 	* @details Reads the xml different configuration files
 	* and set up the program actuators.
 	*
 	* @param xml_cadence <const std::string> cadence xml conf file
 	* @param xml_technology <const std::string> technology xml conf file
 	* @param xml_experiment <const std::string> experiment xml conf file
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param spectre_handler <VariabilitySpectreHandler&> spectre simulations handler
 	* @param simulationModesHandler
 	* @param circuitIOHandler
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ReadExperimentEnvironmentFromXML(
		const std::string &xml_cadence, const std::string &xml_technology, const std::string &xml_experiment,
		ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& variabilitySpectreHandler,
		SimulationModesHandler& simulationModesHandler, CircuitIOHandler& circuitIOHandler,
		std::string& experimentTitle );

	/**
 	* @details Exports a circuit to a xml file
 	*
 	* @param circuit <Statement&> target circuit to be exported
 	* @param filename <const std::string> xml file name
 	* @param technology <const std::string> technology name
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ExportCircuit2XML( Statement& circuit, const std::string& filename,
		const std::string technology );

	/// Sets the Log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

private:
	/// Logger
	LogIO* log_io;

	/**
 	* @details Exports a statement to a xml node
 	*
 	* @param node_parent <boost::property_tree::ptree&> parent node
 	* @param ins <Statement*> target statement
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ExportStatement2XML( boost::property_tree::ptree& node_parent, Statement* ins );

	/**
 	* @details Reads the cadence xml configuration file
 	* and set up the program actuators.
 	*
 	* @param xml_cadence <const std::string> cadence xml conf file
 	* @param statementCounter <int&> statement counter
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param spectre_handler <VariabilitySpectreHandler&> spectre simulations handler
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ReadCadenceXML( const std::string &xml_cadence, int& statementCounter,
		ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& spectre_handler );

	/**
 	* @details Reads the technology xml configuration file
 	* and set up the program actuators.
 	*
 	* @param xml_technology <const std::string> technology xml conf file
 	* @param statementCounter <int&> statement counter
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param spectre_handler <VariabilitySpectreHandler&> spectre simulations handler
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ReadTechnologyXML( const std::string &xml_technology, int& statementCounter,
		ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& spectre_handler );

	/**
 	* @details Reads the experiment xml configuration file
 	* and set up the program actuators.
 	*
 	* @param xml_experiment <const std::string> experiment xml conf file
 	* @param statementCounter <int&> statement counter
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param spectre_handler <VariabilitySpectreHandler&> spectre simulations handler
 	* @param simulationModesHandler
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ReadExperimentXML( const std::string &xml_experiment, int& statementCounter,
		ExperimentEnvironment& experimentEnvironment, VariabilitySpectreHandler& spectre_handler,
		SimulationModesHandler& simulationModesHandler,  CircuitIOHandler& circuitIOHandler,
		std::string& experimentTitle );

	/**
	 *
 	* @details Process an analysis statement from cadence conf xml file
 	* to be included in the environment
 	*
 	* @param v <boost::property_tree::ptree::value_type const &> ptree value type node
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param  statementcounter
 	* @param  analysisType
 	*
 	* @return correctly read and set up <bool>
 	*
	 */
	bool ProcessEnvironmentAnalysisStatement(
		boost::property_tree::ptree::value_type const &v,
		ExperimentEnvironment& experimentEnvironment, int& statementCounter );

	/**
	 *
 	* @details Process a control statement from cadence conf xml file
 	* to be included in the environment
 	*
 	* @param v <boost::property_tree::ptree::value_type const &> ptree value type node
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param  statementcounter
 	* @param  control statementype
 	*
 	* @return correctly read and set up <bool>
 	*
	 */
	bool ProcessEnvironmentControlStatement(
		boost::property_tree::ptree::value_type const &v,
		ExperimentEnvironment& experimentEnvironment, int& statementCounter );

	/**
	 *
 	* @details Process a magnitude
 	*
 	* @param v <boost::property_tree::ptree::value_type const &> ptree value type node
 	* @param  statementcounter
 	* @param  variabilitySpectreHandler
 	* @param  circuitIOHandler
 	*
 	* @return correctly read and set up <bool>
 	*
	 */
	bool ProcessMetric(boost::property_tree::ptree::value_type const &v,
		int& statementCounter, VariabilitySpectreHandler& variabilitySpectreHandler, CircuitIOHandler& circuitIOHandler);

	/**
	 *
 	* @details Process the analysis statement of a simulation mode.
 	* Read from experiment xml conf file.
 	*
 	* @param v <boost::property_tree::ptree::value_type const &> ptree value type node
 	* @param experimentEnvironment <ExperimentEnvironment&> experiment environment
 	* @param  statementcounter
 	* @param  analysisType
 	*
 	* @return correctly read and set up <bool>
 	*
	 */
	bool ProcessSimulationModeAnalysisStatement(
		boost::property_tree::ptree::value_type const &v, SimulationMode& simulationMode,
		int& statementCounter );

	/**
	 * @brief Process an analysis child of another analysis
	 *
	 * @param v ptree
	 * @param parent Parent Analysis
	 * @param statementCounter
 	 * @return correctly read and set up <bool>
	 */
	bool ProcessSimulationModeChildAnalysisStatement(
		boost::property_tree::ptree::value_type const &v, AnalysisStatement& parent,
		SimulationMode& simulationMode, int& statementCounter );

	/**
 	 * @details Process the control statement of a simulation mode.
	 *
	 * @param v [description]
	 * @param simulationMode [description]
	 * @param statementCounter [description]
	 * @return [description]
	 */
	bool ProcessSimulationModeControlStatement(
	boost::property_tree::ptree::value_type const &v,  SimulationMode& simulationMode, int& statementCounter );


	/**
	 * @details Process the control statement of a simulation mode.
	 *
	 * @param v [description]
	 * @param simulationMode [description]
	 * @param statementCounter [description]
	 * @return [description]
	 */
	bool ProcessSimulationModeChildControlStatement(
		boost::property_tree::ptree::value_type const &v,  Statement& parent, int& statementCounter );

	/**
 	* @details Process a radiation parameter placed in a xml file
 	*
 	* @param vrp <boost::property_tree::ptree::value_type const &> ptree value type node
 	* @param variabilitySpectreHandler <VariabilitySpectreHandler&> variabilitySpectreHandler mode
 	*
 	* @return correctly read and set up <bool>
 	*
 	*/
	bool ProcessSimulationParameter(
		boost::property_tree::ptree::value_type const &vrp, VariabilitySpectreHandler& variabilitySpectreHandler );

	/**
	 * @brief Process simulation mode from xml
	 * @param pAvailableAnalysis
	 * @param simulationModeName
	 * @param simulationMode
	 * @param statementCounter
	 * @param simulationAnalysisFound
	 * @return method corrrectly ends.
	 */
	bool ProcessSimulationMode( boost::property_tree::ptree pAvailableAnalysis, std::string simulationModeName,
		SimulationMode& simulationMode, int& statementCounter, bool& simulationAnalysisFound );

};

#endif /* XMLIOMANAGER_H */
