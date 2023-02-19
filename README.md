---
title: SWC2OBJ
---

##Keywords
BVH, metaball, swc2vol, vol2obj, mergeOBJ, smoothOBJ

## Task
![picture 3](/images/pipline.png)
> Convert the swc file (a manually marked skeleton file) into a smooth obj, which can be used for line and skeleton expansion and then converted into mesh, corresponding to the expansion operation in gis. This code provides a solution, which is to convert a skeleton (artificially marked skeleton) into a smooth mesh and export it as a mesh in the form of obj.

## Technical process
1. We all know that it is difficult to smoothly join two surfaces, especially when smooth properties such as second-order smoothness are required. Therefore, we tend to use the metaball algorithm to perform expansion and erosion operations on objects.
2. That is to say, voxelize the skeleton by interpolating the radius of the ball in the form of a ball with a certain step size. (swc2vol: implemented in metaball)
3. Marching cube the voxelized data to get the corresponding mesh. (vol2obj)
4. Due to the large-scale volume data, it is necessary to splice the mesh of the marching cube and ensure approximate watertightness. (objmerge)
5. Since the algorithm of marchingcube is not smooth, we use the taubin smooth algorithm for smoothing to obtain a locally smooth mesh(obj).
swc2obj contains two large parts, including four parts swc2vol, vol2obj, objmerge, objsmooth (taubin smooth)

## swc2vol

In the case of the block-wise BVH algorithm, the swc is voxelized and stored as multiple volume data files.

### Instructions for use
> Corresponding to the SWC2obj object in cmake, the command line calling method is as follows:
```C++
Paradigm:
SWC2Obj.exe -s the_path_to_swc_file -o the_path_to_save_vol_data -b block_size
Example:
C:/Users/lidan/Desktop/SWC2Obj/cmake-build-release/SWC2Obj.exe -s C:/Users/lidan/Desktop/brain/14193_30neurons/N001.swc -o C:/Users/lidan/Desktop/SWC2Obj /newResult/ -b 256
```

##vol2obj

Use the block-parallel marching cube algorithm to get obj and write it into the same file. (Based on the secondary development of paraview, the robust marching cube algorithm, if you are curious about the marching cube algorithm, please refer to my other [code warehouse](https://github.com/lidan233/surfaceReconstruct2Marchingcube)

### Instructions for use
> Environment description: Because cuda marchingcube does not support block marchingcube, so I re-developed a python script based on paraview, and used the python interpreter encapsulated by paraview to execute. I am good at what I do. I directly renamed the interpreter encapsulated by paraview to python4, hoping that paraview can help everyone.

> The corresponding python script is vol2obj, and the command line calling method is as follows:
```C++
Paradigm:
python4.obj volume2obj/main.py -i the_path_to_save_vol_data -o the_path_to_save_obj
Example:
"D:/software/ParaView 5.8.1-Windows-Python3.7-msvc2015-64bit/bin/python4.exe" C:/Users/lidan/PycharmProjects/volume2obj/main.py -i C:/Users/lidan/ Desktop/SWC2Obj/newResult/N001.swc -o C:/Users/lidan/Desktop/SWC2Obj/newResult/N001_use.obj
```


## mergeOBJ
At this time, there are many overlapping points in the OBJ, and these overlapping points are not connected, so the current obj will be broken once it is smoothed, so we use mergeOBJ to merge these overlapping points.
Main functions: read obj, deduplicate, merge same nodes, realize space coordinate mapping, etc.

### Instructions for use
The corresponding cmake object is mergeOBJ, and the command line calling method is as follows:
```C++
Paradigm:
mergeOBJ.exe -i the_path_to_save_obj -o the_path_to_save_obj_without_repeat_vertex
Example:
C:/Users/lidan/Desktop/SWC2Obj/cmake-build-release/ObjMerge/mergeOBJ.exe -i C:/Users/lidan/Desktop/SWC2Obj/newResult/N001_use.obj -o C:/Users/lidan/Desktop /SWC2Obj/newResult/N001_new_use.obj
```

## smoothOBJ
I have tried many smoothing algorithms in meshlab, only the taubin algorithm is the most robust, and mu is set to -0.1.
### Instructions for use
The corresponding cmake object is smoothOBJ, and the command line calling method is as follows:
```C++
Paradigm:
python4 C:/Users/lidan/PycharmProjects/volume2obj/last.py -i the_path_to_inputobj -o the_path_to_outputobj
Example:
"D:/software/ParaView 5.8.1-Windows-Python3.7-msvc2015-64bit/bin/python4.exe" C:/Users/lidan/PycharmProjects/volume2obj/last.py -i C:/Users/lidan/ Desktop/allobj/N001_new_use.obj -o C:/Users/lidan/Desktop/allobj/N001_new_use.obj

```

## final effect
Take neurons as an example:
Start wireframe:
![picture 3](/images/0ee91e4526a40f2dbc0e15bde3fa0902b7c9247e062e72331f070a47172aa654.png)
final effect
![picture 1](/images/9c59be8086e897aa53630d1d4b39ffb1cfd8fd6dd876faa3bf9ca63505cc0844.png)
与此原文有关的更多信息要查看其他翻译信息，您必须输入相应原文
发送反馈
侧边栏
