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

    // Keccak keccak256;

    // std::cout << keccak256("Hello World") << std::endl;

    // const char* buffer = "How are you";
    // std::cout << keccak256(buffer, 11) << std::endl;


    // std::vector<std::string> list = { "hello" };
    // std::any any_list = list;
    // std::vector<uint8_t> rlp_value = Utils::RLP::Encode(any_list);

    // for (uint32_t i = 0; i < rlp_value.size(); ++i)
    // {
    //     if (i < 4)
    //         std::cout << std::hex << (uint32_t)rlp_value[i];
    // }
    // std::cout << std::dec << std::endl;

    // char b = 'a';
    // std::bitset<4> bits_first = (b & 0xF0) >> 4;
    // std::bitset<4> bits_second = b & 0x0F;

    // std::cout << bits_first.to_string() << std::endl;
    // std::cout << bits_second.to_string() << std::endl;

    Utils::MPT tree;
    tree.update("32fa7b", "10");
    tree.update("32fa7c", "20"); // keccak of key - 4f6c1c50fde5f5d4f20c2979974a8f465b24e65062f02ef80f722200f35105e2
    tree.print_contents();

    // tree.update("abbc", "2");
    // tree.print_contents();
    // tree.update("abbd", "2");
    // tree.print_contents();
    // tree.update("abbcc", "2");
    // tree.print_contents();

    //tree.update("cac", 2);
    //tree.print_contents();
    // tree.update("cb", 2);
    // tree.print_contents();
    // tree.update("caac", 2);
    // tree.print_contents();
    // tree.update("a71135", 45);
    // tree.update("a77d337", 1);
    // tree.update("a7f9365", 1);
    // tree.update("a77d397", 1);


    //int a = 42;

//    ClientApp client(multiaddress);
//    client.run();
}
