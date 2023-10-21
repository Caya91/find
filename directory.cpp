#include <string>
#include <iostream>
#include <dirent.h>
#include<filesystem>
#include <set>
#include <fnmatch.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include "argparse/argparse.hpp"
#include <unistd.h>

char *root;
void bare(const std::string &path, const std::string& dirname);

int main (int argc, char *argv[])
{
//initialise argument parser
    argparse::ArgumentParser find_args("find");
    //adding arguments to the parser
    find_args.add_argument("directory").default_value(".").required()
            .help("Verzeichnis in dem gesucht werden soll");

    // argumente parsen

    try{

        find_args.parse_args(argc, argv);
    } catch (const std::runtime_error& err){
        std::cerr << err.what() << std::endl;
        std::cerr << find_args;
        exit(0);
    }

    auto dirname = find_args.get<std::string>("directory");
    std::cout <<"directory dessen Pfade durchsucht werden sollen: "<< dirname << std::endl;

    std::string cute ="cute";
    if (cute.ends_with("/")){
        printf("cute");
    }


    // holt sich das aktuelle working directory
    root = getcwd(nullptr, 0);
    std::cout << "aktuelles Directory " << root << std::endl;


    DIR *directory = opendir(dirname.c_str());
    std::cout << directory << std::endl;
    if (directory == nullptr) {
        std::cout << "directory not found" << std::endl;
        exit(0);
    }
    else{
        std::cout << "directory found" << std::endl;
        std::cout << root << "/" << dirname << std::endl;
    }

    bare(root, dirname);


    return 0;

}

void bare(const std::string &path,const std::string& dirname){
    DIR *directory = opendir(path.c_str());
    while (dirent * entry = readdir(directory)) {
        if (strcmp(entry->d_name ,".")==0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        else if (entry->d_type == DT_DIR) {
            std::cout << "directory: "<< entry->d_name << std::endl;
            std::cout << "suchen in Pfad: " << path + "/" + entry->d_name << std::endl;
            bare(std::string(path + "/" + entry->d_name), dirname);

        } else if (entry->d_type == DT_REG) {
            std::cout << "file: " << entry->d_name << std::endl;;
        } else {
            std::cout << "other: " <<  entry->d_name << std::endl;;
        }
    }


}