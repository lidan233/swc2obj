#include <iostream>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <omp.h>


#include "SWCReader.h"
#include "cmdline.h"
#include "BVH.h"
#include "config.h"

std::vector< Path > searchPath(std::vector<Vertex>& point_vector,std::map<int, int>& vertex_hash)
{
	std::vector<int> leaf_nodes;
//	std::map<int, int> vertex_hash;
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
//		vertex.radius /= 0.5 ;

		max_x = std::max(max_x, vertex.x+RADIUS*vertex.radius);
		max_y = std::max(max_y, vertex.y+RADIUS*vertex.radius);
		max_z = std::max(max_z, vertex.z+RADIUS*vertex.radius);
		min_x = std::min(min_x, vertex.x-RADIUS*vertex.radius);
		min_y = std::min(min_y, vertex.y-RADIUS*vertex.radius);
		min_z = std::min(min_z, vertex.z-RADIUS*vertex.radius);
	}

	std::cout << "Bounding box : [" << min_x << ", " << min_y << ", " << min_z << "] --- [" << max_x << ", " << max_y << ", " << max_z << "]" << std::endl;

	std::cout << "Dimension x : " << (max_x - min_x) << std::endl;
	std::cout << "Dimension y : " << (max_y - min_y) << std::endl;
	std::cout << "Dimension z : " << (max_z - min_z) << std::endl;

	start_position = { min_x-1, min_y-1, min_z-1};
	dimension = {int(max_x - min_x + 2), int(max_y - min_y + 2), int(max_z - min_z + 2) };
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

void average1(std::vector<float>* mask_vector, glm::dvec3 dimension)
{
    int offset[8][3] = { {0, 0, 0}, {0, 0, 1}, {0 ,1, 0}, {0, 1, 1},
                         {1, 0, 0}, {1, 0, 1}, {1 ,1, 0}, {1, 1, 1}};
//    std::cout<<glm::to_string(glm::dvec3(dimension))<<std::endl ;
//    std::cout<<mask_vector->size()<<std::endl ;
    for(auto k = 0;k< dimension.z-1; k++)
    {
        for(auto j=0;j<dimension.y-1; j++)
        {
            for(auto i=0; i<dimension.x-1; i++)
            {
                auto sum = 0.0;
                for (auto& p : offset)
                {
                    sum += (*mask_vector)[(p[0] + k) * int(dimension.x) * int(dimension.y) + (p[1] + j) * int(dimension.x) + (p[2] + i)];
                }
                (*mask_vector)[k * int(dimension.x) * int(dimension.y) + j * int(dimension.x) + i] = sum/8;
            }
        }
        std::cout << "Process for averaging mask vector : " << k << " in " << dimension.z - 1 << std::endl;
    }
}




void averageAll(BVHNode* root)
{
    if(root->id>=0 && root->isUsingNode()) average1(root->getData(),root->getDimension()) ;
    else{
        if(root->children[0]!= nullptr) averageAll(root->children[0]) ;
        if(root->children[1]!= nullptr) averageAll(root->children[1]) ;
    }
}

void change(BVHNode* node)
{
    int padding = 4 ;
    glm::dvec3 dim = node->getDimension() ;
    glm::dvec3 newdim = node->getDimension() - glm::dvec3(padding,padding,padding) ;
    std::vector<float>* data = node->getData() ;
    std::vector<float>* t = new std::vector<float>(int(newdim[0]*newdim[1]*newdim[2])) ;

    for(int i = 0 ; i< newdim[2] ; i++)
    {
        for(int j = 0; j < newdim[1] ; j++)
        {
            for(int z = 0; z< newdim[0] ; z++)
            {
                (*t)[i*int(newdim[1])*int(newdim[0])+j*int(newdim[0])+z] =
                        (*data)[(i+2)*int(dim[1])*int(dim[0])+(j+2)*int(dim[0])+(z+2) ];
            }
        }
    }
    std::cout<<"new dimension is"<<glm::to_string(newdim)<<std::endl ;

    glm::dvec3 start = node->getStart() + glm::dvec3(padding/2,padding/2,padding/2) ;
    VoxelBox* box = new VoxelBox(start, start+newdim) ;
    box->data = t ;

    node->setNewBox(box) ;
}
void changeAll(BVHNode* root)
{
    if(root->id>=0 && root->isUsingNode()) change(root) ;
    else{
        if(root->children[0]!= nullptr) changeAll(root->children[0]) ;
        if(root->children[1]!= nullptr) changeAll(root->children[1]) ;
    }
}



