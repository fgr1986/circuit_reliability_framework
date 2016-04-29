/*
 * node.h
 *
 *  Created on: Feb 20, 2013
 *      Author: fernando
 */


#ifndef NODE_H
#define NODE_H

#include <string>

class Node {
public:
	// Node();
	// Node(int position, std::string name);

	Node( std::string name, bool injectable );
	Node( std::string name, bool injectable, bool pin_in_subcircuit );
	Node(const Node& orig);
	virtual ~Node();

	// int get_position() const {return position;}
	std::string get_name() const {return name;}
	void set_name(std::string name) { this->name = name; }
	bool get_injected() const {return injected;}
	void set_pin_in_subcircuit(bool pin_in_subcircuit) { this->pin_in_subcircuit = pin_in_subcircuit; }
	bool get_pin_in_subcircuit() const {return pin_in_subcircuit;}
	void set_injected(bool injected) { this->injected = injected; }
	bool get_injectable() const {return injectable;}
	void set_injectable(bool injectable) { this->injectable = injectable; }

private:
	// int position;
	std::string name;
	bool injected;
	bool injectable;
	bool pin_in_subcircuit;
};

#endif /* NODE_H */
