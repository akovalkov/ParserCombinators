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

