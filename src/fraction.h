#ifndef FRACTION_H
#define FRACTION_H

#include <openssl/bn.h>
#include <string>

#include <contrib/logger/logger.h>

#include "pch.h"

namespace newlang {

    struct BigNum {
        BIGNUM *value;

        BigNum() : value(BN_new()) {
            ASSERT(value);
        }

        BigNum(const unsigned long var) : BigNum() {
            BN_set_word(value, var);
        }

        BigNum(const std::string str) : BigNum() {
            SetFromString(str);
        }

        BigNum(const BigNum &copy) : BigNum() {
            VERIFY(BN_copy(value, copy.value));
        }
        BigNum& operator=(const BigNum&) = delete; // Disallow copying

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

    class Fraction {
    public:

        BigNum m_numerator; // Числитель
        BigNum m_denominator; // Знаменатель

        // Функция нужна для сокращения дроби
        void reduce();
    public:
        // Конструктор принимает значения числителя и знаменателя

        Fraction() : Fraction("0", "1") {
        }

        Fraction(const unsigned long value) : Fraction() {
            BN_set_word(m_numerator.value, value);
        }

        Fraction(const Fraction &copy) : m_numerator(copy.m_numerator), m_denominator(copy.m_denominator) {
        }

        Fraction(const std::string numerator, const std::string denominator) {
            m_numerator.SetFromString(numerator);
            m_denominator.SetFromString(denominator);
        }

        std::string GetAsString() const {
            std::string result = m_numerator.GetAsString();
            result += "\\";
            result += m_denominator.GetAsString();
            return result;
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

        //    // Наибольший общий делитель
        //    // (англ.) greatest common divisor
        //
        //    static int gcd(int a, int b) {
        //        while (b > 0) {
        //            int c = a % b;
        //            a = b;
        //            b = c;
        //        }
        //        return a;
        //    }
        //
        //    // Наименьшее общее кратное
        //    // (англ.) least common multiple
        //
        //    static int lcm(int a, int b) {
        //        return gcd(a, b) * a * b;
        //    }
        //public void reduce ()
        //{
        //    BigInteger num = BigInteger.valueOf(numerator);
        //    int gcd = num.gcd(BigInteger.valueOf(denominator)).intValue();
        //
        //    this.denominator /= gcd;
        //    this.numerator /= gcd;
        //
        //}

        Fraction& operator*(const Fraction &fraction) {
            LOG_RUNTIME("Not implemented!");
            //        m_numerator = m_numerator * fraction.getNumerator();
            //        m_denominator = m_denominator * fraction.getDenominator();
            //        reduce();
            return *this;
        }

        Fraction& operator/(const Fraction &fraction) {
            LOG_RUNTIME("Not implemented!");
            //        m_numerator = m_numerator * fraction.getDenominator();
            //        m_denominator = m_denominator * fraction.getNumerator();
            //        reduce();
            return *this;
        }

        Fraction& operator-(const Fraction &fraction) {
            LOG_RUNTIME("Not implemented!");
            //        int relNumerator = m_numerator * fraction.getDenominator();
            //        m_numerator = m_numerator * fraction.getDenominator() - m_denominator * fraction.getNumerator();
            //        m_denominator = gcd(m_denominator, fraction.getDenominator());
            //        reduce();
            return *this;
        }

        Fraction& operator+(const Fraction &fraction) {
            LOG_RUNTIME("Not implemented!");
            //        int unionDenominator = lcm(m_denominator, fraction.getDenominator());
            //        int relNumerator = m_numerator * unionDenominator;
            //        int mulNumerator = fraction.m_numerator * unionDenominator;
            //        m_numerator = relNumerator * mulNumerator;
            //        m_denominator = unionDenominator;
            //        reduce();
            return *this;
        }
    };
};
#endif /* FRACTION_H */

