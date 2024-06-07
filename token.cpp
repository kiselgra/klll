#include "token.h"

using std::vector, std::string;
	
std::ostream& operator<<(std::ostream &out, const token &t) {
	out << "[";
	switch (t.kind) {
	case token::ID:      out << "I:" << t.identifier; break;
	case token::P_OPEN:  out << "(";                  break;
	case token::P_CLOSE: out << ")";                  break;
	}
	out << "@" << t.linenum << ":" << t.column << "]";
	return out;
}

vector<token> tokenize(std::istream &source) {
	vector<token> tokens;
	char c;
	int line = 1, col = 1;
	while (source.get(c)) {
		if (c == '(')
			tokens.push_back(token::open_paren(line, col++));
		else if (c == ')')
			tokens.push_back(token::close_paren(line, col++));
		else if (isgraph(c)) {
			string accum(1, c);
			while (source.get(c))
				if (c == '(' || c == ')' || isspace(c)) {
					source.putback(c);
					break;
				}
				else
					accum += c;
			tokens.push_back(token(accum, line, col));
			col += accum.length();
		}
		else if (c == '\n') line++, col=1;
		else if (c == '\r') ;
		else col++;
// 		else
// 			std::cout << "IGN: [" << (int)c << "]" << std::endl;
	}
	return tokens;
}


