// ParserCombinations.cpp: определяет точку входа для приложения.
//
#include <print>
#include <iostream>
#include <string>
#include "ParserCombinators.h"

struct ParserState
{
	std::string_view targetString;
	size_t index = 0;
	std::string_view result;
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
		return std::format_to(fc.out(), "{{\n\ttargetString: \"{}\",\n\tindex: {},\n\tresult: \"{}\"\n}}", t.targetString, t.index, t.result);
	}
};


template<typename T> concept ParserFunction = requires(T t, const ParserState & state)  {
	{ t(state) } -> std::same_as<ParserState>;
};


auto make_str(const std::string_view& prefix) {
	auto str = [&prefix](const ParserState& state) {
		const auto [targetString, index, _] = state;
		if (targetString.substr(index).starts_with(prefix)) {
			ParserState nextState{ state };
			nextState.index += prefix.length();
			nextState.result = prefix;
			return nextState;
		}
		throw std::runtime_error(std::format("Tried to match \"{}\", but got \"{}\"", 
								prefix, targetString.substr(index, index + 10)));
	};

	return str;
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
	auto str = make_str("Hello there!");
	try {
		auto result = run(str, "Hello there!");
		std::println("Result: {}", result);
		run(str, "test");
	}
	catch (std::runtime_error& err) {
		std::println(std::cerr, "Error: {}", err.what());
	}
	return 0;
}
