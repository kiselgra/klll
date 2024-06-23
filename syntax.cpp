#include "syntax.h"
#include "tree.h"
#include "traverser.h"

#include <sstream>

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
		// std::cout << "---> " << tokenstream[pos] << std::endl;
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
 * Syntax errors
 *
 */

syntax_error::syntax_error(const std::string &msg, std::optional<token> t) : runtime_error(msg), at_token(t) {
	std::ostringstream oss;
	oss << "Logic error";
	if (at_token)
		oss << " at token " << *at_token;
	oss << ": " << runtime_error::what();
	message = oss.str();
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
		else if (t.type() == token::STR)
			return new string(t);
		else if (t.type() == token::BOOL)
			return new boolean(t.boolean());
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
	block *global = new toplevel_block;
	for (const form &f : f)
		global->subnodes.push_back(to_tree(f));
	return global;
}



/* 
 * Now work on the nested list structure to replace built-in operations
 *
 */

//! replaces ONLY top-level forms
static node* toplevel_replacement(list *curr_node) {
	if (auto *head = dynamic_cast<name*>(curr_node->head())) {
		if (head->value == "define") {
			// definitions can have two forms:
			// (define var val)
			// (define (functionname arg1 ... argn) body...)
			// case 1: 2nd list element is a variable name
			if (auto *varname = dynamic_cast<name*>(curr_node->elements[1])) {
				if (curr_node->elements.size() != 3)
					throw syntax_error("definition needs single expression as value");
				auto *replacement = new var_definition(varname->value, curr_node->elements[2]);
				curr_node->elements[2] = nullptr;
				return replacement;
			}
			// case 2: 2nd list element is itself a list (containing function name and parameter names)
			else if (auto *fn_list = dynamic_cast<list*>(curr_node->elements[1])) {
				if (curr_node->elements.size() < 3)
					throw syntax_error("f-definition needs at least a parameter list and simple body");
				if (fn_list->elements.size() < 1)
					throw syntax_error("f-definition needs at least a function name and simple body");
				auto *fn_name = dynamic_cast<name*>(fn_list->head());
				if (!fn_name)
					throw syntax_error("function names must be simple names");
				auto *replacement = new fun_definition(fn_name->value);
				for (int i = 1; i < fn_list->elements.size(); ++i)
					if (auto *param = dynamic_cast<name*>(fn_list->elements[i]))
						replacement->params.push_back(new var_definition(param->value, nullptr));
					else
						throw syntax_error("function parameter " + std::to_string(i) + " is not a simple name");
				delete fn_list;
				replacement->body = new block;
				for (int i = 2; i < curr_node->elements.size(); ++i)
					replacement->body->subnodes.push_back(curr_node->elements[i]);
				// all list elements have been copied. empty list to avoid recursive removal.	
				curr_node->elements.clear();
				delete curr_node->elements[0];
				return replacement;
			}
			// no other case allowed
			else throw syntax_error("unexpected stuff after define");
		}
	}
	return nullptr;
}

static node* nested_replacement(node *n) {
	if (auto *curr_node = dynamic_cast<list*>(n))
		if (auto *head = dynamic_cast<name*>(curr_node->head())) {
			if (head->value == "if") { // TODO: these can be in blocks and lists
				if (curr_node->elements.size() > 4)
					throw syntax_error("if-conditional requires consequent (and optionally alternative) expression, nothing more. "
									   "Did you mean to group one of the two?", *head->tok);
				if (curr_node->elements.size() < 3)
					throw syntax_error("if-conditional requires at least a consequent", *head->tok);
				branch *replacement = new branch(curr_node->elements[1], curr_node->elements[2]);
				if (curr_node->elements.size() == 4) {
					replacement->false_case = curr_node->elements[3];
					curr_node->elements[3] = nullptr;
				}
				curr_node->elements[1] = curr_node->elements[2] = nullptr;
				return replacement;
			}
			else if (head->value == "block") {
				block *replacement = new block;
				for (int i = 1; i < curr_node->elements.size(); ++i) {
					replacement->subnodes.push_back(curr_node->elements[i]);
					curr_node->elements[i] = nullptr;
				}
				return replacement;
			}
			else if (node *r = toplevel_replacement(curr_node); r)
				return r;
			// any other list-heads are function calls or semantic errors
			else {
				fun_call *replacement = new fun_call(head);
				for (int i = 1; i < curr_node->elements.size(); ++i)
					replacement->args.push_back(curr_node->elements[i]);
				curr_node->elements.clear();
				return replacement;
			}
		}
	return nullptr;
}

bool find_built_ins::enter(toplevel_block *b) {
	for (int i = 0; i < b->subnodes.size(); ++i)
		if (auto *curr_node = dynamic_cast<list*>(b->subnodes[i])) {
			node *replacement = toplevel_replacement(curr_node);
			if (!replacement)
				replacement = nested_replacement(curr_node);
			if (replacement) {
				delete curr_node;
				b->subnodes[i] = replacement;
			}
		}
	return true;
}

bool find_built_ins::enter(block *b) {
	for (node *&n : b->subnodes)
		if (node *replacement = nested_replacement(n)) {
			delete n;
			n = replacement;
		}
	return true;
}

bool find_built_ins::enter(list *l) {
	for (node *&n : l->elements)
		if (node *replacement = nested_replacement(n)) {
			delete n;
			n = replacement;
		}
	return true;
}

#define replace_nested(X) if (node *r = nested_replacement(X)) { delete X; X = r; }

bool find_built_ins::enter(branch *b) {
	replace_nested(b->condition);
	replace_nested(b->true_case);
	replace_nested(b->false_case);
	return true;
}

bool find_built_ins::enter(var_definition *vd) {
	replace_nested(vd->value);
	return true;
}

bool find_built_ins::enter(fun_call *fc) {
	for (node *&arg : fc->args)
		replace_nested(arg);
	return true;
}

