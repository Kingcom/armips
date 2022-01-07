#pragma once

#include "Util/FileSystem.h"

#include <cstring>
#include <ostream>
#include <string>

class Identifier
{
public:
	explicit Identifier() = default;
	explicit Identifier(std::string name);
	Identifier(const Identifier &) = default;
	Identifier(Identifier &&) = default;

	Identifier &operator=(const Identifier &) = default;
	Identifier &operator=(Identifier &&) = default;

	size_t size() const
	{
		return _name.size();
	}

	bool startsWith(char value) const
	{
		return _name.front() == value;
	}

	bool startsWith(std::string_view value) const
	{
		if (_name.size() < value.size())
			return false;

		return memcmp(_name.data(), value.data(), value.size()) == 0;
	}

	const std::string &string() const
	{
		return _name;
	}

	bool operator<(const Identifier& other) const
	{
		return _name < other._name;
	}

	bool operator==(const Identifier& other) const
	{
		return _name == other._name;
	}

	bool operator!=(const Identifier& other) const
	{
		return _name != other._name;
	}

	bool operator==(const std::string_view other) const
	{
		return _name == other;
	}

	bool operator!=(const std::string_view other) const
	{
		return _name != other;
	}
private:
	std::string _name;
};

inline bool operator==(const std::string_view first, const Identifier &second)
{
	return second == first;
}

inline bool operator!=(const std::string_view first, const Identifier &second)
{
	return second != first;
}

std::ostream& operator<<(std::ostream &output, const Identifier &identifier);


class StringLiteral
{
public:
	StringLiteral() = default;
	StringLiteral(std::string value);
	StringLiteral(const StringLiteral &) = default;
	StringLiteral(StringLiteral &&) = default;

	StringLiteral &operator=(const StringLiteral &) = default;
	StringLiteral &operator=(StringLiteral &&) = default;

	size_t size() const
	{
		return _value.size();
	}

	const std::string &string() const
	{
		return _value;
	}

	fs::path path() const
	{
		return fs::u8path(_value);
	}

	StringLiteral operator+(const StringLiteral &other) const;
	bool operator==(const StringLiteral &other) const;
	bool operator!=(const StringLiteral &other) const;
	bool operator<(const StringLiteral &other) const;
	bool operator<=(const StringLiteral &other) const;
	bool operator>(const StringLiteral &other) const;
	bool operator>=(const StringLiteral &other) const;

private:
	std::string _value;
};

std::ostream& operator<<(std::ostream &output, const StringLiteral &string);
