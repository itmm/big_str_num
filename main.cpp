#include <cassert>
#include <memory>

#include "embedded-rsa.h"

template<int SZ> class Buffered_Result : public Embedded_RSA::Result {
        unsigned short buffer_[SZ] { };
    public:
        Buffered_Result() : Result(buffer_, buffer_ + SZ) { }
};

void fill(Embedded_RSA::Result& result, unsigned long long value) {
    result = Embedded_RSA::Num { };
    while (value != 0) {
        result.push_back(static_cast<unsigned short>(value % Embedded_RSA::base));
        value = value / Embedded_RSA::base;
    }
}

class Long_Result : public Buffered_Result<4> {
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

        explicit Long_Add_State(std::unique_ptr<State> state) :
            Add_State { state->value_buffer, state->modulus_buffer },
            state { std::move(state) } { }

    public:
        Long_Add_State(unsigned long long value, unsigned long long modulus) :
            Long_Add_State { std::make_unique<State>(value, modulus) } { }

        Long_Add_State& operator=(unsigned long long value) {
            fill(state->value_buffer, value);
            return *this;
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

        explicit Buffered_Mul_State(std::unique_ptr<State> state) :
            Embedded_RSA::Mul_State(
                state->value_buffer, state->modulus_buffer,
                state->scratch1_buffer, state->scratch2_buffer
            ), state { std::move(state) } { }
};

class Long_Mul_State : public Buffered_Mul_State {
    public:
        Long_Mul_State(unsigned long long value, unsigned long long modulus) :
            Buffered_Mul_State { std::make_unique<State>(value, modulus) } { }

        Long_Mul_State& operator=(unsigned long long value) {
            fill(state->value_buffer, value);
            return *this;
        }

        operator Embedded_RSA::Num() const { return state->value_buffer; } // NOLINT
};

class Long_Pow_State : public Embedded_RSA::Pow_State {
    protected:
        struct State {
            Long_Result value;
            Long_Result modulus;
            Long_Result scratch1 { };
            Long_Result scratch2 { };
            Long_Result scratch3 { };
            Long_Result scratch4 { };

            explicit State(unsigned long long value, unsigned long long modulus) :
                value { value }, modulus { modulus } { }
        };

        std::unique_ptr<State> state;

        explicit Long_Pow_State(std::unique_ptr<State> state) :
            Embedded_RSA::Pow_State(
                state->value, state->modulus, state->scratch1, state->scratch2, state->scratch3, state->scratch4
            ),
            state { std::move(state) } { }

