#include "traverser.h"
#include "syntax.h"
#include <iostream>

bool V = false;

using std::cout, std::endl;

namespace built_in {
	using value = interprete::value;

	void display(std::vector<value> &values) {
		for (int i = 0; i < values.size(); ++i) {
			auto v = values[i];
			if      (v.kind == value::INT)  cout << v.integer;
			else if (v.kind == value::BOOL) cout << (v.b ? std::string("true") : std::string("false"));
			else if (v.kind == value::STR)  cout << v.str;
			else throw syntax_error("Cannot print value of kind " + std::to_string((int)v.kind));
		}
		values.clear();
		values.push_back(value::value_true());
	}
	#define binary_int_cmp(OP, N) \
	void N(std::vector<value> &values) { \
		if (values.size() != 2) throw syntax_error("function < requires exactly 2 arguments"); \
		if (values[0].kind != value::INT || values[1].kind != value::INT)  \
			throw syntax_error("function < requires arguments to be integers"); \
		values.clear(); \
		if (values[0].integer OP values[1].integer) \
			values.push_back(value::value_true()); \
		else values.push_back(value::value_false()); \
	}
	binary_int_cmp(<,  less)
	binary_int_cmp(<=, less_eq)
	binary_int_cmp(>,  greater)
	binary_int_cmp(>=, greater_eq)
	binary_int_cmp(==, equal)
	#undef binary_int_cmp
	#define int_seq(OP, N) \
	void N(std::vector<value> &values) { \
		long acc = values[0].integer; \
		for (int i = 1; i < values.size(); ++i) \
			acc OP values[i].integer; \
		values.clear(); \
		values.push_back(acc); \
	}
	int_seq(+=, plus)
	int_seq(-=, minus)
	int_seq(*=, mult)
	int_seq(/=, div)
	#undef int_seq
}
// TODO add functions to bindings and resolve using those if not shadowed

interprete::interprete(std::map<std::string, builtin_function*> &builtins,
					   std::map<std::string, var_definition*> &predefs) {
	std::map<std::string, value::built_in> actual_builtins {
		{ "display", built_in::display },
		{ "+", built_in::plus },
		{ "*", built_in::mult },
		{ "-", built_in::minus },
		{ "/", built_in::div },
		{ "=", built_in::equal },
		{ "<", built_in::less },
		{ ">", built_in::greater },
		{ "<=", built_in::less_eq },
		{ ">=", built_in::greater_eq },
	};
	bindings.push_back({});
	for (auto [name, fn] : builtins)
		if (auto actual = actual_builtins.find(name); actual != actual_builtins.end())
			add_binding(fn, actual->second);
		else
			std::cerr << "Warning: built-in function " << name << " not implemented" << endl;
	// pre defined variables
	for (auto [name, def] : predefs)
		if      (auto *i = dynamic_cast<integer*>(def->value)) add_binding(def, value(i->value));
		else if (auto *s = dynamic_cast<string*>(def->value))  add_binding(def, value(s->value));
		else    throw std::logic_error("Cannot handle predefined value " + name);
}

void interprete::add_binding_frame() {
	bindings.push_back({});
}

void interprete::drop_binding_frame() {
	bindings.pop_back();
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

bool interprete::enter(string *s) {
	push_value(s->value);
	return true;
}

bool interprete::enter(name *n) {
	value v = find_binding(n->definition.ptr);
	push_value(v);
	return true;
}

bool interprete::enter(block *b) {
	return true;
}

void interprete::leave(block *b) {
}

bool interprete::enter(toplevel_block *) {
	if (V) cout << "setup value frame for a TL block" << endl;
	push_value_frame();
	return true;
}

void interprete::leave(toplevel_block *) {
	if (V) cout << "drop value frame for a TL block" << endl;
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
	if (V) cout << "var_def " << def->name << " has value " << v.integer << endl;
}

bool interprete::enter(fun_definition *def) {
	add_binding(def, def);
	if (V) cout << "fun_def " << def->name << " entered" << endl;
	return false;
}

void interprete::leave(fun_definition *) {
}


bool interprete::enter(fun_call *c) {
	push_value_frame();
	if (V) cout << "setup value frame for call " << c->name->value << endl;
	return true;
}

void interprete::leave(fun_call *c) {
	// the first value pushed was the function itself, not an argument.
	// pop it, use it.
	value v = pop_first_value(); 
	if (v.kind == value::BI)
		v.bi(value_frame());
	else if (v.kind == value::FUN) {
		// bind parameters to arguments
		if (value_frame().size() != v.fd->params.size())
			throw syntax_error("Function " + v.fd->name + " needs " + std::to_string(v.fd->params.size()) + " arguments");
		add_binding_frame();
		for (int i = 0; i < v.fd->params.size(); ++i)
			add_binding(v.fd->params[i], value_frame()[i]);
		value_frame().clear();
		// here we deviate from the regular traversal logic:
		// inside the fun-call we start traversing the called function's body
		v.fd->body->traverse(this);
		// correctly handle last value in block
		// correctly remove bindings
		drop_binding_frame();
	}
	else
		throw syntax_error("Cannot call this");
	value res = pop_value();
	pop_value_frame();
	if (V) cout << "drop value frame for call " << c->name->value << endl;
	if (V) cout << "put result " << res.integer << " into outer frame" << endl;
	push_value(res); // push value to surrounding frame (call might be inside a call, etc)
}


bool interprete::enter(branch *b) {
	push_value_frame();
	b->condition->traverse(this);
	value v = pop_value();
	pop_value_frame();
	if (v.kind != value::BOOL) throw syntax_error("If expression requires boolean condition");
	if (v.b)
		b->true_case->traverse(this);
	else
		b->false_case->traverse(this);
	return false;
}

void interprete::leave(branch *) {
}


