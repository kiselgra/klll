#include "tree.h"
#include "traverser.h"


/* 
 * Basic tree node visiting logic
 *
 */

void integer::traverse(visitor *v) { 
	v->enter(this);
}

void string::traverse(visitor *v) {
	v->enter(this);
}

void boolean::traverse(visitor *v) {
	v->enter(this);
}

void name::traverse(visitor *v) {
	v->enter(this);
}

void block::traverse(visitor *v) {
	if (v->enter(this)) {
		for (int i = 0; i < subnodes.size(); ++i) {
			if (i != 0) v->between_subs(this, i);
			subnodes[i]->traverse(v);
		}
		v->leave(this);
	}
}

void toplevel_block::traverse(visitor *v) {
	if (v->enter(this)) {
		for (int i = 0; i < subnodes.size(); ++i) {
			if (i != 0) v->between_subs(this, i);
			subnodes[i]->traverse(v);
		}
		v->leave(this);
	}
}

void list::traverse(visitor *v) {
	if (v->enter(this)) {
		for (int i = 0; i < elements.size(); ++i) {
			if (i != 0) v->between_subs(this, i);
			elements[i]->traverse(v);
		}
		v->leave(this);
	}
}

void var_definition::traverse(visitor *v) {
	if (v->enter(this)) {
		if (value)
			value->traverse(v);
		v->leave(this);
	}
}

void fun_definition::traverse(visitor *v) {
	if (v->enter(this)) {
		for (int i = 0; i < params.size(); ++i) {
			v->between_subs(this, i+1);
			params[i]->traverse(v);
		}
		v->between_subs(this, params.size()+1);
		body->traverse(v);
		v->leave(this);
	}
}

void fun_call::traverse(visitor *v) {
	if (v->enter(this)) {
		name->traverse(v);
		for (int i = 0; i < args.size(); ++i) {
			v->between_subs(this, i+1);
			args[i]->traverse(v);
		}
		v->leave(this);
	}
}

void branch::traverse(visitor *v) {
	if (v->enter(this)) {
		condition->traverse(v);
		true_case->traverse(v);
		if (false_case)
			false_case->traverse(v);
		v->leave(this);
	}
}


/* 
 * Tree debug printer
 *
 */
	
std::ostream& operator<<(std::ostream &out, print_xml_tree::indent_manip im) {
	out << "\n";
	for (int i = 0; i < im.tree->indent; ++i)
		out << "  ";
	return out;
}

bool print_xml_tree::enter(integer *i) {
	stream << ind << "<integer value=" << i->value << "/>";
	return true;
}

bool print_xml_tree::enter(string *s) {
	stream << ind << "<string value=" << s->value << "/>";
	return true;
}

bool print_xml_tree::enter(boolean *b) {
	stream << ind << "<boolean value=" << std::boolalpha << b->value << "/>";
	return true;
}

bool print_xml_tree::enter(name *n) {
	stream << ind << "<name value=" << n->value << "/>";
	return true;
}

bool print_xml_tree::enter(toplevel_block *l) {
	stream << ind << "<toplevel_block>";
	indent++;
	return true;
}

void print_xml_tree::leave(toplevel_block *l) {
	indent--;
	stream << ind << "</toplevel_block>\n\n";
}

bool print_xml_tree::enter(block *l) {
	stream << ind << "<block>";
	indent++;
	return true;
}

void print_xml_tree::leave(block *l) {
	indent--;
	stream << ind << "</block>";
}

bool print_xml_tree::enter(list *l) {
	stream << ind << "<list>";
	indent++;
	return true;
}

void print_xml_tree::leave(list *l) {
	indent--;
	stream << ind << "</list>";
}

bool print_xml_tree::enter(var_definition *d) {
	stream << ind << "<define name=" << d->name << ">";
	indent++;
	return true;
}

void print_xml_tree::leave(var_definition *d) {
	indent--;
	stream << ind << "</define>";
}

bool print_xml_tree::enter(fun_definition *d) {
	stream << ind << "<define name=" << d->name << ">";
	indent++;
	return true;
}

void print_xml_tree::leave(fun_definition *d) {
	indent--;
	stream << ind << "</define>";
}

bool print_xml_tree::enter(fun_call *d) {
	stream << ind << "<fun-call>";
	indent++;
	return true;
}

void print_xml_tree::leave(fun_call *d) {
	indent--;
	stream << ind << "</fun-call>";
}

bool print_xml_tree::enter(branch *) {
	stream << ind << "<if>";
	indent++;
	return true;
}

void print_xml_tree::leave(branch *) {
	indent--;
	stream << ind << "</if>";
}

