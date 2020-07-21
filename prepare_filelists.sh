#!/bin/bash

stanford_dir=$1

for area in area_1 area_2 area_3 area_4 area_5a area_5b area_6
do
    ls $stanford_dir/$area/data/pose > $stanford_dir/$area/data/poses.txt
    ls $stanford_dir/$area/data/rgb > $stanford_dir/$area/data/rgbs.txt
done
