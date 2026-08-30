#include "sha256.hpp"

#include <array>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <vector>

namespace {

constexpr std::array<uint32_t, 64> K = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U
};

inline uint32_t rotr(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (32 - n));
}

class Sha256Context {
public:
    Sha256Context()
        : h{0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
            0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U},
          bitLength(0)
    {
    }

    void update(const unsigned char* data, size_t length)
    {
        bitLength += static_cast<uint64_t>(length) * 8;

        buffer.insert(buffer.end(), data, data + length);

        while (buffer.size() >= 64) {
            processBlock(buffer.data());
            buffer.erase(buffer.begin(), buffer.begin() + 64);
        }
    }

    std::array<uint8_t, 32> finalize()
    {
        // Append the 0x80 padding byte.
        buffer.push_back(0x80);

        // Pad with zeros until length % 64 == 56.
        while (buffer.size() % 64 != 56) {
            buffer.push_back(0x00);
        }

        // Append original bit length as a big-endian 64-bit integer.
        for (int i = 7; i >= 0; --i) {
            buffer.push_back(static_cast<uint8_t>((bitLength >> (i * 8)) & 0xFF));
        }

        while (buffer.size() >= 64) {
            processBlock(buffer.data());
            buffer.erase(buffer.begin(), buffer.begin() + 64);
        }

        std::array<uint8_t, 32> digest{};
        for (int i = 0; i < 8; ++i) {
            digest[i * 4 + 0] = static_cast<uint8_t>((h[i] >> 24) & 0xFF);
            digest[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xFF);
            digest[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xFF);
            digest[i * 4 + 3] = static_cast<uint8_t>(h[i] & 0xFF);
        }
        return digest;
    }

private:
    void processBlock(const unsigned char* block)
    {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(block[i * 4 + 0]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   (static_cast<uint32_t>(block[i * 4 + 3]));
        }

        for (int i = 16; i < 64; ++i) {
            const uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            const uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; ++i) {
            const uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const uint32_t ch = (e & f) ^ ((~e) & g);
            const uint32_t temp1 = hh + s1 + ch + K[i] + w[i];
            const uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const uint32_t temp2 = s0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    uint32_t h[8];
    uint64_t bitLength;
    std::vector<unsigned char> buffer;
};

} // namespace

namespace sha256 {

std::string hash(const std::string& input)
{
    Sha256Context context;
    context.update(reinterpret_cast<const unsigned char*>(input.data()), input.size());
    const std::array<uint8_t, 32> digest = context.finalize();

    std::ostringstream oss;
    for (const uint8_t byte : digest) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

} // namespace sha256
