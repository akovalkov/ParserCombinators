#include <print>


TEST_CASE("str parser") {
	// str
	auto str_parser = make_str("Hello there!");
	// success
	auto result = str_parser.run("Hello there!");
	CHECK(result == ParserState{
		 "Hello there!", 12, { {"Hello there!"} }
	});
	// fail
	result = str_parser.run("test");
	CHECK(result == ParserState{
		 "test", 0, {}, true, "str: Tried to match \"Hello there!\", but got \"test\""
	});
}


TEST_CASE("letters parser") {
	// letters
	auto letters_parser = make_letters();
	// success
	auto result = letters_parser.run("Hello");
	CHECK(result == ParserState{
		 "Hello", 5, { {"Hello"} }
	});
	// fail
	result = letters_parser.run("123456");
	CHECK(result == ParserState{
		 "123456", 0, {}, true, "letters: Couldn't match letters at index 0"
	});
}


TEST_CASE("digits parser") {
	// digits
	auto digits_parser = make_digits();
	// success
	auto result = digits_parser.run("123456");
	CHECK(result == ParserState{
		 "123456", 6, { {"123456"} }
	});
	// fail
	result = digits_parser.run("Hello");
	CHECK(result == ParserState{
		 "Hello", 0, {}, true, "digits: Couldn't match digits at index 0"
	});
}


TEST_CASE("regexp parser") {
	// phone regexp
	std::regex phoneRegex("\\+\\d \\d{3} \\d{3} \\d{4}");
	auto phone_parser = make_regexp(phoneRegex, "phone");
	// success
	auto result = phone_parser.run("+7 921 123 4567");
	CHECK(result == ParserState{
		 "+7 921 123 4567", 15, { {"+7 921 123 4567"} }
	});
	// fail
	result = phone_parser.run("Hello");
	CHECK(result == ParserState{
		 "Hello", 0, {}, true, "phone: Couldn't match phone at index 0"
	});
}

TEST_CASE("compile time choice parser") {
	// compile choice
	auto compile_choice_parser = make_choice(
		make_letters(),
		make_digits()
	);
	// success
	auto result = compile_choice_parser.run("Hello");
	CHECK(result == ParserState{
		"Hello", 5, { {"Hello"} }
	});
	result = compile_choice_parser.run("123456");
	CHECK(result == ParserState{
		 "123456", 6,{ {"123456"} }
	});
	// fail
	result = compile_choice_parser.run("---");
	ParserState test{
		 "---", 0, {}, true, "choice: Unable to match with any parser at index 0"
	};
	CHECK(result == test);
}

TEST_CASE("run time choice parser") {
	// runtime choice
	auto runtime_choice_parser = make_choice({
		make_letters(),
		make_digits()
	});
	// success
	auto result = runtime_choice_parser.run("Hello");
	CHECK(result == ParserState{
		"Hello", 5, { {"Hello"} }
	});
	result = runtime_choice_parser.run("123456");
	CHECK(result == ParserState{
		 "123456", 6,{ {"123456"} }
	});
	// fail
	result = runtime_choice_parser.run("---");
	ParserState test{
		 "---", 0, {}, true, "choice: Unable to match with any parser at index 0"
	};
	CHECK(result == test);
}


TEST_CASE("star parser") {
	// star parser with choice
	auto star_parser = make_star(make_choice({
		make_letters(),
		make_digits()
	}));
	// success
	auto result = star_parser.run("Hello12345there");
	CHECK(result == ParserState{
		"Hello12345there", 15, { {"Hello", "12345", "there"}}
	});
	result = star_parser.run("");
	CHECK(result == ParserState{
		 "", 0, {}
	});
	result = star_parser.run("hello12345---");
	CHECK(result == ParserState{
		 "hello12345---", 10, { {"hello", "12345"}}
	});
	// fail
	result = star_parser.run("---");
	ParserState test{
		 "---", 0, {}
	};
	CHECK(result == test);
}


