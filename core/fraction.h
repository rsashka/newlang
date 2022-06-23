#ifndef FRACTION_H
#define FRACTION_H

#include <openssl/bn.h>
#include <string>

#include <contrib/logger/logger.h>
#include <bits/stdint-intn.h>

class BigNum {
    SCOPE(private) :
    BIGNUM *m_value;
    BN_CTX *m_ctx;
    bool m_is_init;
public:

#define CHECK_INIT(value)  if (!(value)->m_is_init) { LOG_RUNTIME("Fail init!!!"); }

    BigNum() : m_value(nullptr), m_ctx(nullptr) {
        m_is_init = false;
        m_value = BN_new();
        m_ctx = BN_CTX_new();
    }

    BigNum(std::string str) {
        SetFromString(str);
    }

    virtual ~BigNum() {
        if (m_value) {
            BN_free(m_value);
            m_value = nullptr;
        }
        if (m_ctx) {
            BN_CTX_free(m_ctx);
            m_ctx = nullptr;
        }
    }

    int64_t GetAsInteger() {
        CHECK_INIT(this);

        int64_t result = BN_get_word(m_value);
        if (result == ~0) {
            LOG_RUNTIME("Bignum integer overflow!");
        }

        if (BN_is_negative(m_value)) {
            result = -result;
        }
        return result;
    }

    double GetAsNumber() {
        CHECK_INIT(this);
        //@todo Refactor convert big integer to double!
        return GetAsInteger();
    }

    inline bool SetFromString(const std::string str) {
        m_is_init = true;
        return BN_dec2bn(&m_value, str.c_str());
    }

    std::string GetAsString() {
        CHECK_INIT(this);
        char * number_str = BN_bn2dec(m_value);
        std::string result(number_str);
        OPENSSL_free(number_str);
        return result;
    }

    BigNum &add(BigNum &val) {
        CHECK_INIT(this);
        CHECK_INIT(&val);

        BigNum temp;
        if (!BN_add(temp.m_value, m_value, val.m_value)) {
            LOG_RUNTIME("Bignum operation fail!");
        }
        std::swap(temp.m_value, m_value);
        return *this;
    }

    BigNum &sub(BigNum &val) {
        CHECK_INIT(this);
        CHECK_INIT(&val);

        BigNum temp;
        if (!BN_sub(temp.m_value, m_value, val.m_value)) {
            LOG_RUNTIME("Bignum operation fail!");
        }
        std::swap(temp.m_value, m_value);
        return *this;
    }

    BigNum &mul(BigNum &val) {
        CHECK_INIT(this);
        CHECK_INIT(&val);

        BigNum temp;
        if (!BN_mul(temp.m_value, m_value, val.m_value, m_ctx)) {
            LOG_RUNTIME("Bignum operation fail!");
        }
        std::swap(temp.m_value, m_value);
        return *this;
    }

    BigNum &div(BigNum &val, BigNum &rem) {
        CHECK_INIT(this);
        CHECK_INIT(&val);

        BigNum dv;
        BigNum rm;
        if (!BN_div(dv.m_value, rm.m_value, m_value, val.m_value, m_ctx)) {
            LOG_RUNTIME("Bignum operation fail!");
        }
        std::swap(dv.m_value, m_value);
        std::swap(rm.m_value, rem.m_value);
        rem.m_is_init = true;
        return *this;
    }

};

class Fraction {
private:
    // Числитель
    BigNum m_numerator;
    // Знаменатель
    BigNum m_denominator;

    // Функция нужна для сокращения дроби
    void reduce();
public:
    // Конструктор принимает значения числителя и знаменателя

    Fraction(BigNum & numerator, BigNum &denominator) {
        m_numerator = numerator;
        m_denominator = denominator;
    }

    Fraction(const std::string &numerator, const std::string &denominator) {
        //        m_numerator = numerator;
        //        m_denominator = denominator;
    }

    //    Fraction(const std::string &string) {
    //        // Выбрасываем исключение, если не задана строка
    //        assert(string == "");
    //        // Ищем знак '/'
    //        int pos = string.find("/");
    //
    //        // Если символ не найден - то вся строка является числом 
    //        if (pos == std::string::npos) {
    //            m_numerator = stoi(string);
    //            m_denominator = 1;
    //        } else {
    //            // Числитель - левая часть
    //            m_numerator = std::stoi(string.substr(0, pos));
    //            // Знаменатель - правая часть
    //            m_denominator = std::stoi(string.substr(pos, string.length()));
    //
    //            // Знаменатель не должен быть равен нулю
    //            assert(m_denominator == 0);
    //        }
    //    }
    //    // Возвращаем дробь в виде строки
    //
    //    std::string toString() {
    //        std::string fraction = "";
    //        if (m_numerator == 0) {
    //            fraction.append("0");
    //            return fraction;
    //        }
    //
    //        fraction.append(std::to_string(m_numerator));
    //        if (m_denominator != 1) {
    //            fraction.append("/");
    //            fraction.append(std::to_string(m_denominator));
    //        }
    //        return fraction;
    //    }
    //    // Геттеры
    //
    //    int getNumerator() const {
    //        return m_numerator;
    //    }
    //
    //    int getDenominator() const {
    //        return m_denominator;
    //    }


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

#endif /* FRACTION_H */