    public:
        explicit Long_Pow_State(unsigned long long value, unsigned long long modulus) :
            Long_Pow_State { std::make_unique<State>(value, modulus) } { }
};

void assert_num(const Embedded_RSA::Num& num, unsigned long long expected) {
    if (expected > 0) {
        assert(num == Long_Result(expected));
    } else {
        assert(num.begin() == num.end());
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
    assert(!Long_Result(initial).empty());
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

void assert_multiply(unsigned long long a, unsigned long long b, unsigned long long expected) {
    Long_Mul_State result { a, { } };
    result *= Long_Result { b };
    assert(result == Long_Result { expected });
}

void assert_pow(unsigned long long a, unsigned long long b, unsigned long long m, unsigned long long expected) {
    Long_Pow_State res { a, m };
    res.pow(Long_Result { b });
    assert(res.value == Long_Result { expected });
}

void assert_byte_size(unsigned long long key, int expected) {
    auto result = Embedded_RSA::Rsa_State::byte_size(Long_Result { key });
    assert(result == expected);
}

const unsigned short rsa_modulus[128] {
    0x68ed, 0x6d7a, 0x97c8, 0x2e32, 0xe44f, 0x36db, 0xb9bf, 0xd05e,
    0xd548, 0x6caf, 0xf80a, 0xeec8, 0x6b3f, 0xe0e7, 0x4cad, 0xd1e4,
    0xafa2, 0x69cd, 0xd824, 0xe629, 0x9ab0, 0x7be5, 0x99d0, 0x8ee8,
    0xf988, 0xc784, 0x0f9d, 0xc61e, 0xd050, 0xb112, 0x7ba2, 0x008c,
    0x4f31, 0x9546, 0x371c, 0x29e5, 0x4c03, 0x6f9a, 0x890f, 0xeff6,
    0x96c7, 0x9afa, 0x1739, 0xf99e, 0x5256, 0xd075, 0x1580, 0x34d3,
    0x6e86, 0xf0a4, 0xef90, 0x80eb, 0xbb09, 0xde09, 0x78f7, 0xf830,
    0xa139, 0x7b78, 0x040b, 0x9e01, 0xd1b0, 0x0695, 0xee31, 0x57b4,
    0xc423, 0x5233, 0x2177, 0x2da6, 0x7fd7, 0x2dd0, 0x2d19, 0x28b0,
    0xabee, 0x5dc8, 0x3e57, 0xa5f9, 0x53d8, 0x1fce, 0x4c09, 0xdd70,
    0xaeb4, 0xcd0a, 0x9c6e, 0xbb89, 0x5e40, 0x90dc, 0xef37, 0x99c3,
    0xdd02, 0x7d9a, 0x9bdb, 0x1763, 0x4fa5, 0xd092, 0x3db3, 0xa174,
    0xc69f, 0x9463, 0x2648, 0xeac4, 0x9cdb, 0xda24, 0xd907, 0x6690,
    0x172b, 0x81f6, 0x71e3, 0x95df, 0x7033, 0x4ad5, 0x84b8, 0x05c5,
    0x1ae0, 0x736e, 0x9d7d, 0xc074, 0x8bc6, 0x8839, 0xca4a, 0x3787,
    0xdb6a, 0xf2b2, 0x61fc, 0x1fff, 0x6ddd, 0x62e7, 0x1693, 0xd54c
};

const unsigned short rsa_public_exponent[2] { 0x0001, 0x0001 };

const unsigned short rsa_private_exponent[128] {
    0xbe41, 0xa41f, 0xedb8, 0x476e, 0xd203, 0x0087, 0xfb12, 0xe46f,
    0x1526, 0xd130, 0xce28, 0xa622, 0xdc15, 0x40d2, 0x0a29, 0x3951,
    0x2185, 0x92c7, 0x38f2, 0xa65e, 0x439d, 0xd1fa, 0x207b, 0x20bd,
    0x8074, 0x1a02, 0xd064, 0x922d, 0x8bcd, 0x2411, 0xbd42, 0xe54d,
    0x4962, 0xb949, 0x9dbb, 0x1122, 0xb255, 0xd98d, 0xefee, 0x73bd,
    0xfd35, 0xc3fa, 0xcb03, 0x57de, 0x16b9, 0xeb31, 0x7a0d, 0x474e,
    0xf886, 0xd3b9, 0x4622, 0xe6bf, 0x87be, 0x57af, 0x93ae, 0x1cfa,
    0x95fb, 0xb491, 0x5d14, 0xe2f0, 0xdc4e, 0xfcac, 0x2c81, 0xe2eb,
    0xff28, 0xdabb, 0x0d57, 0x53f1, 0xc9f9, 0xb4b6, 0x69fc, 0x3bc3,
    0x7adb, 0xb4e7, 0xb033, 0x3d11, 0x4f56, 0x0f4e, 0xbee9, 0x398e,
    0xb4aa, 0x6dfd, 0x215f, 0x4040, 0xa68d, 0x583a, 0x54e6, 0x83de,
    0x8275, 0x8adf, 0xd2cb, 0x4717, 0xb656, 0x15ef, 0xc3fb, 0x2d9b,
    0xdeb5, 0xc74f, 0x514a, 0x1d75, 0xb322, 0x764f, 0x648b, 0x534f,
    0xbede, 0xddcb, 0x1180, 0x3610, 0xe538, 0xa3a1, 0x557b, 0x75ab,
    0xd51d, 0xb1c1, 0x944d, 0xbda5, 0xdb72, 0xd7fe, 0x044a, 0x65bb,
    0x749f, 0x07fc, 0x13d4, 0x0587, 0x4111, 0xb6c7, 0xf711, 0x08f7
};

using Rsa_Result = Buffered_Result<129>;

void assert_rsa(const char* data) {
    const char *data_end { data + strlen(data) };
    Embedded_RSA::Num modulus { rsa_modulus, rsa_modulus + sizeof(rsa_modulus)/sizeof(*rsa_modulus) };
    Embedded_RSA::Num private_exponent { rsa_private_exponent, rsa_private_exponent + sizeof(rsa_private_exponent)/sizeof(*rsa_private_exponent) };
    Rsa_Result scratch1, scratch2, scratch3, scratch4, scratch5;
    Embedded_RSA::Rsa_State enc_state {
        modulus, private_exponent, scratch1, scratch2, scratch3, scratch4, scratch5
    };
    char encrypted[255];
    const char* encrypted_end { encrypted + sizeof(encrypted)/sizeof(*encrypted) };
    char *got { enc_state.encrypt(data, data_end, encrypted, encrypted_end) };
    assert(got == encrypted_end);

    Embedded_RSA::Num public_exponent { rsa_public_exponent, rsa_public_exponent + sizeof(rsa_public_exponent) / (*rsa_public_exponent) };
    Embedded_RSA::Rsa_State dec_state {
        modulus, public_exponent, scratch1, scratch2, scratch3, scratch4, scratch5
    };
    char decrypted[255 - 11];
    const char* decrypted_end { decrypted + sizeof(decrypted)/sizeof(*decrypted) };
    got = dec_state.decrypt(encrypted, encrypted_end, decrypted, decrypted_end);
    assert(got - decrypted == strlen(data));
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

    assert(Long_Result { 12 + Embedded_RSA::base } == Long_Result { 12 + Embedded_RSA::base });
    assert(Long_Result { 12 + Embedded_RSA::base } != Long_Result { 13 + Embedded_RSA::base });
    assert(Long_Result { 12 + Embedded_RSA::base } < Long_Result { 13 + Embedded_RSA::base });
    assert(Long_Result { 12 + Embedded_RSA::base } <= Long_Result { 13 + Embedded_RSA::base });
    assert(Long_Result { 12 + Embedded_RSA::base } <= Long_Result { 12 + Embedded_RSA::base });
    assert(Long_Result { 13 + Embedded_RSA::base } > Long_Result { 12 + Embedded_RSA::base });
    assert(Long_Result { 13 + Embedded_RSA::base } >= Long_Result { 12 + Embedded_RSA::base });
    assert(Long_Result { 13 + Embedded_RSA::base } >= Long_Result { 13 + Embedded_RSA::base });

    assert(Long_Result { 12 * Embedded_RSA::base } == Long_Result { 12 * Embedded_RSA::base });
    assert(Long_Result { 12 * Embedded_RSA::base } != Long_Result { 13 * Embedded_RSA::base });
    assert(Long_Result { 12 * Embedded_RSA::base } < Long_Result { 13 * Embedded_RSA::base });
    assert(Long_Result { 12 * Embedded_RSA::base } <= Long_Result { 13 * Embedded_RSA::base });
    assert(Long_Result { 12 * Embedded_RSA::base } <= Long_Result { 12 * Embedded_RSA::base });
    assert(Long_Result { 13 * Embedded_RSA::base } > Long_Result { 12 * Embedded_RSA::base });
    assert(Long_Result { 13 * Embedded_RSA::base } >= Long_Result { 12 * Embedded_RSA::base });
    assert(Long_Result { 13 * Embedded_RSA::base } >= Long_Result { 13 * Embedded_RSA::base });

    assert(Long_Result { 100 } < Long_Result { 13 * Embedded_RSA::base });
    assert(Long_Result { 13 * Embedded_RSA::base } > Long_Result { 100 });

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

    assert_multiply(123, 0, 0);
    assert_multiply(123, 1, 123);
    assert_multiply(0, 2, 0);
    assert_multiply(1234, 10001, 12341234);
    assert_multiply(10001, 1234, 12341234);
    assert_multiply(11, 11, 121);
    assert_multiply(10, 13, 130);
    assert_multiply(13, 10, 130);
    assert_multiply(1234, 1000000, 1234000000);
    assert_multiply(
        Embedded_RSA::base, Embedded_RSA::base,
        static_cast<unsigned long long>(Embedded_RSA::base) * Embedded_RSA::base
    );
    assert_multiply(1000, 100, 100000);

    assert_pow(2, 10, 1025, 1024);
    assert_pow(2, 10, 10, 4);
    assert_pow(2, 10, 5, 4);
    assert_pow(3, 15, 17, 6);

    assert_byte_size(16777215, 2);
    assert_byte_size(16777216, 3);
    assert_byte_size(16777217, 3);

    assert_rsa("Hallo");

    return 0;
}
