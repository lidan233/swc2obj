#include <iostream>
#include <fstream>
#include <glm/gtx/string_cast.hpp>

#include "SWCReader.h"
#include "cmdline.h"
#include "BVH.h"
#include "config.h"
std::vector< Path > searchPath(std::vector<Vertex>& point_vector)
{
	std::vector<int> leaf_nodes;
	std::map<int, int> vertex_hash;
	for (auto i = 0; i < point_vector.size(); i++)
	{
		vertex_hash[point_vector[i].current_id] = i;
	}


	for (auto& i : point_vector)
	{
		if (i.previous_id != -1)
		{
			point_vector[vertex_hash[i.previous_id]].degree++;
		}
	}


	for (auto& i : point_vector)
	{
		if (i.degree == 0 && i.previous_id != -1)
		{
			leaf_nodes.push_back(i.current_id);
		}
	}


	std::vector< Path > paths;
	auto path_index = 0;
	for (auto& left_node : leaf_nodes)
	{
		Path path;
		std::vector<Vertex> vertices;

		auto cur_idx = left_node;
		while (cur_idx != -1 && !point_vector[vertex_hash[cur_idx]].is_visited)
		{
			auto& buf_vertex = point_vector[vertex_hash[cur_idx]];
			buf_vertex.is_visited = true;
			vertices.push_back(buf_vertex);
			cur_idx = buf_vertex.previous_id;
		}

		if (cur_idx != -1)
		{
			vertices.push_back(point_vector[vertex_hash[cur_idx]]);
		}

		path.path = vertices;
		path.path_type = path_index;
		path_index++;
		paths.push_back(path);
	}
	return paths;
}


void boundingBox(std::vector<Vertex>& point_vector,
                 double x_space, double y_space, double z_space,
                 Vec3D<double>& start_position,
                 Vec3D<int>& dimension)
{
	
	double max_x = -999999, min_x = 999999;
	double max_y = -999999, min_y = 999999;
	double max_z = -999999, min_z = 999999;

	for (auto& vertex : point_vector)
	{

		//write_file << "v " << (vertex.x) / x_space << " " << (vertex.y) / y_space << " " << (vertex.z) / z_space << std::endl;
		//vertex.x /= x_space;
		//vertex.y /= y_space;
		//vertex.z /= z_space;
		vertex.x /= 0.5;
		vertex.y /= 0.5;
		vertex.z /= 0.5;

		max_x = std::max(max_x, vertex.x);
		max_y = std::max(max_y, vertex.y);
		max_z = std::max(max_z, vertex.z);
		min_x = std::min(min_x, vertex.x);
		min_y = std::min(min_y, vertex.y);
		min_z = std::min(min_z, vertex.z);
	}

	std::cout << "Bounding box : [" << min_x << ", " << min_y << ", " << min_z << "] --- [" << max_x << ", " << max_y << ", " << max_z << "]" << std::endl;

	std::cout << "Dimension x : " << (max_x - min_x) << std::endl;
	std::cout << "Dimension y : " << (max_y - min_y) << std::endl;
	std::cout << "Dimension z : " << (max_z - min_z) << std::endl;

	start_position = { min_x, min_y, min_z};
	dimension = {int(max_x - min_x + 1), int(max_y - min_y + 1), int(max_z - min_z + 1) };
}


#include <math.h>
void getSphereBoundingBox(Vec3D<double>& center_point, double radius, Vec3D<int> &low_bounding_int, Vec3D<int> &high_bounding_int)
{
	auto low_bounding = center_point - radius;
	low_bounding_int = { int(center_point.x - radius), int(center_point.y - radius), int(center_point.z - radius) };
	high_bounding_int = { int(ceil(center_point.x + radius)+0.1), int(ceil(center_point.y + radius)+0.1), int (ceil(center_point.z + radius)+0.1) };
}

Sphere* getSphere(Vec3D<double>& center_point, double radius)
{
    Sphere* ss = new Sphere(radius, center_point.to_vec3()) ;
    return ss ;
}


