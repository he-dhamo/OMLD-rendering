# Render OMLD dataset

This repository contains an OpenGL pipeline, to render object RGBA-D layers from real 3D reconstructions (Stanford 2D-3D S). We provide C++ rendering code together with Python post-processing scripts. These layers can be merged based on per-pixel depth order to create Layered Depth Images (LDI).
If you find this code useful, consider citing

```
@inproceedings{dhamo2019,
  title={Object-Driven Multi-Layer Scene Decomposition From a Single Image},
  author={Dhamo, Helisa and  Navab, Nassir and Tombari, Federico},
  booktitle={ICCV},
  year={2019}
```
We used this rendered data to train the learned models from this paper. 

## Dependencies

- CMake 2.8 or above
- OpenCV 3.0 or above
- OpenGL 3.3 
- <a href="https://www.assimp.org/">Assimp</a>

We tested the code with cuda=9.0, but other versions should be possible.

## Running instructions

Download the Stanford 2D-3D S dataset from <a href="http://buildingparser.stanford.edu/dataset.html"> the official site </a>. <br/>

Clone this repository and move to the root directory 
```
git clone https://github.com/he-dhamo/OMLD-rendering.git
cd OMLD-rendering/
```
Prepare files to index pose and rgb files of the stanford data, for every area:
```
chmod +x prepare_filelists.sh
./prepare_filelists.sh <stanford_root_path>
```
Create output directories for every area
```
chmod +x make_dirs.sh
./make_dirs.sh 
```
The default output directory is set to `./generated_images`. If you change this, please update the config file with the same output dir. 

Create build directory and move there: `mkdir build && cd build` <br/>

Set options and paths in `include/config.h` <br/>

Run CMake: `cmake ..` <br/>

Compile: `make` <br/>

Run executable: `./RenderLayers`

The program will save output images under `OUT_DIR`, in a `depth/`, `rgb/` and `instance/` folder; e.g. `depth/<frame_idx>.png` for the whole image as well as `depth/<frame_idx>_<layer_idx>.png` for each instance layer. In addition, we save the camera intrinsics and original frame file name (for all generated frames) in `metadata.json`.

The C++ code is based on <a href="https://learnopengl.com">Learn OpenGL</a>.

## Post processing
The `post_pocessing/` directory contains python scripts that: <br/>
`merge_layout.py` merge structural components (wall, floor, window, ceiling) to a common layout layer. The script will remove the current layers and create new ones including a layout layer. 

```
python merge_layout.py --area area_1 --global_dir ../generated_images/
```
`create_splits.py` create train and validation splits. The layers for training and testing are selected based on the level of amodal overlap with other layers. For example, to create the train splits:

```
python create_splits.py --split train --resume_json_state no --global_dir ../generated_images/
```

