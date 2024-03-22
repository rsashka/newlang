#include "pch.h"

#ifndef RATIONAL_H
#define RATIONAL_H

#include <openssl/bn.h>

namespace newlang {

    struct BigNum {
        BIGNUM *value;

        BigNum() : value(BN_new()) {
            ASSERT(value);
        }

        BigNum(const int64_t var) {
            set_(var);
        }

        BigNum(const std::string str) : BigNum() {
            SetFromString(str);
        }

        BigNum(const BigNum &copy) : BigNum() {
            VERIFY(BN_copy(value, copy.value));
        }

        inline BigNum& set_(const BigNum & copy) {
            VERIFY(BN_copy(value, copy.value));
            return *this;
        }

        inline BigNum& set_(const int64_t var) {
            if (var < 0) {
                BN_set_word(value, -var);
                BN_set_negative(value, -1);
            } else {
                BN_set_word(value, var);
            }
            return *this;
        }

        BigNum& operator=(const BigNum & var) {
            return set_(var);
        }

        BigNum& operator=(const int64_t var) {
            return set_(var);
        }

        virtual ~BigNum() {
            if (value) {
                BN_free(value);
                value = nullptr;
            }
        }

        struct CtxHelper {
            BN_CTX *ctx;

            CtxHelper() : ctx(BN_CTX_new()) {
                ASSERT(ctx);
            }

            ~CtxHelper() {
                if (ctx) {
                    BN_CTX_free(ctx);
                    ctx = nullptr;
                }
            }
        };

        int64_t GetAsInteger() const {
            if (isOverflow()) {
                LOG_RUNTIME("BigNum integer overflow!");
            }

            int64_t result = BN_get_word(value);
            if (BN_is_negative(value)) {
                result = -result;
            }
            return result;
        }

