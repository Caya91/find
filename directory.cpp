#include <string>
#include <iostream>
#include <dirent.h>
#include <filesystem>
#include <set>
#include <fnmatch.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include "argparse/argparse.hpp"
#include <unistd.h>

// this file has the standard behaviour for find without any flags
// only directory which is necessary with default behaviour and inputs


namespace fs = std::filesystem;
std::string root;
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

    dirname.ends_with("/");


    // aktueller Pfad des working directory aktuelle working directory
    root = getcwd(nullptr, 0);


    DIR *directory = opendir(dirname.c_str());
    if (directory == nullptr) {
        //ist dirname kein Pfad sondern ein direkter Dateiname, printet es den Namen
        // aus und verlässt das if statement
        auto file_n =fs::status(root + "/" + dirname);
        if (is_regular_file(file_n)){
            std::cout << dirname << std::endl;
            exit(0);
        }
        std::cout << "find:" << "'" << dirname << "': "
        << "No such file or directory" << std::endl;
        exit(0);

    }
    else{
        std::cout << dirname << std::endl;

    }



    bare(dirname, dirname);



    return 0;

}

void bare(const std::string &path,const std::string& dirname){
    DIR *directory = opendir(path.c_str());
    while (dirent * entry = readdir(directory)) {
        if (strcmp(entry->d_name ,".")==0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        else if (entry->d_type == DT_DIR) {
            std::cout <<path + "/" + entry->d_name << std::endl;
            bare(std::string(path + "/" + entry->d_name), dirname);

        } else if (entry->d_type == DT_REG) {
            std::cout <<path + "/" + entry->d_name << std::endl;;
        } else {
            std::cout << "other: " <<  entry->d_name << std::endl;;
        }
    }


}