TEST_CASE("plus parser") {
	// plus parser with choice
	auto plus_parser = make_plus(make_choice({
		make_letters(),
		make_digits()
	}));
	// success
	auto result = plus_parser.run("Hello12345there");
	CHECK(result == ParserState{
		"Hello12345there", 15, { {"Hello", "12345", "there"}}
	});
	result = plus_parser.run("hello12345---");
	CHECK(result == ParserState{
		 "hello12345---", 10, { {"hello", "12345"}}
	});
	// fail
	result = plus_parser.run("");
	CHECK(result == ParserState{
		 "", 0, {}, true, "plus: Unable to match any input using parser at index 0"
	});
	result = plus_parser.run("---");
	ParserState test{
		 "---", 0, {}, true, "plus: Unable to match any input using parser at index 0"
	};
	CHECK(result == test);
}


TEST_CASE("between brackets parser") {
	// plus parser with choice
	auto parser = make_betweenBrackets(make_letters());
	// success
	auto result = parser.run("(hello)");
	auto test = ParserState{
		"(hello)", 7, { {"hello"} }
	};
	CHECK(result == test);
	// fail
	result = parser.run("(hello");
	test = ParserState{
		 "(hello", 6, {}, true, "str: Tried to match \")\", but got unexpected end of input."
	};
	CHECK(result == test);
}


TEST_CASE("map and mapError") {
	auto str_parser = make_str("Hello there!").map([](const ParseResult& result) -> ParseResult {
		ParseResult ret;
		for (auto value : result.values) {
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { 
				return  static_cast<unsigned char>(std::toupper(c)); 
			});
			ret.values.push_back(value);
		}
		return ret;
	}).mapError([](const std::string&, std::size_t index) -> std::string {
		return std::format("Expected a greeting at index {}", index);
	});

	// success
	auto result = str_parser.run("Hello there!");
	CHECK(result == ParserState{
		 "Hello there!", 12, { {"HELLO THERE!"} }
	});
	// fail
	result = str_parser.run("test");
	CHECK(result == ParserState{
		 "test", 0, {}, true, "Expected a greeting at index 0"
	});

}

TEST_CASE("digits letters sequenceOf parser") {
	// runtime sequenceOf
	auto seq_parser = make_sequenceOf({
		make_digits(),
		make_letters(),
		make_digits()
	});
	// success
	auto result = seq_parser.run("1d2");
	CHECK(result == ParserState{
		 "1d2", 3, { {"1", "d", "2"}}
	});
	// fail
	result = seq_parser.run("12345hello");
	//std::println("{}", result);
	CHECK(result == ParserState{
		 "12345hello", 10, {}, true, "digits: Got unexpected end of input."
		});
	result = seq_parser.run("Hello there!test");
	// fail
	CHECK(result == ParserState{
		 "Hello there!test", 0, {}, true, "digits: Couldn't match digits at index 0"
	}); 
	// empty fail
	result = seq_parser.run("");
	CHECK(result == ParserState{
		 "", 0, {}, true, "digits: Got unexpected end of input."
	});
}


