#pragma once

#include <iostream>
#include <string>

#include <tuple>

#include <openssl/evp.h>
#include <openssl/pem.h>

std::tuple<std::string, std::string> GenerateKey();