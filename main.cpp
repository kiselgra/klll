#include "token.h"
#include "syntax.h"
#include "traverser.h"

#include <fstream>

using std::cout, std::endl;

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
// 
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

	delete tree;
}
