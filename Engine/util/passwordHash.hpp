#ifndef PASSWORD_HASH_HPP
#define PASSWORD_HASH_HPP

#include <string>

// Salted SHA-256 password hashing.
// Stored format: "<hex-salt>$<hex-sha256(salt + password)>"
// This is dependency-free (no OpenSSL/bcrypt needed) and good enough to
// avoid storing/comparing plaintext passwords. For a production system,
// a slow KDF like bcrypt/argon2/scrypt would be preferable to plain
// salted SHA-256, since SHA-256 is fast and therefore easier to brute force
// offline if the database is ever leaked.
namespace passwordHash {

// Hashes `password` with a freshly generated random salt and returns the
// combined "salt$hash" string to store in the database.
std::string hashPassword(const std::string& password);

// Checks `password` against a previously stored "salt$hash" string.
bool verifyPassword(const std::string& password, const std::string& stored);

} // namespace passwordHash

#endif
