#include <algorithm>

#include "big-str-num.h"

namespace Big_Str_Num {
    void Result::copy(const Num &num, int shift) {
        used_ = end_;
        for (; shift > 0; --shift) {
            if (used_ <= begin_) { throw Error { }; }
            *--used_ = 0;
        }
        const unsigned short* begin { num.begin() };
        const unsigned short* cur { num.end() };
        while (cur > begin) {
            if (used_ <= begin_) { throw Error { }; }
            *--used_ = *--cur;
        }
    }

    void Num::trim() {
        while (begin_ < end_ && *begin_ == 0) { ++begin_; }
        if (begin_ == end_) { begin_ = end_ = nullptr; }
    }

    bool operator==(const Num &a, const Num &b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }

    bool operator<(const Num &a, const Num &b) {
        auto as { a.size() };
        auto bs { b.size() };
        if (as == bs) {
            return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
        } else { return as < bs; }
    }

    void Result::push_back(unsigned short num) {
        if (used_ == begin_) { throw Error { }; }
        --used_;
        *used_ = num;
    }

    Result &add(Result &res, const Num &num) {
        if (num.empty()) { return res; }

        auto rb { res.begin_ };
        auto rc { res.end_ - 1 };
        auto ru { res.used_ };
        auto nb { num.begin() };
        auto nc { num.end() - 1 };
        int overflow { 0 };

        while (nc >= nb || overflow > 0) {
            int sum { overflow };
            overflow = 0;
            if (nc >= nb ) { sum += *nc; --nc; }
            if (rc < rb) { throw Error { }; }
            if (rc >= ru) { sum += *rc; }
            if (sum >= base) { sum -= base; overflow = 1; }
            *rc = static_cast<unsigned short>(sum); --rc;
        }
        if (rc < ru) { res.used_ = rc + 1; }

        return res;
    }

    Result &sub(Result &res, const Num &num) {
        if (num.empty()) { return res; }

        auto rb { res.begin_ };
        auto rc { res.end_ - 1 };
        auto re { res.end_ };
        auto ru { res.used_ };
        auto nb { num.begin() };
        auto nc { num.end() - 1 };
        int borrow { 0 };

        while (nc >= nb || borrow > 0) {
            int sum { -borrow };
            borrow = 0;
            if (nc >= nb) { sum -= *nc; --nc; }
            if (rc < rb) { throw Error { }; }
            if (rc >= ru) { sum +=  *rc; }
            if (sum < 0) { sum += base; borrow = 1; }
            *rc = static_cast<unsigned short>(sum); --rc;
        }

        while (rc < re && *rc == 0) { ++rc; }

        if (rc < ru) { res.used_ = rc + 1; }

        return res;
    }

    inline Result& multiply_and_add(Result& res, const Num& num, int factor, int shift) {
        if (num.empty() || factor == 0) { return res; }

        auto rb { res.begin_ };
        auto ru { res.used_ };
        auto rc { res.end_ - 1 };

        for (; shift > 0; --shift) {
            if (rc <= rb) { throw Error { }; }
            if (rc < ru) { *rc = 0; }
            --rc;
        }

        int overflow { 0 };
        auto nb { num.begin() };
        auto nc { num.end() - 1 };
        while (nc >= nb || overflow) {
            int sum { overflow };
            if (nc >= nb) { sum += *nc * factor; }
            if (rc < rb) { throw Error { }; }
            if (rc >= ru) { sum += *rc; }
            overflow = sum / base;
            *rc = static_cast<unsigned short>(sum % base);
            --rc; --nc;
        }
        if (rc < ru) { res.used_ = rc + 1; }
        return res;
    }

    Result &mult(Result& res, const Num& a, const Num& b) {
        res.clear();
        if (a.empty() || b.empty()) { return res; }

        auto ab { a.begin() };
        auto ac { a.end() - 1 };

        for (int shift { 0 }; ab <= ac; ++shift, --ac) {
            multiply_and_add(res, b, *ac, shift);
        }
        return res;
    }

    inline Result& div_by_2(Result &value) {
        int overflow = 0;

        auto rc { value.used_ };
        auto re { value.end_ };
        for (; rc < re; ++rc) {
            int sum = overflow;
            int digit = *rc;
            sum += digit/2;
            overflow = (digit % 2) * (base/2);
            *rc = static_cast<unsigned short>(sum);
            if (sum == 0 && rc == value.used_) { ++value.used_; }
        }

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