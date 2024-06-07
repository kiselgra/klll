#pragma once
#include "token.h"

#include <string>
#include <optional>
#include <vector>

class visitor;

struct node {
	node *parent = nullptr;
	node(node *parent = nullptr) : parent(parent) {
	}
	virtual ~node() {}
	
	virtual void traverse(visitor *v) = 0;
};

struct block : public node {
	~block() {}
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

struct name : public literal {
	std::string value;
	
	name(const token &t) : literal(t), value(t.id()) {}
	name(const std::string &value) : value(value) {}
	
	void traverse(visitor *v) override;
};

struct list : public node {
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
};

struct var_definition : public definition {
	node *value;
	void traverse(visitor *v) override;

	var_definition(::name *name, node *value) : definition(name), value(value) {}
};

struct fun_definition : public definition {
	std::vector<::name*> params;
	block *body = nullptr;
	void traverse(visitor *v) override;

	fun_definition(::name *name) : definition(name) {}
};
