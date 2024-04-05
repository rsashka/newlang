//#include "pch.h"

#include <cmath>
#include <openssl/bn.h>

#include "rational.h"
#include "logger.h"
//#include "types.h"

using namespace newlang;

BigNum::CtxHelper::CtxHelper() : ctx(BN_CTX_new()) {
    ASSERT(ctx);
}

BigNum::CtxHelper::~CtxHelper() {
    if (ctx) {
        BN_CTX_free(static_cast<BN_CTX *> (ctx));
        ctx = nullptr;
    }
}

BigNum::BigNum() : value(BN_new()) {
    ASSERT(value);
}

BigNum::BigNum(const int64_t var) {
    set_(var);
}

BigNum::BigNum(const std::string_view str) : BigNum() {
    SetFromString(str);
}

BigNum::BigNum(const BigNum &copy) : BigNum() {
    VERIFY(BN_copy(static_cast<BIGNUM *> (value), static_cast<BIGNUM *> (copy.value)));
}

BigNum & BigNum::set_(const BigNum & copy) {
    VERIFY(BN_copy(static_cast<BIGNUM *> (value), static_cast<BIGNUM *> (copy.value)));
    return *this;
}

BigNum & BigNum::set_(const int64_t var) {
    if (var < 0) {
        BN_set_word(static_cast<BIGNUM *> (value), -var);
        BN_set_negative(static_cast<BIGNUM *> (value), -1);
    } else {

        BN_set_word(static_cast<BIGNUM *> (value), var);
    }
    return *this;
}

BigNum & BigNum::operator=(const BigNum & var) {

    return set_(var);
}

BigNum & BigNum::operator=(const int64_t var) {

    return set_(var);
}

BigNum::~BigNum() {
    if (value) {
        BN_free(static_cast<BIGNUM *> (value));
        value = nullptr;
    }
}

int64_t BigNum::GetAsInteger() const {
    if (isOverflow()) {
        LOG_RUNTIME("BigNum integer overflow!");
    }

    int64_t result = BN_get_word(static_cast<BIGNUM *> (value));
    if (BN_is_negative(static_cast<BIGNUM *> (value))) {

        result = -result;
    }
    return result;
}

/*
 * Конвертировать длинное целое в число с плавающей точкой (с потерей точности)
 */
double BigNum::GetAsNumber() const {
    int num_bits = BN_num_bits(static_cast<BIGNUM *> (value));

    if (num_bits <= 52) { // Размер мантиссы double
        ASSERT(!isOverflow());
        // Точности мантисы хватает для хранения всех значащих бит длинного числа
        return static_cast<double> (GetAsInteger());
    } else {
        BigNum temp(*this);

        BigNum one;
        one.SetOne();

        BigNum divider;
        BN_lshift(static_cast<BIGNUM *> (divider.value), static_cast<BIGNUM *> (one.value), num_bits - 52);

        BigNum remander;
        temp.div(divider, remander);

        ASSERT(!temp.isOverflow());
        double result = static_cast<double> (temp.GetAsInteger());

        result *= std::pow(2, num_bits - 52);

        return result;
    }
    /*
             1     \ / 2         3
    123456789123465789123456789123465789
    123456789123465786113767616146309120.000000
     */
}

bool BigNum::SetFromString(const std::string_view str) {
    ASSERT(!str.empty());
    BIGNUM * temp = static_cast<BIGNUM *> (value);
    if (!BN_dec2bn(&temp, str.begin())) {
        LOG_RUNTIME("Fail create BinNum from string '%s'!", str.begin());
    }
    value = (void *) temp;
    return value;
}

std::string BigNum::GetAsString() const {
    char * number_str = BN_bn2dec(static_cast<BIGNUM *> (value));
    std::string result(number_str);
    OPENSSL_free(number_str);

    return result;
}

BigNum & BigNum::add(const BigNum &val) {
    const BigNum temp(*this);
    if (!BN_add(static_cast<BIGNUM *> (value), static_cast<BIGNUM *> (temp.value), static_cast<BIGNUM *> (val.value))) {
        LOG_RUNTIME("BN_add operation fail!");
    }
    return *this;
}

