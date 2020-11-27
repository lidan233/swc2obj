//
// Created by 李源 on 2020-11-23.
//

#ifndef SWC2OBJ_BVH_H
#define SWC2OBJ_BVH_H

#include <glm/glm.hpp>
#include <vector>
#include <stack>
#include <assert.h>

#include "based.h"

struct BVHNode{
public:
    int id ;
    VoxelBox* box= nullptr;
    BVHNode* children[2] = {nullptr, nullptr};


    BVHNode(int nid,glm::vec3 pmin,glm::vec3 pmax){
        box = new VoxelBox(pmin,pmax) ;
        id = nid ;
    } ;
    BVHNode(int nid, VoxelBox* box_):id(nid),box(box_){}
    BVHNode(int nid,BVHNode* childrenL, BVHNode* childrenR) ;

    void usingNode() {
        assert(box!= nullptr) ;
        box->usingBox() ;
    }

    bool isUsingNode() {return box!= nullptr ;}
    std::vector<float>* getData() { return box->getBoxData() ; }
    void setData(Sphere* ss) ;
    glm::vec3 getDimension() { return box->getDimension() ;}

};


class BVH {
private:
    BVHNode* root = nullptr ;
public:
    BVH(glm::i32vec3 pmin, glm::i32vec3 pmax ,glm::vec3 voxelBox) ;
    void getInteract(std::vector<BVHNode*>& nodes, Sphere& ss ) ;
    BVHNode* getRoot() {return root ;}
};


#endif //SWC2OBJ_BVH_H
