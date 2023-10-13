#include "argparse/argparse.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    //initialise argument parser
    argparse::ArgumentParser find_args("find");
    //adding arguments to the parser
    find_args.add_argument("directory").default_value(".").required()
        .help("Verzeichnis in dem gesucht werden soll");
    find_args.add_argument("-type")
        .help("f fuer file | d fuer directory");
    find_args.add_argument("-name")
        .default_value("*")
        .help("Akzeptiert wildcards");
    find_args.add_argument("-xdev")
        .default_value(false).implicit_value(true)
        .help("Nur auf dem aktuellen Dateisystem suchen");
    find_args.add_argument("-follow")
        .default_value(false).implicit_value(true)
        .help("Symbolic links folgen");
    // hier noch .remaining einfügen um überschüssige Argumente zu speichern

    // argumente parsen

    try{

        find_args.parse_args(argc, argv);
    } catch (const std::runtime_error& err){
        std::cerr << err.what() << std::endl;
        std::cerr << find_args;
        exit(0);
    }
    std::cout << find_args.get("directory") << std::endl;
    std::cout << find_args << std::endl;

    auto a = find_args.get<bool>("-xdev");
    std::cout << a << std::endl;
    auto b = find_args.get<bool>("-follow");
    std::cout << b << std::endl;

    return 0;
}