void average(std::vector<float>& mask_vector, glm::vec3 dimension)
{
	int offset[8][3] = { {0, 0, 0}, {0, 0, 1}, {0 ,1, 0}, {0, 1, 1},
							{1, 0, 0}, {1, 0, 1}, {1 ,1, 0}, {1, 1, 1}};
    std::cout<<dimension[0]<<" "<<dimension[1]<<" "<<dimension[2]<<std::endl ;
	for(auto i = 0 ; i< dimension[0]-1;i++)
    {
	    for(auto j = 0 ; j< dimension[1]-1;j++)
        {
	        for(auto z = 0 ; z< dimension[2]-1;z++)
            {
                auto sum = 0.0;
                for (auto& p : offset)
                {
                    int id = (p[0]+i)*dimension[1]*dimension[2]
                             + (p[1] + j)*dimension[2] + (p[2]+z) ;
//                    std::cout<<"access "<<id<<std::endl ;
                    sum += mask_vector[id] ;
                }
                mask_vector[i*dimension[1]*dimension[2]+j*dimension[2]+z] = sum/8 ;
            }
        }
        std::cout << "Process for averaging mask vector : " << i << " in " << dimension[0] - 1 << std::endl;
    }
}



void averageAll(BVHNode* root)
{
    if(root->id>=0 && root->isUsingNode()) average(*root->getData(),root->getDimension()) ;
    else{
        if(root->children[0]) averageAll(root->children[0]) ;
        if(root->children[1]) averageAll(root->children[1]) ;
    }
}


void writeMHD(std::string file_name, int NDims, glm::vec3 DimSize, glm::vec3 ElementSize, glm::vec3 ElementSpacing, glm::vec3 Position, bool ElementByteOrderMSB, std::string ElementDataFile)
{
	std::ofstream writer(file_name.c_str());
	writer << "NDims = " << NDims << std::endl;
	writer << "DimSize = " << DimSize.x << " " << DimSize.y << " " << DimSize.z << std::endl;
	writer << "ElementSize = " << ElementSize.x << " " << ElementSize.y << " " << ElementSize.z << std::endl;
	writer << "ElementSpacing = " << ElementSpacing.x << " " << ElementSpacing.y << " " << ElementSpacing.z << std::endl;
	writer << "ElementType = " << "MET_FLOAT" << std::endl;
	writer << "Position = " << Position.x << " " << Position.y << " " << Position.z << std::endl;
	if (ElementByteOrderMSB)
		writer << "ElementByteOrderMSB = " << "True" << std::endl;
	else
		writer << "ElementByteOrderMSB = " << "False" << std::endl;
	writer << "ElementDataFile = " << ElementDataFile << std::endl;
	std::cout << "File " << ElementDataFile << " has been saved." << std::endl;
}



void write(std::vector<float>& mask_vector,std::string rawname,std::string mhdname, glm::vec3 dimension,glm::vec3 start_point)
{
    std::ofstream outFile(rawname, std::ios::out | std::ios::binary);
    for (int i = 0; i < mask_vector.size(); i++) {

        outFile.write(reinterpret_cast<char*>(&mask_vector[i]), sizeof(float));
    }
    outFile.close();
    std::cout << "File has been saved." << std::endl;
    writeMHD(mhdname, 3, dimension, { 1, 1, 1 }, { 0.5, 0.5, 0.5 }, start_point, false, rawname);

}

void writeAll(BVHNode* root,std::string& path)
{
    if(root->id>=0 && root->isUsingNode())
    {
        std::string name = path + glm::to_string(root->box->getStart())+"_"+glm::to_string(root->box->getDimension())+".";
        std::string rawname = name+"raw" ;
        std::string mhdname = name+"mhd" ;
        write(*root->getData(),rawname,mhdname,root->getDimension(),root->box->getStart()) ;
    }else{
        if(root->children[0]!= nullptr) writeAll(root->children[0],path) ;
        if(root->children[1]!= nullptr) writeAll(root->children[1],path) ;
    }
}

