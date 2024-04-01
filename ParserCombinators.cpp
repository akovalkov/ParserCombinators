// ParserCombinations.cpp: определяет точку входа для приложения.
//
#include <print>
#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <functional>
#include "ParserCombinators.h"


struct ParseResult
{
	ParseResult() = default;
	ParseResult(const std::string_view& value) 
		: values{ value } {}

	ParseResult& operator += (const ParseResult& result) {
		values.insert(end(values), std::begin(result.values), std::end(result.values)); // ????
		return *this;
	}

	std::vector<std::string_view> values;
};

struct ParserState
{
	std::string_view targetString;
	std::size_t index = 0;
	ParseResult result;
	// error stuff
	bool isError = false;
	std::string error;
};

template<>
struct std::formatter<ParseResult>
{
	// parse() is inherited from the base class
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	// Define format() by calling the base class implementation with the wrapped value
	auto format(const ParseResult& t, std::format_context& fc) const
	{
		auto str_results = t.values | std::views::transform([](auto word) { return std::format("\"{}\"", word); }) |
			std::views::join_with(',') | std::ranges::to<std::string>();
		//		std::ranges::copy(t.results | std::views::transform([](auto word) { return std::format("\"{}\"", word); }) |
		//							std::views::join_with(','), std::back_inserter(fc.out()));
		return std::format_to(fc.out(), "{}", str_results);
	}
};

template<>
struct std::formatter<ParserState>
{
	// parse() is inherited from the base class
	constexpr auto parse(std::format_parse_context& ctx) {
		return ctx.begin();
	}

	// Define format() by calling the base class implementation with the wrapped value
	auto format(const ParserState& t, std::format_context& fc) const
	{
		if (t.isError) {
			return std::format_to(fc.out(), "{{\n\ttargetString: \"{}\",\n\tindex: {},\n\terror: {{ {} }}\n}}", t.targetString, t.index, t.error);
		}
		else {
			return std::format_to(fc.out(), "{{\n\ttargetString: \"{}\",\n\tindex: {},\n\tresult: {{ {} }}\n}}", t.targetString, t.index, t.result);
		}
	}
};

struct Parser
{
	// parser transformer = ParserState in -> ParserState out
	// can be lambda, function, method
	std::function<ParserState(const ParserState& state)> transformerFn;

	auto run(const std::string_view& targetString) const
	{
		ParserState initialState{ targetString , 0 };
		return transformerFn(initialState);
	}

	auto map() {}
};

static const auto updateParserState(const ParserState& state, std::size_t index, const ParseResult& result) {
	return ParserState{
		state.targetString,
		index,
		result
	};
}

static const auto updateParserResults(const ParserState& state, const ParseResult& result) {
	return updateParserState(state, state.index, result);
}

static const auto updateParserError(const ParserState& state, const std::string& errorMsg) {
	return ParserState{
		state.targetString,
		state.index,
		state.result,
		true,
		errorMsg
	};
}

auto make_str(const std::string_view& prefix) {
	auto str = [&prefix](const ParserState& state) {
		const auto [targetString, index, _, isError, __] = state;
		if (isError) {
			return state;
		}
		auto slicedTarget = targetString.substr(index);
		if (slicedTarget.length() == 0) {
			// error
			return updateParserError(state,
				std::format("str: Tried to match \"{}\", but got unexpected end of input.", prefix));
		}

		if (slicedTarget.starts_with(prefix)) {
			// success
			return updateParserState(state, index + prefix.length(), prefix);
		}
		// error
		return updateParserError(state,
						std::format("str: Tried to match \"{}\", but got \"{}\"",
						prefix, slicedTarget.substr(0, 10)));
	};

	return Parser{ str };
}

// compile time sequence 
template<typename ... Parsers>
auto make_sequenceOfCompile(Parsers&& ... parsers) {
	auto sequenceOf = [... parsers = std::forward<Parsers>(parsers)](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		ParseResult result;
		auto nextState = state;
		([&]{
			nextState = parsers.transformerFn(nextState);
			if (!nextState.isError) {
				result += nextState.result;
			}
			return !nextState.isError;
		}() && ...);
		// check result
		if (nextState.isError) {
			return nextState;
		} else {
			return updateParserResults(nextState, result);
		}
	};
	return Parser{ sequenceOf };
}

// runtime sequence
auto make_sequenceOf(const std::vector<Parser>& parsers) {
	auto sequenceOf = [parsers](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		ParseResult result;
		auto nextState = state;
		for (auto& parser : parsers) {
			nextState = parser.transformerFn(nextState);
			if (nextState.isError) {
				return nextState;
			}
			result += nextState.result;
		}
		return updateParserResults(nextState, result);
	};
	return Parser{ sequenceOf };
}

void func(const ParserState& state) {}

int main()
{
	try {
		std::println("str");
		auto str_parser = make_str("Hello there!");
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
