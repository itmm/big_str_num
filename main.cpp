#include <iostream>
#include <cassert>

#include "big-str-num.h"

template<int SZ> class Buffered_Result: public Big_Str_Num::Result {
    char buffer_[SZ + 1];
public:
    Buffered_Result(): Result {buffer_, buffer_ + SZ } { buffer_[SZ] = '\0'; }
    using Result::operator=;
};

template<int SZ> class Buffered_Div_Result: public Big_Str_Num::Div_Result {
    char div_buffer_[SZ + 1];
    char rem_buffer_[SZ + 1];
    char scratch1_buffer_[SZ + 1];
    char scratch2_buffer_[SZ + 2];
public:
    Buffered_Div_Result(): Div_Result(
            div_buffer_, div_buffer_ + SZ,
            rem_buffer_, rem_buffer_ + SZ,
            scratch1_buffer_, scratch1_buffer_ + SZ,
            scratch2_buffer_, scratch2_buffer_ + SZ
    ) {
        div_buffer_[SZ] = rem_buffer_[SZ] = scratch1_buffer_[SZ] = scratch2_buffer_[SZ] = '\0';
    }
};

template<int SZ> class Buffered_Pow_Result: public Big_Str_Num::Pow_Result {
    Buffered_Div_Result<SZ> div_result_;
    char result_[SZ + 1];
    char scratch1_buffer_[SZ + 1];
    char scratch2_buffer_[SZ + 1];
    char scratch3_buffer_[SZ + 1];
public:
    Buffered_Pow_Result(): Pow_Result(
            result_, result_ + SZ,
            scratch1_buffer_, scratch1_buffer_ + SZ,
            scratch2_buffer_, scratch2_buffer_ + SZ,
            scratch3_buffer_, scratch3_buffer_ + SZ,
            div_result_
    ) {
        result_[SZ] = scratch1_buffer_[SZ] = scratch2_buffer_[SZ] = scratch3_buffer_[SZ] = '\0';
    }
};

void assert_num(const Big_Str_Num::Num &num, const char* cstr) {
    auto end { cstr ? cstr + strlen(cstr) : nullptr };
    if (cstr) {
        assert(std::equal(num.begin(), num.end(), cstr, end));
    } else {
        assert(num.begin() == nullptr && num.end() == nullptr);
    }
}

void assert_num(const char* initial, const char* expected) {
    Big_Str_Num::Num a { initial };
    assert_num(a, expected);
    Big_Str_Num::Num b { initial, initial + (initial ? strlen(initial) : 0) };
    assert_num(b, expected);
}

void assert_num(const char* initial) { assert_num(initial, initial); }

void assert_empty(const char* initial) { assert(Big_Str_Num::Num { initial }.empty()); }
void assert_not_empty(const char* initial) { assert(! Big_Str_Num::Num { initial }.empty()); }

void assert_add(const char* a, const char* b, const char* expected) {
    Buffered_Result<10> result;
    result = a;
    Big_Str_Num::add(result, b);
    assert(result == expected);
}

void assert_sub(const char* a, const char* b, const char* expected) {
    Buffered_Result<10> result;
    result = a;
    Big_Str_Num::sub(result, b);
    assert(result == expected);
}

void assert_mult(const char* a, const char* b, const char* expected) {
    Buffered_Result<10> result;
    Big_Str_Num::mult(result, a, b);
    assert(result == expected);
}

void assert_div(const char* a, const char* b, const char* expected_div, const char* expected_rem) {
    Buffered_Div_Result<10> res;
    Big_Str_Num::div(res, a, b);
    assert(res.div == expected_div);
    assert(res.rem == expected_rem);
}

void assert_mod(const char* a, const char* m, const char* expected) {
    Buffered_Div_Result<10> tmp;
    Buffered_Result<10> res;
    res = a;
    Big_Str_Num::mod(res, m, tmp);
    assert(res == expected);
}

