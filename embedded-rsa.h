#pragma once

#include <cstring>
#include <cstdio>
#include <exception>
#include <limits>


namespace Embedded_RSA {
    constexpr int base { 0x10000 }; // number of values per digit

    static_assert(std::numeric_limits<unsigned short>::max() >= (base - 1));

    class Num {
        // represents a readonly number

        const unsigned short* begin_ { nullptr }; // lowest digit
        const unsigned short* end_ { nullptr }; // one after highest digit

        void trim();

    public:
        Num() = default;
        Num(const unsigned short* begin, const unsigned short* end):
            begin_ { begin }, end_ { end }
        { trim(); }

        Num(const Num& other) = default;
        Num& operator=(const Num& other) = default;

        [[nodiscard]] const unsigned short* begin() const { return begin_; }
        [[nodiscard]] const unsigned short* end() const { return end_; }
        [[nodiscard]] ssize_t size() const { return end_ - begin_; }
        explicit operator bool() const { return begin_ < end_; }
    };

    bool operator==(const Num& a, const Num& b);
    inline bool operator!=(const Num& a, const Num& b) { return !(a == b); }
    bool operator<(const Num& a, const Num& b);
    inline bool operator>(const Num& a, const Num& b) { return b < a; }
    inline bool operator<=(const Num& a, const Num& b) { return !(b < a); }
    inline bool operator>=(const Num& a, const Num& b) { return !(a < b); }

    class Error : public std::exception {
        // thrown if not enough space to store a result/number
    };

    class Result {
        // place to store a result of a calculation

        private:
            unsigned short *const begin_; // lowest digit
            unsigned short *const end_; // one after the maximum digit usable
            unsigned short* used_; // one after the highest digit

            void trim();
            void copy(const Num& num);

        public:
            Result(unsigned short *begin, unsigned short *end):
                begin_ { begin }, end_ { end }, used_ { begin }
            { }

            Result(const Result&) = delete;

            Result& operator=(const Result& other) {
                if (&other != this) { copy(other); }
                return *this;
            }
            Result& operator=(const Num& num) { copy(num); return *this; }

            operator Num() const { return { begin_, used_ }; } // NOLINT

            Result& operator-=(const Num& other); // perform subtraction

            void push_back(unsigned short num); // add a new higher digit

            [[nodiscard]] bool empty() const { return used_ <= begin_; }
            [[nodiscard]] bool odd() const { return used_ > begin_ && (*begin_ % 2); }

            template<typename OP> Result& perform_add_op(OP op, const Num& other);
                // processing all digits one by one; used for addition and subtraction

            Result& div_by_2();
    };

    struct Add_State {
        // store addition results and modulus; if modulus is 0, no modulo operation
        // will be performed; Result must be dimensioned to store the value
        // 2 * modulus (if modulus is not zero) or the the whole result, if no
        // modulus is given

        Result& value; // result to add a value to
        const Num modulus;

        Add_State(Result& value, const Num& modulus): value { value }, modulus { modulus } { }

        operator Result&() { return value; } // NOLINT
        operator Num() const { return value; } // NOLINT

        Add_State& operator=(const Num& other) { value = other; return *this; }
        Add_State& operator=(const Add_State& other) { value = other.value; return *this;}

        Add_State& operator+=(const Num& other); // perform addition
    };

    struct Mul_State {
        // store multiplication results and temporary variables needed while
        // performing the multiplication; value.value, scratch1_.value, and
        // scratch2_ must all point to different (non-overlapping) memory ranges;
        // the modulus of value and scratch1_ must be the same; the same
        // memory dimension as in the Add_State must be considered

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

    struct Pow_State {
        // store exponentiation results and temporary values while performing
        // the operation; result and scratch1 can share their scratch buffers
        // only one multiplication is performed at a time; the modulus for all
        // Add_States (in the Mul_States) must be identical

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

}