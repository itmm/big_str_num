#pragma once

#include <cstring>
#include <cstdio>
#include <exception>
#include <limits>


namespace Big_Str_Num {
    constexpr int base { 0x10000 };

    static_assert(std::numeric_limits<unsigned short>::max() >= (base - 1));

    class Result;

    class Num {
        const unsigned short* begin_ { nullptr };
        const unsigned short* end_ { nullptr };

        void trim();

    public:
        Num() = default;
        Num(const unsigned short* begin, const unsigned short* end): begin_ { begin }, end_ { end } { trim(); }
        Num(const Result &result);

        Num(const Num& other) = default;
        Num& operator=(const Num& other) = default;

        [[nodiscard]] const unsigned short* begin() const { return begin_; }
        [[nodiscard]] const unsigned short* end() const { return end_; }
        [[nodiscard]] ssize_t size() const { return end_ - begin_; }
        [[nodiscard]] bool empty() const { return begin_ >= end_; }
    };


    class Error : public std::exception { };

    class Result {
            unsigned short *const begin_;
            unsigned short *const end_;
            unsigned short* used_;
            friend class Num;
            friend Result& add(Result& res, const Num& num);
            friend Result& sub(Result& res, const Num& num);
            friend Result& multiply_and_add(Result& res, const Num& num, int factor, int shift);
            friend Result& div_by_2(Result& value);

        public:
            Result(unsigned short *begin, unsigned short *end): begin_ { begin }, end_ { end }, used_ { end } { }
            Result(const Result&) = delete;
            Result& operator=(const Result& other) { if (&other != this) { copy(other); } return *this; }
            Result& operator=(const Num& num) { copy(num); return *this; }

            void copy(const Num& num, int shift = 0);
            void clear() { used_ = end_; }
            void push_back(unsigned short num);

            [[nodiscard]] bool empty() const { return used_ >= end_; }
            [[nodiscard]] bool odd() const { return used_ < end_ && (end_[-1] % 2); }
    };

    inline Num:: Num(const Result& result): begin_ { result.used_ }, end_ { result.end_ } { trim(); }

    struct Div_Result {
        Result div;
        Result rem;
        Result scratch1;
        Result scratch2;

        Div_Result(
            unsigned short* div_begin, unsigned short* div_end,
            unsigned short* rem_begin, unsigned short* rem_end,
            unsigned short* scratch1_begin, unsigned short* scratch1_end,
            unsigned short* scratch2_begin, unsigned short* scratch2_end
        ):
            div { div_begin, div_end },
            rem { rem_begin, rem_end },
            scratch1 { scratch1_begin, scratch1_end },
            scratch2 { scratch2_begin, scratch2_end }
        { }
    };

    struct Pow_Result {
        Result result;
        Result scratch1;
        Result scratch2;
        Result scratch3;
        Div_Result& div_result;

        Pow_Result(
            unsigned short* result_begin, unsigned short* result_end,
            unsigned short* scratch1_begin, unsigned short* scratch1_end,
            unsigned short* scratch2_begin, unsigned short* scratch2_end,
            unsigned short* scratch3_begin, unsigned short* scratch3_end,
            Div_Result& div_result
        ):
            result { result_begin, result_end },
            scratch1 { scratch1_begin, scratch1_end },
            scratch2 { scratch2_begin, scratch2_end },
            scratch3 { scratch3_begin, scratch3_end },
            div_result { div_result }
        { }
    };

    bool operator==(const Num& a, const Num& b);
    inline bool operator!=(const Num& a, const Num& b) { return !(a == b); }
    bool operator<(const Num& a, const Num& b);
    inline bool operator>(const Num& a, const Num& b) { return b < a; }
    inline bool operator<=(const Num& a, const Num& b) { return !(b < a); }
    inline bool operator>=(const Num& a, const Num& b) { return !(a < b); }

    Result& add(Result& res, const Num& num);
    Result& sub(Result& res, const Num& num);
    Result& mult(Result& res, const Num& a, const Num& b);
    Div_Result& div(Div_Result& res, const Num& a, const Num& b);

    Result& mod(Result& res, const Num& m, Div_Result& tmp);
    Result& mult_mod(Result& res, const Num& a, const Num& b, const Num& m, Div_Result& tmp);
    Pow_Result& pow_mod(Pow_Result& res, const Num& a, const Num& b, const Num& m);
}