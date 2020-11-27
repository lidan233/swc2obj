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
    this->children[2] = childrenR ;

    this->box->merge(*(childrenL->box)) ;
    this->box->merge(*(childrenR->box)) ;
}


void BVHNode::setData(Sphere* ss)
{
    VoxelBox* ss_box = ss->getBoundBox() ;
    VoxelBox* t = ss_box->interact(this->box) ;

    if(t != nullptr)
    {

        glm::vec3 dimension = t->pmax - t->pmin ;
        std::vector<float>* data = t->getBoxData() ;
        for(int i = 0 ; i< dimension[0]; i++)
        {
            for(int j = 0; j < dimension[1]; j++)
            {
                for(int z = 0; z< dimension[2]; z++)
                {
                    if(ss->isInBox(glm::vec3(i,j,z)+t->pmin))
                    {
                        (*data)[i*dimension[1]*dimension[2]+j*dimension[2]+z] = 1 ;
                    }
                }
            }
        }
        delete t ;
    }

}

BVHNode* splitBuild(std::vector<BVHNode*> nodes,int begin, int end)
{
    if(begin==end) { return nullptr; }
    VoxelBox box ;
    for(int i = begin ;i<end;i++)
    {
        box.merge(*(nodes[i]->box)) ;
    }

    int size = end-begin ;
    if(size==1)
    {
        return nodes[begin] ;

    }else{
        std::vector<int> dimension = argsort(box.pmax-box.pmin) ;
        int dimension1 = dimension[2] ;

        std::sort(&nodes[begin],&nodes[end-1]+1,[dimension1](BVHNode*& node1, BVHNode*& node2){
            return node1->box->pmin[dimension1] < node2->box->pmin[dimension1]  ;
        }) ;

        BVHNode* t = nodes[int(nodes.size()/2)]  ;
        double split = t->box->pmin[dimension1] ;

        BVHNode** middle = std::partition(&nodes[begin],&nodes[end-1]+1,[split,dimension1](const BVHNode* node1){
            return node1->box->pmin[dimension1] < split ;
        }) ;


        BVHNode* parent = new BVHNode(-1,splitBuild(nodes,begin,*middle-nodes[begin]) ,
                                         splitBuild(nodes,*middle-nodes[begin],end)) ;
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

        for(int i = 0 ; i*(voxelBox[0]-2)<size[0] ;i+=voxelBox[0]-2 )
        {
            for(int j = 0 ;j*(voxelBox[1]-2)<size[1];j+=voxelBox[1]-2)
            {
                for(int k = 0; k*(voxelBox[2]-2)<size[2];k+=voxelBox[2]-2)
                {
                    glm::vec3 s = startPoint + glm::vec3(i* (voxelBox[0]-2),
                                                         j* (voxelBox[1]-2),
                                                         k* (voxelBox[2]-2));

                    VoxelBox* t = new VoxelBox(s,s+voxelBox) ;
                    BVHNode* n = new BVHNode(nodes.size(),t) ;
                    nodes.push_back(n) ;
                }
            }
    }
    this->root = splitBuild(nodes, 0, nodes.size()) ;
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
