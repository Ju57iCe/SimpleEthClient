#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "Utils/RLP.h"
#include "Utils/Hex.h"

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

    std::any any_str ;
    any_str = std::string("Test");

    std::vector<std::string> vec_one = { "111", /*"333", "555"*/ };
    std::vector<std::string> vec_two = { "222", /*"444", "666"*/ };
    
    std::any any_vec_one = std::make_any<std::vector<std::string>>(vec_one);
    std::any any_vec_two = std::make_any<std::vector<std::string>>(vec_two);

    std::vector<std::any> nested_any_vec;
    nested_any_vec.push_back(any_vec_one);
    nested_any_vec.push_back(any_vec_two);

    std::vector<uint8_t> bytes = Utils::RLP::Encode(nested_any_vec);
    std::any any_res = Utils::RLP::DecodeAny(bytes);

    std::vector<std::any> any_vec_res = std::any_cast<std::vector<std::any>>(any_res);
    std::cout << "any_vec_res " << any_res.type().name() << std::endl;
    std::cout << "any_vec_res[0] " << any_vec_res[0].type().name() << std::endl;
    std::cout << "any_str" << any_str.type().name() << std::endl;

    std::vector<std::string> str_vec_res_one = std::any_cast<std::vector<std::string>>(any_vec_res[0]);
    std::vector<std::string> str_vec_res_two = std::any_cast<std::vector<std::string>>(any_vec_res[1]);

    int a = 42;
    // ClientApp client(multiaddress);
    //client.run();
}