TEST_CASE("chain") {
	auto str_parser = make_letters().map([](const ParseResult& result) -> ParseResult {
		ParseResult ret(result);
		ret += ParseResult{ {"string"} };
		return ret;
	});

	auto number_parser = make_digits().map([](const ParseResult& result) -> ParseResult {
		ParseResult ret(result);
		ret += ParseResult{ {"number"} };
		return ret;
	});

	auto dice_parser = make_sequenceOf(
		make_digits(),
		make_str("d"),
		make_digits()
	).map([](const ParseResult& result) -> ParseResult {
		ParseResult ret;
		ret += result.values[0];
		ret += result.values[2];
		ret += ParseResult{ {"dice"} };
		return ret;
	});

	auto err_parser = make_fail("Unknown type");

	auto parser = make_sequenceOf(
		make_letters(),
		make_str(":")
	).map([](const ParseResult& result) -> ParseResult {
		ParseResult ret({ result.values[0] });
		return ret;
	}).chain([&](const ParseResult& result) -> const Parser {
		auto type = result.values[0];
		if (type == "string") {
			return str_parser;
		} else if (type == "number") {
			return number_parser;
		} else if (type == "dice") {
			return dice_parser;
		}
		return err_parser;
	});
	// success
	auto result = parser.run("string:hello");
	auto test = ParserState{
		"string:hello", 12, { {"hello", "string"}}
	};
	CHECK(result == test);
	result = parser.run("number:42");
	test = ParserState{
		"number:42", 9, { {"42", "number"}}
	};
	CHECK(result == test);
	result = parser.run("dice:2d6");
	test = ParserState{
		"dice:2d6", 8, { {"2", "6", "dice"}}
	};
	CHECK(result == test);
	// fail
	result = parser.run("test:2d6");
	test = ParserState{
		"test:2d6", 5, {}, true, "Unknown type"
	};
	CHECK(result == test);
}


TEST_CASE("sepBy") {
	// separator by comma
	auto brackets_parser = make_between(make_str("["), make_str("]"));
	auto comma_parser = make_sepBy_star(make_str(","));

	auto parser = brackets_parser(comma_parser(make_digits()));
	// success
	auto result = parser.run("[1,2,3,4,5,6]");
	auto test = ParserState{
		"[1,2,3,4,5,6]", 13, { {"1", "2", "3", "4", "5", "6"}}
	};
	CHECK(result == test);
}

TEST_CASE("lazy recursive sepBy") {
	// separator by comma
	auto brackets_parser = make_between(make_str("["), make_str("]"));
	auto comma_parser = make_sepBy_star(make_str(","));

	Parser array_parser;
	auto value_parser = make_lazy([&array_parser]() {
		return make_choice(
			make_digits(),
			array_parser
		);
	});

	array_parser = brackets_parser(comma_parser(value_parser));
	// success
	auto result = array_parser.run("[1,[2,[3],4],5]");
	auto test = ParserState{
		"[1,[2,[3],4],5]", 15, { {"1", "2", "3", "4", "5"}}
	};
	CHECK(result == test);
}

TEST_CASE("recursive sepBy") {
	// separator by comma
	auto brackets_parser = make_between(make_str("["), make_str("]"));
	auto comma_parser = make_sepBy_star(make_str(","));

	Parser array_parser;
	auto value_parser = make_choice(
		make_digits(),
		array_parser
	);

	array_parser = brackets_parser(comma_parser(value_parser));
	// exception
	CHECK_THROWS_AS(array_parser.run("[1,[2,[3],4],5]"), std::exception); 
}


TEST_CASE("succeed and fail") {
	// success
	ParseResult value{ {"succeed"} };
	auto succeed_parser = make_succeed(value);
	auto result = succeed_parser.run("test");
	CHECK(result.result == value);
	// fail
	auto err_parser = make_fail("Unknown type");
	result = err_parser.run("test");
	auto test = ParserState{
		"test", 0, {}, true, "Unknown type"
	};
	CHECK(result == test);
}