        /*
         * Конвертировать длинное целое в число с плавающей точкой (с потерей точности)
         */
        double GetAsNumber() const {
            int num_bits = BN_num_bits(value);

            if (num_bits <= 52) { // Размер мантиссы double
                ASSERT(!isOverflow());
                // Точности мантисы хватает для хранения всех значащих бит длинного числа
                return static_cast<double> (GetAsInteger());
            } else {
                BigNum temp(*this);

                BigNum one;
                one.SetOne();

                BigNum divider;
                BN_lshift(divider.value, one.value, num_bits - 52);

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

        inline bool SetFromString(const std::string str) {
            ASSERT(!str.empty());
            if (!BN_dec2bn(&value, str.c_str())) {
                LOG_RUNTIME("Fail create BinNum from string '%s'!", str.c_str());
            }
            return value;
        }

        std::string GetAsString() const {
            char * number_str = BN_bn2dec(value);
            std::string result(number_str);
            OPENSSL_free(number_str);
            return result;
        }

        BigNum &add(const BigNum &val) {
            const BigNum temp(*this);
            if (!BN_add(value, temp.value, val.value)) {
                LOG_RUNTIME("BN_add operation fail!");
            }
            return *this;
        }

        BigNum &sub(const BigNum &val) {
            const BigNum temp(*this);
            if (!BN_sub(value, temp.value, val.value)) {
                LOG_RUNTIME("BN_sub operation fail!");
            }
            return *this;
        }

        BigNum &mul(const BigNum &val) {
            CtxHelper ctx;
            const BigNum temp(*this);
            if (!BN_mul(value, temp.value, val.value, ctx.ctx)) {
                LOG_RUNTIME("BN_mul operation fail!");
            }
            return *this;
        }

        BigNum &div(const BigNum &val, BigNum &rem) {
            CtxHelper ctx;
            const BigNum temp(*this);
            if (!BN_div(value, rem.value, temp.value, val.value, ctx.ctx)) {
                LOG_RUNTIME("BN_div operation fail!");
            }
            return *this;
        }

        inline void SetOne() {
            BN_one(value);
        }

        inline void SetZero() {
            BN_zero(value);
        }

        inline bool isOverflow() const {
            return BN_get_word(value) == ~0;
        }

        inline bool isZero() const {
            return BN_is_zero(value);
        }

        inline bool isOne() const {
            return BN_is_one(value);
        }

        inline bool isNegative() const {
            return BN_is_negative(value);
        }

    };

    class Rational {
    public:

        BigNum m_numerator; // Числитель
        BigNum m_denominator; // Знаменатель

    public:
        // Конструктор принимает значения числителя и знаменателя

        Rational() : Rational("0", "1") {
        }

        Rational(const int64_t value) {
            set_(value);
        }

        Rational(const Rational &copy) {
            set_(copy);
        }

        Rational(const std::string numerator, const std::string denominator) {
            set_(numerator, denominator);
        }

        inline std::shared_ptr<Rational> clone() const {
            std::shared_ptr<Rational> result = std::make_shared<Rational>(*this);
            return result;
        }

        std::string GetAsString() const {
            std::string result = m_numerator.GetAsString();
            result += "\\";
            result += m_denominator.GetAsString();
            return result;
        }

        int64_t GetAsBoolean() const {
            return !m_numerator.isZero();
        }

        int64_t GetAsInteger() const {
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

        double GetAsNumber() const {
            if (m_denominator.isZero()) {
                LOG_RUNTIME("Denominator must be different from zero!");
            }
            if (m_denominator.isOne()) {
                return m_numerator.GetAsNumber();
            }
            return m_numerator.GetAsNumber() / m_denominator.GetAsNumber();
        }

        // Сокращения дроби

        void reduce() {
            BigNum::CtxHelper ctx;
            BigNum gcd;

            if (!BN_gcd(gcd.value, m_numerator.value, m_denominator.value, ctx.ctx)) {
                LOG_RUNTIME("Fail call BN_gcd!");
            }

            BigNum rem;
            m_numerator.div(gcd, rem);
            ASSERT(rem.isZero());
            m_denominator.div(gcd, rem);
            ASSERT(rem.isZero());
        }

        Rational &set_(const int64_t value) {
            if (value < 0) {
                BN_set_word(m_numerator.value, -value);
                BN_set_negative(m_numerator.value, -1);
            } else {
                BN_set_word(m_numerator.value, value);
            }
            BN_set_word(m_denominator.value, 1);
            return *this;
        }

        Rational &set_(const Rational &copy) {
            m_numerator.set_(copy.m_numerator);
            m_denominator.set_(copy.m_denominator);
            return *this;
        }

        Rational &set_(const std::string numerator, const std::string denominator) {
            m_numerator.SetFromString(numerator);
            m_denominator.SetFromString(denominator);
            return *this;
        }

        Rational& operator*=(const Rational &rational) {
            m_numerator.mul(rational.m_numerator);
            m_denominator.mul(rational.m_denominator);
            reduce();
            return *this;
        }

        Rational& operator/=(const Rational &rational) {
            m_numerator.mul(rational.m_denominator);
            m_denominator.mul(rational.m_numerator);
            reduce();

            return *this;
        }

        Rational& operator-=(const Rational &rational) {

            BigNum sub_num(rational.m_numerator);
            sub_num.mul(m_denominator);

            m_numerator.mul(rational.m_denominator);
            m_denominator.mul(rational.m_denominator);

            m_numerator.sub(sub_num);

            reduce();

            return *this;
        }

        Rational& operator+=(const Rational &rational) {
            BigNum add_num(rational.m_numerator);
            add_num.mul(m_denominator);

            m_numerator.mul(rational.m_denominator);
            m_denominator.mul(rational.m_denominator);

            m_numerator.add(add_num);

            reduce();

            return *this;
        }

        Rational& operator%=(const Rational &rational) {
            LOG_RUNTIME("Not implemented!");

            return *this;
        }

        Rational &operator^=(const Rational &) {
            LOG_RUNTIME("Operator '^=' not implementd!");

            return *this;
        }

        Rational & operator|=(const Rational &) {
            LOG_RUNTIME("Operator '|=' not implementd!");

            return *this;
        }

        Rational &op_lshift_set(const Rational &) {
            LOG_RUNTIME("Operator '<<=' not implementd!");

            return *this;
        }

        Rational & op_rshift_set(const Rational &) {
            LOG_RUNTIME("Operator '>>=' not implementd!");

            return *this;
        }

        const Rational & op_rrshift_set(const Rational &) {
            LOG_RUNTIME("Operator '>>>=' not implementd!");

            return *this;
        }

        Rational& op_pow_(const Rational &rational) {
            LOG_RUNTIME("Not implemented!");

            return *this;
        }

        bool op_equal(const Rational &rational) const {
            return BN_cmp(m_numerator.value, rational.m_numerator.value) == 0 &&
                    BN_cmp(m_denominator.value, rational.m_denominator.value) == 0;
        }

        int op_compare(const Rational &rational) const {
            if (BN_cmp(m_denominator.value, rational.m_denominator.value) == 0) {
                return BN_cmp(m_numerator.value, rational.m_numerator.value);
            }

            Rational first(*this);
            Rational second(rational);

            Rational mul;
            mul.m_numerator.set_(m_denominator);
            second *= mul;
            mul.m_numerator.set_(rational.m_denominator);
            first *= mul;

            ASSERT(BN_cmp(first.m_denominator.value, second.m_denominator.value) == 0);
            return BN_cmp(first.m_numerator.value, second.m_numerator.value);
        }

        Rational &op_div_ceil_(Rational &rational) {
            LOG_RUNTIME("Not implemented!");
            return *this;
        }

    };
};
#endif /* RATIONAL_H */

