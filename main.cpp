#include "token.h"
#include "syntax.h"
#include "traverser.h"

#include <fstream>
#include <set>

using std::cout, std::endl, std::cerr;

bool show_parsed = false, show_replaced = false, show_resolved = false, verbose_steps = true, show_tokens = false, show_forms = false;

struct interpreter {
	std::vector<std::pair<visitor*, std::string>> passes;
	node *tree = nullptr;
	template<typename T> T* add_pass(T *v, const std::string &text) {
		passes.push_back({v,text});
		return v;
	}
	interpreter() {
		auto *pp = new print_xml_tree(cout);
		if (show_parsed)
			add_pass(pp, "Tree as parsed from tokenstream:");
		
		add_pass(new find_built_ins, "Replacing lists with built-in forms...");
		if (show_replaced)
			add_pass(pp, "Tree with language primitives inserted:");

		auto *rn = add_pass(new resolve_names, "Resolving names, aka linking names to their definitions...");
		if (show_resolved)
			add_pass(pp, "Tree with resolved names:");

		add_pass(new interprete(rn->builtins(), rn->pre_defined()), "Interpreting program...");
	}
	void run(const std::vector<form> &forms) {
		node *tree = to_tree(forms);
		try {
			for (auto [pass,text] : passes) {
				if (verbose_steps)
					cout << text << endl;
				tree->traverse(pass);
			}
		}
		catch (syntax_error e) {
			cerr << e.what() << endl;
		}
		delete tree;
	}
	~interpreter() {
		std::set<visitor*> deleted;
		for (auto [pass,text] : passes)
			if (deleted.count(pass) == 0) {
				delete pass;
				deleted.insert(pass);
			}
	}
};

int main(int argc, char **argv) {
	std::string filename;
	for (int i = 1; i < argc; ++i) { // for such things we should rather use argp, my implementation here sucks ;)
		std::string arg = argv[i];   // see https://www.gnu.org/software/libc/manual/html_node/Argp.html
		if (argv[i][0] == '-')       
			if (arg == "--print-parsed")        show_parsed = true;
			else if (arg == "--print-replaced") show_replaced = true;
			else if (arg == "--print-resolved") show_resolved = true;
			else if (arg == "--print-tokens")   show_tokens = true;
			else if (arg == "--print-nesting")  show_forms = true;
			else if (arg == "-p")               show_parsed = true;
			else if (arg == "-r")               show_replaced = true;
			else if (arg == "-R")               show_resolved = true;
			else if (arg == "-t")               show_tokens = true;
			else if (arg == "-n")               show_forms = true;
			else if (arg == "-q")               verbose_steps = false;
			else if (arg == "-Q")               verbose_steps = false;
			else { cerr << "Invalid argument '" << argv[i] << "'" << endl; return -1; }
		else if (filename == "")
			filename = arg;
		else { cerr << "Only one input file is supported at the moment, sorry." << endl; return -1; }
	}

	if (verbose_steps)
		cout << "Reading token stream from file..." << endl;
	std::ifstream input(argv[1]);
	std::vector<token> tokenstream = tokenize(input);
	if (show_tokens) {
		for (auto x : tokenstream)
			cout << x << endl;
		cout << endl;
	}

	if (verbose_steps)
		cout << "Parsing nested structure from tokens..." << endl;
	std::vector<form> forms = parse(tokenstream);
	if (show_forms) {
		for (auto x : forms)
			cout << x << endl;
		cout << endl;
	}

	interpreter().run(forms);
}
