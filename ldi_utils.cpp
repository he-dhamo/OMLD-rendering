#include <utils.hpp>
#include <config.h>

/**
 * Save index in temp and remove from the list of indices
 */
void moveIndices(std::vector<unsigned int> &indices, std::vector<unsigned int> &temp, int vIndex){
    // iterate through triangle face (+=3)
    for(int i=0; i<indices.size(); i+=3){
        if(indices[i] == vIndex || indices[i+1] == vIndex || indices[i+2] == vIndex){

            temp.push_back(indices[i]);
            temp.push_back(indices[i + 1]);
            temp.push_back(indices[i + 2]);
            indices.erase(indices.begin() + i, indices.begin() + i + 3);
        }
    }
}

/**
 * extract unique room identifier from the string ID of a frame
 */
std::string getRoomLabel(std::string str){

    std::stringstream slabel(str);
    std::string camera, id, roomName, roomNumber;

    std::getline(slabel, camera, '_');
    std::getline(slabel, id, '_');
    std::getline(slabel, roomName, '_');
    std::getline(slabel, roomNumber, '_');

    return roomName + "_" + roomNumber;
}

/**
 * Moves from indices to tempIndices, all vertices from the mesh that have an instance ID = label
 * Used to extract one layer (i.e. render a single instance)
 */
void extractLayer(cv::Vec4b label, MeshItem &mesh){

    for(int i = 0; i < mesh.vertices.size(); i++){
        // glm is rgb, cv is bgr!
        if(!(mesh.vertices[i].label.x == label[2] &&
             mesh.vertices[i].label.y == label[1] &&
             mesh.vertices[i].label.z == label[0])){

            moveIndices(mesh.indices, mesh.tempIndices, i);
        }
    }
}

/**
 * move tempIndices back to indices and reset temp
 */
void restoreIndices(std::vector<MeshItem> &meshes){
    for(int m=0; m<meshes.size(); m++){
        for(int i=0; i<meshes[m].tempIndices.size(); i++){
            meshes[m].indices.push_back(meshes[m].tempIndices[i]);
        }
        meshes[m].tempIndices.erase(meshes[m].tempIndices.begin(), meshes[m].tempIndices.end());
    }
}

// Currently not used
cv::Point minDepthPos(const cv::Mat depth){

    cv::Mat depthCopy(SCR_HEIGHT, SCR_WIDTH, CV_16U);
    depthCopy = depth;
    depth.copyTo(depthCopy);

    for(int i=0; i<depthCopy.rows; i++){
        for(int j=0; j<depthCopy.cols; j++){       
            //take care of zeros
            if(depthCopy.at<unsigned short>(i, j) == 0){
              depthCopy.at<unsigned short>(i, j) = 16500;
            }
        }
    }

    double min, max;
    cv::Point min_loc, max_loc;
    cv::minMaxLoc(depthCopy, &min, NULL, &min_loc, NULL);

    return min_loc;
}

// Currently not used
void swapLayers(cv::Mat &l1, cv::Mat &l2){
    cv::Mat temp;
    l1.copyTo(temp);
    l2.copyTo(l1);
    temp.copyTo(l2);
}

// sort layers based on depth comparison in overlap areas - Currently not used
void sortLayers(std::vector<cv::Mat> &depth, std::vector<cv::Mat> &color, std::vector<cv::Mat> &label){

    for(int l=0; l<depth.size()-1; l++){
        for(int k=l+1; k<depth.size(); k++){
            //get amodal mask overlap btween two objects
            cv::Mat mask1 = (depth[l] > 0);
            cv::Mat mask2 = (depth[k] > 0);
            cv::Mat overlap;
            cv::bitwise_and(mask1, mask2, overlap);
            overlap.convertTo(overlap, CV_16U);
            mask1.convertTo(mask1, CV_16U);
            mask2.convertTo(mask2, CV_16U);

            if(cv::sum(overlap)[0] != 0){

                double depth1 = cv::sum(depth[l].mul(overlap))[0] / cv::sum(overlap)[0];
                double depth2 = cv::sum(depth[k].mul(overlap))[0] / cv::sum(overlap)[0];

                if(depth2 < depth1){
                    swapLayers(depth[l], depth[k]);
                    swapLayers(color[l], color[k]);
                    swapLayers(label[l], label[k]);
                }
            }else{
                double depth1 = cv::mean(depth[l])[0] / cv::sum(mask1)[0];
                double depth2 = cv::mean(depth[k])[0] / cv::sum(mask2)[0];

                if(depth2 < depth1){
                    swapLayers(depth[l], depth[k]);
                    swapLayers(color[l], color[k]);
                    swapLayers(label[l], label[k]);
                }
            }
        }
    }
}
