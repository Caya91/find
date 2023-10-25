//
// Created by carnein on 10/25/23.
//
#include <sys/stat.h>
#include <iostream>

#define Log(x) std::cout << x << std::endl;

int main (int argc, char * argv[]){
    std::string path1 = "/home/carnein/find/link";
    std::string path2 = "~/find/link";
    std::string path3 = "link";
    std::string path4 = "Makefile";
    struct stat statbuf;

    lstat(path1.c_str(),&statbuf);
    Log(statbuf.st_dev);
    Log(statbuf.st_mode);
    if (S_ISLNK(statbuf.st_mode)){
        Log("is symlink");
    }
    lstat(path2.c_str(),&statbuf);
    Log(statbuf.st_dev);
    Log(statbuf.st_mode);
    if (S_ISLNK(statbuf.st_mode)){
        Log("is symlink");
    }
    lstat(path3.c_str(),&statbuf);
    Log(statbuf.st_dev);
    Log(statbuf.st_mode);
    if (S_ISLNK(statbuf.st_mode)){
        Log("is symlink");
    }
    lstat(path4.c_str(),&statbuf);
    Log(statbuf.st_dev);
    Log(statbuf.st_mode);
    if (S_ISLNK(statbuf.st_mode)){
        Log("is symlink");
    }
}