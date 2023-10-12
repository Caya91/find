#include "argparse/argparse.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    //initialise argument parser
    argparse::ArgumentParser program("program_name");
    //adding arguments to the parser
    program.add_argument("-type")
        .help("f fuer file | d fuer directory");
    program.add_argument("-name")
        .default_value("*")
        .help("Akzeptiert wildcards");
    program.add_argument("-xdev")
        .default_value(false)
        .help("Nur auf dem aktuellen Dateisystem suchen");
    program.add_argument("-follow")
        .default_value(false)
        .help("Symbolic links folgen");
// hier noch .remaining einfügen um überschüssige Argumente zu speichern


    std::cout << program;

    return 0;
}
