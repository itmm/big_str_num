#include <algorithm>

#include "embedded-rsa.h"

namespace Embedded_RSA {
    void Result::copy(const Num &num, int shift) {
        used_ = begin_;
        for (; shift > 0; --shift) {
            if (used_ >= end_) { throw Error { }; }
            *used_++ = 0;
        }
        auto nc {num.begin() };
        auto ne {num.end() };
        while (nc < ne) {
            if (used_ >= end_) { throw Error { }; }
            *used_++ = *nc++;
        }
    }

    void Num::trim() {
        while (begin_ < end_ && end_[-1] == 0) { --end_; }
        if (begin_ == end_) { begin_ = end_ = nullptr; }
    }

    bool operator==(const Num &a, const Num &b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }

    bool operator<(const Num &a, const Num &b) {
        auto as { a.size() };
        auto bs { b.size() };
        if (as == bs) {
            auto ab { a.begin() };
            auto ac { a.end() - 1 } ;
            auto bc { b.end() - 1 };
            for (; ac >= ab; --ac, --bc) {
                if (*ac < *bc) { return true; }
                else if (*ac > *bc) { return false; }
            }
        }
        return as < bs;
    }

    void Result::push_back(unsigned short num) {
        if (used_ >= end_) { throw Error { }; }
        *used_++ = num;
    }

    Result& simple_mod(Result& res, const Num& m) {
        if (m) {
            while (res >= m) { res -= m; }
        }
        return res;
    }

    template<typename OP> Result& Result::perform_add_op(OP op, const Num& other) {
        auto cur {begin_ };
        auto oc { other.begin() };
        auto oe { other.end() };
        int overflow { 0 };

        while (oc < oe || overflow != 0) {
            int sum { overflow };
            int digit { 0 };
            overflow = 0;
            if (oc < oe ) { digit = *oc++; }
            if (cur >= end_) { throw Error { }; }
            if (cur < used_) { sum += *cur; }
            overflow = op(digit, sum, overflow);
            *cur++ = static_cast<unsigned short>(sum);
        }
        if (cur > used_) { used_ = cur; }

        return *this;
    }

    inline int single_add(int digit, int& sum, int& overflow) {
        sum += digit;
        if (sum >= base) {
            sum -= base; overflow = 1;
        }
        return overflow;
    }

    Add_State& Add_State::operator+=(const Num& other) {
        value.perform_add_op<>(single_add, other);
        simple_mod(value, modulus);
        return *this;
    }

    void Result::trim() {
        while (used_ > begin_ && used_[-1] == 0) { --used_; }
    }

    inline int single_sub(int digit, int& sum, int& overflow) {
        sum -= digit;
        if (sum < 0) {
            sum += base; overflow = -1;
        }
        return overflow;
    }

    Result& Result::operator-=(const Num& other) {
        this->perform_add_op<>(single_sub, other);
        trim();
        return *this;
    }

    Result& Result::div_by_2() {
        int overflow = 0;

        auto rc { used_ - 1 };
        for (; rc >= begin_; --rc) {
            int sum = overflow;
            int digit = *rc;
            sum += digit/2;
            overflow = (digit % 2) * (base/2);
            *rc = static_cast<unsigned short>(sum);
        }

        trim();

        return *this;
    }

    constexpr unsigned short one[1] = { 1 };

    template<typename OP, typename RES>
    void bit_process(OP op, RES& res, const Num& num, RES& scratch1, Result& scratch2) {
        for (scratch2 = num; ! scratch2.empty(); scratch2.div_by_2()) {
            if (scratch2.odd()) {
                op(res, scratch1);
            }
            op(scratch1, scratch1);
        }
    }

    inline void do_add(Add_State& a, const Num& b) { a += b; }

    Mul_State& Mul_State::operator*=(const Num& other) {
        scratch1_ = Num { value };
        value = Num { };
        bit_process<>(do_add, value, other, scratch1_, scratch2_);
        return *this;
    }

    inline void do_mult(Mul_State& a, const Num& b) { a *= b; }

    Pow_State& Pow_State::pow(const Num& b) {
        scratch1 = Num { result };
        result = Num { one, one + 1 };
        bit_process<>(do_mult, result, b, scratch1, scratch2);
        return *this;
    }
}