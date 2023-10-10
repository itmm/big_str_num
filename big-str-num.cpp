#include <algorithm>

#include "big-str-num.h"

namespace Big_Str_Num {
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

    Result &add(Result &res, const Num &num) {
        if (num.empty()) { return res; }

        auto rc { res.begin_ };
        auto ru { res.used_ };
        auto re { res.end_ };
        auto nc { num.begin() };
        auto ne { num.end() };
        int overflow { 0 };

        while (nc < ne || overflow > 0) {
            int sum { overflow };
            overflow = 0;
            if (nc < ne ) { sum += *nc++; }
            if (rc >= re) { throw Error { }; }
            if (rc < ru) { sum += *rc; }
            if (sum >= base) { sum -= base; overflow = 1; }
            *rc++ = static_cast<unsigned short>(sum);
        }
        if (rc > ru) { res.used_ = rc; }

        return res;
    }

    void Result::trim() {
        while (used_ > begin_ && used_[-1] == 0) { --used_; }
    }

    Result &sub(Result &res, const Num &num) {
        if (num.empty()) { return res; }

        auto rc { res.begin_ };
        auto ru { res.used_ };
        auto re { res.end_ };
        auto nc { num.begin() };
        auto ne { num.end() };
        int borrow { 0 };

        while (nc < ne || borrow > 0) {
            int sum { -borrow };
            borrow = 0;
            if (nc < ne) { sum -= *nc++; }
            if (rc >= re) { throw Error { }; }
            if (rc < ru) { sum +=  *rc; }
            if (sum < 0) { sum += base; borrow = 1; }
            *rc++ = static_cast<unsigned short>(sum);
        }

        if (rc > ru) { res.used_ = rc; }

        res.trim();

        return res;
    }

    inline Result& multiply_and_add(Result& res, const Num& num, int factor, int shift) {
        if (num.empty() || factor == 0) { return res; }

        auto rc { res.begin_ };
        auto ru { res.used_ };
        auto re { res.end_ };

        for (; shift > 0; --shift, ++rc) {
            if (rc >= re) { throw Error { }; }
            if (rc >= ru) { *rc = 0; }
        }

        int overflow { 0 };
        auto nc { num.begin() };
        auto ne { num.end() };
        while (nc < ne || overflow) {
            int sum { overflow };
            if (nc < ne) { sum += *nc++ * factor; }
            if (rc >= re) { throw Error { }; }
            if (rc < ru) { sum += *rc; }
            overflow = sum / base;
            *rc++ = static_cast<unsigned short>(sum % base);
        }
        if (rc > ru) { res.used_ = rc; }
        return res;
    }

    Result &mult(Result& res, const Num& a, const Num& b) {
        res.clear();
        if (a.empty() || b.empty()) { return res; }

        auto ac { a.begin() };
        auto ae { a.end() };

        for (int shift { 0 }; ac < ae; ++shift, ++ac) {
            multiply_and_add(res, b, *ac, shift);
        }
        return res;
    }

    Result& div_by_2(Result &value) {
        int overflow = 0;

        auto rc { value.used_ - 1 };
        auto re { value.begin_ };
        for (; rc >= re; --rc) {
            int sum = overflow;
            int digit = *rc;
            sum += digit/2;
            overflow = (digit % 2) * (base/2);
            *rc = static_cast<unsigned short>(sum);
        }

        value.trim();

        return value;
    }

    inline Result& middle(Result& res, const Num& a, const Num& b) {
        res = a;
        add(res, b);
        return div_by_2(res);
    }

    constexpr unsigned short one[1] = { 1 };

    Div_Result &div(Div_Result& res, const Num& a, const Num& b) {
        if (b.empty()) { throw Error { }; }
        res.div.clear();
        res.rem = a; add(res.rem, Num { one, one + 1 });
        for (;;) {
            middle(res.scratch1, res.div, res.rem);
            mult(res.scratch2, res.scratch1, b);
            if (res.scratch1 == res.div) { break; }
            if (res.scratch2 == a) {
                res.div = res.scratch1;
                res.rem.clear();
                return res;
            }
            if (res.scratch2 < a) { res.div = res.scratch1; } else { res.rem = res.scratch1; }
        }
        res.rem = a;
        sub(res.rem, res.scratch2);
        return res;
    }

    Result& mod(Result& res, const Num& m, Div_Result& tmp) {
        div(tmp, res, m);
        res = tmp.rem;
        return res;
    }

    Result& mult_mod(Result& res, const Num& a, const Num& b, const Num& m, Div_Result& tmp) {
        mult(res, a, b);
        return mod(res, m, tmp);
    }

    Pow_Result& pow_mod(Pow_Result& res, const Num& a, const Num& b, const Num& m) {
        if (b.empty()) { res.result.clear(); return res; }
        res.result = Num { one, one + 1 };
        res.scratch2 = a;
        for (res.scratch1 = b;! res.scratch1.empty(); div_by_2(res.scratch1)) {
            if (res.scratch1.odd()) {
                mult_mod(res.scratch3, res.result, res.scratch2, m, res.div_result);
                res.result = res.scratch3;
            }
            mult_mod(res.scratch3, res.scratch2, res.scratch2, m, res.div_result);
            res.scratch2 = res.scratch3;
        }
        return res;
    }
}