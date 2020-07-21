#include <render.h>
#include "config.h"

void initBuffers(std::vector<MeshItem> &mItem){

    for(int m = 0; m<mItem.size(); m++){
        glBindBuffer(GL_ARRAY_BUFFER, mItem[m].VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * mItem[m].vertices.size(), &(mItem[m].vertices[0]), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mItem[m].EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mItem[m].indices.size(), &(mItem[m].indices[0]), GL_STATIC_DRAW);
    }
}

void initTextures(std::vector<Texture> &mTextures){

    //init textures
    for(int t = 0; t < mTextures.size(); t++){
        glBindTexture(GL_TEXTURE_2D, mTextures[t].id);
        // set the texture wrapping/filtering options (on the currently bound texture object)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load and generate the texture
        int width, height, nrChannels;
        unsigned char *data = stbi_load(mTextures[t].name.c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture!" << std::endl;

        }
    }
}

void configFramebuffer(unsigned int &textureColorbuffer){

    // framebuffer configuration
    // -------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a color attachment texture

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

/**
 * @brief renderView: Renders meshes in a view, either RGBA, depth or instance
 * @param window: GLFWwindow
 * @param mItem: vector of mesh items
 * @param mTextures: vector of all texture maps
 * @param fName: file name of the fragment shader (color, depth or instance)
 * @param dio: DataIO object where intrinsics are kept
 * @param perturbed: if true, render in a perturbed view (using perturbedPose from dio)
 */
void renderView(GLFWwindow* window, std::vector<MeshItem> &mItem, std::vector<Texture> &mTextures, const std::string fName, Frame &frame, bool perturbed){


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    char file[25];
    std::string temp = "../shaders/" + fName + ".fs";
    strcpy(file, temp.c_str());

    // build and compile our shader program
    // --------------------------------
    Shader ourShader("../shaders/camera.vs", file);

    ourShader.use();

    float near = 0.1, far = 10.0;

    // pass as args to shader
    if(fName == "depth"){
        ourShader.setFloat("near", near);
        ourShader.setFloat("far", far);
    }

    // create perspective glm matrix from opencv camera pose
    glm::mat4 projection = perspective_glm(frame.fx, frame.fy, frame.cx, frame.cy, near, far, (double) SCR_WIDTH, (double) SCR_HEIGHT);

    // camera view transformation
    glm::mat4 flipz = glm::mat4();
    flipz[2][2] = -1;
    glm::mat4 view = flipz * frame.pose;

    if(perturbed){
        // used to synthesise another view e.g. for novel view synthesis supervision
        view = flipz * (frame.perturbPose * frame.pose);
    }

    // adapting for model matrix of stanford 3d, 90 degree along y-axis
    glm::mat4 model = glm::mat4();
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    ourShader.setMat4("model", model);
    ourShader.setMat4("view", view);
    ourShader.setMat4("projection", projection);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    for(int m = 0; m<mItem.size(); m++){

        glBindBuffer(GL_ARRAY_BUFFER, mItem[m].VBO);

        // specify vertex attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture));
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, label));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mItem[m].EBO);

        // TEXTURE
        glBindTexture(GL_TEXTURE_2D, mTextures[mItem[m].materialIndex].id);

        // shader variable
        ourShader.setInt("texture1", 0);

        // draw triangles
        glDrawElements(GL_TRIANGLES, mItem[m].numIndices, GL_UNSIGNED_INT, 0);

        //glDeleteBuffers(1, &mItem[m].VBO);
        //glDeleteBuffers(1, &mItem[m].EBO);

    }
    //glfwSwapBuffers(window);
    //glfwPollEvents();

}

/**
* Read intrinsics (different for each frame)
*/
void readK(Frame &f)
{
    Json::Value intrinsics;
    std::ifstream k_file(f.posePath, std::ifstream::binary);
    k_file >> intrinsics;
    float k[3][3];

    for (int j = 0; j < intrinsics.get("camera_k_matrix", "").size(); j++) {
        for (int m = 0; m < intrinsics.get("camera_k_matrix", "")[j].size(); m++) {
            k[j][m] = intrinsics.get("camera_k_matrix", "")[j][m].asFloat();
        }
    }

    f.fx = k[0][0];
    f.fy = k[1][1];
    f.cx = k[0][2];
    f.cy = k[1][2];


    float p[16];

    for (int j = 0; j < intrinsics.get("camera_rt_matrix", "").size(); j++) {
        for (int m = 0; m < intrinsics.get("camera_rt_matrix", "")[j].size(); m++) {
            p[4*j + m] = intrinsics.get("camera_rt_matrix", "")[j][m].asFloat();
        }
    }
    p[12] = 0;
    p[13] = 0;
    p[14] = 0;
    p[15] = 1;

    f.pose = glm::transpose(glm::make_mat4(p));

}
