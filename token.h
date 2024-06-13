#pragma once

#include <string>
#include <vector>
#include <iostream>

class token {
public:
	enum kind { ID, STR, BOOL, P_OPEN, P_CLOSE };
private:
	kind kind;
	std::string identifier;
	bool boolval;
	int linenum, column;
	token(enum kind kind, int line, int col) : kind(kind), linenum(line), column(col) {}
public:
	explicit token(const std::string &id, int line, int col) : kind(ID), linenum(line), column(col), identifier(id) {}
	explicit token(bool b, int line, int col) : kind(BOOL), linenum(line), column(col), boolval(b) {}
	static token open_paren(int line, int col) {
		return token(P_OPEN, line, col);
	}
	static token close_paren(int line, int col) {
		return token(P_CLOSE, line, col);
	}
	static token string(const std::string &str, int line, int col) {
		token t(STR, line, col);
		t.identifier = str;
		return t;
	}
	enum kind type() const { return kind; }
	const std::string& id() const { return identifier; }
	const bool boolean() const { return boolval; }
	friend std::ostream& operator<<(std::ostream &out, const token &t);
};

std::vector<token> tokenize(std::istream &source);
