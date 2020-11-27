//
// Created by 李源 on 2020-11-23.
//

#include "BVH.h"
#include "Geo_Util.h"


std::vector<int> argsort(const glm::vec3 array)
{
    const int array_len(3);
    std::vector<int> array_index(array_len, 0);
    for (int i = 0; i < array_len; ++i)
        array_index[i] = i;

    std::sort(array_index.begin(), array_index.end(),
              [&array](int pos1, int pos2) {return (array[pos1] < array[pos2]);});

    return array_index;
}



BVHNode::BVHNode(int nid,BVHNode* childrenL, BVHNode* childrenR)
{
    this->id = nid ;
    this->children[0] = childrenL ;
    this->children[1] = childrenR ;
    this->box = new VoxelBox() ;
    this->box->merge(*(childrenL->box) ) ;
    this->box->merge(*(childrenR->box) ) ;
}


void BVHNode::setData(Sphere* ss)
{
    VoxelBox* ss_box = ss->getBoundBox() ;
    VoxelBox* t = ss_box->interact(this->box) ;
    int count = 0 ;
    if(t != nullptr)
    {
        glm::i32vec3 boxdimension = this->getDimension() ;
        glm::i32vec3 dimension = (t->pmax - t->pmin + glm::vec3(0.5,0.5,0.5)) ;
        std::vector<float>* data = this->box->getBoxData() ;

        glm::vec3 center = ss->position ;
        glm::i32vec3 t1 = glm::round(center - box->pmin);
        if(t1[0]>=0 && t1[1]>=0&&t1[2]>=0&& t1[0]<boxdimension[0] && t1[1]<boxdimension[1] && t1[2]<boxdimension[2])
        {
            (*data)[t1[0]*boxdimension[1]*boxdimension[2]+t1[1]*boxdimension[2]+t1[2]] = 1 ;
            count++ ;
        }


        for(int i = 0 ; i< dimension[0]; i++)
        {
            for(int j = 0; j < dimension[1]; j++)
            {
                for(int z = 0; z< dimension[2]; z++)
                {
                    if(ss->isInBox(glm::vec3(i,j,z)+t->pmin))
                    {
                        count++ ;
                        glm::i32vec3 t1 = glm::vec3(i,j,z)+ t->pmin - box->pmin;
                        (*data)[t1[0]*boxdimension[1]*boxdimension[2]+t1[1]*boxdimension[2]+t1[2]] = 1 ;
                    }
                }
            }
        }
        delete t ;
    }
//    std::cout<<"there is not zero "<<count<<" "<<ss->radius<<std::endl ;

}

BVHNode* splitBuild(std::vector<BVHNode*> nodes,int begin, int end)
{
    if(begin == end) {
        std::cout<<"begin shit"<<begin<<std::endl ;
        return nullptr;
    }

    VoxelBox box ;
    for(int i = begin ;i<end;i++)
    {
        box.merge(*(nodes[i]->box)) ;
    }

    int size = end-begin ;
    if(size==1)
    {
        return nodes[begin] ;
    } else {
        std::vector<int> dimension = argsort(box.pmax-box.pmin) ;
        int dimension1 = dimension[2] ;

        std::sort(&nodes[begin],&nodes[end-1]+1,[dimension1](BVHNode*& node1, BVHNode*& node2){
            return node1->box->pmin[dimension1] < node2->box->pmin[dimension1]  ;
        }) ;

        BVHNode* t = nodes[int((begin+end)/2)]  ;
        double split = t->box->pmin[dimension1] ;

        auto middle = std::partition(&nodes[begin],&nodes[end-1]+1,[split,dimension1](const BVHNode* node){
            return node->box->pmin[dimension1] < float(split) ;
        }) ;

        int mid = begin ;
        for(auto t = nodes[mid]; t->id!=(*middle)->id;t= nodes[++mid]) {};
        std::cout<<"split mid is "<<mid <<"between "<<begin <<"and "<<end<<std::endl ;
//        std::cout<<"split mid is "<<mid <<"between "<<begin <<"and "<<end<<std::endl ;
        BVHNode* parent = new BVHNode(-1,
                                         splitBuild(nodes,begin,mid) ,
                                         splitBuild(nodes,mid,end)
        ) ;


        return parent ;
    }

}

BVH::BVH(glm::i32vec3 pmin, glm::i32vec3 pmax ,glm::vec3 voxelBox)
{
    glm::i32vec3 size = pmax - pmin ;
    if(int(size[0])%int(voxelBox[0]-2)==0) size[0] = (int((size[0])/(voxelBox[0]-2))+1)*voxelBox[0] ;
    if(int(size[1])%int(voxelBox[1]-2)==0) size[1] = (int((size[1])/(voxelBox[1]-2))+1)*voxelBox[1] ;
    if(int(size[2])%int(voxelBox[2]-2)==0) size[2] = (int((size[2])/(voxelBox[2]-2))+1)*voxelBox[2] ;



    glm::vec3 startPoint = glm::i32vec3(-1,-1,-1) + pmin;
    glm::vec3 endPoint = glm::vec3(size[0]-1,size[1]-1,size[2]-1) ;
    std::vector<BVHNode*> nodes ;

        for(int i = 0 ; i*(voxelBox[0]-2)<size[0] ;i++ )
        {
            for(int j = 0 ;j*(voxelBox[1]-2)<size[1];j++)
            {
                for(int k = 0; k*(voxelBox[2]-2)<size[2];k++)
                {
                    glm::vec3 s = startPoint + glm::vec3(i* (voxelBox[0]-2),
                                                         j* (voxelBox[1]-2),
                                                         k* (voxelBox[2]-2));

                    VoxelBox* t = new VoxelBox(s,s+voxelBox) ;
                    BVHNode* n = new BVHNode(nodes.size(),t) ;
                    nodes.push_back(n) ;
                    std::cout<<"box is "<<glm::to_string(s)<<" "<<glm::to_string(s+voxelBox)<<std::endl ;
                }
            }
    }
    this->root = splitBuild(nodes, 0, nodes.size()) ;
        std::cout<<"build end"<<std::endl ;
}

void BVH::getInteract(std::vector<BVHNode*>& nodes, Sphere& ss )
{
    std::stack<BVHNode*> st ;
    st.push(root) ;


    while(st.size()>0)
    {
        BVHNode* top = st.top() ;
        st.pop() ;

        if(doesBoxIntersectSphere(*(top->box),ss))
        {
            if(top->id>=0) nodes.push_back(top) ;
            else{
                if(doesBoxIntersectSphere(*(top->children[0]->box),ss)) st.push(top->children[0]) ;
                if(doesBoxIntersectSphere(*(top->children[1]->box),ss)) st.push(top->children[1]) ;
            }

        }
    }

}
