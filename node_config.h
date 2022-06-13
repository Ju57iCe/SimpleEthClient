#pragma once

#include <string>

namespace config {

const static std::string logger_config(R"(
# ----------------
sinks:
  - name: console
    type: console
    color: true
groups:
  - name: main
    sink: console
    level: info
    children:
      - name: libp2p
# ----------------
  )");

const static std::string hex_bootstrap_priv_key = "4a9361c525840f7086b893d584ebbe475b4ec7069951d2e897e8bceb0a3f35ce";
const static std::string hex_bootstrap_pub_key = "48453469c62f4885373099421a7365520b5ffb0d93726c124166be4b81d852e6";

}