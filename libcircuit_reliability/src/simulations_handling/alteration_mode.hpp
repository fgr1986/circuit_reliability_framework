 /**
 * @file simulation_mode.hpp
 *
 * @date Created on: Jan 2, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class contains the statements regarding a simulation mode
 * 		+ Simulation mode id
 * 		+ Radiation Mode
 * 		+ Analysis Statement
 * 		+ Control Statements
 *
 */


#ifndef ALTERATION_MODE_H
#define ALTERATION_MODE_H

///AlterationMode Class
// c++ std libraries includes
#include <string>
#include <vector>
#include <map>
// radiation io simulator includes
#include "../io_handling/log_io.hpp"
// Netlist modeling includes
#include "../netlist_modeling/statements/radiation_source_subcircuit_statement.hpp"
#include "../netlist_modeling/parameter.hpp"
#include "../netlist_modeling/simulation_parameter.hpp"

class AlterationMode {
public:
	/// Default constructor
	AlterationMode( );
	/// Default constructor
	AlterationMode( int simulation_mode_type );
	/// Default destructor
	virtual ~AlterationMode();

	/**
	 * @brief sets name
	 *
	 * @param name
	 */
	void set_name( std::string name) {this->name = name;}

	/**
	 * @brief gets name
	 * @return name
	 */
	std::string get_name() const{return name;}

	/**
	* @details sets log_io
	* @param log_io <LogIO*>
	*/
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

	/**
	* @brief get_injection_mode method
	* @return injection_mode <bool>
	*/
 	bool get_injection_mode() const{ return injection_mode; }

	/**
	* @brief get inject_all_mode method
	* @return inject_all_mode <bool>
	*/
 	bool get_inject_all_mode() const{ return inject_all_mode; }

	/**
	* @brief get perform_alteration method
	* @return perform_alteration <bool>
	*/
 	bool get_perform_alteration() const{ return perform_alteration; }

	/**
 	 * @brief sets injection_mode
 	 *
 	 * @param injection_mode
 	 */
 	void set_injection_mode( bool injection_mode ){this->injection_mode = injection_mode;}

	/**
 	 * @brief sets inject_all_mode
 	 *
 	 * @param inject_all_mode
 	 */
 	void set_inject_all_mode( bool inject_all_mode ){this->inject_all_mode = inject_all_mode;}

	/**
 	 * @brief sets perform_alteration
 	 *
 	 * @param perform_alteration
 	 */
 	void set_perform_alteration( bool perform_alteration ){this->perform_alteration = perform_alteration;}


	/**
	 * @brief gets available_radiation_sources
	 * @return available_radiation_sources vector
	 */
	std::vector<RadiationSourceSubcircuitStatement*>* get_available_radiation_sources() {
		return &available_radiation_sources;}

	/**
	 * @brief sets the selected_radiation_source
	 * @details sets selected_radiation_source
	 *
	 * @param selected_radiation_source
	 */
	void set_selected_radiation_source( RadiationSourceSubcircuitStatement* selected_radiation_source){
		this->selected_radiation_source = selected_radiation_source;
	}

	/**
	 * @brief gets selected_radiation_source
	 * @return selected_radiation_source vector
	 */
	RadiationSourceSubcircuitStatement* get_selected_radiation_source() {
		return selected_radiation_source;}

	/**
	 * @details adds a radiationSource
	 *
	 * @param radiationSource
	 */
	void AddRadiationSource(
		RadiationSourceSubcircuitStatement* radiationSource );

	/**
	 * @details adds an unalterableStatement
	 *
	 * @param unalterableStatement
	 */
	void AddUnalterableStatement( std::string unalterableStatement );

	/**
	 * @details adds an alterableStatement
	 *
	 * @param alterableStatement
	 */
	void AddAlterableStatement( std::string alterable_statement,
	std::string new_statement );

	/**
	 * @brief selects the radiation source
	 * @details selects the radiation source from the available radiation sources list using the index
	 *
	 * @param index
	 */
	void set_selected_radiation_source_index( int index ){
		this->selected_radiation_source = available_radiation_sources.at(index);
	}

	/**
	 * @brief get alterable_statements list
	 * @return alterable_statements
	 */
	std::map<std::string,std::string>* get_alterable_statements(){ return &alterable_statements; }

	/**
	 * @brief Test the attributes
	 */
	void TestAlterationMode();

private:
	/// name
	std::string name;
	/// wheter perform or not the alteration
	bool perform_alteration;
	/// whether inject or substitute radiation modes
	bool injection_mode;
	/// Whether inject all the statements instead only the selected ones
	bool inject_all_mode;
	/// Logger
	LogIO* log_io;
	/// Radiation subcircuits
	std::vector<RadiationSourceSubcircuitStatement*> available_radiation_sources;
	/// Selected radiation source
	RadiationSourceSubcircuitStatement* selected_radiation_source;
	/// alterable_statements
	std::map<std::string,std::string> alterable_statements;
};

#endif /* ALTERATION_MODE_H */
