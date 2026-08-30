#ifndef SHA256_HPP
#define SHA256_HPP

#include <cstdint>
#include <string>

// Minimal, dependency-free SHA-256 implementation (public-domain algorithm,
// reimplemented here so the project doesn't need to link OpenSSL just for
// password hashing).
namespace sha256 {

// Returns the lowercase hex-encoded SHA-256 digest of `input`.
std::string hash(const std::string& input);

} // namespace sha256

#endif
