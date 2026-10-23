#include "bigmath.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <limits>
#include <cctype>

namespace
{
	using Words = std::vector<uint32_t>;

	void normalize(Words &a)
	{
		while (!a.empty() && a.back() == 0)
			a.pop_back();
	}

	Words fromHex(const std::string &input)
	{
		std::string s = input;

		if (s.size() >= 2 && s[0] == '0' &&
			(s[1] == 'x' || s[1] == 'X'))
			s.erase(0, 2);

		Words result;
		uint32_t word = 0;
		unsigned shift = 0;

		for (auto it = s.rbegin(); it != s.rend(); ++it)
		{
			unsigned char c = static_cast<unsigned char>(*it);

			if (!std::isxdigit(c))
				throw std::invalid_argument("Invalid hexadecimal number");

			uint32_t digit;

			if (c >= '0' && c <= '9')
				digit = c - '0';
			else if (c >= 'a' && c <= 'f')
				digit = c - 'a' + 10;
			else
				digit = c - 'A' + 10;

			word |= digit << shift;
			shift += 4;

			if (shift == 32)
			{
				result.push_back(word);
				word = 0;
				shift = 0;
			}
		}

		if (shift != 0)
			result.push_back(word);

		normalize(result);
		return result;
	}

	std::string toHexWords(const Words &input)
	{
		Words a = input;
		normalize(a);

		if (a.empty())
			return "0";

		static const char hex[] = "0123456789ABCDEF";
		std::string result;

		for (size_t i = a.size(); i-- > 0;)
		{
			for (int shift = 28; shift >= 0; shift -= 4)
				result += hex[(a[i] >> shift) & 0xF];
		}

		size_t first = result.find_first_not_of('0');

		if (first == std::string::npos)
			return "0";

		return result.substr(first);
	}

	/* Words addWords(const Words &a, const Words &b)
	{
		Words result(std::max(a.size(), b.size()), 0);
		uint64_t carry = 0;

		for (size_t i = 0; i < result.size(); ++i)
		{
			uint64_t x = i < a.size() ? a[i] : 0;
			uint64_t y = i < b.size() ? b[i] : 0;
			uint64_t sum = x + y + carry;

			result[i] = static_cast<uint32_t>(sum);
			carry = sum >> 32;
		}

		if (carry)
			result.push_back(static_cast<uint32_t>(carry));

		normalize(result);
		return result;
	} */

	Words subtractWords(const Words &a, const Words &b)
	{
		if (a.size() < b.size())
			throw std::invalid_argument("Negative unsigned subtraction");

		Words result = a;
		uint64_t borrow = 0;

		for (size_t i = 0; i < result.size(); ++i)
		{
			uint64_t x = result[i];
			uint64_t y = (i < b.size() ? b[i] : 0) + borrow;

			result[i] = static_cast<uint32_t>(x - y);
			borrow = x < y;
		}

		if (borrow)
			throw std::logic_error("Subtraction underflow");

		normalize(result);
		return result;
	}

	int compareWords(const Words &a, const Words &b)
	{
		if (a.size() != b.size())
			return a.size() < b.size() ? -1 : 1;

		for (size_t i = a.size(); i-- > 0;)
		{
			if (a[i] != b[i])
				return a[i] < b[i] ? -1 : 1;
		}

		return 0;
	}

	Words multiplyWords(const Words &a, const Words &b)
	{
		if (a.empty() || b.empty())
			return {};

		Words result(a.size() + b.size() + 1, 0);

		for (size_t i = 0; i < a.size(); ++i)
		{
			uint64_t carry = 0;

			for (size_t j = 0; j < b.size(); ++j)
			{
				uint64_t cur =
					static_cast<uint64_t>(a[i]) * b[j] + result[i + j] + carry;

				result[i + j] = static_cast<uint32_t>(cur);
				carry = cur >> 32;
			}

			size_t k = i + b.size();

			while (carry != 0)
			{
				uint64_t cur =
					static_cast<uint64_t>(result[k]) + carry;

				result[k] = static_cast<uint32_t>(cur);
				carry = cur >> 32;
				++k;
			}
		}

		normalize(result);
		return result;
	}

