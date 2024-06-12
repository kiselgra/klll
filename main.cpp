#include "token.h"
#include "syntax.h"
#include "traverser.h"

#include <fstream>
#include <set>

using std::cout, std::endl, std::cerr;

struct interpreter {
	std::vector<visitor*> passes;
	node *tree = nullptr;
	interpreter() {
		auto *pp = new print_xml_tree(cout);
		passes.push_back(pp);
		passes.push_back(new find_built_ins);
		passes.push_back(pp);
		auto *rn = new resolve_names;
		passes.push_back(rn);
		passes.push_back(pp);
		passes.push_back(new interprete(rn->builtins()));
	}
	void run(const std::vector<form> &forms) {
		node *tree = to_tree(forms);
		try {
			for (auto *pass : passes)
				tree->traverse(pass);
		}
		catch (syntax_error e) {
			cerr << e.what() << endl;
		}
		delete tree;
	}
	~interpreter() {
		std::set<visitor*> deleted;
		for (auto *pass : passes)
			if (deleted.count(pass) == 0) {
				delete pass;
				deleted.insert(pass);
			}
	}
};

int main(int argc, char **argv) {
	std::ifstream input(argv[1]);
	std::vector<token> tokenstream = tokenize(input);
	for (auto x : tokenstream)
		cout << x << endl;

	cout << " - - - - - - - - - - - - " << endl;
	std::vector<form> forms = parse(tokenstream);
	for (auto x : forms)
		cout << x << endl;
	cout << " - - - - - - - - - - - - " << endl;

	interpreter().run(forms);
// 
	/*
	node *tree = to_tree(forms);

	print_xml_tree pt(cout);
	tree->traverse(&pt);
	
	cout << endl << " - - - - - - - - - - - - " << endl;
	cout << "finding builtins..." << endl;

	find_built_ins bi;
	tree->traverse(&bi);
	tree->traverse(&pt);

	cout << endl << " - - - - - - - - - - - - " << endl;
	cout << "resolving names..." << endl;
	resolve_names rn;
	tree->traverse(&rn);

	tree->traverse(&pt);
	
	cout << endl << " - - - - - - - - - - - - " << endl;
	cout << "interpreting program..." << endl;
// 	interprete ip;
// 	tree->traverse(&ip);

	delete tree;
	*/
}