void writeMHD(std::string file_name, int NDims, glm::dvec3 DimSize, glm::dvec3 ElementSize, glm::dvec3 ElementSpacing, glm::dvec3 Position, bool ElementByteOrderMSB, std::string ElementDataFile)
{
	std::ofstream writer(file_name.c_str());
	writer << "NDims = " << NDims << std::endl;
	writer << "DimSize = " << DimSize.x << " " << DimSize.y << " " << DimSize.z << std::endl;
	writer << "ElementSize = " << ElementSize.x << " " << ElementSize.y << " " << ElementSize.z << std::endl;
	writer << "ElementSpacing = " << ElementSpacing.x << " " << ElementSpacing.y << " " << ElementSpacing.z << std::endl;
	writer << "ElementType = " << "MET_DOUBLE" << std::endl;
	writer << "Position = " << Position.x << " " << Position.y << " " << Position.z << std::endl;
	if (ElementByteOrderMSB)
		writer << "ElementByteOrderMSB = " << "True" << std::endl;
	else
		writer << "ElementByteOrderMSB = " << "False" << std::endl;
	writer << "ElementDataFile = " << ElementDataFile << std::endl;
	std::cout << "File " << ElementDataFile << " has been saved." << std::endl;
}



void write(std::vector<float>& mask_vector,std::string rawname,std::string mhdname, glm::dvec3 dimension,glm::dvec3 start_point)
{
    std::ofstream outFile(rawname, std::ios::out | std::ios::binary);
    double t = 0.0 ;
    for (int i = 0; i < mask_vector.size(); i++) {
        t = mask_vector[i] ;
        outFile.write(reinterpret_cast<char*>(&t), sizeof(double));
    }
    std::cout<<"mask_vector"<<mask_vector.size()<<std::endl ;
    outFile.close();
//    start_point[0] /= 0.32 ;
//    start_point[1] /= 0.32 ;
//    start_point[2] /= 2 ;

    std::cout << "File has been saved." << std::endl;
    writeMHD(mhdname, 3, dimension, { 1, 1, 1 }, { 0.5, 0.5, 0.5 }, start_point/double(2.0), false, rawname);
//    writeMHD(mhdname, 3, dimension, { 1, 1, 1 }, { 1.0,1.0,1.0 }, start_point, false, rawname);
}

int filecount = 0 ;
void writeAll(BVHNode* root,std::string& path)
{
    if(root->id>=0 && root->isUsingNode())
    {
        std::string name = path +std::to_string(filecount++)+"_"+
                std::to_string(int(root->box->getStart()[0]))+"_"+
                std::to_string(int(root->box->getStart()[1]))+"_"+
                std::to_string(int(root->box->getStart()[2]))+".";
        std::string rawname = name+"raw" ;
        std::string mhdname = name+"mhd" ;
        write(*root->getData(),rawname,mhdname,root->getDimension(),root->box->getStart()) ;
    }else{
        if(root->children[0]!= nullptr) writeAll(root->children[0],path) ;
        if(root->children[1]!= nullptr) writeAll(root->children[1],path) ;
    }
}




int getDetph(std::vector< Path >& paths,std::vector<Vertex>& point_vector,std::map<int, int>& vertex_hash)
{
    int maxid = -1 ;
    double max_degree = -1 ;

    for (auto& path : paths) {
        for (int i = 0; i < path.path.size(); i++){
            if(path.path[i].degree>max_degree)
            {
                max_degree = path.path[i].degree ;
                maxid = path.path[i].current_id ;
            }
        }
    }


    std::cout<<"get maxid is "<<maxid <<std::endl ;
    int id = maxid ;
    int countdis = 0 ;
    while(id != -1)
    {
        point_vector[vertex_hash[id]].depth = countdis++ ;
        double radius1 = point_vector[vertex_hash[id]].radius ;
        int degree1 = point_vector[vertex_hash[id]].degree ;
        id = point_vector[vertex_hash[id]].previous_id ;

        if(id!=-1 && point_vector[vertex_hash[id]].radius>radius1)
        {
            std::cout<<id<<" a change to "<<radius1<<std::endl ;
            point_vector[vertex_hash[id]].radius = radius1 ;
        }
    }

    std::stack<int> point_vector_index ;
    for(int i = 0 ; i < paths.size() ; i++)
    {
        point_vector_index.push(paths[i].path[0].current_id) ;
    }

    while(point_vector_index.size()!=0)
    {
        int id = point_vector_index.top() ;
        int next_id = point_vector[vertex_hash[id]].previous_id ;
        if(next_id == -1)
        {
            point_vector[vertex_hash[id]].depth = -2 ;
            point_vector_index.pop() ;
            continue;
        }
        int depth_next = point_vector[vertex_hash[next_id]].depth ;
        double radius_next = point_vector[vertex_hash[next_id]].radius ;
        int degree_next = point_vector[vertex_hash[next_id]].degree ;

        if(radius_next==0.0)
        {
            std::cout<<point_vector[vertex_hash[next_id]].current_id ;
        }

        if(depth_next ==-2)
        {
            point_vector[vertex_hash[id]].depth = -2 ;
            point_vector_index.pop() ;
            continue;
        }else if(depth_next >=0 )
        {
            point_vector[vertex_hash[id]].depth = depth_next+1 ;
            if( point_vector[vertex_hash[id]].radius >= radius_next)
            {
                std::cout<<id<<" radius "<<point_vector[vertex_hash[id]].radius<<" change to "<<radius_next<<std::endl ;
                point_vector[vertex_hash[id]].radius = radius_next ;
            }

            point_vector_index.pop() ;
            continue ;
        }else if(depth_next==-1)
        {
            point_vector_index.push(next_id) ;
            continue;
        }
    }

    for (int j = 0 ; j < paths.size(); j++) {
        for (int i = 0; i < paths[j].path.size(); i++){
            paths[j].path[i].radius = point_vector[vertex_hash[paths[j].path[i].current_id]].radius ;
        }
    }
    std::cout<<" get depth end"<<std::endl ;
    return maxid ;

}