BigNum & BigNum::sub(const BigNum &val) {
    const BigNum temp(*this);
    if (!BN_sub(static_cast<BIGNUM *> (value), static_cast<BIGNUM *> (temp.value), static_cast<BIGNUM *> (val.value))) {
        LOG_RUNTIME("BN_sub operation fail!");
    }
    return *this;
}

BigNum & BigNum::mul(const BigNum &val) {
    CtxHelper ctx;
    const BigNum temp(*this);
    if (!BN_mul(static_cast<BIGNUM *> (value), static_cast<BIGNUM *> (temp.value), static_cast<BIGNUM *> (val.value), static_cast<BN_CTX *> (ctx.ctx))) {
        LOG_RUNTIME("BN_mul operation fail!");
    }
    return *this;
}

BigNum & BigNum::div(const BigNum &val, BigNum &rem) {
    CtxHelper ctx;
    const BigNum temp(*this);
    if (!BN_div(static_cast<BIGNUM *> (value), static_cast<BIGNUM *> (rem.value), static_cast<BIGNUM *> (temp.value), static_cast<BIGNUM *> (val.value), static_cast<BN_CTX *> (ctx.ctx))) {
        LOG_RUNTIME("BN_div operation fail!");
    }
    return *this;
}

void BigNum::SetOne() {

    BN_one(static_cast<BIGNUM *> (value));
}

void BigNum::SetZero() {

    BN_zero(static_cast<BIGNUM *> (value));
}

bool BigNum::isOverflow() const {

    return BN_get_word(static_cast<BIGNUM *> (value)) == static_cast<uint64_t>(~0);
}

bool BigNum::isZero() const {

    return BN_is_zero(static_cast<BIGNUM *> (value));
}

bool BigNum::isOne() const {

    return BN_is_one(static_cast<BIGNUM *> (value));
}

bool BigNum::isNegative() const {

    return BN_is_negative(static_cast<BIGNUM *> (value));
}

/*
 * 
 * 
 */

Rational::Rational() : Rational("0", "1") {
}

Rational::Rational(const int64_t value) {
    set_(value);
}

Rational::Rational(const Rational &copy) {
    set_(copy);
}

Rational::Rational(const std::string_view numerator, const std::string_view denominator) {
    set_(numerator, denominator);
}

std::shared_ptr<Rational> Rational::clone() const {
    std::shared_ptr<Rational> result = std::make_shared<Rational>(*this);

    return result;
}

std::string Rational::GetAsString() const {
    std::string result = m_numerator.GetAsString();
    result += "\\";
    result += m_denominator.GetAsString();

    return result;
}

int64_t Rational::GetAsBoolean() const {

    return !m_numerator.isZero();
}

int64_t Rational::GetAsInteger() const {
    if (m_denominator.isZero()) {
        LOG_RUNTIME("Denominator must be different from zero!");
    }

    if (m_denominator.isOne()) {
        return m_numerator.GetAsInteger();
    }

    BigNum result = m_numerator;

    BigNum rem;
    result.div(m_denominator, rem);

    return result.GetAsInteger();
}

double Rational::GetAsNumber() const {
    if (m_denominator.isZero()) {
        LOG_RUNTIME("Denominator must be different from zero!");
    }
    if (m_denominator.isOne()) {

        return m_numerator.GetAsNumber();
    }
    return m_numerator.GetAsNumber() / m_denominator.GetAsNumber();
}

// Сокращения дроби

void Rational::reduce() {
    BigNum::CtxHelper ctx;
    BigNum gcd;

    if (!BN_gcd(static_cast<BIGNUM *> (gcd.value), static_cast<BIGNUM *> (m_numerator.value), static_cast<BIGNUM *> (m_denominator.value), static_cast<BN_CTX *> (ctx.ctx))) {

        LOG_RUNTIME("Fail call BN_gcd!");
    }

    BigNum rem;
    m_numerator.div(gcd, rem);
    ASSERT(rem.isZero());
    m_denominator.div(gcd, rem);
    ASSERT(rem.isZero());
}

