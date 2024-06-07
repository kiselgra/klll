#pragma once

#include "syntax.h"

#include <optional>

class value {
	enum kind { ATOM, LIST };
public:
	std::optional<::token> token;
};

form eval(const form &f);
