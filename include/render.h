#ifndef RENDER_H
#define RENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <opencv2/opencv.hpp>

#include <json/json.h>
#include <fstream>

#include <utils.hpp>

struct Frame
{
    std::string posePath;
    std::string intrinsicsPath;

    float fx;
    float fy;
    float cx;
    float cy;
    glm::mat4 pose;
    glm::mat4 perturbPose;

};

void renderView(GLFWwindow* window, std::vector<MeshItem> &mItem, std::vector<Texture> &mTextures, std::string fName, Frame &frame, bool perturbed);
void initBuffers(std::vector<MeshItem> &mItem);
void initTextures(std::vector<Texture> &mTextures);
void configFramebuffer(unsigned int &textureColorbuffer);

void readK(Frame &f);

// callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);


#endif // RENDER_H

