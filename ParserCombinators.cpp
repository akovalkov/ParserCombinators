// ParserCombinations.cpp: определяет точку входа для приложения.
//
#include <print>
#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <functional>
#include "ParserCombinators.h"

struct ParserState
{
	std::string_view targetString;
	std::size_t index = 0;
	std::vector<std::string_view> results;
	// error stuff
	bool isError = false;
	std::string error;
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
		} else {
			auto str_results = t.results | std::views::transform([](auto word) { return std::format("\"{}\"", word); }) |
				std::views::join_with(',') | std::ranges::to<std::string>();
			//		std::ranges::copy(t.results | std::views::transform([](auto word) { return std::format("\"{}\"", word); }) |
			//							std::views::join_with(','), std::back_inserter(fc.out()));
			return std::format_to(fc.out(), "{{\n\ttargetString: \"{}\",\n\tindex: {},\n\tresult: {{ {} }}\n}}", t.targetString, t.index, str_results);
		}
	}
};


template<typename T> concept ParserFunction = requires(T t, const ParserState & state)  {
	{ t(state) } -> std::same_as<ParserState>;
};

static const auto updateParserState(const ParserState& state, std::size_t index, const std::vector<std::string_view>& results) {
	return ParserState{
		state.targetString,
		index,
		results
	};
}

static const auto updateParserState(const ParserState& state, std::size_t index, const std::string_view& result) {
	return updateParserState(state, index, std::vector<std::string_view>{ result });
}

static const auto updateParserResults(const ParserState& state, const std::string_view& result) {
	return updateParserState(state, state.index, result);
}

static const auto updateParserResults(const ParserState& state, const std::vector<std::string_view>& results) {
	return updateParserState(state, state.index, results);
}

static const auto updateParserError(const ParserState& state, const std::string& errorMsg) {
	return ParserState{
		state.targetString,
		state.index,
		state.results,
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

	return str;
}

auto make_sequenceOf(const std::vector<std::function<ParserState (const ParserState& state)>>& parsers) {
	auto sequenceOf = [parsers](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		std::vector<std::string_view> results;
		auto nextState = state;
		for (auto& parser : parsers) {
			nextState = parser(nextState);
			if (nextState.isError) {
				return nextState;
			}
			results.insert(end(results), begin(nextState.results), end(nextState.results)); // ????
		}
		return updateParserResults(nextState, results);
	};
	return sequenceOf;
}

// parser = ParserState in -> ParserState out
template<ParserFunction Parser>
auto run(Parser parser, const std::string_view& targetString) {
	ParserState initalState{ targetString , 0};
	return parser(initalState);
}


void func(const ParserState& state) {}


int main()
{
	try {
		std::println("str");
		auto str_parser = make_str("Hello there!");
		auto result = run(str_parser, "Hello there!");
		std::println("Success Result: {}", result);
		result = run(str_parser, "test");
		std::println("Error Result: {}", result);


		std::println("sequenceOf");
		auto seq_parser = make_sequenceOf({
			make_str("Hello there!"),
			make_str("Goodbye there!")
		});
		result = run(seq_parser, "Hello there!Goodbye there!");
		std::println("Success Result: {}", result);
		result = run(seq_parser, "Hello there!test");
		std::println("Error Result: {}", result);
		result = run(seq_parser, "");
		std::println("Error Result: {}", result);


	}
	catch (std::runtime_error& err) {
		std::println(std::cerr, "Error: {}", err.what());
	}
	return 0;
}
