#pragma once

#include <string>
#include <vector>
#include <iostream>

class token {
public:
	enum kind { ID, P_OPEN, P_CLOSE };
private:
	kind kind;
	std::string identifier;
	int linenum, column;
	token(enum kind kind, int line, int col) : kind(kind), linenum(line), column(col) {}
public:
	explicit token(const std::string &id, int line, int col) : kind(ID), linenum(line), column(col), identifier(id) {}
	static token open_paren(int line, int col) {
		return token(P_OPEN, line, col);
	}
	static token close_paren(int line, int col) {
		return token(P_CLOSE, line, col);
	}
	enum kind type() const { return kind; }
	const std::string& id() const { return identifier; }
	friend std::ostream& operator<<(std::ostream &out, const token &t);
};

std::vector<token> tokenize(std::istream &source);
