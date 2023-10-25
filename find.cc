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

namespace fs = std::filesystem;
static bool xdev, follow;
static dev_t dev;
enum Type{All=0, File=1, Directory=2};
Type arg_type;
//std::string root;

Type cast_Type(std::string type);
void bare(const std::string &path, const std::string& dirname, const std::string& name);

int main(int argc, char *argv[]) {
    //initialise argument parser
    argparse::ArgumentParser find_args("find");
    //adding arguments to the parser
    find_args.add_argument("directory").default_value(".").required()
        .help("Verzeichnis in dem gesucht werden soll");
    find_args.add_argument("-type")
        .default_value(std::string{"a"})
        .action([](const std::string& value){
            static const std::vector<std::string> choices = { "d", "f"};
            if (std::find(choices.begin(), choices.end(), value) !=choices.end()){
                return value;
            }
            std::cout <<"find: Unknown argument to -type: " << value << std::endl;
            exit(0); // bei unbekanntem Input für "-type" beenden
        })
        .help("f fuer file | d fuer directory");
    find_args.add_argument("-name")
        .default_value("*")
        .help("Akzeptiert wildcards");
    find_args.add_argument("-xdev")
        .implicit_value(true).default_value(false)
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



    // set global variables

    arg_type = cast_Type(find_args.get<std::string>("-type"));
    std::cout << arg_type << std::endl;
    auto name = find_args.get<std::string>("-name");
    follow = find_args.get<bool>("-follow");
    xdev = find_args.get<bool>("-xdev");

    if (follow){
        std::cout << "follow present" <<  std::endl;
    }else {
        std::cout << "follow not present" << std::endl;
    }


    //folgende line gibt generelle Information über Argumente zu dem Parser aus
    //std::cout << find_args << std::endl;

    // standard behaviour for find with no added flags
    auto dirname = find_args.get<std::string>("directory");

    // aktueller Pfad desd working directory aktuelle working directory
    //root = getcwd(nullptr, 0);

    std::string root = getcwd(nullptr, 0);
    DIR *directory = opendir(dirname.c_str());
    if (xdev){
        struct stat statbuf;
        stat(dirname.c_str(), &statbuf);
        dev = statbuf.st_dev;
    }
    if (directory == nullptr) {
        //ist dirname kein Pfad sondern ein direkter Dateiname, printet es den Namen
        // aus und verlässt das if statement

        auto file_n =fs::status(root + "/" + dirname);
        if ((is_regular_file(file_n) && ( arg_type == File ||arg_type ==  All))){
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

    bare(dirname, dirname, find_args.get<std::string>("-name"));

    return 0;
}

void bare(const std::string &path,const std::string& dirname, const std::string& name){
    DIR *directory = opendir(path.c_str());
    while (dirent * entry = readdir(directory)) {
        if (strcmp(entry->d_name ,".")==0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        else if (entry->d_type == DT_DIR ) {

            if ( (arg_type == Directory || arg_type == All)
            && (fnmatch(name.c_str(), entry->d_name,FNM_FILE_NAME)==0))
            {
                    std::cout << path + "/" + entry->d_name << std::endl;
            }
            if (xdev){
                struct stat barr_buf;
                stat(entry->d_name, &barr_buf);
                if (dev != barr_buf.st_dev) continue;       // falls xdev gesetzt ist, wird nur auf dem aktuellen
                                                            // Dateisystem gesucht
            }
            bare(std::string(path + "/" + entry->d_name), dirname, name);

        } else if (entry->d_type == DT_REG && (arg_type ==File || arg_type == All)
            && (fnmatch(name.c_str(), entry->d_name,FNM_FILE_NAME)==0) ){

            std::cout <<path + "/" + entry->d_name << std::endl;;
        } else { // vllt müssen noch andere Fälle abgedeckt werden
                // falls nicht alle files gefunden werden wie sonst im GNU find
            continue;
        }
    }
}

Type cast_Type(std::string const type){
    if(type.starts_with("d")){
        return Directory;
    }else if (type.starts_with("f")){
        return File;
    }else{
        return All;
    }
}