#include <cassert>
#include <memory>

#include "embedded-rsa.h"

template<int SZ> class Buffered_Result: public Embedded_RSA::Result {
        unsigned short buffer_[SZ] { };
    public:
        Buffered_Result(): Result(buffer_, buffer_ + SZ) { }
};

void fill(Embedded_RSA::Result& result, unsigned long long value) {
    result = Embedded_RSA::Num { };
    while (value != 0) {
        result.push_back(static_cast<unsigned short>(value % Embedded_RSA::base));
        value = value / Embedded_RSA::base;
    }
}

class Long_Result: public Buffered_Result<4> {
    public:
        explicit Long_Result(unsigned long long value = 0) { fill(*this, value); }
        using Embedded_RSA::Result::operator=;
};

class Long_Add_State : public Embedded_RSA::Add_State {
    protected:
        struct State {
            Long_Result value_buffer;
            Long_Result modulus_buffer;

            State(unsigned long long value, unsigned long long modulus) {
                fill(value_buffer, value);
                fill(modulus_buffer, modulus);
            }
        };

        std::unique_ptr<State> state;

        explicit Long_Add_State(std::unique_ptr<State> state):
            Add_State { state->value_buffer, state->modulus_buffer },
            state { std::move(state) }
        { }

    public:
        Long_Add_State(unsigned long long value, unsigned long long modulus):
            Long_Add_State { std::make_unique<State>(value, modulus) }
        { }

        Long_Add_State& operator=(unsigned long long value) {
            fill(state->value_buffer, value); return *this;
        }

        operator Embedded_RSA::Num() const { return state->value_buffer; } // NOLINT
};

class Buffered_Mul_State : public Embedded_RSA::Mul_State {
    protected:
        struct State {
            Long_Result value_buffer;
            Long_Result modulus_buffer;
            Long_Result scratch1_buffer;
            Long_Result scratch2_buffer;

            State(unsigned long long value, unsigned long long modulus) {
                fill(value_buffer, value);
                fill(modulus_buffer, modulus);
            }
        };

        std::unique_ptr<State> state;

        explicit Buffered_Mul_State(std::unique_ptr<State> state):
            Embedded_RSA::Mul_State(
                state->value_buffer, state->modulus_buffer,
                state->scratch1_buffer, state->scratch2_buffer
            ), state {std::move(state) } { }
};

class Long_Mul_State: public Buffered_Mul_State {
    public:
        Long_Mul_State(unsigned long long value, unsigned long long modulus):
                Buffered_Mul_State { std::make_unique<State>(value, modulus) }
        { }
        Long_Mul_State& operator=(unsigned long long value) {
            fill(state->value_buffer, value); return *this;
        }
        operator Embedded_RSA::Num() const { return state->value_buffer; } // NOLINT
};

class Long_Pow_State: public Embedded_RSA::Pow_State {
    protected:
        struct State {
            Long_Mul_State value;
            Long_Mul_State scratch1;
            Long_Result scratch2 { };

            explicit State(unsigned long long value, unsigned long long modulus):
                value { value, modulus }, scratch1{ 0, modulus }
            { }
        };

        std::unique_ptr<State> state;

        explicit Long_Pow_State(std::unique_ptr<State> state):
            Embedded_RSA::Pow_State(state->value, state->scratch1, state->scratch2),
            state { std::move(state) }
        { }

    public:
        explicit Long_Pow_State(unsigned long long value, unsigned long long modulus):
            Long_Pow_State { std::make_unique<State>(value, modulus) }
        { }
};

void assert_num(const Embedded_RSA::Num &num, unsigned long long expected) {
    if (expected > 0) {
        assert(num == Long_Result(expected));
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
    Long_Add_State result { a, 0 };
    result += Long_Result { b };
    assert(result == Long_Result(expected));
}

void assert_sub(unsigned long long a, unsigned long long b, unsigned long long expected) {
    Long_Result result { a };
    result -= Long_Result { b };
    assert(result == Long_Result(expected));
}

void assert_mult(unsigned long long a, unsigned long long b, unsigned long long expected) {
    Long_Mul_State result { a, { } };
    result *= Long_Result { b };
    assert(result == Long_Result { expected });
}

void assert_pow(unsigned long long a, unsigned long long b, unsigned long long m, unsigned long long expected) {
    Long_Pow_State res { a, m };
    res.pow(Long_Result { b });
    assert(res.result == Long_Result { expected });
}

int main() {
    assert_num(Embedded_RSA::Num { }, 0);
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
        result = Embedded_RSA::Num { };
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

    assert(Long_Result {12 + Embedded_RSA::base } == Long_Result {12 + Embedded_RSA::base });
    assert(Long_Result {12 + Embedded_RSA::base } != Long_Result {13 + Embedded_RSA::base });
    assert(Long_Result {12 + Embedded_RSA::base } < Long_Result {13 + Embedded_RSA::base });
    assert(Long_Result {12 + Embedded_RSA::base } <= Long_Result {13 + Embedded_RSA::base });
    assert(Long_Result {12 + Embedded_RSA::base } <= Long_Result {12 + Embedded_RSA::base });
    assert(Long_Result {13 + Embedded_RSA::base } > Long_Result {12 + Embedded_RSA::base });
    assert(Long_Result {13 + Embedded_RSA::base } >= Long_Result {12 + Embedded_RSA::base });
    assert(Long_Result {13 + Embedded_RSA::base } >= Long_Result {13 + Embedded_RSA::base });

    assert(Long_Result {12 * Embedded_RSA::base } == Long_Result {12 * Embedded_RSA::base });
    assert(Long_Result {12 * Embedded_RSA::base } != Long_Result {13 * Embedded_RSA::base });
    assert(Long_Result {12 * Embedded_RSA::base } < Long_Result {13 * Embedded_RSA::base });
    assert(Long_Result {12 * Embedded_RSA::base } <= Long_Result {13 * Embedded_RSA::base });
    assert(Long_Result {12 * Embedded_RSA::base } <= Long_Result {12 * Embedded_RSA::base });
    assert(Long_Result {13 * Embedded_RSA::base } > Long_Result {12 * Embedded_RSA::base });
    assert(Long_Result {13 * Embedded_RSA::base } >= Long_Result {12 * Embedded_RSA::base });
    assert(Long_Result {13 * Embedded_RSA::base } >= Long_Result {13 * Embedded_RSA::base });

    assert(Long_Result { 100 } < Long_Result {13 * Embedded_RSA::base });
    assert(Long_Result {13 * Embedded_RSA::base } > Long_Result {100 });

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
        Long_Add_State res { 12, 0 };
        res += res;
        assert(res == Long_Result { 24 });
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
    assert_mult(
        Embedded_RSA::base, Embedded_RSA::base,
        static_cast<unsigned long long>(Embedded_RSA::base) * Embedded_RSA::base
    );
    assert_mult(1000, 100, 100000);

    assert_pow(2, 10, 1025, 1024);
    assert_pow(2, 10, 10, 4);
    assert_pow(2, 10, 5, 4);
    assert_pow(3, 15, 17, 6);

    return 0;
}
