#include "passwordHash.hpp"
#include "sha256.hpp"

#include <array>
#include <iomanip>
#include <random>
#include <sstream>

namespace {

std::string generateSaltHex(size_t byteLength = 16)
{
    std::random_device rd;
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<int> distribution(0, 255);

    std::ostringstream oss;
    for (size_t i = 0; i < byteLength; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << distribution(generator);
    }
    return oss.str();
}

} // namespace

namespace passwordHash {

std::string hashPassword(const std::string& password)
{
    const std::string salt = generateSaltHex();
    const std::string digest = sha256::hash(salt + password);
    return salt + "$" + digest;
}

bool verifyPassword(const std::string& password, const std::string& stored)
{
    const std::size_t separatorPos = stored.find('$');
    if (separatorPos == std::string::npos) {
        // Not in "salt$hash" format. Treat as a non-match rather than
        // silently accepting a plaintext-stored legacy password.
        return false;
    }

    const std::string salt = stored.substr(0, separatorPos);
    const std::string expectedDigest = stored.substr(separatorPos + 1);
    const std::string actualDigest = sha256::hash(salt + password);

    return actualDigest == expectedDigest;
}

} // namespace passwordHash
