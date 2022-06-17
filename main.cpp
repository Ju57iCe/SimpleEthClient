/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ClientApp.h"

int main(int argc, char *argv[])
{
    ClientApp client("/ip4/127.0.0.1/tcp/3333");
    client.run();
}
