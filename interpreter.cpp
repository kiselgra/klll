#include "traverser.h"
#include <iostream>

using std::cout, std::endl;

namespace built_in {
	using value = interprete::value;

	void display(std::vector<value> &values) {
		for (auto v : values)
			cout << v.integer << " ";
		cout << endl;
		values.clear();
		values.push_back(value::value_true());
	}
	void plus(std::vector<value> &values) {
		long acc = values[0].integer;
		for (int i = 1; i < values.size(); ++i)
			acc += values[i].integer;
		values.clear();
		values.push_back(value(acc));
	}
	void mult(std::vector<value> &values) {
		long acc = values[0].integer;
		for (int i = 1; i < values.size(); ++i)
			acc *= values[i].integer;
		values.clear();
		values.push_back(value(acc));
	}
}
// TODO add functions to bindings and resolve using those if not shadowed

interprete::interprete(std::map<std::string, builtin_function*> &builtins) {
	std::map<std::string, value::built_in> actual_builtins {
		{ "display", built_in::display },
		{ "+", built_in::plus },
		{ "*", built_in::mult },
	};
	bindings.push_back({});
	for (auto [name, fn] : builtins)
		if (auto actual = actual_builtins.find(name); actual != actual_builtins.end())
			add_binding(fn, actual->second);
		else
			std::cerr << "Warning: built-in function " << name << " not implemented" << endl;
}

void interprete::add_binding(definition *def, value v) {
	bindings.back().insert({def, v});
}

interprete::value interprete::find_binding(definition *def) {
	for (int scope = bindings.size()-1; scope >= 0; --scope)
		if (auto it = bindings[scope].find(def); it != bindings[scope].end())
			return it->second;
	throw std::runtime_error("Binding not found: " + def->name);
}

void interprete::push_value_frame() {
	value_stack.push_back({});
} 

void interprete::pop_value_frame() {
	value_stack.pop_back();
}

void interprete::push_value(value v) {
	value_stack.back().push_back(v);
}

interprete::value interprete::pop_value() {
	value v = value_stack.back().back();
	value_stack.back().pop_back();
	return v;
}

interprete::value interprete::pop_first_value() {
	value v = value_stack.back().front();
	value_stack.back().erase(value_stack.back().begin());
	return v;
}

std::vector<interprete::value>& interprete::value_frame() {
	return value_stack.back();
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

bool interprete::enter(toplevel_block *b) {
	return enter((block*)b);
}

void interprete::leave(toplevel_block *b) {
	leave((block*)b);
}

bool interprete::enter(block *) {
	cout << "setup value frame for a block" << endl;
	push_value_frame();
	return true;
}

void interprete::leave(block *) {
	cout << "drop value frame for a block" << endl;
	pop_value_frame();
}


bool interprete::enter(list *) {
	return true;
}

void interprete::leave(list *) {
}


bool interprete::enter(var_definition *def) {
	push_value_frame();
	return true;
}

void interprete::leave(var_definition *def) {
	if (value_stack.back().size() != 1)
		throw std::runtime_error("Value stack error");
	value v = value_stack.back().back();
	add_binding(def, v);
	pop_value_frame();
	cout << "var_def " << def->name << " has value " << v.integer << endl;
}

bool interprete::enter(fun_definition *def) {
	add_binding(def, def);
	cout << "fun_def " << def->name << " entered" << endl;
	return false;
}

void interprete::leave(fun_definition *) {
}


bool interprete::enter(fun_call *c) {
	push_value_frame();
	cout << "setup value frame for call " << c->name->value << endl;
	return true;
}

void interprete::leave(fun_call *c) {
	// the first value pushed was the function itself, not an argument.
	// pop it, use it.
	value v = pop_first_value(); 
	if (v.kind == value::BI)
		v.bi(value_frame());
	else
		std::cerr << "Unsupported non-builtin call: " << c->name->value << endl;
	value res = pop_value();
	pop_value_frame();
	cout << "drop value frame for call " << c->name->value << endl;
	cout << "put result " << res.integer << " into outer frame" << endl;
	push_value(res); // push value to surrounding frame (call might be inside a call, etc)
}


bool interprete::enter(branch *) {
	return true;
}

void interprete::leave(branch *) {
}