TEST_CASE("contextual simple") {
	auto parser = make_choice({
		make_str("VAR "),
		make_str("GLOBAL_VAR ")
	}).chain([&](const ParseResult& declarationType) -> const Parser {
		return make_letters().chain([&](const ParseResult & varName) -> const Parser {
			return make_choice({
				make_str(" INT "),
				make_str(" STRING "),
				make_str(" BOOL ")
			}).chain([&](const ParseResult& type) -> const Parser {
				auto strType = type.values[0];
				if (strType == " INT ") {
					return make_digits().map([&](const ParseResult& result) -> ParseResult {
						ParseResult newresult;
						newresult += declarationType.values[0].substr(0, declarationType.values[0].length() - 1);
						newresult += varName.values[0];
						newresult += "number";
						newresult += result.values[0];
						return newresult;
					});
				} else if (strType == " STRING ") {
					return make_between(
						make_str("\""),
						make_str("\"")
					)(make_letters()).map([&](const ParseResult& result) -> ParseResult {
						ParseResult newresult;
						newresult += declarationType.values[0].substr(0, declarationType.values[0].length() - 1);
						newresult += varName.values[0];
						newresult += "string";
						newresult += result.values[0];
						return newresult;
					});
				} else if (strType == " BOOL ") {
					return make_choice({
						make_str("true"),
						make_str("false")
					}).map([&](const ParseResult& result) -> ParseResult {
						ParseResult newresult;
						newresult += declarationType.values[0].substr(0, declarationType.values[0].length() - 1);
						newresult += varName.values[0];
						newresult += "boolean";
						newresult += result.values[0];
						return newresult;
					});
				} else {
					return make_fail("Unknown variable type");
				}
			});
		});
	});


	auto result = parser.run("VAR theAnswer INT 42");
	auto test = ParserState{
		"VAR theAnswer INT 42", 20, { {"VAR", "theAnswer", "number", "42"}}
	};
	CHECK(result == test);

	result = parser.run("GLOBAL_VAR greeting STRING \"Hello\"");
	test = ParserState{
		"GLOBAL_VAR greeting STRING \"Hello\"", 34, { {"GLOBAL_VAR", "greeting", "string", "Hello"}}
	};
	CHECK(result == test);
	
	result = parser.run("VAR skyIsBlue BOOL true");
	test = ParserState{
		"VAR skyIsBlue BOOL true", 23, { {"VAR", "skyIsBlue", "boolean", "true"}}
	};
	CHECK(result == test);
}


TEST_CASE("contextual coroutine") {
	auto parser = make_contextual([]() -> Generator<ParseResult, Parser> {
		const ParseResult declarationType = co_yield make_choice(
														make_str("VAR "),
														make_str("GLOBAL_VAR ")
													 );
		const ParseResult varName = co_yield make_letters();
		const ParseResult type = co_yield make_choice(
											make_str(" INT "),
											make_str(" STRING "),
											make_str(" BOOL ")
										);
		ParseResult data;
		auto strType = type.values[0];
		std::string resultType;
		if (strType == " INT ") {
			resultType = "number";
			data = co_yield make_digits();
		}else if (strType == " STRING ") {
			resultType = "string";
			data = co_yield make_between(
								make_str("\""),
								make_str("\"")
							)(make_letters());
		}
		else if (strType == " BOOL ") {
			resultType = "boolean";
			data = co_yield make_choice(
								make_str("true"),
								make_str("false")
							);
		} else {
			ParseResult error = co_yield make_fail("Unknown variable type");
			co_return error;
		}
		ParseResult newresult;
		newresult += declarationType.values[0].substr(0, declarationType.values[0].length() - 1);
		newresult += varName.values[0];
		newresult += resultType;
		newresult += data.values[0];
		co_return newresult;
	});

	auto result = parser.run("VAR theAnswer INT 42");
	auto test = ParserState{
		"VAR theAnswer INT 42", 20, { {"VAR", "theAnswer", "number", "42"}}
	};
	CHECK(result == test);

	result = parser.run("GLOBAL_VAR greeting STRING \"Hello\"");
	test = ParserState{
		"GLOBAL_VAR greeting STRING \"Hello\"", 34, { {"GLOBAL_VAR", "greeting", "string", "Hello"}}
	};
	CHECK(result == test);

	result = parser.run("VAR skyIsBlue BOOL true");
	test = ParserState{
		"VAR skyIsBlue BOOL true", 23, { {"VAR", "skyIsBlue", "boolean", "true"}}
	};
	CHECK(result == test);
}