int swc2vol()
{
	std::string based_path = "C:\\Users\\lidan\\Desktop\\brain\\14193_30neurons\\";
	std::string swc_path = "N030.swc";
	std::string swc_file = based_path + swc_path;

	
	std::string raw_file_name = swc_path.substr(0, swc_path.size() - 3) + "raw";
	std::string mhd_file_name = swc_path.substr(0, swc_path.size() - 3) + "mhd";
	int x_dimension = 28452;
	int y_dimension = 21866;
	int z_dimension = 4834;

	double x_space = 0.32;
	double y_space = 0.32;
	double z_space = 2;

	//int x_n = 889;
	//int y_n = 683;
	//int z_n = 151;

	SWCReader swc_reader;
	swc_reader.readSWC(swc_file);

	auto point_vector = swc_reader.getPointVector();

	std::cout << "swc file " + swc_file + " has been loaded." << std::endl;
	std::cout << point_vector.size() << std::endl;

	

	Vec3D<double> start_point{};
	Vec3D<int> dimension{};
    boundingBox(point_vector, x_space, y_space, z_space, start_point, dimension);
	BVH* bvh = new BVH( start_point.to_i32vec3(),
	                    start_point.to_i32vec3()+dimension.to_i32vec3(),
	                    glm::vec3(64.0,64.0,64.0)) ;
    Vec3D<double> start_point_need = start_point ;



	Vec3D<double> space{x_space, y_space, z_space};

	auto paths = searchPath(point_vector);
	std::cout << paths.size() << std::endl;


	for (auto& path : paths)
	{
		for (int i = 0; i < path.path.size(); i++)
		{
			path.path[i].x -= start_point.x;
			path.path[i].y -= start_point.y;
			path.path[i].z -= start_point.z;
			if (path.path[i].radius < 0.01) path.path[i].radius = 0.1;
		}
	}


	start_point *= space;

	std::cout << "Start Points : " << start_point << std::endl;
	std::cout << "Dimension: " << dimension << std::endl;
	
	double step = 0.01;
	int window_offset = 0;
	for(int j = 0;j< paths.size();j++)
	{
		const auto path = paths[j];
		for(int i=1; i < path.path.size(); i++)
		{
			Vec3D<double> start_point = { path.path[i - 1].x, path.path[i - 1].y, path.path[i - 1].z };
			Vec3D<double> end_point = {path.path[i].x, path.path[i].y, path.path[i].z };
			auto direction = (end_point - start_point).normalized();
			auto vec_length = (end_point - start_point).length();
			std::vector<Vec3D<double>> points;
			std::vector<double> radius;
			std::vector<Sphere*> spheres ;
			double r = path.path[i].radius - path.path[i - 1].radius;

			Vec3D<double> startP = (start_point+start_point_need) ;
            spheres.push_back(getSphere(startP,path.path[i-1].radius)) ;
			for(int t=1;t<vec_length/step;t++)
			{
				auto cur_point = direction*(t*step) + start_point;
                Vec3D<double> startP = (cur_point+start_point_need) ;
                spheres.push_back(getSphere(startP,r * (t * step) / vec_length + path.path[i - 1].radius)) ;
			}
			startP = end_point + start_point_need ;
			spheres.push_back(getSphere(startP,path.path[i].radius) ) ;


            std::vector<BVHNode*> nodes ;
			for(auto p=0;p<spheres.size();p++)
			{
			    nodes.clear() ;

			    bvh->getInteract(nodes,*spheres[p]) ;
			    for(auto begin = nodes.begin() ; begin != nodes.end() ; begin++)
                {
			        BVHNode* node = *begin ;
			        node->usingNode() ;
			        node->setData(spheres[p]) ;
                }

			}

			std::cout << "Process : " << j << " in " << paths.size() << ", " << i << " in " << paths[j].path.size() << std::endl;
		}
	}
    averageAll(bvh->getRoot());
    averageAll(bvh->getRoot());

    std::string path = std::string(CONDIF_DIR)+"Result/" ;
	writeAll(bvh->getRoot(),path) ;
	return 0;
}

int main()
{
    swc2vol() ;
}