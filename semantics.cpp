#include "traverser.h"

#include <cassert>
#include <stdexcept>

// #include <string>
// #include <map>
// #include <functional>
// 
// using std::string, std::map, std::function, std::vector;

bool find_built_ins::enter(block *l) {
	for (int i = 0; i < l->subnodes.size(); ++i)
		if (auto *sub = dynamic_cast<list*>(l->subnodes[i]))
			if (auto *head = dynamic_cast<name*>(sub->head())) {
				if (head->value == "define") {
					// definitions can have two forms:
					// (define var val)
					// (define (functionname arg1 ... argn) body...)
					if (auto *varname = dynamic_cast<name*>(sub->elements[1])) {
						if (sub->elements.size() != 3)
							throw std::runtime_error("Syntax error: definition needs single expression as value");
						auto *replacement = new var_definition(varname, sub->elements[2]);
						sub->elements.clear();
						delete l->subnodes[i];
						l->subnodes[i] = replacement;
					}
					else if (auto *fn_list = dynamic_cast<list*>(sub->elements[1])) {
						if (sub->elements.size() < 3)
							throw std::runtime_error("Syntax error: f-definition needs at least a parameter list and simple body");
						if (fn_list->elements.size() < 1)
							throw std::runtime_error("Syntax error: f-definition needs at least a function name and simple body");
						auto *fn_name = dynamic_cast<name*>(fn_list->head());
						if (!fn_name)
							throw std::runtime_error("Syntax error: function names must be simple names");
						auto *replacement = new fun_definition(fn_name);
						for (int i = 1; i < fn_list->elements.size(); ++i)
							if (auto *param = dynamic_cast<name*>(fn_list->elements[i]))
								replacement->params.push_back(param);
							else
								throw std::runtime_error("Syntax error: function parameter " + std::to_string(i) + " is not a simple name");
						fn_list->elements.clear();
						delete fn_list;
						replacement->body = new block;
						for (int i = 2; i < sub->elements.size(); ++i)
							replacement->body->subnodes.push_back(sub->elements[i]);
						// all list elements have been copied. empty list to avoid recursive removal.	
						sub->elements.clear();
						delete l->subnodes[i];
						l->subnodes[i] = replacement;
					}
					else throw std::runtime_error("Syntax error: unexpected stuff after define");
				}

			}
	return false;
}
