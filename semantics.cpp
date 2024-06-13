#include "traverser.h"
#include "syntax.h"

#include <cassert>


resolve_names::resolve_names() {
#define add(X) builtin_functions[X] = new builtin_function(X)
	add("<=");
	add("<");
	add(">");
	add(">=");
	add("=");
	add("not");
	add("eq");
	add("*");
	add("+");
	add("-");
	add("/");
	add("1-");
	add("1+");
	add("display");
#undef add
	builtin_values["endl"] = new var_definition("endl", new string("\n"));
};

resolve_names::~resolve_names() {
	for (auto [k,v] : builtin_functions)
		delete v;
	for (auto [k,v] : builtin_values)
		delete v;
}


bool resolve_names::enter(var_definition *def) {
	definitions.push_back(def);
	return true;
}

bool resolve_names::enter(fun_definition *def) {
	definitions.push_back(def);
	return true;
}

void resolve_names::leave(fun_definition *def) {
	for (auto p : def->params)
		definitions.pop_back();
}

bool resolve_names::enter(name *n) {
	for (int i = definitions.size()-1; i >= 0; --i)
		if (definitions[i]->name == n->value) {
			n->definition = definitions[i];
			return true;
		}
	if (auto found = builtin_functions.find(n->value); found != builtin_functions.end()) {
		n->definition = found->second;
		return true;
	}
	if (auto found = builtin_values.find(n->value); found != builtin_values.end()) {
		n->definition = found->second;
		return true;
	}
	throw syntax_error("Unbound variable " + n->value);
}
