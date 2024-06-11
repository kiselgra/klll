#pragma once
#include "token.h"

#include <string>
#include <optional>
#include <vector>

class visitor;

//! This implements a pointer that simply refers to some data, but is not responsible for managing it.
template<typename T> struct ref {
	T *ptr;
	T* operator->() { return ptr; }
	T& operator*() { return *ptr; }
	ref(T *t) : ptr(t) {}
};

struct node {
	virtual ~node() {}
	
	virtual void traverse(visitor *v) = 0;
};

struct block : public node {
	~block() { for (node *n : subnodes) delete n; }
	std::vector<node*> subnodes;
	
	void traverse(visitor *v) override;
};

struct toplevel_block : public block {
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
	ref<::definition> definition = nullptr;
	
	name(const token &t) : literal(t), value(t.id()) {}
	name(const std::string &value) : value(value) {}
	~name() {}

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
	std::string name;
	definition(const std::string &name) : name(name) {}
};

struct var_definition : public definition {
	node *value;
	void traverse(visitor *v) override;

	var_definition(const std::string &name, node *value) : definition(name), value(value) {}
	~var_definition() { delete value; }
};

struct fun_definition : public definition {
	std::vector<::var_definition*> params;
	block *body = nullptr;
	void traverse(visitor *v) override;

	fun_definition(const std::string &name) : definition(name) {}
	~fun_definition() { delete body; for (node *n : params) delete n; }
};

struct builtin_function : public fun_definition {
	builtin_function(const std::string &name) : fun_definition(name) {}
};

struct fun_call : public node {
	::name *name;
	std::vector<node*> args;
	void traverse(visitor *v) override;

	fun_call(::name *name) : name(name) {}
	~fun_call() { delete name; for (node *n : args) delete n; }
};

struct branch : public node {
	node *condition, *true_case, *false_case;
	void traverse(visitor *v) override;

	branch(node *condition, node *true_case, node *false_case = nullptr) : condition(condition), true_case(true_case), false_case(false_case) {}
	~branch() { delete condition; delete true_case; delete false_case; }
};
