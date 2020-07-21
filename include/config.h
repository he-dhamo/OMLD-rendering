#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>

// screen size
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 1080;

// settings
const std::string AREA = "area_5a";
const int NVIEWS = 6261;
// max views available - area_1: 10327, area_2: 15714, area_3: 3704, area_4: 13268, area_5a: 6261, area_5b: 11332, area_6: 9890
// used in our splits  - area_1: 10192, area_2: 1615,  area_3: 3630, area_4: 11958, area_5a: 2854, area_5b: 4296,  area_6: 9873

const std::string ROOT_DIR = "../../stanford3d/" + AREA + "/";
const std::string OUT_DIR = "../generated_images/";

// if true, additionally render an image in perturbed view
// to use, make sure you have a folder of pose perturbations under <OUT_DIR>/<AREA>/pose_st !
const bool RENDER_PERTURBED_VIEW = false;

#endif // CONFIG_H

