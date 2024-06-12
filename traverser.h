#pragma once

#include "tree.h"

#include <iostream>
#include <map>

class visitor {
public:
	virtual ~visitor() {}
	virtual bool enter(node *) { return true;}
	virtual void leave(node *) {}
	virtual void between_subs(node *, int at) {}

	virtual bool enter(integer *n)        { return enter((node*)n); }
	virtual bool enter(name *n)           { return enter((node*)n); }

	virtual bool enter(block *n)          { return enter((node*)n); }
	virtual void leave(block *n)          {}
	virtual bool enter(toplevel_block *n) { return enter((node*)n); }
	virtual void leave(toplevel_block *n) {}

	virtual bool enter(list *n)           { return enter((node*)n); }
	virtual void leave(list *n)           {}

	virtual bool enter(var_definition *n) { return enter((node*)n); }
	virtual void leave(var_definition *n) {}
	virtual bool enter(fun_definition *n) { return enter((node*)n); }
	virtual void leave(fun_definition *n) {}
	
	virtual bool enter(fun_call *n)       { return enter((node*)n); }
	virtual void leave(fun_call *n)       {}
	
	virtual bool enter(branch *n)         { return enter((node*)n); }
	virtual void leave(branch *n)         {}
};

class print_tree : public visitor {
	std::ostream &stream;
	int indent = 0;
public:
	print_tree(std::ostream &stream) : stream(stream) {
	}
	~print_tree() {}
	
	void between_subs(node *n, int at) override;

	bool enter(integer *) override;
	bool enter(name *)    override;

	bool enter(list *) override;
	void leave(list *) override;
	
	bool enter(var_definition *) override;
	void leave(var_definition *) override;
};

class print_xml_tree : public visitor {
	std::ostream &stream;
	int indent = 0;
	struct indent_manip {
		print_xml_tree *tree;
		indent_manip(print_xml_tree *tree) : tree(tree) {}
	} ind;
public:
	print_xml_tree(std::ostream &stream) : stream(stream), ind(this) {
	}
	~print_xml_tree() {}
	friend std::ostream& operator<<(std::ostream &out, indent_manip i);
	
	bool enter(integer *) override;
	bool enter(name *)    override;

	bool enter(toplevel_block *)  override;
	void leave(toplevel_block *)  override;
	bool enter(block *) override;
	void leave(block *) override;

	bool enter(list *) override;
	void leave(list *) override;
	
	bool enter(var_definition *) override;
	void leave(var_definition *) override;
	bool enter(fun_definition *) override;
	void leave(fun_definition *) override;
	
	bool enter(fun_call *) override;
	void leave(fun_call *) override;

	bool enter(branch *) override;
	void leave(branch *) override;
};

class find_built_ins : public visitor {
public:
// 	bool enter(integer *) override {}
// 	bool enter(name *)    override {}

	bool enter(toplevel_block *)  override;
	bool enter(block *)           override;
	bool enter(list *)            override;
	bool enter(branch *)          override;
	bool enter(var_definition *)  override;
	bool enter(fun_call *)        override;
	void between_subs(node *, int at)  override {}
};

class resolve_names : public visitor {
	std::vector<definition*> definitions;
	std::map<std::string, builtin_function*> builtin_functions;
public:
	resolve_names();
	~resolve_names();
	// what to record
	bool enter(var_definition *) override;
	bool enter(fun_definition *) override;
	void leave(fun_definition *) override;
	// where to record
	bool enter(name *) override;

	std::map<std::string, builtin_function*>& builtins() { return builtin_functions; }
};

class interprete : public visitor {
public:
	struct value {
		typedef void (*built_in)(std::vector<value> &);
		enum kind { INT, BOOL, FUN, BI } kind;
		long integer = 0;
		fun_definition *fd = nullptr;
		built_in bi = nullptr;
		bool b = false;
		value(int i) : integer(i), kind(INT) {}
		value(fun_definition *fd) : fd(fd), kind(FUN) {}
		value(built_in bi) : bi(bi), kind(BI) {}
		static value value_true()  { value v = 0; v.b = true;  v.kind = value::BOOL; return v; }
		static value value_false() { value v = 0; v.b = false; v.kind = value::BOOL; return v; }
	};
private:
	std::vector<std::map<definition*, value>> bindings;
	std::vector<std::vector<value>> value_stack;
	void push_value_frame();
	void pop_value_frame();
	void push_value(value v);
	value pop_value();
	value pop_first_value();
	std::vector<value>& value_frame();

	void add_binding_frame();
	void drop_binding_frame();
	void add_binding(definition *def, value v);
	value find_binding(definition *def);

public:
	interprete(std::map<std::string, builtin_function*> &builtins);
	bool enter(integer *) override;
	bool enter(name *) override;
	
	bool enter(toplevel_block *) override;
	void leave(toplevel_block *) override;
	bool enter(block *)          override;
	void leave(block *)          override;

	bool enter(list *) override;
	void leave(list *) override;
	
	bool enter(var_definition *) override;
	void leave(var_definition *) override;
	bool enter(fun_definition *) override;
	void leave(fun_definition *) override;
	
	bool enter(fun_call *) override;
	void leave(fun_call *) override;

	bool enter(branch *) override;
	void leave(branch *) override;
};
