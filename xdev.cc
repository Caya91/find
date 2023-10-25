//
// Created by carnein on 10/24/23.
//
#include <sys/stat.h>
#include <iostream>

std::string path1 = "/home/carnein/find/find";
std::string path2 = "~/find";
std::string path3 = "directory";

#define Log(x) std::cout << x << std::endl;

int main(int argc, char * argv[]){

    struct stat statbuf;
    stat(path1.c_str(), &statbuf);
    std::cout <<" Pfad mit " + path1 + " "<< statbuf.st_dev << std::endl;
    stat(path2.c_str(), &statbuf);
    std::cout <<" Pfad mit " +path2 + " "<< statbuf.st_dev << std::endl;
    stat(path3.c_str(), &statbuf);
    std::cout <<" Pfad mit " + path3 + " "<< statbuf.st_dev << std::endl;
    Log(path3);

    return 0;
    //int stat(const char *restrict pathname,
    //        struct stat *restrict statbuf);

}