	Words shiftLeftOne(const Words &a)
	{
		Words result = a;
		uint64_t carry = 0;

		for (size_t i = 0; i < result.size(); ++i)
		{
			uint64_t value =
				(static_cast<uint64_t>(result[i]) << 1) | carry;

			result[i] = static_cast<uint32_t>(value);
			carry = value >> 32;
		}

		if (carry)
			result.push_back(static_cast<uint32_t>(carry));

		return result;
	}

	bool getBit(const Words &a, size_t bit)
	{
		size_t index = bit / 32;

		if (index >= a.size())
			return false;

		return ((a[index] >> (bit % 32)) & 1U) != 0;
	}

	size_t bitLength(const Words &a)
	{
		if (a.empty())
			return 0;

		uint32_t top = a.back();
		size_t bits = 0;

		while (top != 0)
		{
			++bits;
			top >>= 1;
		}

		return (a.size() - 1) * 32 + bits;
	}

	Words divideRemainder(
		const Words &dividend,
		const Words &divisor,
		Words *quotient = nullptr)
	{
		if (divisor.empty())
			throw std::invalid_argument("Division by zero");

		Words q;
		Words r;

		size_t bits = bitLength(dividend);

		for (size_t i = bits; i-- > 0;)
		{
			r = shiftLeftOne(r);

			if (getBit(dividend, i))
			{
				if (r.empty())
					r.push_back(1);
				else
					r[0] |= 1U;
			}

			if (compareWords(r, divisor) >= 0)
			{
				r = subtractWords(r, divisor);

				size_t index = i / 32;

				if (q.size() <= index)
					q.resize(index + 1, 0);

				q[index] |= uint32_t(1) << (i % 32);
			}
		}

		normalize(q);
		normalize(r);

		if (quotient)
			*quotient = q;

		return r;
	}

	Words addSmall(Words a, uint32_t value)
	{
		uint64_t carry = value;
		size_t i = 0;

		while (carry != 0)
		{
			if (i == a.size())
				a.push_back(0);

			uint64_t sum = static_cast<uint64_t>(a[i]) + carry;
			a[i] = static_cast<uint32_t>(sum);
			carry = sum >> 32;
			++i;
		}

		return a;
	}

	Words multiplySmall(const Words &a, uint32_t value)
	{
		if (value == 0 || a.empty())
			return {};

		Words result(a.size(), 0);
		uint64_t carry = 0;

		for (size_t i = 0; i < a.size(); ++i)
		{
			uint64_t product =
				static_cast<uint64_t>(a[i]) * value + carry;

			result[i] = static_cast<uint32_t>(product);
			carry = product >> 32;
		}

		if (carry)
			result.push_back(static_cast<uint32_t>(carry));

		return result;
	}

	Words divideSmall(Words a, uint32_t divisor, uint32_t *remainder)
	{
		if (divisor == 0)
			throw std::invalid_argument("Division by zero");

		uint64_t rem = 0;

		for (size_t i = a.size(); i-- > 0;)
		{
			uint64_t current = (rem << 32) | a[i];

			a[i] = static_cast<uint32_t>(current / divisor);
			rem = current % divisor;
		}

		normalize(a);

		if (remainder)
			*remainder = static_cast<uint32_t>(rem);

		return a;
	}

	BigInt makeBigInt(const Words &a)
	{
		return BigInt(toHexWords(a));
	}

	Words getWords(const BigInt &a)
	{
		return fromHex(a.toHex());
	}

	bool isOdd(const Words &a)
	{
		return !a.empty() && (a[0] & 1U);
	}

	uint32_t montgomeryNPrime(uint32_t n0)
	{
		uint32_t inverse = 1;

		for (int i = 0; i < 5; ++i)
			inverse *= 2U - n0 * inverse;

		return 0U - inverse;
	}

