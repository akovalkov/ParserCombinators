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
	size_t index = 0;
	std::vector<std::string_view> results;
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
		auto str_results = t.results | std::views::transform([](auto word) { return std::format("\"{}\"", word); }) |
			 std::views::join_with(',') | std::ranges::to<std::string>();
//		std::ranges::copy(t.results | std::views::transform([](auto word) { return std::format("\"{}\"", word); }) |
//							std::views::join_with(','), std::back_inserter(fc.out()));
		return std::format_to(fc.out(), "{{\n\ttargetString: \"{}\",\n\tindex: {},\n\tresult: {{ {} }}\n}}", t.targetString, t.index, str_results);
	}
};


template<typename T> concept ParserFunction = requires(T t, const ParserState & state)  {
	{ t(state) } -> std::same_as<ParserState>;
};


auto make_str(const std::string_view& prefix) {
	auto str = [&prefix](const ParserState& state) {
		const auto [targetString, index, _] = state;
		if (targetString.substr(index).starts_with(prefix)) {
			ParserState nextState{ state.targetString };
			nextState.index += prefix.length();
			nextState.results.push_back(prefix);
			return nextState;
		}
		throw std::runtime_error(std::format("Str: Tried to match \"{}\", but got \"{}\"", 
								prefix, targetString.substr(index, index + 10)));
	};

	return str;
}

auto make_sequenceOf(const std::vector<std::function<ParserState (const ParserState& state)>>& parsers) {
	auto sequenceOf = [parsers = std::move(parsers)](const ParserState& state) {
		ParserState finalState{ state.targetString };
		std::vector<std::string_view> results;
		auto nextState = state;
		for (auto& parser : parsers) {
			nextState = parser(nextState);
			finalState.results.insert(end(finalState.results), begin(nextState.results), end(nextState.results)); // ????
		}
		finalState.index = nextState.index;
		return finalState;
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
	auto parser = make_sequenceOf({
		make_str("Hello there!"),
		make_str("Goodbye there!")
	});
	try {
		auto result = run(parser, "Hello there!Goodbye there!");
		std::println("Result: {}", result);
		run(parser, "test");
	}
	catch (std::runtime_error& err) {
		std::println(std::cerr, "Error: {}", err.what());
	}
	return 0;
}
