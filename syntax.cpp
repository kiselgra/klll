#include "syntax.h"
#include "tree.h"

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


