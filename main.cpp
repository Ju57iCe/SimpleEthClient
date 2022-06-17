/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "ClientApp.h"

int main(int argc, char *argv[])
{
    boost::program_options::options_description desc
        ("\nInvocation : <program> --multiaddress <multiaddress_string> --bootstrap_node <true/false\nAgruments");

    desc.add_options ()
    ("multiaddress", boost::program_options::value<std::string>()->required(),
                 "* Multiaddress.")
    ("bootstrap_node", boost::program_options::bool_switch()->default_value(false),
                 "Run as a bootstrap node.");

    boost::program_options::variables_map vm;

    try {
        boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
                                                              options(desc).
                                                              run(), vm);
        boost::program_options::notify(vm);

    } catch (boost::program_options::error& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        std::cout << desc << "\n";
        return 1;
    }

    std::string multiaddress(vm["multiaddress"].as<std::string>());
    bool bootstrap(vm["bootstrap_node"].as<bool>());

    std::cout << "Client multiaddress : " << multiaddress << std :: endl;
    std::cout << "Running as a boostrap node : " << (bootstrap ? "True" : "False") << std :: endl;

    ClientApp client(multiaddress);
    client.run();
}