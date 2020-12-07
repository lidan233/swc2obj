//
// Created by lidan on 4/12/2020.
//

#include "ObjMerger.h"

ObjMerger::ObjMerger(std::string filename)
{
    std::ifstream file(filename.c_str(),std::ios::in) ;

    if (!file) {
        assert(false);    // Couldn't read file
    }

    std::string line ;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string method;

        iss >> method;

        if (method == "v")
        {
            double x, y, z;
            iss >> x >> y >> z;
            points.push_back(Point(points.size()+1,glm::dvec3(x,y,z))) ;
//            std::cout << "add vertices " << x << ", " << y << ", " << z<<std::endl ;
        }
        else if(method == "vn")
        {
            double x,y,z ;
            iss>>x>>y>>z ;
            vns.push_back(Point(vns.size()+1,glm::dvec3(x,y,z))) ;
//            std::cout << "add vertices normal " << x << ", " << y << ", " << z<<std::endl ;
        }
        else if (method == "f")
        {
            int a, b, c;
            char s;

            while ((iss >> a >> s >>s>> a>> b >>s >>s>>b>>c>>s>> s >> c) && s == '/')
            {
//                std::cout << "New triangle using vertices " << a << ", " << b << ", " << c;
                trias.push_back(Trias(trias.size(),glm::i32vec3(a,b,c))) ;

            }
        }
        else
        {
            std::cout << "Ignored line";
        }
    }

    std::cout<<"read file end"<<std::endl ;
    this->removerepeat() ;



}

void ObjMerger::writeOld(std::string filename)
{
    std::ofstream out(filename.c_str(),std::ios::out) ;
    for(int i =0 ; i < points.size(); i++)
    {
        out<<"v "<<points[i].poi[0]<<" "
           <<points[i].poi[1]<<" "
           <<points[i].poi[2]<<std::endl ;
    }

    for(int i =0 ; i < vns.size(); i++)
    {
        out<<"vn "<<vns[i].poi[0]<<" "
           <<vns[i].poi[1]<<" "
           <<vns[i].poi[2]<<std::endl ;
    }


    for(int i = 0 ; i < trias.size() ; i++)
    {
        out<<"f "<<trias[i].idxs[0]<<"//"<<trias[i].idxs[0]<<" "
           <<trias[i].idxs[1]<<"//"<<trias[i].idxs[1]<<" "
           <<trias[i].idxs[2]<<"//"<<trias[i].idxs[2]<<std::endl ;


    }


}

void ObjMerger::writeNew(std::string filename)
{
    std::ofstream out(filename.c_str(),std::ios::out) ;
    int* newid = new int[points.size()] ; ;
    int count = 1 ;

    for(int i =0 ; i < points.size();i++)
    {
        if(ids.find(points[i].id)==ids.end())
        {
            out<<"v "<<points[i].poi[0]<<" "
               <<points[i].poi[1]<<" "
               <<points[i].poi[2]<<std::endl ;

            out<<"vn "<<vns[i].poi[0]<<" "
               <<vns[i].poi[1]<<" "
               <<vns[i].poi[2]<<std::endl ;

            newid[i] = count++ ;
        }else{
            newid[i] = newid[ids[points[i].id]-1] ;
            std::cout<<"map "<<points[i].id<<" "<<newid[i]<<std::endl ;
        }

        if(newid[i]==0) std::cout<<"is zero"<<i <<std::endl ;
    }


    for(int i = 0 ; i < trias.size() ; i++)
    {
        out<<"f "<<newid[trias[i].idxs[0]-1]<<"//"<<newid[trias[i].idxs[0]-1]<<" "
                 <<newid[trias[i].idxs[1]-1]<<"//"<<newid[trias[i].idxs[1]-1]<<" "
                 <<newid[trias[i].idxs[2]-1]<<"//"<<newid[trias[i].idxs[2]-1]<<std::endl ;

        if(newid[trias[i].idxs[0]-1]>=count) std::cout<<"shit"<<trias[i].idxs[0]-1<<" "<<newid[trias[i].idxs[0]-1]<<std::endl ;
        if(newid[trias[i].idxs[1]-1]>=count) std::cout<<"shit"<<trias[i].idxs[1]-1<<" "<<newid[trias[i].idxs[1]-1]<<std::endl ;
        if(newid[trias[i].idxs[2]-1]>=count) std::cout<<"shit"<<trias[i].idxs[2]-1<<" "<<newid[trias[i].idxs[2]-1]<<std::endl ;

    }

    out.close() ;
    delete newid ;
}