// ParserCombinations.cpp: определяет точку входа для приложения.
//
#include "ParserCombinators.h"



const ParserState updateParserState(const ParserState& state, std::size_t index, const ParseResult& result) {
	return ParserState{
		state.targetString,
		index,
		result
	};
}

const ParserState updateParserResults(const ParserState& state, const ParseResult& result) {
	return updateParserState(state, state.index, result);
}

const ParserState updateParserError(const ParserState& state, const std::string& errorMsg) {
	return ParserState{
		state.targetString,
		state.index,
		{},
		//state.result,
		true,
		errorMsg
	};
}

Parser make_str(const std::string_view& prefix) {
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
			return updateParserState(state, index + prefix.length(), { {std::string(prefix)} });
		}
		// error
		return updateParserError(state,
						std::format("str: Tried to match \"{}\", but got \"{}\"",
						prefix, slicedTarget.substr(0, 10)));
	};
	return Parser{ str };
}

Parser make_regexp(const std::regex& re, const std::string_view& name) {
	auto regexp = [re, name](const ParserState& state) {
		const auto [targetString, index, _, isError, __] = state;
		if (isError) {
			return state;
		}
		auto slicedTarget = targetString.substr(index);
		if (slicedTarget.length() == 0) {
			// error
			return updateParserError(state, std::format("{}: Got unexpected end of input.", name));
		}
		std::cmatch match;
		if (std::regex_search(slicedTarget.data(), match, re, std::regex_constants::match_continuous)) {
			// success
			return updateParserState(state, index + match[0].length(), {{match[0]}});
		}
		// error
		return updateParserError(state,
			std::format("{}: Couldn't match {} at index {}", name, name, index));
	};
	return Parser{ regexp };
}

Parser make_letters() {
	std::regex letterRegex("[^\\W\\d]+");
	return make_regexp(letterRegex, "letters");
}

Parser make_digits() {
	std::regex digitsRegex("\\d+");
	return make_regexp(digitsRegex, "digits");
}

// runtime sequence
Parser make_sequenceOf(const std::vector<Parser>& parsers) {
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

// runtime choice
Parser make_choice(const std::vector<Parser>& parsers) {
	auto choice = [parsers](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		for (auto& parser : parsers) {
			const auto nextState = parser.transformerFn(state);
			if (!nextState.isError) {
				return nextState;
			}
		}
		return updateParserError(state,
			std::format("choice: Unable to match with any parser at index {}", state.index));
	};
	return Parser{ choice };
}

Parser make_plus(const Parser& parser)
{
	auto plus = [parser](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		ParseResult result;
		auto nextState = state;
		bool done = false;
		while (!done) {
			const auto testState = parser.transformerFn(nextState);
			if (!testState.isError) {
				nextState = testState;
				result += testState.result;
				continue;
			}
			done = true;
		}
		if (result.values.empty()) {
			return updateParserError(state,
				std::format("plus: Unable to match any input using parser at index {}", state.index));
		}
		return updateParserResults(nextState, result);
	};
	return Parser{ plus };
}

Parser make_star(const Parser& parser)
{
	auto star = [parser](const ParserState& state) {
		if (state.isError) {
			return state;
		}
		ParseResult result;
		auto nextState = state;
		bool done = false;
		while (!done) {
			const auto testState = parser.transformerFn(nextState);
			if (!testState.isError) {
				nextState = testState;
				result += testState.result;
				continue;
			}
			done = true;
		}
		return updateParserResults(nextState, result);
	};
	return Parser{ star };
}
