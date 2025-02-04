#ifndef BIGNUM_HPP
#define BIGNUM_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <complex>
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <iomanip>
#include "../parser/errors.hpp"

using namespace std;

const double PI = acos(-1);
const int DECIMAL_LIMIT = 20;

class BigNum {
private:
    bool is_negative;
    vector<int> integer;
    vector<int> decimal;

    BigNum trunc() const {
        BigNum result = *this;
        result.decimal.clear();
        result.normalize();
        return result;
    }

    static void fft(vector<complex<double>>& a, bool invert) {
        int n = a.size();
        if (n == 0) return;

        for (int i = 1, j = 0; i < n; i++) {
            int bit = n >> 1;
            for (; j >= bit; bit >>= 1)
                j -= bit;
            j += bit;
            if (i < j)
                swap(a[i], a[j]);
        }

        for (int len = 2; len <= n; len <<= 1) {
            double ang = 2 * PI / len * (invert ? -1 : 1);
            complex<double> wlen(cos(ang), sin(ang));
            for (int i = 0; i < n; i += len) {
                complex<double> w(1);
                for (int j = 0; j < len/2; j++) {
                    complex<double> u = a[i+j];
                    complex<double> v = a[i+j+len/2] * w;
                    a[i+j] = u + v;
                    a[i+j+len/2] = u - v;
                    w *= wlen;
                }
            }
        }

        if (invert) {
            for (complex<double>& x : a)
                x /= n;
        }
    }

    bool is_zero() const {
        return integer.size() == 1 && integer[0] == 0 && decimal.empty();
    }


    void normalize() {

        while (integer.size() > 1 && integer.back() == 0)
            integer.pop_back();


        while (decimal.size() > DECIMAL_LIMIT)
            decimal.pop_back();
        while (decimal.size() > 0 && decimal.back() == 0)
            decimal.pop_back();


        if (integer.size() == 1 && integer[0] == 0 && decimal.empty())
            is_negative = false;
    }

public:

    BigNum() : is_negative(false), integer(1, 0) {}

    explicit BigNum(const string& s) {
        string num = s;
        is_negative = false;


        if (!num.empty() && num[0] == '-') {
            is_negative = true;
            num = num.substr(1);
        }


        size_t e_pos = num.find_first_of("eE");
        string exp_str;
        int exponent = 0;
        if (e_pos != string::npos) {
            exp_str = num.substr(e_pos + 1);
            num = num.substr(0, e_pos);
            try {
                exponent = stoi(exp_str);
            } catch (...) {
                throw invalid_argument("Invalid exponent: " + exp_str);
            }
        }


        size_t dot_pos = num.find('.');
        string int_part = (dot_pos != string::npos) ? num.substr(0, dot_pos) : num;
        string dec_part = (dot_pos != string::npos) ? num.substr(dot_pos + 1) : "";


        string digits = int_part + dec_part;
        int original_dot_pos = int_part.size();


        int new_dot_pos = original_dot_pos + exponent;


        string new_int_part, new_dec_part;
        if (new_dot_pos >= 0) {
            if (new_dot_pos <= digits.size()) {
                new_int_part = digits.substr(0, new_dot_pos);
                new_dec_part = digits.substr(new_dot_pos);
            } else {
                new_int_part = digits + string(new_dot_pos - digits.size(), '0');
            }
        } else {
            new_int_part = "0";
            new_dec_part = string(-new_dot_pos, '0') + digits;
        }


        if (new_int_part.empty()) new_int_part = "0";
        new_int_part.erase(0, min(new_int_part.find_first_not_of('0'), new_int_part.size()-1));


        new_dec_part.resize(DECIMAL_LIMIT, '0');


        integer.resize(new_int_part.size());
        transform(new_int_part.rbegin(), new_int_part.rend(), integer.begin(),
                  [](char c) { return c - '0'; });


        decimal.resize(DECIMAL_LIMIT);
        transform(new_dec_part.begin(),
                  new_dec_part.begin() + DECIMAL_LIMIT,
                  decimal.begin(),
                  [](char c) { return c - '0'; });

        normalize();
    }


    long long get_ll() const {
        constexpr long long LL_MAX = std::numeric_limits<long long>::max();
        constexpr long long LL_MIN = std::numeric_limits<long long>::min();


        const bool is_neg = is_negative && !is_zero();
        long long result = 0;


        for (auto it = integer.rbegin(); it != integer.rend(); ++it) {
            int digit = *it;


            if (!is_neg) {
                if (result > LL_MAX / 10) return LL_MAX;
                result *= 10;
                if (result > LL_MAX - digit) return LL_MAX;
                result += digit;
            }

            else {
                if (result < LL_MIN / 10) return LL_MIN;
                result *= 10;
                if (result < LL_MIN + digit) return LL_MIN;
                result -= digit;
            }
        }

        return result;
    }

