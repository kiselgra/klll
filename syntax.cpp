#include "syntax.h"
#include "tree.h"
#include "traverser.h"

#include <stdexcept>

using std::vector;

/* 
 * First, convert token stream to nested forms.
 *
 */

std::ostream& operator<<(std::ostream &out, const form &f) {
	if (f.is_atom())
		out << f.value();
	else {
		out << "(   ";
		for (auto x : f.sub())
			out << x << "   ";
		out << ")   ";
	}
	return out;
}

form parse(const vector<token> &tokenstream, int &pos) {
	::form form;
	while (pos < tokenstream.size()) {
		std::cout << "---> " << tokenstream[pos] << std::endl;
		if (tokenstream[pos].type() == token::P_OPEN)
			form.add(parse(tokenstream, ++pos));
		else if (tokenstream[pos].type() == token::P_CLOSE)
			break;
		else
			form.add(tokenstream[pos++]);
	}
	++pos;
	return form;
}

vector<form> parse(const vector<token> &tokenstream) {
	vector<form> forms;

	if (tokenstream.size() == 0)
		throw std::runtime_error("Syntax error: empty program!");

	int pos = 0;
	while (pos < tokenstream.size()) {
		if (tokenstream[pos].type() != token::P_OPEN)
			throw std::runtime_error("Syntax error: top level forms need to start with open parens!");
		forms.push_back(parse(tokenstream, ++pos));
	}

	return forms;
}

/* 
 * Then convert nested form structure to a proper tree
 *
 */

// simplistic number check
bool is_number(const token &t) {
	if (t.type() != token::ID) return false;
	if (t.id() == "") return false;
	for (int i = 0; i < t.id().length(); ++i)
		if (!isdigit(t.id()[i]))
			return false;
	return true;
}

node* to_tree(const form &f) {
	if (f.is_atom()) {
		token t = f.value();
		if (is_number(t))
			return new integer(t);
		else
			return new name(t);
	}
	else {
		list *listnode = new list;
		for (const form &f : f.sub())
			listnode->elements.push_back(to_tree(f));
		return listnode;
	}
}

node* to_tree(const std::vector<form> &f) {
	block *global = new block;
	for (const form &f : f)
		global->subnodes.push_back(to_tree(f));
	return global;
}

/* 
 * Now work on the nested list structure to replace built-in operations
 *
 */

bool find_built_ins::enter(block *l) {
	// go over all "statements" in a block
	for (int i = 0; i < l->subnodes.size(); ++i)
		// see if they are nested statements, i.e. lists
		// SUB is the currently analyzed subnode of the current block
		if (auto *sub = dynamic_cast<list*>(l->subnodes[i]))
			// if the first element is a NAME then this might be a built-in form
			// (or a function call, but we don't take care of those, here)
			// HEAD is the first element of SUB
			if (auto *head = dynamic_cast<name*>(sub->head())) {
				
				// handle definitions
				if (head->value == "define") {
					// definitions can have two forms:
					// (define var val)
					// (define (functionname arg1 ... argn) body...)
					// case 1: 2nd list element is a variable name
					if (auto *varname = dynamic_cast<name*>(sub->elements[1])) {
						if (sub->elements.size() != 3)
							throw syntax_error("definition needs single expression as value");
						auto *replacement = new var_definition(varname, sub->elements[2]);
						sub->elements.clear();		// clear 
						delete sub->elements[0];
						delete l->subnodes[i];
						l->subnodes[i] = replacement;
					}
					// case 2: 2nd list element is itself a list (containing function name and parameter names)
					else if (auto *fn_list = dynamic_cast<list*>(sub->elements[1])) {
						if (sub->elements.size() < 3)
							throw syntax_error("f-definition needs at least a parameter list and simple body");
						if (fn_list->elements.size() < 1)
							throw syntax_error("f-definition needs at least a function name and simple body");
						auto *fn_name = dynamic_cast<name*>(fn_list->head());
						if (!fn_name)
							throw syntax_error("function names must be simple names");
						auto *replacement = new fun_definition(fn_name);
						for (int i = 1; i < fn_list->elements.size(); ++i)
							if (auto *param = dynamic_cast<name*>(fn_list->elements[i]))
								replacement->params.push_back(new var_definition(param, nullptr));
							else
								throw syntax_error("function parameter " + std::to_string(i) + " is not a simple name");
						fn_list->elements.clear();
						delete fn_list;
						replacement->body = new block;
						for (int i = 2; i < sub->elements.size(); ++i)
							replacement->body->subnodes.push_back(sub->elements[i]);
						// all list elements have been copied. empty list to avoid recursive removal.	
						sub->elements.clear();
						delete sub->elements[0];
						delete l->subnodes[i];
						l->subnodes[i] = replacement;
					}
					// no other case allowed
					else throw syntax_error("unexpected stuff after define");
				}

			}
	return false;
}
