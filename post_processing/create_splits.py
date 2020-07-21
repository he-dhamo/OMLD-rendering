import numpy as np
import argparse
import json

from utils import *

'''
Post-processing script that reads the generated layers and creates train and validation splits. 
Since the dataset is a real reconstruction, it has holes, therefore for training we select a subset of layers that 
are occluded more than a threshold, to make learning more meaningful. 
'''

MAX_DEPTH = 100000
W = 1080
H = 1080

parser = argparse.ArgumentParser()
parser.add_argument('--global_dir', type=str, help='path to generated images', default='../generated_images/')
parser.add_argument('--split', type=str, help='(train/val) generate train or val split ids', default='train')
parser.add_argument('--resume_json_state', type=str, default='no',
                    help='(yes/no) resume from existing split json. Resuming is not duplicate safe!')
args = parser.parse_args()

assert args.split in ['train', 'val']
assert args.resume_json_state in ['yes', 'no']

global_path = args.global_dir

if args.split == 'val':
    areas = ['area_5a', 'area_5b']
    if args.resume_json_state == 'yes':
        val_split = json.load(open(global_path + 'val_split.json', 'r'))
    else:
        val_split = []
else:
    areas = ['area_1', 'area_2', 'area_3', 'area_4', 'area_6']
    if args.resume_json_state == 'yes':
        train_split = json.load(open(global_path + 'train_split.json', 'r'))
    else:
        train_split = {'object': [], 'layout': []}

for area in areas:

    metadata_file = global_path + area + "/metadata.json"
    imagelist = json.load(open(metadata_file, 'r'))

    counter_images = 0

    for i in imagelist.keys():

        print("-------- frame " + i + " --------")

        layout_d = np.zeros([H, W])
        numLayers = getNumLayers(global_path + area, i)

        layers_depth = []
        selected_layers = []

        for j in range(numLayers):
            layers_depth.append(getDepth(global_path + area, i, str(j)))

        layers_depth.append(getDepth(global_path + area, i, "layout"))

        if args.split == 'train':
            num_overlap = np.zeros(numLayers+1)

            layers_depth_nozeros = np.stack(layers_depth, axis=0) > 0 # list to numpy array, mask of nonzeros
            # fill zeros with a big number so that they dont count for the min computation
            layers_depth_nozeros = MAX_DEPTH * (1-layers_depth_nozeros) + np.stack(layers_depth, axis=0)
            # need to train with object that are not occluded also, to avoid over-inpainting
            # get index of front-most object in an image
            front_object = np.unravel_index(np.argmin(layers_depth_nozeros), layers_depth_nozeros.shape)

            counter_layers = 0

            # we want to train with layers that are occluded
            for j in range(numLayers + 1):  # +1 includes layout

                mask1 = layers_depth[j] > 0

                for k in range(numLayers + 1):  # +1 includes layout
                    if j != k:
                        mask2 = layers_depth[k] > 0
                        overlap = mask1 * mask2

                        if np.mean(overlap * layers_depth[j]) > np.mean(overlap * layers_depth[k]):
                            # j is the occluded (back) layer
                            num_overlap[j] += np.sum(overlap)

                if j < numLayers: # object layer
                    if num_overlap[j] > 0.2 * np.sum(mask1) or front_object[0] == j:
                        selected_layers.append(i)

                else: # layout
                    if num_overlap[j] > 0.2 * np.sum(mask1):
                        temp_dict = {'area': area, 'frame': i}

                        train_split['layout'].append(temp_dict)

                        with open(global_path + 'train_split.json', "w") as json_file:
                            json.dump(train_split, json_file)

                        counter_images += 1
                        print(str(counter_images) + " images for layout")

            if len(selected_layers) > 30 or len(selected_layers) <= 1:
                # scenes with huge number of repeating objects, or only a front object - skip
                continue

            counter_images += 1

            temp_dict = {'area': area, 'frame': i, 'layers': selected_layers}
            train_split['object'].append(temp_dict)

            with open(global_path + 'train_split.json', "w") as json_file:
                json.dump(train_split, json_file)

            print(str(counter_images) + " images for objects")
            print(str(len(selected_layers)) + "selected layers out of " + str(len(layers_depth)))

        else:  # val split
            num_overlap = 0
            for j in range(numLayers + 1):  # +1 includes layout
                for k in range(numLayers + 1):  # +1 includes layout
                    if j != k:
                        mask1 = layers_depth[j] > 0
                        mask2 = layers_depth[k] > 0
                        overlap = mask1 * mask2
                        num_overlap += np.sum(overlap)
            
            # general overlap is beyond a threshold
            if num_overlap > 0.2 * H * W and numLayers < 15:
                counter_images += 1

                temp_dict = {'area': area, 'frame': i}

                val_split.append(temp_dict)

                with open(global_path + 'val_split.json', "w") as json_file:
                    json.dump(val_split, json_file)

                print(str(counter_images) + " images ")
