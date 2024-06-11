#pragma once

#include <stdexcept>
#include <optional>

#include "token.h"
#include "tree.h"

class form {
	enum kind { ATOM, LIST };
	kind kind;
	std::vector<form> subforms;
	token atom;
public:
	form(token t) : kind(ATOM), atom(t) {}
	form() : kind(LIST), atom("", 0, 0) {}
	void add(const form &f) { subforms.push_back(f); }
// 	void add(form &&f) { subforms.push_back(std::move(f)); }
	bool is_atom() const { return subforms.size() == 0; }
	const token& value() const { return atom; }
	const std::vector<form>& sub() const { return subforms; }
	friend std::ostream& operator<<(std::ostream &out, const form &t);
};

std::vector<form> parse(const std::vector<token> &tokenstream);

class syntax_error : public std::runtime_error {
	std::optional<token> at_token;
	std::string message;
	syntax_error(const std::string &msg, std::optional<token> t);
public:
	syntax_error(const std::string &msg, const token &t) : syntax_error(msg, std::optional<token>(t)) {}
	syntax_error(const std::string &msg) : syntax_error(msg, std::nullopt) {}
	const char* what() const noexcept {
		return message.c_str();
	}
};

node* to_tree(const std::vector<form> &f);

