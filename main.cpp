#include <print>
#include "ParserCombinators.h"

using namespace Combinators;

void func(const ParserState& state) {}

int main()
{
	try {
		std::println("str");
		auto str_parser = Parsers::str("Hello there!")/*.map([](const ParseResult& result) -> ParseResult {
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
		auto compile_seq_parser = Parsers::sequenceOf(
			Parsers::str("Hello there!"),
			Parsers::str("Goodbye there!")
		);
		result = compile_seq_parser.run("Hello there!Goodbye there!");
		std::println("Success Result: {}", result);
		result = compile_seq_parser.run("Hello there!test");
		std::println("Error Result: {}", result);
		result = compile_seq_parser.run("");
		std::println("Error Result: {}", result);

		// runtime sequenceOf
		std::println("runtime sequenceOf");
		auto seq_parser = Parsers::sequenceOf({
			Parsers::str("Hello there!"),
			Parsers::str("Goodbye there!")
		});
		result = seq_parser.run("Hello there!Goodbye there!");
		std::println("Success Result: {}", result);
		result = seq_parser.run("Hello there!test");
		std::println("Error Result: {}", result);
		result = seq_parser.run("");
		std::println("Error Result: {}", result);
		// letters 
		std::println("letters");
		auto letters_parser = Parsers::letters();
		result = letters_parser.run("Hello123");
		std::println("Success Result: {}", result);
		result = letters_parser.run("123456");
		std::println("Error Result: {}", result);
		// digits
		std::println("digits");
		auto digits_parser = Parsers::digits();
		result = digits_parser.run("123456");
		std::println("Success Result: {}", result);
		result = digits_parser.run("123Hello");
		std::println("Error Result: {}", result);
		// regexp
		std::println("regexp");
		std::regex phoneRegex("\\+\\d \\d{3} \\d{3} \\d{4}");
		auto phone_parser = Parsers::regexp(phoneRegex, "phone");
		result = phone_parser.run("+7 921 123 4567");
		std::println("Success Result: {}", result);
		result = phone_parser.run("Hello");
		std::println("Error Result: {}", result);

	}
	catch (std::runtime_error& err) {
		std::println(std::cerr, "Error: {}", err.what());
	}
	return 0;
}