Rational & Rational::set_(const int64_t value) {
    if (value < 0) {
        BN_set_word(static_cast<BIGNUM *> (m_numerator.value), -value);
        BN_set_negative(static_cast<BIGNUM *> (m_numerator.value), -1);
    } else {
        BN_set_word(static_cast<BIGNUM *> (m_numerator.value), value);
    }
    BN_set_word(static_cast<BIGNUM *> (m_denominator.value), 1);

    return *this;
}

Rational & Rational::set_(const Rational &copy) {
    m_numerator.set_(copy.m_numerator);
    m_denominator.set_(copy.m_denominator);

    return *this;
}

Rational & Rational::set_(const std::string_view numerator, const std::string_view denominator) {
    m_numerator.SetFromString(numerator);
    m_denominator.SetFromString(denominator);

    return *this;
}

Rational& Rational::operator*=(const Rational &rational) {
    m_numerator.mul(rational.m_numerator);
    m_denominator.mul(rational.m_denominator);
    reduce();

    return *this;
}

Rational& Rational::operator/=(const Rational &rational) {
    m_numerator.mul(rational.m_denominator);
    m_denominator.mul(rational.m_numerator);
    reduce();

    return *this;
}

Rational& Rational::operator-=(const Rational &rational) {

    BigNum sub_num(rational.m_numerator);
    sub_num.mul(m_denominator);

    m_numerator.mul(rational.m_denominator);
    m_denominator.mul(rational.m_denominator);

    m_numerator.sub(sub_num);

    reduce();

    return *this;
}

Rational & Rational::operator+=(const Rational &rational) {
    BigNum add_num(rational.m_numerator);
    add_num.mul(m_denominator);

    m_numerator.mul(rational.m_denominator);
    m_denominator.mul(rational.m_denominator);

    m_numerator.add(add_num);

    reduce();

    return *this;
}

Rational & Rational::operator%=(const Rational &) {
    LOG_RUNTIME("Not implemented!");

    return *this;
}

Rational & Rational::operator^=(const Rational &) {
    LOG_RUNTIME("Operator '^=' not implementd!");

    return *this;
}

Rational & Rational::operator|=(const Rational &) {
    LOG_RUNTIME("Operator '|=' not implementd!");
    return *this;
}

Rational & Rational::op_lshift_set(const Rational &) {
    LOG_RUNTIME("Operator '<<=' not implementd!");

    return *this;
}

Rational & Rational::op_rshift_set(const Rational &) {
    LOG_RUNTIME("Operator '>>=' not implementd!");

    return *this;
}

const Rational & Rational::op_rrshift_set(const Rational &) {
    LOG_RUNTIME("Operator '>>>=' not implementd!");

    return *this;
}

Rational & Rational::op_pow_(const Rational &) {
    LOG_RUNTIME("Not implemented!");

    return *this;
}

bool Rational::op_equal(const Rational &rational) const {

    return BN_cmp(static_cast<BIGNUM *> (m_numerator.value), static_cast<BIGNUM *> (rational.m_numerator.value)) == 0 &&
            BN_cmp(static_cast<BIGNUM *> (m_denominator.value), static_cast<BIGNUM *> (rational.m_denominator.value)) == 0;
}

int Rational::op_compare(const Rational &rational) const {
    if (BN_cmp(static_cast<BIGNUM *> (m_denominator.value), static_cast<BIGNUM *> (rational.m_denominator.value)) == 0) {
        return BN_cmp(static_cast<BIGNUM *> (m_numerator.value), static_cast<BIGNUM *> (rational.m_numerator.value));
    }

    Rational first(*this);
    Rational second(rational);

    Rational mul;
    mul.m_numerator.set_(m_denominator);
    second *= mul;
    mul.m_numerator.set_(rational.m_denominator);
    first *= mul;

    ASSERT(BN_cmp(static_cast<BIGNUM *> (first.m_denominator.value), static_cast<BIGNUM *> (second.m_denominator.value)) == 0);

    return BN_cmp(static_cast<BIGNUM *> (first.m_numerator.value), static_cast<BIGNUM *> (second.m_numerator.value));
}

Rational & Rational::op_div_ceil_(Rational &) {
    LOG_RUNTIME("Not implemented!");
    return *this;
}

