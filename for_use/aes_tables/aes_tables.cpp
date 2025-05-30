#include <iostream>
#include <iomanip>
#include <array>
#include <cstdint>

// GF(2^8) multiplicative inverse using Extended Euclidean Algorithm
uint8_t gf_inverse(uint8_t a) {
    uint8_t t0 = 0, t1 = 1;
    uint8_t r0 = 0x11b, r1 = a;
    while (r1 != 0) {
        int q = r0 / r1;
        uint8_t temp = r0 % r1;
        r0 = r1;
        r1 = temp;
        temp = t0 ^ gf_mul(q, t1);
        t0 = t1;
        t1 = temp;
    }
    return t0;
}

// Multiply two bytes in GF(2^8)
uint8_t gf_mul(uint8_t a, uint8_t b) {
    uint8_t result = 0;
    while (b) {
        if (b & 1) result ^= a;
        bool carry = a & 0x80;
        a <<= 1;
        if (carry) a ^= 0x1B;
        b >>= 1;
    }
    return result;
}

// Affine transformation for AES S-box
uint8_t affine_transform(uint8_t x) {
    uint8_t result = 0x63; // initial constant
    for (int i = 0; i < 8; ++i) {
        uint8_t bit = (x >> i) & 1;
        uint8_t t = ((x >> ((i + 4) % 8)) ^ (x >> ((i + 5) % 8)) ^
                    (x >> ((i + 6) % 8)) ^ (x >> ((i + 7) % 8))) & 1;
        result ^= (bit ^ t) << i;
    }
    return result;
}

int main() {
    std::array<uint8_t, 256> sbox = {};
    std::array<uint8_t, 256> inv_sbox = {};
    std::array<uint8_t, 256> rcon = {};

    // Generate S-box and inverse S-box
    sbox[0] = 0x63;
    inv_sbox[0x63] = 0;
    for (int i = 1; i < 256; ++i) {
        uint8_t inv = gf_inverse(static_cast<uint8_t>(i));
        uint8_t s = affine_transform(inv);
        sbox[i] = s;
        inv_sbox[s] = static_cast<uint8_t>(i);
    }

    // Generate Rcon table
    rcon[1] = 0x01;
    for (int i = 2; i < 256; ++i) {
        rcon[i] = gf_mul(rcon[i - 1], 0x02);
    }

    // Output
    std::cout << "AES S-box:\n";
    for (int i = 0; i < 256; ++i) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(sbox[i]);
        if ((i + 1) % 16 == 0) std::cout << '\n';
        else std::cout << ", ";
    }

    std::cout << "\nInverse S-box:\n";
    for (int i = 0; i < 256; ++i) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(inv_sbox[i]);
        if ((i + 1) % 16 == 0) std::cout << '\n';
        else std::cout << ", ";
    }

    std::cout << "\nRcon Table (first 16 used):\n";
    for (int i = 1; i <= 16; ++i) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rcon[i]);
        if (i < 16) std::cout << ", ";
    }

    std::cout << "\n";
    return 0;
}
