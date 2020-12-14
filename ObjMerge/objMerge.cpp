//
// Created by lidan on 5/12/2020.
//
#include <windows.h>

#include "cxxopts.hpp"
#include "ObjMerger.h"



void read_directory(const std::string& name, std::vector<std::string>& v)
{
    std::string pattern(name);
    pattern.append("*");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            if(data.cFileName[0]!='.')
            {
                v.push_back((name+data.cFileName));
                std::cout<<"add file: "<<data.cFileName<<std::endl ;
            }
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}

bool dirExists(const std::string& dirName_in) {
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}


void parse(char** argv,int argc,cxxopts::ParseResult& result)
{
    try {
        cxxopts::Options options(argv[0], " - example command line options");
        options
                .positional_help("[optional args]")
                .show_positional_help();

        options
                .allow_unrecognised_options()
                .add_options()
                        ("i,inputobj", "input obj is your input", cxxopts::value<std::string>())
                        ("o,outputobj", "objdir you want to output ", cxxopts::value<std::string>()) ;


        result = options.parse(argc, argv);

        if (result.count("i")) {
            std::cout << "input_obj = " << result["i"].as<std::string>()
                      << std::endl;
        }

        if (result.count("o")) {
            std::cout << "output_obj = " << result["o"].as<std::string>()
                      << std::endl;
        }



        std::cout << "Arguments remain = " << argc << std::endl;

        auto arguments = result.arguments();
        std::cout << "Saw " << arguments.size() << " arguments" << std::endl;
    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

}



int main(int argc, char** argv)
{
    cxxopts::ParseResult res ;
    parse(argv,argc,res) ;


    std::string inputobj ;
    if (res.count("i")) {
        inputobj =  res["i"].as<std::string>() ;
    }


    std::string outputobj ;
    if (res.count("o")) {
        outputobj = res["o"].as<std::string>() ;
    }

    std::string outputobj1 = outputobj.substr(0,outputobj.size()-4)+"1.obj";
    ObjMerger merger(inputobj) ;
    std::cout<<"write one obj"<<outputobj<<std::endl ;
    merger.writeNew(outputobj) ;
    std::cout<<"write one obj1"<<outputobj1<<std::endl ;
    merger.writeNew1(outputobj1) ;

    return 0 ;
}

//WinExec("instsrv GameManagerServer C:\\Windows\\System32\\srvany.exe",SW_NORMAL);