#include <memory>
#include <string>
#include <string_view>
#include <stdint.h>

#ifndef RATIONAL_H
#define RATIONAL_H

namespace newlang {

    struct BigNum {
        void *value;

        BigNum();
        BigNum(const int64_t var);
        BigNum(const std::string_view str);
        BigNum(const BigNum &copy);

        BigNum & set_(const BigNum & copy);
        BigNum & set_(const int64_t var);
        BigNum & operator=(const BigNum & var);
        BigNum & operator=(const int64_t var);
        virtual ~BigNum();

        struct CtxHelper {
            void *ctx;
            CtxHelper();
            ~CtxHelper();
        };

        int64_t GetAsInteger() const;

        /*
         * Конвертировать длинное целое в число с плавающей точкой (с потерей точности)
         */
        double GetAsNumber() const;

        bool SetFromString(const std::string_view str);
        std::string GetAsString() const;

        BigNum & add(const BigNum &val);
        BigNum & sub(const BigNum &val);
        BigNum & mul(const BigNum &val);
        BigNum & div(const BigNum &val, BigNum &rem);

        void SetOne();
        void SetZero();
        bool isOverflow() const;
        bool isZero() const;
        bool isOne() const;
        bool isNegative() const;
    };

    class Rational {
    public:

        BigNum m_numerator; // Числитель
        BigNum m_denominator; // Знаменатель

    public:
        // Конструктор принимает значения числителя и знаменателя

        Rational();
        Rational(const int64_t value);
//        Rational(const Rational &copy);
        Rational(const std::string_view numerator, const std::string_view denominator);

        std::shared_ptr<Rational> clone() const;

        std::string GetAsString() const;
        int64_t GetAsBoolean() const;
        int64_t GetAsInteger() const;
        double GetAsNumber() const;

        // Сокращения дроби

        void reduce();
        Rational & set_(const int64_t value);
        Rational & set_(const Rational &copy);
        Rational & set_(const std::string_view numerator, const std::string_view denominator);

        Rational & operator*=(const Rational &rational);
        Rational & operator/=(const Rational &rational);
        Rational & operator-=(const Rational &rational);
        Rational & operator+=(const Rational &rational);
        Rational & operator%=(const Rational &rational);
        Rational & operator^=(const Rational &);
        Rational & operator|=(const Rational &);

        Rational & op_lshift_set(const Rational &);
        Rational & op_rshift_set(const Rational &);
        Rational & op_pow_(const Rational &rational);
        Rational & op_div_ceil_(Rational &rational);

        const Rational & op_rrshift_set(const Rational &);
        bool op_equal(const Rational &rational) const;
        int op_compare(const Rational &rational) const;
    };
};
#endif /* RATIONAL_H */

