// ParserCombinations.cpp: определяет точку входа для приложения.
//
#include <print>
#include <iostream>
#include <string>
#include "ParserCombinators.h"

template<typename T> concept ParserFunction = requires(T t, const std::string_view & targetString) {
	t(targetString);
};


auto make_str(const std::string_view& prefix) {
	auto str = [&prefix](const std::string_view& targetString) {
		if (targetString.starts_with(prefix)) {
			return prefix;
		}
		throw std::runtime_error(std::format("Tried to match \"{}\", but got \"{}\"", prefix, targetString.substr(0, 10)));
	};
	return str;
}

template<ParserFunction Parser>
auto run(Parser parser, const std::string_view& targetString) {
	return parser(targetString);
}


void func(const std::string_view& targetString) {}


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
