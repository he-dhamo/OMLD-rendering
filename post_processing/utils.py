import cv2
import glob
import numpy as np


def save(global_path, stri, rgb, depth, instance, strj):
    filename = global_path + "/instance/" + stri + "_" + strj + ".png"
    instance = instance.astype(np.uint16)
    cv2.imwrite(filename, instance)

    filename = global_path + "/rgb/" + stri + "_" + strj + ".png"
    cv2.imwrite(filename, rgb)

    filename = global_path + "/depth/" + stri + "_" + strj + ".png"
    depth = depth.astype(np.uint16)
    cv2.imwrite(filename, depth)


def getNumLayers(global_path, i):
    filelist = glob.glob(global_path + "/rgb/" + i + "_[0-9]*") # only numeric labels
    #print ("num layers " + str(len(filelist)))
    return len(filelist)


def getInstance(global_path, i, j):
    filename = global_path + "/instance/" + i + "_" + j + ".png"
    image = cv2.imread(filename, -1)
    return image


def getRGB(global_path, i, j):
    filename = global_path + "/rgb/" + i + "_" + j + ".png"
    image = cv2.imread(filename, -1)
    return image


def getDepth(global_path, i, j):
    filename = global_path + "/depth/" + i + "_" + j + ".png"
    image = cv2.imread(filename, -1)
    return image


def getSemanticLabel(global_path, i, j):
    instance = getInstance(global_path, i, j)
    label = instance[np.nonzero(instance)][0]
    semantic = int(label / (256*4))

    return semantic
