/*
 * AlteredScenarioSummary.hpp
 *
 *  Created on: 09/08/2015
 *      Author: fgarcia@die.upm.es
 */


#ifndef ALTERED_SCENARIO_SUMMARY
#define ALTERED_SCENARIO_SUMMARY

#include <string>

class AlteredScenarioSummary {
public:

	AlteredScenarioSummary( int altered_scenario_id,
		std::string altered_scenario_folder_path, std::string altered_scenario_altered_element_path );
	AlteredScenarioSummary(const AlteredScenarioSummary& orig);
	virtual ~AlteredScenarioSummary();

	int get_altered_scenario_id() const {return altered_scenario_id;}
	std::string get_altered_scenario_folder_path() const {return altered_scenario_folder_path;}
	std::string get_altered_scenario_altered_element_path() const {return altered_scenario_altered_element_path;}

private:

	int altered_scenario_id;
	std::string altered_scenario_folder_path;
	std::string altered_scenario_altered_element_path;
};

#endif /* ALTERED_SCENARIO_SUMMARY */
