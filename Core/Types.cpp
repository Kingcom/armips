#include "Core/Types.h"

Identifier::Identifier(std::string name) :
	_name(std::move(name))
{
}

std::ostream &operator<<(std::ostream &output, const Identifier &identifier)
{
	output << identifier.string();
	return output;
}

StringLiteral::StringLiteral(std::string value) :
	_value(std::move(value))
{
}

StringLiteral StringLiteral::operator+(const StringLiteral &other) const
{
	return StringLiteral(_value + other._value);
}

bool StringLiteral::operator==(const StringLiteral &other) const
{
	return _value == other._value;
}

bool StringLiteral::operator!=(const StringLiteral &other) const
{
	return _value != other._value;
}

bool StringLiteral::operator<(const StringLiteral &other) const
{
	return _value < other._value;
}

bool StringLiteral::operator<=(const StringLiteral &other) const
{
	return _value <= other._value;
}

bool StringLiteral::operator>(const StringLiteral &other) const
{
	return _value > other._value;
}

bool StringLiteral::operator>=(const StringLiteral &other) const
{
	return _value >= other._value;
}

std::ostream &operator<<(std::ostream &output, const StringLiteral &value)
{
	output << value.string();
	return output;
}
