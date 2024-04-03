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
	auto compile_choice_parser = make_choiceCompile(
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


TEST_CASE("map and mapError") {
	auto str_parser = make_str("Hello there!").map([](const ParseResult& result) -> ParseResult {
		ParseResult ret;
		for (auto value : result.values) {
			std::transform(value.begin(), value.end(), value.begin(), ::toupper);
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
