#include <algorithm>

#include "embedded-rsa.h"

namespace Embedded_RSA {
    void Num::trim() {
        while (begin_ < end_ && end_[-1] == 0) { --end_; }
        if (begin_ == end_) { begin_ = end_ = nullptr; }
    }

    bool operator==(const Num& a, const Num& b) {
        return std::equal(a.begin(), a.end(), b.begin(), b.end());
    }

    bool operator<(const Num& a, const Num& b) {
        auto as { a.size() };
        auto bs { b.size() };
        if (as == bs) {
            auto ab { a.begin() };
            auto ac { a.end() - 1 };
            auto bc { b.end() - 1 };
            for (
                ; ac >= ab; --ac, --bc
                ) {
                if (*ac < *bc) { return true; }
                else if (*ac > *bc) { return false; }
            }
        }
        return as < bs;
    }

    void Result::copy(const Num& num) {
        auto nc { num.begin() };
        auto ne { num.end() };
        used_ = begin_;
        while (nc < ne) {
            if (used_ >= end_) { throw Error(); }
            *used_++ = *nc++;
        }
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
        auto cur { begin_ };
        auto oc { other.begin() };
        auto oe { other.end() };
        int overflow { 0 };

        while (oc < oe || overflow != 0) {
            int sum { overflow };
            int digit { 0 };
            overflow = 0;
            if (oc < oe) { digit = *oc++; }
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
            sum -= base;
            overflow = 1;
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
            sum += base;
            overflow = -1;
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
        for (
            ; rc >= begin_; --rc
            ) {
            int sum = overflow;
            int digit = *rc;
            sum += digit / 2;
            overflow = (digit % 2) * (base / 2);
            *rc = static_cast<unsigned short>(sum);
        }

        trim();

        return *this;
    }

    constexpr unsigned short one[1] = { 1 };

    template<typename OP, typename RES>
    void bit_process(OP op, RES& res, const Num& num, RES& scratch1, Result& scratch2) {
        for (
            scratch2 = num; !scratch2.empty(); scratch2.div_by_2()
            ) {
            if (scratch2.odd()) {
                op(res, scratch1);
            }
            op(scratch1, scratch1);
        }
    }

    inline void do_add(Add_State& a, const Num& b) { a += b; }

    Mul_State& Mul_State::operator*=(const Num& other) {
        scratch1 = Num { value };
        value = Num { };
        bit_process<>(do_add, value, other, scratch1, scratch2);
        return *this;
    }

    inline void do_multiply(Mul_State& a, const Num& b) { a *= b; }

    Pow_State& Pow_State::pow(const Num& b) {
        scratch1 = Num { value };
        value = Num { one, one + 1 };
        bit_process<>(do_multiply, value, b, scratch1, scratch2);
        return *this;
    }

    int Rsa_State::byte_size(const Num& key) {
        if (! key) { return 0; }
        int result = static_cast<int>(key.end() - key.begin() - 1) * 2;
        if (key.end()[-1] > 255) { ++result; }
        return result;
    }

    unsigned char random_char() { return 42; } // TODO

    char* Rsa_State::encrypt(const char* plain_begin, const char* plain_end, char* crypt_begin, const char* crypt_end) {
        auto len = plain_end - plain_begin;
        if (len < 0 || len + 11 > bytes_per_num) { throw Error { }; }
        if (crypt_end - crypt_begin < bytes_per_num) { throw Error { }; }
        crypt_end = crypt_begin + bytes_per_num;

        state.value = Num { };
        auto& res { state.value.value.value };
        res.push_back(0x0200);
        for (auto i { bytes_per_num - len - 3 }; i >= 1; i -= 2) {
             res.push_back((random_char() << 8) + random_char());
        }

        if ((bytes_per_num - len - 3) % 2) {
            res.push_back(random_char() << 8);
        } else if (len > 0) {
            res.push_back(*plain_begin++);
        }

        for (; plain_begin + 1 < plain_end; plain_begin += 2) {
            res.push_back(static_cast<unsigned char>(plain_begin[0]) + (static_cast<unsigned char>(plain_begin[1]) << 8));
        }
        if (plain_begin < plain_end) { res.push_back(static_cast<unsigned char>(*plain_begin)); }

        state.pow(exponent);

        auto cur { Num { res }.begin() };
        auto end { Num { res }.end() };
        for (; crypt_begin + 1 < crypt_end && cur < end; ++cur) {
            *crypt_begin++ = static_cast<char>(*cur % 0x100);
            *crypt_begin++ = static_cast<char>(*cur >> 8);
        }
        if (crypt_begin < crypt_end && cur < end) { *crypt_begin++ = static_cast<char>(*cur % 0x100); }
        while (crypt_begin < crypt_end) { *crypt_begin++ = '\0'; }
        return crypt_begin;
    }

    char *Rsa_State::decrypt(const char* crypt_begin, const char* crypt_end, char* plain_begin, const char* plain_end) {
        if (crypt_end - crypt_begin < bytes_per_num) { throw Error { }; }
        crypt_end = crypt_begin + bytes_per_num;

        state.value = Num { };
        auto& res { state.value.value.value };

        for (; crypt_begin + 1 < crypt_end; crypt_begin += 2) {
            res.push_back(static_cast<unsigned char>(crypt_begin[0]) + (static_cast<unsigned char>(crypt_begin[1]) << 8));
        }
        if (crypt_begin < crypt_end) { res.push_back(static_cast<unsigned char>(*crypt_begin)); }

        state.pow(exponent);

        const char* out_end { plain_begin + bytes_per_num };

        auto cur { Num { res }.begin() };
        auto end { Num { res }.end() };
        if (cur >= end || *cur != 0x0200) { throw Error { }; }
        out_end -= 2;
        ++cur;

        for (; cur < end; ++cur, out_end -= 2) {
            if ((*cur & 0xff) == 0) {
                if (plain_begin >= out_end || plain_begin >= plain_end) { throw Error { }; }
                *plain_begin++ = static_cast<char>(*cur >> 8);
                out_end -= 2;
                break;
            }
            if ((*cur & 0xff00) == 0) { ++cur; out_end -= 2; break; }
        }
        for (; cur < end; ++cur) {
            if (plain_begin >= out_end) {
                if (plain_begin >= plain_end) { throw Error { }; }
                *plain_begin++ = static_cast<char>(*cur % 0x100);
            }
            if (plain_begin >= out_end) {
                if (plain_begin >= plain_end) { throw Error { }; }
                *plain_begin++ = static_cast<char>(*cur >> 8);
            }
        }
        while (plain_begin < out_end) {
            if (plain_begin >= plain_end) { throw Error { }; }
            *plain_begin++ = static_cast<char>(*cur % 0x100);
        }

        return plain_begin;
    }

}