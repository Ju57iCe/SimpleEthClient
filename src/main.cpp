#include <iostream>
#include <string>
#include <bitset>
#include <boost/program_options.hpp>

#include <Utils/MPT.h>
#include <Utils/RLP.h>

#include "ClientApp.h"

int main(int argc, char *argv[])
{
    // boost::program_options::options_description desc
    //     ("\nInvocation : <program> --multiaddress <multiaddress_string> --bootstrap_node <true/false\nAgruments");

    // desc.add_options ()
    // ("multiaddress", boost::program_options::value<std::string>()->required(),
    //              "* Multiaddress.")
    // ("bootstrap_node", boost::program_options::bool_switch()->default_value(false),
    //              "Run as a bootstrap node.");

    // boost::program_options::variables_map vm;

    // try {
    //     boost::program_options::store(boost::program_options::command_line_parser(argc, argv).
    //                                                           options(desc).
    //                                                           run(), vm);
    //     boost::program_options::notify(vm);

    // } catch (boost::program_options::error& e) {
    //     std::cout << "ERROR: " << e.what() << "\n";
    //     std::cout << desc << "\n";
    //     return 1;
    // }

    // std::string multiaddress(vm["multiaddress"].as<std::string>());
    // bool bootstrap(vm["bootstrap_node"].as<bool>());

    // std::cout << "Client multiaddress : " << multiaddress << std :: endl;
    // std::cout << "Running as a bootsrap node : " << (bootstrap ? "True" : "False") << std :: endl;

    Utils::MPT tree;
    tree.update("32fa7b", "10");
    tree.update("32fa7c", "20");
    //tree.update("32fa7c", "20"); // keccak of key - 4f6c1c50fde5f5d4f20c2979974a8f465b24e65062f02ef80f722200f35105e2
    // tree.update("32fa7d", "20");
    // tree.update("32fa7e", "20");
    // tree.update("32fa7f", "20");
    // tree.update("32fa7g", "20");
    tree.print_contents();


    //// f851808080a02fd2c9e2e74e9d07a920dd1ebf94f1bd7a5aa1764464769c83ce1cbb38137d65a0b7f631fbd6cfb1aeb19411e75fc33769934c7ea2242a47b54ed6895e9627a0fc808080808080808080808080
//    ClientApp client(multiaddress);
//    client.run();
}
