#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include <hash-library/keccak.h>
#include <Utils/MPT.h>

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
    std::cout << "Running as a bootsrap node : " << (bootstrap ? "True" : "False") << std :: endl;

    Keccak keccak256;

    std::cout << keccak256("Hello World") << std::endl;

    const char* buffer = "How are you";
    std::cout << keccak256(buffer, 11) << std::endl;

    Utils::MPT tree;

    tree.add_node("ca", 2);
    tree.print_contents();
    tree.add_node("cac", 2);
    tree.print_contents();
    tree.add_node("cb", 2);
    tree.print_contents();
    tree.add_node("caac", 2);
    tree.print_contents();
    tree.add_node("a71135", 45);
    tree.add_node("a77d337", 1);
    tree.add_node("a7f9365", 1);
    tree.add_node("a77d397", 1);


    int a = 42;

//    ClientApp client(multiaddress);
//    client.run();
}