    template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    BigNum(T value) {
        if constexpr (std::is_integral<T>::value) {

            *this = BigNum(std::to_string(value));
        } else {

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(340) << value;
            std::string s = oss.str();


            size_t dot_pos = s.find('.');
            if (dot_pos != std::string::npos) {
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') s.pop_back();
            }
            *this = BigNum(s);
        }
    }


    bool operator<(const BigNum& rhs) const {
        if (is_negative != rhs.is_negative)
            return is_negative;

        int cmp = compare_abs(rhs);
        return is_negative ? (cmp == 1) : (cmp == -1);
    }

    bool operator==(const BigNum& rhs) const {
        return is_negative == rhs.is_negative &&
               integer == rhs.integer &&
               decimal == rhs.decimal;
    }

    bool operator!=(const BigNum& rhs) const { return !(*this == rhs); }
    bool operator>(const BigNum& rhs) const { return rhs < *this; }
    bool operator<=(const BigNum& rhs) const { return !(rhs < *this); }
    bool operator>=(const BigNum& rhs) const { return !(*this < rhs); }

    BigNum operator+(const BigNum& rhs) const {
        if (is_negative != rhs.is_negative) {
            BigNum tmp = rhs;
            tmp.is_negative = !tmp.is_negative;
            return *this - tmp;
        }

        BigNum result;
        result.is_negative = is_negative;


        int max_int = max(integer.size(), rhs.integer.size());
        result.integer.resize(max_int);
        int carry = 0;
        for (int i = 0; i < max_int; ++i) {
            int a = (i < integer.size()) ? integer[i] : 0;
            int b = (i < rhs.integer.size()) ? rhs.integer[i] : 0;
            int sum = a + b + carry;
            result.integer[i] = sum % 10;
            carry = sum / 10;
        }
        if (carry) result.integer.push_back(carry);


        int max_dec = max(decimal.size(), rhs.decimal.size());
        result.decimal.resize(max_dec);
        carry = 0;
        for (int i = max_dec-1; i >= 0; --i) {
            int a = (i < decimal.size()) ? decimal[i] : 0;
            int b = (i < rhs.decimal.size()) ? rhs.decimal[i] : 0;
            int sum = a + b + carry;
            result.decimal[i] = sum % 10;
            carry = sum / 10;
        }
        if (carry) {
            result.integer[0] += carry;
            int i = 0;
            while (result.integer[i] >= 10) {
                if (i+1 >= result.integer.size()) result.integer.push_back(0);
                result.integer[i+1] += result.integer[i] / 10;
                result.integer[i] %= 10;
                ++i;
            }
        }

        result.normalize();
        return result;
    }

    BigNum operator-(const BigNum& rhs) const {
        if (is_negative != rhs.is_negative) {
            BigNum tmp = rhs;
            tmp.is_negative = !tmp.is_negative;
            return *this + tmp;
        }

        if (abs() < rhs.abs()) {
            BigNum result = rhs - *this;
            result.is_negative = !is_negative;
            return result;
        }

        BigNum result;
        result.is_negative = is_negative;


        result.integer = integer;
        int borrow = 0;
        for (size_t i = 0; i < result.integer.size(); ++i) {
            int sub = (i < rhs.integer.size()) ? rhs.integer[i] : 0;
            result.integer[i] -= sub + borrow;
            borrow = 0;
            if (result.integer[i] < 0) {
                result.integer[i] += 10;
                borrow = 1;
            }
        }


        int max_dec = max(decimal.size(), rhs.decimal.size());
        result.decimal.resize(max_dec);
        borrow = 0;
        for (int i = max_dec-1; i >= 0; --i) {
            int a = (i < decimal.size()) ? decimal[i] : 0;
            int b = (i < rhs.decimal.size()) ? rhs.decimal[i] : 0;
            int diff = a - b - borrow;
            borrow = 0;
            if (diff < 0) {
                diff += 10;
                borrow = 1;
            }
            result.decimal[i] = diff;
        }


        if (borrow) {
            for (size_t i = 0; i < result.integer.size(); ++i) {
                if (result.integer[i] > 0) {
                    result.integer[i]--;
                    break;
                } else {
                    result.integer[i] = 9;
                }
            }
        }

        result.normalize();
        return result;
    }

