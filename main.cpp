#include <iostream>
#include <cassert>

#include "big-str-num.h"

template<int SZ> class Buffered_Result: public Big_Str_Num::Result {
        unsigned short buffer_[SZ] { };
    public:
        Buffered_Result(): Result {buffer_, buffer_ + SZ } { }
};

void fill(Big_Str_Num::Result& result, unsigned long long value) {
    result.clear();
    while (value != 0) {
        result.push_back(static_cast<unsigned short>(value % Big_Str_Num::base));
        value = value / Big_Str_Num::base;
    }
}

class Long_Result: public Buffered_Result<4> {
    public:
        explicit Long_Result(unsigned long long value = 0) { fill(*this, value); }
};

template<int SZ> class Buffered_Div_Result: public Big_Str_Num::Div_Result {
        unsigned short div_buffer_[SZ] { };
        unsigned short rem_buffer_[SZ] { };
        unsigned short scratch1_buffer_[SZ] { };
        unsigned short scratch2_buffer_[SZ] { };
    public:
        Buffered_Div_Result(): Div_Result(
                div_buffer_, div_buffer_ + SZ,
                rem_buffer_, rem_buffer_ + SZ,
                scratch1_buffer_, scratch1_buffer_ + SZ,
                scratch2_buffer_, scratch2_buffer_ + SZ
        ) { }
};

class Long_Div_Result: public Buffered_Div_Result<4> { };

template<int SZ> class Buffered_Pow_Result: public Big_Str_Num::Pow_Result {
    Buffered_Div_Result<SZ> div_result_;
    unsigned short result_[SZ] { };
    unsigned short scratch1_buffer_[SZ] { };
    unsigned short scratch2_buffer_[SZ] { };
    unsigned short scratch3_buffer_[SZ] { };
public:
    Buffered_Pow_Result(): Pow_Result(
            result_, result_ + SZ,
            scratch1_buffer_, scratch1_buffer_ + SZ,
            scratch2_buffer_, scratch2_buffer_ + SZ,
            scratch3_buffer_, scratch3_buffer_ + SZ,
            div_result_
    ) { }
};

class Long_Pow_Result: public Buffered_Pow_Result<4> { };

void assert_num(const Big_Str_Num::Num &num, unsigned long long expected) {
    if (expected > 0) {
        assert(num == Big_Str_Num::Num { Long_Result(expected) });
    } else {
        assert(num.begin() == nullptr && num.end() == nullptr);
    }
}

void assert_num(unsigned long long initial, unsigned long long expected) {
    assert_num(Long_Result(initial), expected);
}

void assert_num(unsigned long long initial) { assert_num(initial, initial); }

void assert_empty(unsigned long long initial) {
    assert(Long_Result(initial).empty());
}

void assert_not_empty(unsigned long long initial) {
    assert(! Long_Result(initial).empty());
}

void assert_add(unsigned long long a, unsigned long long b, unsigned long long expected) {
    Long_Result result { a };
    Big_Str_Num::add(result, Long_Result { b });
    assert(result == Long_Result(expected));
}

void assert_sub(unsigned long long a, unsigned long long b, unsigned long long expected) {
    Long_Result result { a };
    Big_Str_Num::sub(result, Long_Result { b });
    assert(result == Long_Result(expected));
}

void assert_mult(unsigned long long a, unsigned long long b, unsigned long long expected) {
    Long_Result result;
    Big_Str_Num::mult(result, Long_Result { a }, Long_Result { b });
    assert(result == Long_Result { expected });
}

void assert_div_by_2(unsigned long long a, unsigned long long expected) {
    Long_Result res { a };
    Big_Str_Num::div_by_2(res);
    assert(res == Long_Result { expected });
}

void assert_div(unsigned long long a, unsigned long long b, unsigned long long expected_div, unsigned long long expected_rem) {
    Long_Div_Result res;
    Big_Str_Num::div(res, Long_Result { a }, Long_Result { b });
    assert(res.div == Long_Result { expected_div });
    assert(res.rem == Long_Result { expected_rem });
}

void assert_mod(unsigned long long a, unsigned long long m, unsigned long long expected) {
    Long_Div_Result tmp;
    Long_Result res { a };
    Big_Str_Num::mod(res, Long_Result { m }, tmp);
    assert(res == Long_Result { expected });
}

void assert_mult_mod(unsigned long long a, unsigned long long b, unsigned long long m, unsigned long long expected) {
    Long_Div_Result tmp;
    Long_Result res;
    Big_Str_Num::mult_mod(res, Long_Result { a }, Long_Result { b }, Long_Result { m }, tmp);
    assert(res == Long_Result { expected });
}

void assert_pow(unsigned long long a, unsigned long long b, unsigned long long m, unsigned long long expected) {
    Long_Pow_Result res;
    Big_Str_Num::pow_mod(res, Long_Result { a }, Long_Result { b }, Long_Result { m });
    assert(res.result == Long_Result { expected });
}