int swc2vol(std::string swc_file, int blocksize, std::string volume_dir)
{
    std::cout<<"we has "<<processor_N<<" processor"<<std::endl ;

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
    std::map<int, int> vertex_hash ;


	std::cout << "swc file " + swc_file + " has been loaded." << std::endl;
	std::cout << point_vector.size() << std::endl;

	

	Vec3D<double> start_point{};
	Vec3D<int> dimension{};
    boundingBox(point_vector, x_space, y_space, z_space, start_point, dimension);
	BVH* bvh = new BVH( start_point.to_i32vec3(),
	                    start_point.to_i32vec3()+dimension.to_i32vec3(),
	                    glm::dvec3(blocksize,blocksize,blocksize)) ;

	auto paths = searchPath(point_vector,vertex_hash);
	std::cout << paths.size() << std::endl;


	for (auto& path : paths)
	{
		for (int i = 0; i < path.path.size(); i++)
		{

		    if(path.path[i].radius<0.01)
            {
                float sum = 0 ;
                int begin = i ;
                int count = 0 ;

                while(count<1)
                {
                    if(path.path[begin].radius>0.01 ){
                        sum += path.path[begin].radius ;
                        count++ ;
                    }
                    begin-- ;
                    if(begin<0)
                    {
                        if(count<1)
                        {
                            sum += 0.1 ;
                            count++ ;
                        }
                        break ;
                    }
                }
                begin = i ;
                while(count<2)
                {
                    if(path.path[begin].radius>0.01 ){
                        sum += path.path[begin].radius ;
                        count++ ;
                    }

                    begin-- ;
                    if(begin>=path.path.size())
                    {
                        if(count<2)
                        {
                            sum += 0.1 ;
                            count++ ;
                        }
                        break ;
                    }
                }

                path.path[i].radius = sum / count ;
                std::cout<<point_vector[vertex_hash[path.path[i].current_id]].radius<<" change to " <<path.path[i].radius<<std::endl ;
                point_vector[vertex_hash[path.path[i].current_id]].radius = path.path[i].radius ;

            }
		}
	}

	for(int i = 0 ; i < point_vector.size(); i++)
    {
	    if(point_vector[i].radius<=0.0)
        {
	        std::cout<<"shit happened"<<std::endl ;
        }
    }

    int maxid = getDetph(paths,point_vector,vertex_hash) ;



//	start_point *= space;

	std::cout << "Start Points : " << start_point << std::endl;
	std::cout << "Dimension: " << dimension << std::endl;
	
	double step = 0.0001;
	int window_offset = 0;

	#pragma omp parallel for num_threads(24)
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

            Vec3D<double> startP = (start_point) ;
            spheres.push_back(getSphere(startP,path.path[i-1].radius)) ;
			for(int t=1;t<vec_length/step;t++)
			{
				auto cur_point = direction*(t*step) + start_point;
                Vec3D<double> startP = (cur_point) ;
                spheres.push_back(getSphere(startP,r * (t * step) / vec_length + path.path[i - 1].radius)) ;
			}
			startP = end_point  ;
			spheres.push_back(getSphere(startP,path.path[i].radius) ) ;

            std::vector<BVHNode*> nodes ;
			for(auto p=0;p<spheres.size();p++)
			{
			    nodes.clear() ;
			    bvh->getInteract(nodes,*spheres[p]) ;

			    for(auto begin = nodes.begin() ; begin != nodes.end() ; begin++)
                {
			        BVHNode* node = *begin ;
#pragma omp critical
			        {
                        node->usingNode() ;
                        node->setData(spheres[p]) ;
                    };

                }

			}

			std::cout << "Process : " << j << " in " << paths.size() << ", " << i << " in " << paths[j].path.size() << std::endl;
        }
	}
    averageAll(bvh->getRoot());
    averageAll(bvh->getRoot());
    changeAll(bvh->getRoot()) ;
    bvh->check() ;
//    std::string path = std::string(CONDIF_DIR)+"Result/" ;
	writeAll(bvh->getRoot(),volume_dir) ;
	delete bvh ;
	return 0;
}

//int main()
//{
//    swc2vol() ;
//}