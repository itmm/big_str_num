#pragma once

#include <cstring>
#include <cstdio>
#include <exception>
#include <limits>


namespace Embedded_RSA {
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

        Num(const Num& other) = default;
        Num& operator=(const Num& other) = default;

        [[nodiscard]] const unsigned short* begin() const { return begin_; }
        [[nodiscard]] const unsigned short* end() const { return end_; }
        [[nodiscard]] ssize_t size() const { return end_ - begin_; }
        explicit operator bool() const { return begin_ < end_; }
    };


    class Error : public std::exception { };

    class Result {
        private:
            unsigned short *const begin_;
            unsigned short *const end_;
            unsigned short* used_;

            template<typename OP> friend Result& perform_add_op(OP op, Result& value, const Num& other);

            void trim();

        public:
            Result(unsigned short *begin, unsigned short *end): begin_ { begin }, end_ { end }, used_ { begin } { }
            Result(const Result&) = delete;

            Result& operator=(const Result& other) { if (&other != this) { copy(other); } return *this; }
            Result& operator=(const Num& num) { copy(num); return *this; }

            operator Num() const { return { begin_, used_ }; } // NOLINT

            Result& operator-=(const Num& other);

            void copy(const Num& num, int shift = 0);
            void clear() { used_ = begin_; }
            void push_back(unsigned short num);

            [[nodiscard]] bool empty() const { return used_ <= begin_; }
            [[nodiscard]] bool odd() const { return used_ > begin_ && (*begin_ % 2); }
            Result& div_by_2();
    };

    struct Add_State {
        Result& value;
        const Num modulus;

        Add_State(Result& value, const Num& modulus): value { value }, modulus { modulus } { }

        operator Result&() { return value; } // NOLINT
        operator Num() const { return value; } // NOLINT

        Add_State& operator=(const Num& other) { value = other; return *this; }
        Add_State& operator=(const Add_State& other) { value = other.value; return *this;}

        Add_State& operator+=(const Num& other);
    };

    struct Mul_State {
        Add_State value;
        Add_State scratch1_;
        Result& scratch2_;

        Mul_State(Result& value, const Num& modulus, Result& scratch1, Result& scratch2 ):
            value { value, modulus }, scratch1_ { scratch1, modulus }, scratch2_ { scratch2 }
        { }

        operator Result&() { return value; } // NOLINT
        operator Num() const { return value; } // NOLINT

        Mul_State& operator=(const Num& other) { value = other; return *this; }

        Mul_State& operator*=(const Num& other);
    };

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

    struct Pow_State {
        Mul_State& result;
        Mul_State& scratch1;
        Result& scratch2;

        Pow_State(
                Mul_State& result, Mul_State& scratch1, Result& scratch2
        ):
            result { result },
            scratch1 { scratch1 },
            scratch2 { scratch2 }
        { }

        Pow_State& pow(const Num& b);
    };

    bool operator==(const Num& a, const Num& b);
    inline bool operator!=(const Num& a, const Num& b) { return !(a == b); }
    bool operator<(const Num& a, const Num& b);
    inline bool operator>(const Num& a, const Num& b) { return b < a; }
    inline bool operator<=(const Num& a, const Num& b) { return !(b < a); }
    inline bool operator>=(const Num& a, const Num& b) { return !(a < b); }
}