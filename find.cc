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

#define Log(x) std::cout << x << std::endl;

namespace fs = std::filesystem;
char * rootCwd;
static bool xdev, follow;
std::string callPath;
static dev_t dev;
enum Type{All=0, File=1, Directory=2};
Type arg_type;
//std::string root;

Type cast_Type(std::string type);
void bare(const std::string &path, const std::string& dirname, const std::string& name);

// TODO: change Fabi's functions
std::string realPath(const std::string& inputPath) {
    size_t lastSlash = inputPath.find_last_of('/');
    std::string parentDir;
    std::string symlinkName;
    if(lastSlash == std::string::npos) { //path does not contain /, so symlink is in current directory
        parentDir = std::string(getcwd(nullptr, 0));
        symlinkName = inputPath;
    } else {
        parentDir = inputPath.substr(0, lastSlash);
        symlinkName = inputPath.substr(lastSlash + 1);
    }


    char * fullParentPath = realpath(parentDir.c_str(), nullptr);
    return (std::string(fullParentPath) + "/" + symlinkName);
}

// TODO: change Fabi's functions
bool isSymlinkInLoop(const std::string& symlinkPathRelative, std::set<std::string>& visitedPaths) {
    auto symlinkPath = realPath(symlinkPathRelative);

    // Check if path was already visited -> This is a filesystem loop!
    if (visitedPaths.find(symlinkPath) != visitedPaths.end()) return true;

    // Insert the current path into visited paths
    visitedPaths.insert(symlinkPath);

    // Get information about the current path
    struct stat info{};
    if (lstat(symlinkPath.c_str(), &info) != 0) return false;

    if (S_ISLNK(info.st_mode)) {
        // It's a symlink -> check the target
        char * targetPath = realpath(symlinkPath.c_str(), nullptr);
        return isSymlinkInLoop(targetPath, visitedPaths);
    } else if(S_ISDIR(info.st_mode)) {
        // It's a directory -> check if it contains the/a symlink to itself.
        DIR *directory = opendir(symlinkPath.c_str());

        if (directory) {
            struct dirent *entry;

            // Loop over files in directory, skipping current (.) and parent directory (..)
            while ((entry = readdir(directory)) != nullptr) {
                if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                if (isSymlinkInLoop(symlinkPath + "/" + std::string(entry->d_name), visitedPaths)) return true;
            }
        }
    }

    return false; // No loops found.
}

std::string cleanPath(std::string dirPrefix){
    std::string tmp = dirPrefix;
    while(tmp.length() > 0 && tmp.substr(0, 3) == "../") {
        tmp = tmp.substr(3);
    }
    return "/" + tmp;

}


bool detect_loop(const std::string& path, std::set<std::string>& seen){
    if (seen.find(path) != seen.end()){
        return true;
    }
    seen.insert(path);
    struct stat statbuf;
    if (lstat(path.c_str(), &statbuf) == -1){
        return false;
    }
    if (!S_ISDIR(statbuf.st_mode)){
        return false;
    }
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr){
        return false;
    }
    bool loop = false;
    while (dirent* entry = readdir(dir)){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }
        std::string new_path = path + "/" + entry->d_name;
        if (detect_loop(new_path, seen)){
            loop = true;
            break;
        }
    }
    closedir(dir);
    seen.erase(path);
    if (loop){
        Log("LOOP FOUND:    REMOVE WHEN DONE")
    }
    return loop;
}


std::string calculateRelativePath(const std::string& absolutePath, const std::string& basePath) {
    std::vector<std::string> absoluteComponents;
    std::vector<std::string> baseComponents;

    // Split the absolute and base paths into individual components
    size_t pos;
    std::string remainingAbsolute = absolutePath;
    while ((pos = remainingAbsolute.find('/')) != std::string::npos) {
        absoluteComponents.push_back(remainingAbsolute.substr(0, pos));
        remainingAbsolute = remainingAbsolute.substr(pos + 1);
    }
    absoluteComponents.push_back(remainingAbsolute);

    std::string remainingBase = basePath;
    while ((pos = remainingBase.find('/')) != std::string::npos) {
        baseComponents.push_back(remainingBase.substr(0, pos));
        remainingBase = remainingBase.substr(pos + 1);
    }
    baseComponents.push_back(remainingBase);

    // Find the longest common prefix
    size_t commonCount = 0;
    while (commonCount < absoluteComponents.size() && commonCount < baseComponents.size() &&
           absoluteComponents[commonCount] == baseComponents[commonCount]) {
        commonCount++;
    }

    std::string relativePath;

    // Construct the relative path
    for (size_t i = commonCount; i < baseComponents.size(); i++) {
        relativePath += "../";
    }

    for (size_t i = commonCount; i < absoluteComponents.size(); i++) {
        relativePath += absoluteComponents[i] + "/";
    }

    // Remove the trailing slash if it exists
    if (!relativePath.empty() && relativePath.back() == '/') {
        relativePath.pop_back();
    }

    return relativePath;
}


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
        std::cout << err.what() << std::endl;
        std::cout << find_args;
        exit(0);
    }



    // set global variables
    callPath = "find";
    arg_type = cast_Type(find_args.get<std::string>("-type"));
    auto name = find_args.get<std::string>("-name");
    follow = find_args.get<bool>("-follow");
    xdev = find_args.get<bool>("-xdev");

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
    if(dirname.ends_with("/"))
        dirname = dirname.substr(0, dirname.length() - 1);

    rootCwd = getcwd(nullptr, 0);
    if(!dirname.starts_with("/")) {
        rootCwd = getcwd(nullptr, 0);
    }

    bare(dirname, dirname, find_args.get<std::string>("-name"));

    return 0;
}

