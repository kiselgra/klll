#include "traverser.h"
#include <iostream>

using std::cout, std::endl;

namespace built_in {
	void display(std::vector &values) {
		for (auto v : values)
			cout << v.integer << " ";
		values.clear();
	}
	void plus(std::vector &values) {
		long acc = values[0];
		for (int i = 1; i < values.size(); ++i)
			acc += values[i];
		values.clear();
		values.push_back(acc);
	}
	void mult(std::vector &values) {
		long acc = values[0];
		for (int i = 1; i < values.size(); ++i)
			acc *= values[i];
		values.clear();
		values.push_back(acc);
	}
}
// TODO add functions to bindings and resolve using those if not shadowed

interprete::interprete() {
	bindings.push_back({});
}

interprete::value interprete::find_binding(definition *def) {
	for (int scope = bindings.size()-1; scope >= 0; --scope)
		if (auto it = bindings[scope].find(def); it != bindings[scope].end())
			return it->second;
	throw std::runtime_error("Binding not found: " + def->name);
}

void interprete::push_value(value v) {
	value_stack.back().push_back(v);
}

bool interprete::enter(integer *i) {
	push_value(i->value);
	return true;
}

bool interprete::enter(name *n) {
	value v = find_binding(n->definition.ptr);
	push_value(v);
	return true;
}

bool interprete::enter(block *) {
	return true;
}

void interprete::leave(block *) {
}


bool interprete::enter(list *) {
	return true;
}

void interprete::leave(list *) {
}


bool interprete::enter(var_definition *def) {
	value_stack.push_back({});
	return true;
}

void interprete::leave(var_definition *def) {
	if (value_stack.back().size() != 1)
		throw std::runtime_error("Value stack error");
	value v = value_stack.back().back();
	bindings.back().insert({def, v});
	value_stack.pop_back();
	cout << "var_def " << def->name << " has value " << v.integer << endl;
}

bool interprete::enter(fun_definition *) {
	return false;
}

void interprete::leave(fun_definition *) {
}


bool interprete::enter(fun_call *) {
	return true;
}

void interprete::leave(fun_call *) {
}


bool interprete::enter(branch *) {
	return true;
}

void interprete::leave(branch *) {
}


