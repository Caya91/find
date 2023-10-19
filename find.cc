#include "argparse/argparse.hpp"
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
char * root;

void bare_find(DIR *directory)
{
    struct dirent *entry;
    while ((entry = readdir(directory)) != nullptr) {
        if (entry->d_type == DT_DIR) {

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            //char *path = (char *) malloc(strlen(root) + strlen(entry->d_name) + 2);
            //strcpy(path, root);
            //strcat(path, "/");
            //strcat(path, entry->d_name);
            DIR *subdir = opendir(entry->d_name);
            bare_find(subdir);
            std::cout << "directory: " << entry->d_name << std::endl;
            //free(path);
        }
        else if (entry->d_type == DT_REG)
        {
            std::cout << entry->d_name << std::endl;
        }
    }

};


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

    //folgende line gibt generelle Information über Argumente zu dem Parser aus
    //std::cout << find_args << std::endl;

    // standard behaviour for find with no added flags
    //


    //auto a = find_args.get<bool>("-xdev");
    //std::cout << a << std::endl;
    //auto b = find_args.get<bool>("-follow");
    //std::cout << b << std::endl;
    auto dirname = find_args.get<std::string>("directory");
    std::cout <<"directory dessen Pfade durchsucht werden sollen: "<< dirname << std::endl;

    // holt sich das aktuelle working directory
    root = getcwd(nullptr, 0);
    std::cout << root << std::endl;

    DIR *directory = opendir(dirname.c_str());
    std::cout << directory << std::endl;
    if (directory == nullptr) {
        std::cout << "directory not found" << std::endl;
        exit(0);
    }

    bare_find(directory);


    return 0;
}