---
title: SWC2OBJ
---

## Keywords 
BVH,metaball,swc2vol,vol2obj,mergeOBJ,smoothOBJ

## 任务
![picture 3](/images/pipline.png)
> 将swc文件（一种人工标注的骨架文件）转化为光滑的obj,可以用于线和骨架膨胀之后转化为mesh，对应gis中的膨胀操作。这份代码提供了一份解决方案，也就是将一个骨架（人工标注的骨架）转化为光滑的mesh，并以obj的方式导出为mesh。

## 技术过程
1. 我们都知道，要光滑拼接两个曲面是困难的，尤其是需要2阶光滑等光滑的性质的时候，因此要对物体进行膨胀和腐蚀操作，我们倾向于使用metaball算法。 
2. 也即对骨架，以球的方式，以一定的步长，插值球的半径，进行体素化。(swc2vol:以metaball实现)
3. 对体素化的数据进行marching cube得到对应的mesh。(vol2obj)
4. 由于是超大规模体数据，所以需要对分块marching cube的mesh进行拼接，并保证近似水密性。(objmerge)
5. 由于marchingcube的算法很不平滑，因此，我们使用taubin smooth算法进行smooth，得到局部光滑的mesh(obj).
swc2obj包含两个大的部分，包含四个部分swc2vol,vol2obj,objmerge,objsmooth(taubin smooth) 

## swc2vol

以分块BVH算法的情况，对swc进行体素化，并存储为多个体数据文件。

### 使用说明
> 对应cmake中的SWC2obj对象，命令行调用方式如下所示：
```C++
范式：
SWC2Obj.exe -s the_path_to_swc_file -o the_path_to_save_vol_data -b block_size
示例:
C:/Users/lidan/Desktop/SWC2Obj/cmake-build-release/SWC2Obj.exe -s C:/Users/lidan/Desktop/brain/14193_30neurons/N001.swc -o C:/Users/lidan/Desktop/SWC2Obj/newResult/ -b 256
```

## vol2obj

以分块并行marching cube的算法, 得到obj, 并写入到同一个文件中。（基于paraview的二次开发，鲁棒的marching cube算法，如果您对marching cube的算法好奇，请参照我的另一个[代码仓库](https://github.com/lidan233/surfaceReconstruct2Marchingcube)

### 使用说明
> 环境说明：因为cuda marchingcube对分块marchingcube的不支持, 所以我基于paraview二次开发出python脚本，并使用paraview封装的python解释器进行执行。我从善如流。我将paraview封装的解释器直接重命名为python4,希望paraview可以帮助到大家。

> 对应python脚本为vol2obj, 命令行调用方式如下所示：
```C++
范式:
python4.obj volume2obj/main.py -i the_path_to_save_vol_data -o the_path_to_save_obj 
示例:
"D:/software/ParaView 5.8.1-Windows-Python3.7-msvc2015-64bit/bin/python4.exe" C:/Users/lidan/PycharmProjects/volume2obj/main.py -i C:/Users/lidan/Desktop/SWC2Obj/newResult/N001.swc -o C:/Users/lidan/Desktop/SWC2Obj/newResult/N001_use.obj 
```


## mergeOBJ
此时的OBJ中有很多重合点，这些重合点没有连接关系，所以目前的obj一平滑就会断掉，因此我们使用mergeOBJ合并这些重合点。 
主要功能:读取obj,去重，合并相同节点，实现space坐标映射等。

### 使用说明
对应cmake对象为mergeOBJ, 命令行调用方式如下所示：
```C++
范式:
mergeOBJ.exe -i the_path_to_save_obj -o the_path_to_save_obj_without_repeat_vertex
示例:
C:/Users/lidan/Desktop/SWC2Obj/cmake-build-release/ObjMerge/mergeOBJ.exe -i  C:/Users/lidan/Desktop/SWC2Obj/newResult/N001_use.obj -o C:/Users/lidan/Desktop/SWC2Obj/newResult/N001_new_use.obj
```

## smoothOBJ
试过meshlab很多的平滑算法，只有taubin算法最鲁棒，mu设为-0.1. 
### 使用说明
对应cmake对象为smoothOBJ, 命令行调用方式如下所示：
```C++
范式：
python4 C:/Users/lidan/PycharmProjects/volume2obj/last.py -i the_path_to_inputobj -o the_path_to_outputobj
示例：
"D:/software/ParaView 5.8.1-Windows-Python3.7-msvc2015-64bit/bin/python4.exe"  C:/Users/lidan/PycharmProjects/volume2obj/last.py -i C:/Users/lidan/Desktop/allobj/N001_new_use.obj -o C:/Users/lidan/Desktop/allobj/N001_new_use.obj

```

## 最终效果
以神经元为例:
开始线框：
![picture 3](/images/0ee91e4526a40f2dbc0e15bde3fa0902b7c9247e062e72331f070a47172aa654.png)  
最终效果
![picture 1](/images/9c59be8086e897aa53630d1d4b39ffb1cfd8fd6dd876faa3bf9ca63505cc0844.png)  
