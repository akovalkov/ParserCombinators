#include <print>
#include "ParserCombinators.h"

void func(const ParserState& state) {}

int main()
{
	try {
		std::println("str");
		auto str_parser = make_str("Hello there!")/*.map([](const ParseResult& result) -> ParseResult {
			ParseResult ret;
			for (auto value : result.values) {
				std::transform(value.begin(), value.end(), value.begin(), ::toupper);
				ret.values.push_back(value);
			}
			return ret;
		}).mapError([](const std::string&, std::size_t index) -> std::string {
			return std::format("Expected a greeting at index {}", index);
		})*/;

		auto result = str_parser.run("Hello there!");
		std::println("Success Result: {}", result);
		result = str_parser.run("test");
		std::println("Error Result: {}", result);

		// compile sequenceOf
		std::println("compile time sequenceOf");
		auto compile_seq_parser = make_sequenceOfCompile(
			make_str("Hello there!"),
			make_str("Goodbye there!")
		);
		result = compile_seq_parser.run("Hello there!Goodbye there!");
		std::println("Success Result: {}", result);
		result = compile_seq_parser.run("Hello there!test");
		std::println("Error Result: {}", result);
		result = compile_seq_parser.run("");
		std::println("Error Result: {}", result);

		// runtime sequenceOf
		std::println("runtime sequenceOf");
		auto seq_parser = make_sequenceOf({
			make_str("Hello there!"),
			make_str("Goodbye there!")
		});
		result = seq_parser.run("Hello there!Goodbye there!");
		std::println("Success Result: {}", result);
		result = seq_parser.run("Hello there!test");
		std::println("Error Result: {}", result);
		result = seq_parser.run("");
		std::println("Error Result: {}", result);

	}
	catch (std::runtime_error& err) {
		std::println(std::cerr, "Error: {}", err.what());
	}
	return 0;
}
