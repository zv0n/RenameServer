#include "jwt.hpp"
#include <jwt.h>
#include <iostream>

const std::string secret_key = "supersecretkey";

std::string createLoginToken(const std::string &user) {
    auto alg = JWT_ALG_HS256;
    time_t iat = time(NULL);
    time_t exp = iat + 60*60;
    std::string result = "";

    jwt_t *jwt;
    auto ret = jwt_new(&jwt);
    if (ret != 0 || jwt == NULL) {
		fprintf(stderr, "invalid jwt\n");
		goto finish;
	}
    jwt_add_grant_int(jwt, "iat", iat);
    jwt_add_grant_int(jwt, "exp", exp);
    jwt_add_grant(jwt, "user", user.c_str());
    jwt_add_grant_bool(jwt, "valid", true);
    jwt_set_alg(jwt, alg, reinterpret_cast<const unsigned char*>(secret_key.c_str()), secret_key.length());
    result = std::string(jwt_encode_str(jwt));
finish:
	jwt_free(jwt);
	return result;
}

bool verifyJWT(const std::string &token) {
    jwt_valid_t *jwt_valid;
    jwt_t *jwt;
    bool result = false;
    auto ret = jwt_valid_new(&jwt_valid, JWT_ALG_HS256);
	if (ret != 0 || jwt_valid == NULL) {
		fprintf(stderr, "failed to allocate jwt_valid\n");
		goto finish_valid;
	}
    jwt_valid_set_headers(jwt_valid, 1);
    jwt_valid_set_now(jwt_valid, time(NULL));
    jwt_valid_add_grant_bool(jwt_valid, "valid", true);

    ret = jwt_decode(&jwt, token.c_str(), reinterpret_cast<const unsigned char*>(secret_key.c_str()), secret_key.length());
	if (ret != 0 || jwt == NULL) {
		fprintf(stderr, "invalid jwt\n");
		goto finish;
	}
    if (jwt_validate(jwt, jwt_valid) != 0) {
		fprintf(stderr, "jwt failed to validate: %08x\n", jwt_valid_get_status(jwt_valid));
		jwt_dump_fp(jwt, stderr, 1);
		goto finish;
	}

    result = true;

finished_valid:
    finish:
	jwt_free(jwt);
finish_valid:
	jwt_valid_free(jwt_valid);

	return result;
}