	Words montgomeryReduce(
		const Words &a,
		const Words &b,
		const Words &modulus,
		uint32_t nprime,
		size_t n)
	{
		Words t = multiplyWords(a, b);
		t.resize(std::max(t.size(), 2 * n + 2), 0);

		for (size_t i = 0; i < n; ++i)
		{
			uint32_t m = t[i] * nprime;
			uint64_t carry = 0;

			for (size_t j = 0; j < n; ++j)
			{
				uint64_t cur =
					static_cast<uint64_t>(m) * modulus[j] + t[i + j] + carry;

				t[i + j] = static_cast<uint32_t>(cur);
				carry = cur >> 32;
			}

			size_t k = i + n;

			while (carry != 0)
			{
				uint64_t cur =
					static_cast<uint64_t>(t[k]) + carry;

				t[k] = static_cast<uint32_t>(cur);
				carry = cur >> 32;
				++k;
			}
		}

		Words result(t.begin() + n, t.end());
		normalize(result);

		while (compareWords(result, modulus) >= 0)
			result = subtractWords(result, modulus);

		return result;
	}
}

// BigInt

BigInt::BigInt(size_t words)
	: digits_(words ? new uint32_t[words]() : nullptr),
	  length_(words),
	  sign_(0)
{
}

BigInt::BigInt(const std::string &hexstr)
	: digits_(nullptr), length_(0), sign_(0)
{
	Words words = fromHex(hexstr);
	length_ = words.size();

	if (length_ != 0)
	{
		digits_ = new uint32_t[length_];

		std::copy(words.begin(), words.end(), digits_);
	}
}

BigInt BigInt::fromDec(const std::string &decstr)
{
	if (decstr.empty())
		throw std::invalid_argument("Empty decimal number");

	Words value;

	for (char c : decstr)
	{
		if (c < '0' || c > '9')
			throw std::invalid_argument("Invalid decimal number");

		value = multiplySmall(value, 10);
		value = addSmall(value, static_cast<uint32_t>(c - '0'));
	}

	return makeBigInt(value);
}

BigInt BigInt::fromU64(uint64_t value)
{
	Words words;

	if (value != 0)
	{
		words.push_back(static_cast<uint32_t>(value));

		uint32_t high = static_cast<uint32_t>(value >> 32);

		if (high)
			words.push_back(high);
	}

	return makeBigInt(words);
}

BigInt::BigInt(const BigInt &other)
	: digits_(other.length_ ? new uint32_t[other.length_] : nullptr),
	  length_(other.length_),
	  sign_(other.sign_)
{
	if (length_)
		std::copy(other.digits_, other.digits_ + length_, digits_);
}

BigInt &BigInt::operator=(const BigInt &other)
{
	if (this == &other)
		return *this;

	uint32_t *newDigits =
		other.length_ ? new uint32_t[other.length_] : nullptr;

	if (other.length_)
		std::copy(other.digits_, other.digits_ + other.length_, newDigits);

	delete[] digits_;

	digits_ = newDigits;
	length_ = other.length_;
	sign_ = other.sign_;

	return *this;
}

BigInt::~BigInt()
{
	delete[] digits_;
}

std::string BigInt::toHex() const
{
	Words words;

	for (size_t i = 0; i < length_; ++i)
		words.push_back(digits_[i]);

	normalize(words);

	return toHexWords(words);
}

uint64_t BigInt::toU64OrDie() const
{
	Words a = getWords(*this);

	if (a.size() > 2)
		throw std::overflow_error("Number does not fit in uint64_t");

	uint64_t value = a.empty() ? 0 : a[0];

	if (a.size() == 2)
		value |= static_cast<uint64_t>(a[1]) << 32;

	return value;
}

int BigInt::cmp(const BigInt &other) const
{
	return compareWords(getWords(*this), getWords(other));
}

