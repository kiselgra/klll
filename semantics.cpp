#include "traverser.h"
#include "syntax.h"

#include <cassert>
#include <stdexcept>

bool resolve_names::enter(var_definition *def) {
	vars.push_back(def);
	return true;
}

bool resolve_names::enter(fun_definition *def) {
	funs.push_back(def);
	return true;
}

void resolve_names::leave(fun_definition *def) {
	for (auto p : def->params)
		vars.pop_back();
}

// TODO syntax needs to make sure
bool resolve_names::enter(name *n) {
	for (int i = vars.size()-1; i >= 0; --i)
		if (vars[i]->name->value == n->value) {
			n->definition = vars[i];
			return true;
		}
	throw syntax_error("Unbound variable " + n->value);
}
