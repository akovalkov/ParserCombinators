#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ranges>
#include <algorithm>
#include <functional>
#include <regex>

struct ParseResult
{

	//std::vector<std::string_view> values;
	std::vector<std::string> values; // because the map function

	ParseResult& operator += (const ParseResult& result) {
		values.insert(end(values), std::begin(result.values), std::end(result.values));
		return *this;
	}

	bool operator==(const ParseResult&) const = default;
};

struct ParserState
{
	std::string_view targetString;
	std::size_t index = 0;
	ParseResult result;
	// error stuff
	bool isError = false;
	std::string error;
	bool operator==(const ParserState&) const = default;

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

const ParserState updateParserState(const ParserState& state, std::size_t index, const ParseResult& result);
const ParserState updateParserResults(const ParserState& state, const ParseResult& result);
const ParserState updateParserError(const ParserState& state, const std::string& errorMsg);


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

	// parse result transformer = ParseResult in -> ParseResult out
	// can be lambda, function, method
	auto map(std::function<ParseResult(const ParseResult&)> fn) {
		auto mapFn = [transformerFn = transformerFn, fn](const ParserState& state) {
			const auto nextState = transformerFn(state);
			if (nextState.isError) {
				return nextState;
			}
			return updateParserResults(nextState, fn(nextState.result));
			};
		return Parser{ mapFn };
	}

	// parse error transformer = errMsg and index in -> string out
	// can be lambda, function, method
	auto mapError(std::function<std::string(const std::string&, std::size_t index)> fn) {
		auto mapErrFn = [transformerFn = transformerFn, fn](const ParserState& state) {
			const auto nextState = transformerFn(state);
			if (!nextState.isError) {
				return nextState;
			}
			return updateParserError(nextState, fn(nextState.error, nextState.index));
			};
		return Parser{ mapErrFn };
	}
};


Parser make_str(const std::string_view& prefix);
Parser make_regexp(const std::regex& re, const std::string_view& name = "regexp");
Parser make_letters();
Parser make_digits();

// runtime sequence
Parser make_sequenceOf(const std::vector<Parser>& parsers);

// runtime choice
Parser make_choice(const std::vector<Parser>& parsers);

Parser make_plus(const Parser& parser);
Parser make_star(const Parser& parser);


// compile time sequence 
template<typename ... Parsers>
auto make_sequenceOfCompile(Parsers&& ... parsers) {
	auto sequenceOf = [... parsers = std::forward<Parsers>(parsers)](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		ParseResult result;
		auto nextState = state;
		([&] {
			nextState = parsers.transformerFn(nextState);
			if (!nextState.isError) {
				result += nextState.result;
			}
			return !nextState.isError;
			}() && ...);
		// check result
		if (nextState.isError) {
			return nextState;
		}
		else {
			return updateParserResults(nextState, result);
		}
	};
	return Parser{ sequenceOf };
}

// compile time choice 
template<typename ... Parsers>
auto make_choiceCompile(Parsers&& ... parsers) {
	auto choice = [... parsers = std::forward<Parsers>(parsers)](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		auto nextState = state;
		([&] {
			nextState = parsers.transformerFn(state);
			return nextState.isError;
			}() && ...);
		// check result
		if (!nextState.isError) {
			return nextState;
		} else {
			return updateParserError(state, 
				std::format("choice: Unable to match with any parser at index {}", state.index));
		}
	};
	return Parser{ choice };
}

