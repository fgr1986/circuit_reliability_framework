/*
 * scope.h
 *
 *  Created on: December 12, 2013
 *      Author: fernando
 */


#ifndef SCOPE_H
#define SCOPE_H

/// c++ std required libraries
#include <string>
#include <vector>
/// radiation simulator
#include "node.hpp"

class Scope {
public:
	/// Default Constructor
	Scope( std::string name, std::string main_statement_name, bool is_subcircuit_scope );
	/// Copy Constructor
	Scope(const Scope& orig);
	/// Default Destructor
	virtual ~Scope();

	std::string get_name() const {return name;}
	std::string get_main_statement_name() const {return main_statement_name;}
	bool get_altered() const {return altered;}
	bool get_is_subcircuit_scope() const {return is_subcircuit_scope;}
	void set_name(std::string name) { this->name = name; }
	void set_main_statement_name(std::string main_statement_name) {
		this->main_statement_name = main_statement_name; }

	void set_altered(bool altered) { this->altered = altered; }
	void set_is_subcircuit_scope( bool is_subcircuit_scope ) {
		this->is_subcircuit_scope = is_subcircuit_scope; }
	std::vector<Node*>* get_nodes() { return &nodes; }

	void AddNode(Node* node);
	Node* GetNode(std::string node_name);

private:
	std::string name;
	// Circuit or inline subcircuit to which the scope is related
	std::string main_statement_name;
	bool altered;
	std::vector<Node*> nodes;
	bool is_subcircuit_scope;
};

#endif /* SCOPE_H */
