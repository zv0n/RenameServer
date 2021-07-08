#ifndef JWT_HPP
#define JWT_HPP

#include <string>

std::string createLoginToken(const std::string &user);
bool verifyJWT(const std::string &token);

#endif