int main() {
    assert_num(Big_Str_Num::Num { }, 0);
    assert_num(0);
    assert_num(123457890);
    assert_num(12);

    assert_empty(0);
    assert_not_empty(1);

    {
        Long_Result result;
        assert(result.empty());
        result = Long_Result { 12 };
        assert_num(result, 12);
        result.copy(Big_Str_Num::Num { });
        assert(result.empty());
    }

    assert(Long_Result { 12 } == Long_Result { 12 });
    assert(Long_Result { 12 } != Long_Result { 13 });
    assert(Long_Result { 12 } < Long_Result { 13 });
    assert(Long_Result { 12 } <= Long_Result { 13 });
    assert(Long_Result { 12 } <= Long_Result { 12 });
    assert(Long_Result { 13 } > Long_Result { 12 });
    assert(Long_Result { 13 } >= Long_Result { 12 });
    assert(Long_Result { 13 } >= Long_Result { 13 });

    assert(Long_Result { 12 + Big_Str_Num::base } == Long_Result { 12 + Big_Str_Num::base });
    assert(Long_Result { 12 + Big_Str_Num::base } != Long_Result { 13 + Big_Str_Num::base });
    assert(Long_Result { 12 + Big_Str_Num::base } < Long_Result { 13 + Big_Str_Num::base });
    assert(Long_Result { 12 + Big_Str_Num::base } <= Long_Result { 13 + Big_Str_Num::base });
    assert(Long_Result { 12 + Big_Str_Num::base } <= Long_Result { 12 + Big_Str_Num::base });
    assert(Long_Result { 13 + Big_Str_Num::base } > Long_Result { 12 + Big_Str_Num::base });
    assert(Long_Result { 13 + Big_Str_Num::base } >= Long_Result { 12 + Big_Str_Num::base });
    assert(Long_Result { 13 + Big_Str_Num::base } >= Long_Result { 13 + Big_Str_Num::base });

    assert(Long_Result { 12 * Big_Str_Num::base } == Long_Result { 12 * Big_Str_Num::base });
    assert(Long_Result { 12 * Big_Str_Num::base } != Long_Result { 13 * Big_Str_Num::base });
    assert(Long_Result { 12 * Big_Str_Num::base } < Long_Result { 13 * Big_Str_Num::base });
    assert(Long_Result { 12 * Big_Str_Num::base } <= Long_Result { 13 * Big_Str_Num::base });
    assert(Long_Result { 12 * Big_Str_Num::base } <= Long_Result { 12 * Big_Str_Num::base });
    assert(Long_Result { 13 * Big_Str_Num::base } > Long_Result { 12 * Big_Str_Num::base });
    assert(Long_Result { 13 * Big_Str_Num::base } >= Long_Result { 12 * Big_Str_Num::base });
    assert(Long_Result { 13 * Big_Str_Num::base } >= Long_Result { 13 * Big_Str_Num::base });

    assert(Long_Result { 100 } < Long_Result { 13 * Big_Str_Num::base });
    assert(Long_Result { 13 * Big_Str_Num::base } > Long_Result { 100 });

    assert_add(123, 45, 168);
    assert_add(1, 99, 100);
    assert_add(655, 456, 1111);
    assert_add(12, 0, 12);
    assert_add(0, 12, 12);
    assert_add(99999, 10, 100009);
    assert_add(100, 99999, 100099);

    assert_sub(123, 12, 111);
    assert_sub(100, 1, 99);
    assert_sub(100, 0, 100);
    assert_sub(0, 0, 0);
    assert_sub(1000000, 2, 999998);

    {
        Long_Result res { 12 };
        Big_Str_Num::add(res, res);
        assert(Long_Result { 24 } == res);
    }

    assert_mult(123, 0, 0);
    assert_mult(123, 1, 123);
    assert_mult(0, 2, 0);
    assert_mult(1234, 10001, 12341234);
    assert_mult(10001, 1234, 12341234);
    assert_mult(11, 11, 121);
    assert_mult(10, 13, 130);
    assert_mult(13, 10, 130);
    assert_mult(1234, 1000000, 1234000000);
    assert_mult(Big_Str_Num::base, Big_Str_Num::base, static_cast<unsigned long long>(Big_Str_Num::base) * Big_Str_Num::base);
    assert_mult(1000, 100, 100000);

    assert_div_by_2(10, 5);
    assert_div_by_2(11, 5);
    assert_div_by_2(0, 0);
    assert_div_by_2(10000000, 5000000);
    assert_div_by_2(Big_Str_Num::base, Big_Str_Num::base / 2);

    assert_div(0, 10, 0, 0);
    assert_div(10, 10, 1, 0);
    assert_div(Big_Str_Num::base, 2, Big_Str_Num::base / 2, 0);
    assert_div(100000, 100, 1000, 0);
    assert_div(102, 10, 10, 2);
    assert_div(123, 10, 12, 3);
    assert_div(130, 17, 7, 11);

    assert_mod(130, 17, 11);

    assert_mult_mod(13, 10, 17, 11);
    assert_mult_mod(10, 13, 17, 11);

    {
        Long_Result one { 1 }, two { 2 }, three { 3 }, four { 4 };
        Long_Result five { 5 }, six { 6 }, seven { 7 }, eight { 8 };

        Buffered_Pow_Result<5> result;
        result.result.copy(one);
        result.scratch1.copy(two);
        result.scratch2.copy(three);
        result.scratch3.copy(four);
        result.div_result.div.copy(five);
        result.div_result.rem.copy(six);
        result.div_result.scratch1.copy(seven);
        result.div_result.scratch2.copy(eight);
        assert(result.result == one);
        assert(result.scratch1 == two);
        assert(result.scratch2 == three);
        assert(result.scratch3 == four);
        assert(result.div_result.div == five);
        assert(result.div_result.rem == six);
        assert(result.div_result.scratch1 == seven);
        assert(result.div_result.scratch2 == eight);
    }

    assert_pow(2, 10, 1025, 1024);
    assert_pow(2, 10, 10, 4);
    assert_pow(2, 10, 5, 4);
    assert_pow(3, 15, 17, 6);

    return 0;
}
