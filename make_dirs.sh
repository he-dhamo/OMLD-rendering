#!/bin/bash

mkdir ./generated_images
for area in area_1 area_2 area_3 area_4 area_5a area_5b area_6
do
    mkdir ./generated_images/$area
    mkdir ./generated_images/$area/depth
    mkdir ./generated_images/$area/instance
    mkdir ./generated_images/$area/rgb
done