    BigNum operator*(const BigNum& rhs) const {
        if (*this == 0 || rhs == 0) return BigNum();


        auto extend_num = [](const BigNum& num) {
            vector<int> v = num.decimal;
            reverse(v.begin(), v.end());
            v.insert(v.end(), num.integer.begin(), num.integer.end());
            return v;
        };

        vector<int> a = extend_num(*this);
        vector<int> b = extend_num(rhs);


        size_t n = 1;
        while (n < a.size() + b.size()) n <<= 1;
        vector<complex<double>> fa(n), fb(n);

        copy(a.begin(), a.end(), fa.begin());
        copy(b.begin(), b.end(), fb.begin());

        fft(fa, false);
        fft(fb, false);
        for (size_t i = 0; i < n; i++)
            fa[i] *= fb[i];
        fft(fa, true);

        vector<int> res(n);
        for (size_t i = 0; i < n; i++)
            res[i] = round(fa[i].real());


        int carry = 0;
        for (size_t i = 0; i < n; i++) {
            res[i] += carry;
            carry = res[i] / 10;
            res[i] %= 10;
        }
        while (carry) {
            res.push_back(carry % 10);
            carry /= 10;
        }


        while (res.size() > 1 && res.back() == 0)
            res.pop_back();


        int decimal_digits = decimal.size() + rhs.decimal.size();
        BigNum result;
        result.is_negative = is_negative ^ rhs.is_negative;


        result.decimal.resize(min(decimal_digits, DECIMAL_LIMIT));
        for (int i = 0; i < result.decimal.size(); ++i) {
            if (i < res.size()) {
                result.decimal[i] = res[i];
            } else {
                result.decimal[i] = 0;
            }
        }


        result.integer.clear();
        for (size_t i = decimal_digits; i < res.size(); ++i) {
            result.integer.push_back(res[i]);
        }
        if (result.integer.empty()) {
            result.integer.push_back(0);
        }

        reverse(result.decimal.begin(), result.decimal.end());

        result.normalize();
        return result;
    }

    BigNum operator/(const BigNum& rhs) const {
        if (rhs == 0) throwZeroDivisionError("Division by zero");
        if (*this == 0) return BigNum();

        BigNum dividend = this->abs();
        BigNum divisor = rhs.abs();

        BigNum quotient;
        BigNum remainder;

        for (int i = dividend.integer.size() - 1; i >= 0; --i) {
            remainder = remainder * BigNum(10) + BigNum(dividend.integer[i]);

            int digit = 0;
            while (remainder >= divisor) {
                remainder = remainder - divisor;
                ++digit;
            }
            quotient.integer.insert(quotient.integer.begin(), digit);
        }

        if (!dividend.decimal.empty()) {
            for (int i = 0; i < dividend.decimal.size(); ++i) {
                remainder = remainder * BigNum(10) + BigNum(dividend.decimal[i]);
                int digit = 0;
                while (remainder >= divisor) {
                    remainder = remainder - divisor;
                    ++digit;
                }
                quotient.decimal.push_back(digit);
            }
        }

        for (int i = 0; i < DECIMAL_LIMIT; ++i) {
            remainder = remainder * BigNum(10);
            int digit = 0;
            while (remainder >= divisor) {
                remainder = remainder - divisor;
                ++digit;
            }
            quotient.decimal.push_back(digit);
        }

        quotient.is_negative = is_negative ^ rhs.is_negative;
        quotient.normalize();
        return quotient;
    }

    BigNum operator%(const BigNum& rhs) const {
        if (rhs == 0) {
            throwZeroDivisionError("Modulo by zero");
        }
        BigNum q = *this / rhs;
        BigNum q_trunc = q.trunc();
        BigNum remainder = *this - q_trunc * rhs;
        remainder.is_negative = this->is_negative;
        remainder.normalize();
        if (remainder.is_zero()) {
            remainder.is_negative = false;
        }
        return remainder;
    }

    BigNum abs() const {
        BigNum result = *this;
        result.is_negative = false;
        return result;
    }

    string to_string() const {
        string s;
        if (is_negative && !(integer.size() == 1 && integer[0] == 0 && decimal.empty()))
            s += '-';

        for (auto it = integer.rbegin(); it != integer.rend(); ++it)
            s += char('0' + *it);

        if (!decimal.empty()) {
            s += '.';
            for (int d : decimal)
                s += char('0' + d);
        }
        return s;
    }

private:
    int compare_abs(const BigNum& rhs) const {

        if (integer.size() != rhs.integer.size())
            return (integer.size() < rhs.integer.size()) ? -1 : 1;


        for (int i = integer.size()-1; i >= 0; --i) {
            if (integer[i] != rhs.integer[i])
                return (integer[i] < rhs.integer[i]) ? -1 : 1;
        }


        size_t max_dec = max(decimal.size(), rhs.decimal.size());
        for (size_t i = 0; i < max_dec; ++i) {
            int a = (i < decimal.size()) ? decimal[i] : 0;
            int b = (i < rhs.decimal.size()) ? rhs.decimal[i] : 0;
            if (a != b)
                return (a < b) ? -1 : 1;
        }

        return 0;
    }
};

#endif