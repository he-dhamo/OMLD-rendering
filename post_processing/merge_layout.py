import numpy as np
import os
import argparse
import json
from utils import *

'''
Post-processing script that reads all the rendered layers and merges layout components (wall, floor, ceiling, window).
The script operates on the same (input) directory, removing all the existing layers and writing the new ones -
object layers and layout layer.
! Make sure the script is set up and runs (e.g. correct paths) since it might remove current layers and fail 
to write the new ones. For instance, you can disable the os.remove calls, make sure that the script runs, and then run 
again with os.remove enabled.
'''

W = 1080
H = 1080

parser = argparse.ArgumentParser()
parser.add_argument('--area', type=str, default="area_1", help='area name')
parser.add_argument('--stanford_dir', type=str, help='path to original stanford3d data', default='../../stanford3d/')
parser.add_argument('--global_dir', type=str, help='path to generated images', default='../generated_images/')

args = parser.parse_args()

layoutObj = [2, 3, 4, 7] # structural elements (wall, floor, etc.)

area = args.area
global_path = args.global_dir + area
metadata_file = global_path + "/metadata.json"

imagelist = json.load(open(metadata_file, 'r'))

for i in imagelist.keys():

    print("-------- frame " + i)

    layout_rgb = np.zeros([H, W, 4])
    layout_d = np.zeros([H, W])
    layout_instance = np.zeros([H, W])

    numLayers = getNumLayers(global_path, i)
    keep_rgb = []
    keep_ins = []
    keep_depth = []

    src_rgb_path = args.stanford_dir + imagelist[i]["source_frame"]
    os.system("cp " + src_rgb_path + " " + global_path + "/rgb/" + i + "_raw.png")

    for j in range(numLayers):

        depth = getDepth(global_path, i, str(j))
        rgb = getRGB(global_path, i, str(j))
        instance = getInstance(global_path, i, str(j))

        if getSemanticLabel(global_path, i, str(j)) in layoutObj:
            mask1 = layout_d > 0
            mask2 = getDepth(global_path, i, str(j)) > 0
            overlap = mask1 * mask2
            num_overlap = np.sum(overlap)

            rgb_mask = mask2.astype(int) - overlap.astype(int)
            rgb_mask = np.stack([rgb_mask, rgb_mask, rgb_mask, rgb_mask], axis=2)
            layout_d = layout_d + np.multiply(depth, mask2.astype(int) - overlap.astype(int))
            layout_rgb = layout_rgb + np.multiply(rgb, rgb_mask)
            layout_instance = layout_instance + np.multiply(instance, mask2.astype(int) - overlap.astype(int))
        else:
            keep_depth.append(depth)
            keep_rgb.append(rgb)
            keep_ins.append(instance)

        os.remove(global_path + "/depth/" + i + "_" + str(j) + ".png")
        os.remove(global_path + "/rgb/" + i + "_" + str(j) + ".png")
        os.remove(global_path + "/instance/" + i + "_" + str(j) + ".png")

    for j in range(len(keep_ins)):
        save(global_path, i, keep_rgb[j], keep_depth[j], keep_ins[j], str(j))

    save(global_path, i, layout_rgb, layout_d, layout_instance, "layout")
