

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

TEST_CASE("compile time sequenceOf parser") {
	// compile sequenceOf
	auto compile_seq_parser = make_sequenceOfCompile(
		make_str("Hello there!"),
		make_str("Goodbye there!")
	);
	// success
	auto result = compile_seq_parser.run("Hello there!Goodbye there!");
	ParserState test{
		 "Hello there!Goodbye there!", 26, { {"Hello there!", "Goodbye there!"} }
	};
	CHECK(result == test);
	// fail
	result = compile_seq_parser.run("Hello there!test");
	CHECK(result == ParserState{
		 "Hello there!test", 12, {}, true, "str: Tried to match \"Goodbye there!\", but got \"test\""
	});
	// empty fail
	result = compile_seq_parser.run("");
	CHECK(result == ParserState{
		 "", 0, {}, true, "str: Tried to match \"Hello there!\", but got unexpected end of input."
	});
}

TEST_CASE("run time sequenceOf parser") {
	// runtime sequenceOf
	auto seq_parser = make_sequenceOf({
		make_str("Hello there!"),
		make_str("Goodbye there!")
	});
	// success
	auto result = seq_parser.run("Hello there!Goodbye there!");
	CHECK(result == ParserState{
		 "Hello there!Goodbye there!", 26, { {"Hello there!", "Goodbye there!"} }
	});
	// fail
	result = seq_parser.run("Hello there!test");
	CHECK(result == ParserState{
		 "Hello there!test", 12, {}, true, "str: Tried to match \"Goodbye there!\", but got \"test\""
	});
	// empty fail
	result = seq_parser.run("");
	CHECK(result == ParserState{
		 "", 0, {}, true, "str: Tried to match \"Hello there!\", but got unexpected end of input."
	});
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