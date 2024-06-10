#pragma once
#include "token.h"

#include <string>
#include <optional>
#include <vector>

class visitor;

struct node {
	virtual ~node() {}
	
	virtual void traverse(visitor *v) = 0;
};

struct block : public node {
	~block() { for (node *n : subnodes) delete n; }
	std::vector<node*> subnodes;
	
	void traverse(visitor *v) override;
};

struct literal : public node {
	std::optional<token> tok;
	literal(const token &t) : tok(t) {}
	literal() {}
};

struct integer : public literal {
	long value;
	
	integer(const token &t) : literal(t), value(atoi(t.id().c_str())) {}
	integer(long value) : value(value) {}
	
	void traverse(visitor *v) override;
};

struct definition;

struct name : public literal {
	std::string value;
	::definition *definition = nullptr;
	
	name(const token &t) : literal(t), value(t.id()) {}
	name(const std::string &value) : value(value) {}
	
	void traverse(visitor *v) override;
};

struct list : public node {
	~list() { for (node *n : elements) delete n; }
	std::vector<node*> elements;
	
	void traverse(visitor *v) override;
	node* head() { return elements.front(); }
};

/* 
 * semantic nodes
 * 
 */

struct definition : public node {
	::name *name;
	definition(::name *name) : name(name) {}
	~definition() { delete name; }
};

struct var_definition : public definition {
	node *value;
	void traverse(visitor *v) override;

	var_definition(::name *name, node *value) : definition(name), value(value) {}
	~var_definition() { delete value; }
};

struct fun_definition : public definition {
	std::vector<::var_definition*> params;
	block *body = nullptr;
	void traverse(visitor *v) override;

	fun_definition(::name *name) : definition(name) {}
	~fun_definition() { delete body; for (node *n : params) delete n; }
};
