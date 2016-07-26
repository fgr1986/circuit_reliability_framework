/*
 * node.cpp
 *
 *  Created on: April 20, 2013
 *      Author: fernando
 */

/// radiation simulator
#include "node.hpp"
/// constants
#include "../global_functions_and_constants/global_constants.hpp"

Node::Node( std::string name, bool injectable ) {
	this->name = name;
	this->injected = false;
	this->injectable = injectable;
	this->pin_in_subcircuit = false;
}

Node::Node(  std::string name, bool injectable, bool pin_in_subcircuit ) {
	this->name = name;
	this->injected = false;
	this->injectable = injectable;
	this->pin_in_subcircuit = pin_in_subcircuit;
}

Node::Node(const Node& orig) {
	this->name = orig.name;
	this->injected = orig.injected;
	this->injectable = orig.injectable;
	this->pin_in_subcircuit = orig.pin_in_subcircuit;
}

Node::~Node() {
}
