/*
 * AlteredScenarioSummary.cpp
 *
 *  Created on: 09/08/2015
 *      Author: fgarcia@die.upm.es
 */

/// radiation simulator
#include "altered_scenario_summary.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"

AlteredScenarioSummary::AlteredScenarioSummary( int altered_scenario_id,
	std::string altered_scenario_folder_path, std::string altered_scenario_altered_element_path ) {
	this->altered_scenario_id = altered_scenario_id;
	this->altered_scenario_folder_path = altered_scenario_folder_path;
	this->altered_scenario_altered_element_path = altered_scenario_altered_element_path;
}

AlteredScenarioSummary::AlteredScenarioSummary(const AlteredScenarioSummary& orig) {
	this->altered_scenario_id = orig.altered_scenario_id;
	this->altered_scenario_folder_path = orig.altered_scenario_folder_path;
	this->altered_scenario_altered_element_path = orig.altered_scenario_altered_element_path;
}

AlteredScenarioSummary::~AlteredScenarioSummary() {
}