void bare(const std::string &path,const std::string& dirname, const std::string& name){
    DIR *directory = opendir(path.c_str());
    while (dirent * entry = readdir(directory)) {
        // TODO: Fabis Code entfernen
        std::string curObject(entry->d_name);
        if(curObject == "." || curObject == "..") continue;

        std::string curObjPathWithoutPrefix(dirname);
        curObjPathWithoutPrefix += "/";
        curObjPathWithoutPrefix += curObject;
        std::string curObjPathWithPrefix(path);
        curObjPathWithPrefix += curObjPathWithoutPrefix;

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
                stat((path + "/" + entry->d_name).c_str(), &barr_buf);
                if (dev != barr_buf.st_dev) {continue;}       // falls xdev gesetzt ist, wird nur auf dem aktuellen
                                                            // Dateisystem gesucht
            }
            bare(std::string(path + "/" + entry->d_name), dirname, name);

        } else if (entry->d_type == DT_REG && (arg_type ==File || arg_type == All)
            && (fnmatch(name.c_str(), entry->d_name,FNM_FILE_NAME)==0) ){

            std::cout <<path + "/" + entry->d_name << std::endl;;
        } else if (entry->d_type == DT_LNK && follow){
            std::set<std::string> seen;
            //if (detect_loop(path, seen)){
            //    Log(path + " is part of filesstem loop")
            //    return;
            //}
            char *resolved_name = nullptr;
            char * c_target = realpath(curObjPathWithoutPrefix.c_str(), resolved_name);
            std::string target(c_target);
            
            
            if(isSymlinkInLoop(curObjPathWithoutPrefix, seen)) {
                std::cout << callPath << ": File system loop detected; '";
                if(rootCwd == nullptr) { // directory-to-search was absolute path, so print absolute paths
                    std::cout << realPath(curObjPathWithPrefix);
                    std::cout << "' is part of the same file system loop as '";
                    std::cout << target << "'." << std::endl;
                } else { // directory-to-search was relative, so print relative paths.
                    std::cout << path + entry->d_name;
                    std::cout << "' is part of the same file system loop as '";
                    std::string leading_dirt = calculateRelativePath(target, rootCwd);

                    std::cout << cleanPath(leading_dirt) << "'." << std::endl;
                }

                continue;
            }


            struct stat statbuf;
            stat((path + "/" + entry->d_name).c_str(), &statbuf);
            if (S_ISDIR(statbuf.st_mode)){
                if ( (arg_type == Directory || arg_type == All)
                     && (fnmatch(name.c_str(), entry->d_name,FNM_FILE_NAME)==0))
                {
                    std::cout << path + "/" + entry->d_name << std::endl;
                }
                if (xdev){
                    struct stat barr_buf;
                    stat((path + "/" + entry->d_name).c_str(), &barr_buf);
                    if (dev != barr_buf.st_dev) {continue;}       // falls xdev gesetzt ist, wird nur auf dem aktuellen
                                                                    // Dateisystem gesucht
                }
                //Log(entry->d_name)
                bare(std::string(path + "/" + entry->d_name), dirname, name);
            } else if (S_ISREG(statbuf.st_mode) && (arg_type ==File || arg_type == All)
                       && (fnmatch(name.c_str(), entry->d_name,FNM_FILE_NAME)==0) ){
                //Log(entry->d_name)
                std::cout <<path + "/" + entry->d_name << std::endl;;
            }

        } else {
            Log("Else - Zweig    REMOVE WHEN DONE")
            Log(path + "/" + entry->d_name );

            // vllt müssen noch andere Fälle abgedeckt werden
                // falls nicht alle files gefunden werden wie sonst im GNU find
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