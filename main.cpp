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

    std::vector<std::string> list;
    list.emplace_back("cat");
    list.emplace_back("dogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdogdog");

    std::string str("123");

    std::any any_str = "test";

    std::any test_vector_any = std::vector<std::any>({"123", "456","789"}); 

    std::vector<std::any> any_vec;
    any_vec.emplace_back(test_vector_any);

    //any_vec.emplace_back(str);
    //any_vec.emplace_back(any_str);

    std::vector<uint8_t> bytes = Utils::RLP::Encode(any_vec);
    //std::vector<std::string> decoded_list = Utils::RLP::DecodeList(bytes);

    int a = 42;
    // ClientApp client(multiaddress);
    //client.run();
}
