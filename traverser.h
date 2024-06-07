#pragma once

#include "tree.h"

#include <iostream>

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

	virtual bool enter(list *n)           { return enter((node*)n); }
	virtual void leave(list *n)           {}

	virtual bool enter(var_definition *n) { return enter((node*)n); }
	virtual void leave(var_definition *n) {}
	virtual bool enter(fun_definition *n) { return enter((node*)n); }
	virtual void leave(fun_definition *n) {}
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

	bool enter(block *) override;
	void leave(block *) override;

	bool enter(list *) override;
	void leave(list *) override;
	
	bool enter(var_definition *) override;
	void leave(var_definition *) override;
	bool enter(fun_definition *) override;
	void leave(fun_definition *) override;
};

class find_built_ins : public visitor {
public:
// 	bool enter(integer *) override {}
// 	bool enter(name *)    override {}

	bool enter(block *)  override;
	void between_subs(node *, int at)  override {}
	void leave(list *) override {}
};