bool BigInt::operator==(const BigInt &other) const
{
	return cmp(other) == 0;
}

bool BigInt::operator!=(const BigInt &other) const
{
	return cmp(other) != 0;
}

BigInt BigInt::mulNaive(const BigInt &a, const BigInt &b)
{
	return makeBigInt(multiplyWords(getWords(a), getWords(b)));
}

BigInt BigInt::modNaive(const BigInt &a, const BigInt &m)
{
	Words modulus = getWords(m);

	if (modulus.empty())
		throw std::invalid_argument("Modulo by zero");

	return makeBigInt(divideRemainder(getWords(a), modulus));
}

BigInt BigInt::modExp(
	const BigInt &base,
	const BigInt &exp,
	const BigInt &mod)
{
	Words modulus = getWords(mod);
	Words exponent = getWords(exp);

	if (modulus.empty())
		throw std::invalid_argument("Modulo by zero");

	BigInt result = modNaive(fromU64(1), mod);
	BigInt current = modNaive(base, mod);

	size_t bits = bitLength(exponent);

	for (size_t i = 0; i < bits; ++i)
	{
		if (getBit(exponent, i))
		{
			result = modNaive(mulNaive(result, current), mod);
		}

		if (i + 1 < bits)
			current = modNaive(mulNaive(current, current), mod);
	}

	return result;
}

// MontgomeryCtx

MontgomeryCtx::MontgomeryCtx(const BigInt &mod)
	: mod_(new BigInt(mod)),
	  R_(nullptr),
	  R_inv_(nullptr),
	  nprime_(0),
	  n_words_(0)
{
	Words modulus = getWords(mod);

	if (modulus.empty() || !isOdd(modulus) ||
		compareWords(modulus, Words{1}) <= 0)
	{
		throw std::invalid_argument(
			"Montgomery modulus must be odd and greater than one");
	}

	n_words_ = modulus.size();

	nprime_ = montgomeryNPrime(modulus[0]);

	std::string rHex = "1";
	rHex.append(n_words_ * 8, '0');

	R_.reset(new BigInt(rHex));

	BigInt inv2 = makeBigInt(
		divideSmall(addSmall(modulus, 1), 2, nullptr));

	BigInt exponent = BigInt::fromU64(
		static_cast<uint64_t>(32 * n_words_));

	R_inv_.reset(new BigInt(
		BigInt::modExp(inv2, exponent, *mod_)));
}

MontgomeryCtx::~MontgomeryCtx() = default;

BigInt MontgomeryCtx::toMont(const BigInt &a) const
{
	BigInt reduced = BigInt::modNaive(a, *mod_);

	return BigInt::modNaive(
		BigInt::mulNaive(reduced, *R_), *mod_);
}

BigInt MontgomeryCtx::fromMont(const BigInt &a_bar) const
{
	Words value = getWords(a_bar);
	Words modulus = getWords(*mod_);

	value = divideRemainder(value, modulus);

	Words one{1};

	return makeBigInt(
		montgomeryReduce(value, one, modulus, nprime_, n_words_));
}

BigInt MontgomeryCtx::mul(
	const BigInt &a,
	const BigInt &b) const
{
	Words modulus = getWords(*mod_);

	Words x = divideRemainder(getWords(a), modulus);
	Words y = divideRemainder(getWords(b), modulus);

	return makeBigInt(
		montgomeryReduce(x, y, modulus, nprime_, n_words_));
}

BigInt MontgomeryCtx::exp(
	const BigInt &base,
	const BigInt &expValue) const
{
	Words exponent = getWords(expValue);

	BigInt result = toMont(BigInt::fromU64(1));
	BigInt current = toMont(BigInt::modNaive(base, *mod_));

	size_t bits = bitLength(exponent);

	for (size_t i = 0; i < bits; ++i)
	{
		if (getBit(exponent, i))
			result = mul(result, current);

		if (i + 1 < bits)
			current = mul(current, current);
	}

	return fromMont(result);
}
