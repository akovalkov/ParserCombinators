// ParserCombinations.cpp: определяет точку входа для приложения.
//
#include "ParserCombinators.h"

namespace Combinators {
	const ParserState updateParserState(const ParserState& state, std::size_t index, const ParseResult& result) {
		return ParserState{
			state.targetString,
			index,
			result
		};
	}

	const ParserState updateParserResult(const ParserState& state, const ParseResult& result) {
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

	Parser Parsers::str(const std::string& prefix) {
		auto str = [prefix](const ParserState& state) {
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
				return updateParserState(state, index + prefix.length(), { {prefix} });
			}
			// error
			return updateParserError(state,
				std::format("str: Tried to match \"{}\", but got \"{}\"",
					prefix, slicedTarget.substr(0, 10)));
		};
		return Parser{ str };
	}

	Parser Parsers::regexp(const std::regex& re, const std::string_view& name) {
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
				return updateParserState(state, index + match[0].length(), { {match[0]} });
			}
			// error
			return updateParserError(state,
				std::format("{}: Couldn't match {} at index {}", name, name, index));
		};
		return Parser{ regexp };
	}

	Parser Parsers::letters() {
		std::regex letterRegex("[^\\W\\d]+");
		return Parsers::regexp(letterRegex, "letters");
	}

	Parser Parsers::digits() {
		std::regex digitsRegex("\\d+");
		return Parsers::regexp(digitsRegex, "digits");
	}

	// runtime sequence
	Parser Parsers::sequenceOf(const std::vector<Parser>& parsers) {
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
			return updateParserResult(nextState, result);
		};
		return Parser{ sequenceOf };
	}

	// runtime choice
	Parser Parsers::choice(const std::vector<Parser>& parsers) {
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

	Parser Parsers::plus(const Parser& parser) {
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
			return updateParserResult(nextState, result);
		};
		return Parser{ plus };
	}

	Parser Parsers::star(const Parser& parser) {
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
			return updateParserResult(nextState, result);
		};
		return Parser{ star };
	}


	Parser Parsers::betweenBrackets(const Parser& contentParser) {
		auto betweenBrackets = Parsers::between(str("("), str(")"));
		return Parser{ betweenBrackets(contentParser) };
	}

	Parser Parsers::lazy(std::function<Parser()> fn) {
		auto lazy = [fn](const ParserState& state) {
			if (state.isError) {
				return state;
			}
			auto parser = fn();
			return parser.transformerFn(state);
		};
		return Parser{ lazy };
	}


	Parser Parsers::fail(const std::string& error) {
		auto err = [error](const ParserState& state) {
			// always return error
			return updateParserError(state, error);
		};
		return Parser{ err };
	}

	Parser Parsers::succeed(const ParseResult& result) {
		auto succeed = [result](const ParserState& state) {
			// always return error
			return updateParserResult(state, result);
		};
		return Parser{ succeed };
	}

}