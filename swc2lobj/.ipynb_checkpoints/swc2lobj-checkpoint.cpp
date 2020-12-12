    //
// Created by lidan on 8/12/2020.
//

#include <string>
#include <iostream>
#include <windows.h>

#include "cxxopts.hpp"
#include "cmdline.h"
#include "SWCReader.h"
#include "ObjWriter.h"


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

void parse(char** argv,int argc,cxxopts::ParseResult& result) {
    try {
        cxxopts::Options options(argv[0], " - example command line options");
        options
        .positional_help("[optional args]")
        .show_positional_help();

        options
        .allow_unrecognised_options()
        .add_options()
        ("i,inputswc", "input swc ", cxxopts::value<std::string>())
        ("o,outputobj", "obj path you want to output ", cxxopts::value<std::string>()) ;


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




int main(int argc,char** argv)
{
    std::string swc_file = R"(E:\14193_30neurons\N023.swc)";
    std::string obj_file = R"(E:\14193_30neurons\N023.obj)";


    int x_dimension = 28452;
    int y_dimension = 21866;
    int z_dimension = 4834;

    double x_space = 0.32;
    double y_space = 0.32;
    double z_space = 2;

    int x_n = 889;
    int y_n = 683;
    int z_n = 151;

    int is_normalized = false;

    if(argc<2)
    {
        std::cout << "Please use command line parameter." << std::endl;
        //return 0;
    }
    else
    {
        cmdline::parser a;
        a.add<std::string>("swc", 's', "Input swc file name", true, "");
        a.add<std::string>("obj",'o',"Output obj file name",true," " ) ;
        a.add<int>("x", 'X', "dimension of x direction", false, 28452);
        a.add<int>("y", 'Y', "dimension of y direction", false, 21866);
        a.add<int>("z", 'Z', "dimension of z direction", false, 4834);

        a.add<double>("sx", 'x', "space of x direction", false, 0.32);
        a.add<double>("sy", 'y', "space of y direction", false, 0.32);
        a.add<double>("sz", 'z', "space of z direction", false, 2);

        a.add<int>("nx", 'i', "new dimension of x direction", false, 889);
        a.add<int>("ny", 'j', "new dimension of y direction", false, 683);
        a.add<int>("nz", 'k', "new dimension of z direction", false, 151);

        a.add<int>("normalize", 'n', "whether normalize the obj file to [0,1] or not", false , 0);

        a.parse_check(argc, argv);
        swc_file = a.get<std::string>("swc");
        obj_file = a.get<std::string>("obj") ;
        x_dimension = a.get<int>("x");
        y_dimension = a.get<int>("y");
        z_dimension = a.get<int>("z");

        x_space = a.get<double>("sx");
        y_space = a.get<double>("sy");
        z_space = a.get<double>("sz");


        is_normalized = a.get<int>("normalize");

//        obj_file = swc_file.substr(0, swc_file.find_last_of('.')) + "Line.obj";
    }

    double x_bounding = x_dimension * x_space;
    double y_bounding = y_dimension * y_space;
    double z_bounding = z_dimension * z_space;


    SWCReader swc_reader;
    swc_reader.readSWC(swc_file);

    auto point_vector = swc_reader.getPointVector();

    std::cout << "swc file " +swc_file+ " has been loaded." << std::endl;

    ObjWriter obj_writer(point_vector);

    obj_writer.searchPath();

    if(is_normalized)
    {
        obj_writer.writeObjNormalization(obj_file, x_bounding/(x_n),
                                         y_bounding / (y_n), z_bounding / (z_n*z_space/x_space));
    }
    else
    {
        obj_writer.writeObj(obj_file,x_space,y_space,z_space);
    }
    auto is_normalized_to_one = false;
    if(is_normalized_to_one)
    {
        obj_writer.writeNormalizeToOneObj(obj_file, 6200, 6500, 2250, 512, 512, 512);
    }

    std::cout << "obj file" + obj_file + " has been generated." << std::endl;
}