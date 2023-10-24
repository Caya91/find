//
// Created by carnein on 10/24/23.
//
#include <sys/stat.h>
#include "argparse/argparse.hpp"

std::string path1 = "/home/carnein/find/find";
std::string path2 = "~/find";
std::string path3 = "directory";



int main(int argc, char * argv[]){

    struct stat statbuf;
    stat(path1.c_str(), &statbuf);
    std::cout <<" Pfad mit " + path1 + " "<< statbuf.st_dev << std::endl;
    stat(path2.c_str(), &statbuf);
    std::cout <<" Pfad mit " +path2 + " "<< statbuf.st_dev << std::endl;
    stat(path3.c_str(), &statbuf);
    std::cout <<" Pfad mit " + path3 + " "<< statbuf.st_dev << std::endl;
    return 0;
    //int stat(const char *restrict pathname,
    //        struct stat *restrict statbuf);

}