void assert_mult_mod(const char* a, const char *b, const char* m, const char*expected) {
    Buffered_Div_Result<10> tmp;
    Buffered_Result<10> res;
    Big_Str_Num::mult_mod(res, a, b, m, tmp);
    assert(res == expected);
}

void assert_pow(const char* a, const char* b, const char* m, const char* expected) {
    Buffered_Pow_Result<10> res;
    Big_Str_Num::pow_mod(res, a, b, m);
    assert(res.result == expected);
}

int main() {
    assert_num(Big_Str_Num::Num { }, nullptr);
    assert_num(nullptr);
    assert_num("", nullptr);
    assert_num("0", nullptr);
    assert_num("12345678901234567890");
    assert_num("00012", "12");

    assert_empty(nullptr);
    assert_empty("");
    assert_empty("000");
    assert_not_empty("1");
    assert_not_empty("002");

    {
        Buffered_Result<10> result;
        assert(Big_Str_Num::Num { result }.empty());
        result = Big_Str_Num::Num { "0012" };
        assert_num(result, "12");
    }

    assert(Big_Str_Num::Num { "123" } == "0123");
    assert(Big_Str_Num::Num { "123 "} != "113");
    assert(Big_Str_Num::Num { "0023" } < "0123");
    assert(Big_Str_Num::Num { "0023" } <= "0123");
    assert(Big_Str_Num::Num { "123" } <= "0123");
    assert(Big_Str_Num::Num { "23" } > "22");
    assert(Big_Str_Num::Num { "23" } >= "22");
    assert(Big_Str_Num::Num { "23" } >= "023");

    assert_add("123", "45", "168");
    assert_add("1", "99", "100");
    assert_add("655", "456", "1111");
    assert_add("12", "", "12");
    assert_add("", "12", "12");

    assert_sub("123", "12", "111");
    assert_sub("100", "1", "99");
    assert_sub("100", "0", "100");
    assert_sub("0", "0", "0");

    {
        Buffered_Result<10> res;
        res = "567";
        add(res, res);
        assert("1134" == res);
    }

    {
        Buffered_Result<10> a;
        a = "123";
        assert(a == "0123");
        a = "4";
        assert(a == "04");
        a = Big_Str_Num::Num { };
        assert(Big_Str_Num::Num { a }.empty());
    }

    assert_mult("123", "0", "0");
    assert_mult("123", "1", "123");
    assert_mult("0", "2", "0");
    assert_mult("123", "1001", "123123");
    assert_mult("1001", "123", "123123");
    assert_mult("11", "11", "121");
    assert_mult("10", "13", "130");
    assert_mult("13", "10", "130");

    assert_div("0", "10", "0", "0");
    assert_div("10", "10", "1", "0");
    assert_div("100", "10", "10", "0");
    assert_div("102", "10", "10", "2");
    assert_div("123", "10", "12", "3");
    assert_div("130", "17", "7", "11");

    assert_mod("130", "17", "11");

    assert_mult_mod("13", "10", "17", "11");
    assert_mult_mod("10", "13", "17", "11");

    {
        Buffered_Pow_Result<5> result;
        result.result = "1";
        result.scratch1 = "2";
        result.scratch2 = "3";
        result.scratch3 = "4";
        result.div_result.div = "5";
        result.div_result.rem = "6";
        result.div_result.scratch1 = "7";
        result.div_result.scratch2 = "8";
        assert(result.result == "1");
        assert(result.scratch1 == "2");
        assert(result.scratch2 == "3");
        assert(result.scratch3 == "4");
        assert(result.div_result.div == "5");
        assert(result.div_result.rem == "6");
        assert(result.div_result.scratch1 == "7");
        assert(result.div_result.scratch2 == "8");
    }

    assert_pow("2", "10", "1025", "1024");
    assert_pow("2", "10", "10", "4");
    assert_pow("2", "10", "5", "4");
    assert_pow("3", "15", "17", "6");

    return 0;
}
