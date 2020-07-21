#ifndef UTILS_HPP
#define UTILS_HPP

#include <stb_image.h>

#include <iostream>
#include <vector>

#include <shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <opencv2/opencv.hpp>

#include <string.h>
#include <sstream>


struct Vertex{
    glm::vec3 pos;
    glm::vec2 texture;
    glm::vec3 label;
    std::string room;
};

struct Texture{
    std::string name;
    unsigned int id;

    Texture(){
        glGenTextures(1, &id);
    }
};

struct MeshItem {

public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> tempIndices;

    unsigned int VBO;
    unsigned int EBO;

    unsigned int numIndices;
    unsigned int materialIndex;

    void initBuffers(){
        glGenBuffers(1, &EBO);
        glGenBuffers(1, &VBO);

    }
};

void load3DModel(std::vector<MeshItem> &mItem, std::vector<Texture> &mTextures, std::map<int, std::string> &instance2room);

glm::vec3 convertLabel(std::string label, std::string &room, std::map<int, std::string> &instance2room);
bool findSemanticFace(const aiScene *semantic, const aiVector3D *vertex, const aiFace f, std::string &label);

void print_glm(glm::mat4 m);
cv::Mat getColor(bool saveImage, std::string area, std::string name);
cv::Mat getDepth(bool saveImage, std::string area, std::string name);
cv::Mat getLabel(bool saveImage, std::string area, std::string name);

void saveDepthLayers(std::string area, std::vector<cv::Mat> depth, int frame);
void saveColorLayers(std::string area, std::vector<cv::Mat> color, int frame);
void saveLabelLayers(std::string area, std::vector<cv::Mat> label, int frame);
void saveImage(std::string filename, cv::Mat content);

glm::mat4 perspective_glm(double fx, double fy, double cx, double cy, double n, double f, double w, double h);

void extractLayer(cv::Vec4b label, MeshItem &mesh);
int instanceRGBtoID(cv::Vec4b v);
cv::Vec4b instanceIDtoRGB(unsigned short id);
void restoreIndices(std::vector<MeshItem> &meshes);
std::string getRoomLabel(std::string str);

cv::Point minDepthPos(const cv::Mat depth);
void sortLayers(std::vector<cv::Mat> &depth, std::vector<cv::Mat> &color, std::vector<cv::Mat> &label);

#endif // UTILS_HPP

