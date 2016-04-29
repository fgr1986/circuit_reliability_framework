/*
 * scope.cpp
 *
 *  Created on: December 20, 2013
 *      Author: fernando
 */

/// radiation simulator
#include "scope.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"

Scope::Scope( std::string name, std::string main_statement_name, bool is_subcircuit_scope ) {
	this->name = name;
	this->main_statement_name = main_statement_name;
	this-> is_subcircuit_scope = is_subcircuit_scope;
	this->altered = false;
}

Scope::Scope(const Scope& orig) {
	this->name = orig.name;
	this->main_statement_name = orig.main_statement_name;
	this->altered = orig.altered;
	this->is_subcircuit_scope = orig.is_subcircuit_scope;
	deepCopyVectorOfPointers( orig.nodes, nodes);
}

Scope::~Scope() {
	// Deleting Scope
	deleteContentsOfVectorOfPointers(nodes);
}

void Scope::AddNode(Node* node){
	nodes.push_back( node );
}

Node* Scope::GetNode(std::string node_name){
	for( auto const &node : nodes){
		if( node->get_name().compare( node_name )== 0){
			return node;
		}
	}
	return nullptr;
}
