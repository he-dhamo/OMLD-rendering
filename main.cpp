#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <opencv2/opencv.hpp>

#include <utils.hpp>
#include <render.h>
#include <config.h>


/**
 * Generate layered RGBA-D data for each mesh area, together with intrinsics
 */
int generateLayers(){

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RGB render", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetScrollCallback(window, scroll_callback);


    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------

    unsigned int textureColorbuffer;

    configFramebuffer(textureColorbuffer);

    glEnable(GL_DEPTH_TEST);

    std::vector<MeshItem> mItem;
    std::vector<Texture> mTextures;
    cv::Mat instance, depth;
    cv::Point p;
    cv::Vec4b targetInstance;
    std::map<int, std::vector<int> > hashSubmesh;
    std::map<int, std::string> instance2room;

    int meshId;

    load3DModel(mItem, mTextures, instance2room);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    initTextures(mTextures);

    std::ifstream pstream;
    std::string posefile = ROOT_DIR + "data/poses.txt";
    pstream.open(posefile.c_str(), std::ifstream::in);
    if (!pstream.is_open())
    {
        std::cout << posefile.c_str() << std::endl;
        std::cout << "poses.txt not found" << std::endl;
        throw;
    }

    std::ifstream rgbstream;
    std::string rgbfile = ROOT_DIR + "data/rgbs.txt";
    rgbstream.open(rgbfile.c_str(), std::ifstream::in);
    if (!rgbstream.is_open())
    {
        std::cout << rgbfile.c_str() << std::endl;
        std::cout << "rgbs.txt not found" << std::endl;
        throw;
    }

    // for every view in the area
    for(int view = 0; view < NVIEWS; view++){

        std::map<ushort, int> hashInstance;
        std::vector<cv::Mat> depth;
        std::vector<cv::Mat> color;
        std::vector<cv::Mat> label;

        std::cout << "--------- view  " << view << "  ----------" << std::endl;

        std::string poseName;
        pstream >> poseName;

        std::string rgbName;
        rgbstream >> rgbName;

        Frame frame;

        frame.posePath = ROOT_DIR + "data/pose/" + poseName;

        readK(frame);

        //initialize buffers
        initBuffers(mItem);

        // optinally render perturbed view ====================
        if(RENDER_PERTURBED_VIEW){

            std::ifstream ststream;
            ststream.open(OUT_DIR + AREA + "/pose_st/" + std::to_string(view) + ".txt");
            float trans[16] = {0};
            trans[15] = 1;
            ststream >> trans[0] >> trans[1] >> trans[2] >> trans[3] >>
                        trans[4] >> trans[5] >> trans[6] >> trans[7] >>
                        trans[8] >> trans[9] >> trans[10] >> trans[11];

            frame.perturbPose = glm::make_mat4(trans);
            frame.perturbPose = glm::transpose(frame.perturbPose);
            frame.perturbPose = glm::inverse(frame.perturbPose);
            ststream.close();

            renderView(window, mItem, mTextures, "color", frame, true);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            getColor(true, AREA, std::to_string(view) + "_target");

            renderView(window, mItem, mTextures, "depth", frame, true);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            getDepth(true, AREA, std::to_string(view) + "_target");
        }

        // save intrinsics and original rgb name for every view/frame ============
        Json::Value metadata;
        std::ifstream imetafile(OUT_DIR + AREA + "/metadata.json", std::ifstream::binary);
        if(imetafile){
            imetafile >> metadata;
        }
        imetafile.close();

        Json::Value framedata;
        framedata["source_frame"] = AREA + "/data/rgb/" + rgbName;
        framedata["intrinsics"].append(frame.fx);
        framedata["intrinsics"].append(frame.fy);
        framedata["intrinsics"].append(frame.cx);
        framedata["intrinsics"].append(frame.cy);

        metadata[std::to_string(view)] = framedata;

        std::ofstream ometafile(OUT_DIR + AREA + "/metadata.json", std::ifstream::binary);
        ometafile << metadata;
        ometafile.close();
        // ==================================================


        // render full color, depth and instance image ======
        renderView(window, mItem, mTextures, "color", frame, false);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        getColor(true, AREA, std::to_string(view));

        renderView(window, mItem, mTextures, "depth", frame, false);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        getDepth(true, AREA, std::to_string(view));

        renderView(window, mItem, mTextures, "instance", frame, false);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        instance = getLabel(true, AREA, std::to_string(view));
        // ==================================================


        //cv::waitKey(500);

        // get set of unique instances that appear in the view
        for(int i=0; i<SCR_HEIGHT; i++){
            for(int j=0; j<SCR_WIDTH; j++){
                hashInstance.insert(std::pair<ushort, int> (instance.at<unsigned short>(i,j), 0));
            }
        }

        int t = 0;
        std::string currentRoom = getRoomLabel(poseName);
        // save each unique instance in the current room as a separate layer
        for (std::map<ushort,int>::iterator it=hashInstance.begin(); it!=hashInstance.end(); ++it){
            if(it->first == 0 || currentRoom != instance2room[(int)it->first]){
                continue;
            }
            targetInstance = instanceIDtoRGB(it->first);

            for(int m=0; m<mItem.size(); m++){
                extractLayer(targetInstance, mItem[m]);
            }

            initBuffers(mItem);

            // render color, depth and instance layers ======
            renderView(window, mItem, mTextures, "color", frame, false);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            color.push_back(getColor(false, AREA, std::to_string(view)));
            renderView(window, mItem, mTextures, "depth", frame, false);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            depth.push_back(getDepth(false, AREA, std::to_string(view)));
            renderView(window, mItem, mTextures, "instance", frame, false);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
            label.push_back(getLabel(false, AREA, std::to_string(view)));
            // ==============================================

            restoreIndices(mItem);
            t++;
        }
        //sortLayers(depth, color, label);

        std::cout << "number of layers: " << depth.size() << std::endl;

        // save all layers of a frame
        saveDepthLayers(AREA, depth, view);
        saveColorLayers(AREA, color, view);
        saveLabelLayers(AREA, label, view);

    }

    pstream.close();

    glDeleteVertexArrays(1, &VAO);

    // clear memory leaks ======================
    //glDeleteVertexArrays(1, &VAO);
    for(int m=0; m < mItem.size(); m++){
        glDeleteBuffers(1, &mItem[m].VBO);
        glDeleteBuffers(1, &mItem[m].EBO);

        mItem[m].vertices.clear();
        mItem[m].vertices.shrink_to_fit();
        mItem[m].indices.clear();
        mItem[m].indices.shrink_to_fit();
        mItem[m].tempIndices.clear();
        mItem[m].tempIndices.shrink_to_fit();
    }
    for(int m=0; m<mTextures.size(); m++){
        glDeleteTextures(1, &mTextures[m].id);
    }
    mTextures.clear();
    mTextures.shrink_to_fit();
    mItem.clear();
    mItem.shrink_to_fit();
    // ========================================

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();

    return 0;

}


int main()
{
    generateLayers();
    return 0;
}
