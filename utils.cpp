#include <utils.hpp>
#include <config.h>


/**
 * @brief load3DModel: Loads a textured mesh and an instance mesh and fuses them in a common structure,
 * so that we can render corresponding rgb, depth and semantic layers
 * @param mItem: vector of mesh structures
 * @param mTextures: vector of all texture maps
 * @param instance2room: maps each object instance id to the room it belongs to
 */
void load3DModel(std::vector<MeshItem> &mItem, std::vector<Texture> &mTextures, std::map<int, std::string > &instance2room){

    std::string rgbFile = ROOT_DIR + "3d/rgb.obj";
    std::string semanticFile = ROOT_DIR + "3d/semantic.obj";
    Assimp::Importer importer, importer2;

    const aiScene* sceneColor = importer.ReadFile( rgbFile,
       // aiProcess_CalcTangentSpace       |
          aiProcess_Triangulate            |
          aiProcess_FlipUVs                //|
          //aiProcess_JoinIdenticalVertices
          //aiProcess_SortByPType
                                              );
    const aiScene* sceneSemantic = importer2.ReadFile( semanticFile,
       // aiProcess_CalcTangentSpace       |
          aiProcess_Triangulate            |
          aiProcess_FlipUVs                //|
          //aiProcess_JoinIdenticalVertices
          //aiProcess_SortByPType
                                              );

    // If the import failed, report it
    if( !sceneColor  || (sceneColor->mFlags == AI_SCENE_FLAGS_INCOMPLETE) || (!sceneColor->mRootNode))
    {
      std::cout << importer.GetErrorString();

    }

    // scenes contain multiple meshes
    unsigned int numMeshes = sceneColor->mNumMeshes;

    std::cout << "Load meshes. This will take a while.." << std::endl;
    for(int m=0; m<numMeshes; m++){
       aiMesh *mesh = sceneColor->mMeshes[m];

       MeshItem tmesh;

       tmesh.initBuffers();
       tmesh.materialIndex = mesh->mMaterialIndex -1;

       aiVector3D *vertexStruct = mesh->mVertices;
       aiVector3D *texCoords = mesh->mTextureCoords[0];

       Vertex temp;
       glm::vec3 temp_pos;
       glm::vec2 temp_color;
       std::string label;
       cv::Vec3b l;

       // read all vertices and texture uv coords of a mesh
       for(int i = 0; i < mesh->mNumVertices; i++){

           temp_pos.x = vertexStruct[i].x;
           temp_pos.y = vertexStruct[i].y;
           temp_pos.z = vertexStruct[i].z;
           temp.pos = temp_pos;
           temp_color.x = texCoords[i].x;
           temp_color.y = texCoords[i].y;
           temp.texture = temp_color;

           tmesh.vertices.push_back(temp);
       }

       // read all faces of the mesh
       for(unsigned int i = 0; i < mesh->mNumFaces; i++)
       {
           aiFace face = mesh->mFaces[i];

           for(unsigned int j = 0; j < face.mNumIndices; j++){
               tmesh.indices.push_back(face.mIndices[j]);
           }

           bool found = findSemanticFace(sceneSemantic, vertexStruct,  face, label);
           // if the color face finds a corresponding face in the instance/semantic mesh, store the label
           if(found){
               tmesh.vertices[face.mIndices[0]].label = convertLabel(label, tmesh.vertices[face.mIndices[0]].room, instance2room);
               tmesh.vertices[face.mIndices[1]].label = convertLabel(label, tmesh.vertices[face.mIndices[1]].room, instance2room);
               tmesh.vertices[face.mIndices[2]].label = convertLabel(label, tmesh.vertices[face.mIndices[2]].room, instance2room);
           }
       }
       tmesh.numIndices = tmesh.indices.size();
       if((m+1) % 10 == 0)
           std::cout << (m+1) << " out of " << numMeshes << std::endl;
       mItem.push_back(tmesh);
    }

    // get texture paths and keep in mtextures
    for (unsigned int i = 1 ; i < sceneColor->mNumMaterials ; i++) {
        const aiMaterial* pMaterial = sceneColor->mMaterials[i];
        Texture temp_t;
        if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString Path;

            if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
                std::string FullPath = ROOT_DIR + "/3d/" + std::string(Path.data);
                temp_t.name = FullPath;
            }
            else{
                std::string FullPath = ROOT_DIR + "/3d/" + std::string(Path.data);
                std::cout << "Problem with: " <<  FullPath;
            }
        }
        mTextures.push_back(temp_t);
    }
}


/**
 * are two vertices equal?
 */
bool findVertex(aiVector3D vertexStruct, aiVector3D vertex){

    if(vertexStruct.x == vertex.x &&
       vertexStruct.y == vertex.y &&
       vertexStruct.z == vertex.z){
        return true;
    }
    return false;
}

/**
 * Returns true and updates the instance label if a triangle face is found in the semantic scene
 */
bool findSemanticFace(const aiScene *semantic, const aiVector3D *vertex, const aiFace f, std::string &label){

    for(int m = 0; m < semantic->mNumMeshes; m++){

        aiMesh *mesh = semantic->mMeshes[m];
        aiVector3D *vertexStruct = mesh->mVertices;
        aiMaterial *mat;
        aiString name;
        aiFace face;

        for(int i = 0; i < mesh->mNumFaces; i++){
            face = mesh->mFaces[i];
            if( findVertex(vertexStruct[face.mIndices[0]], vertex[f.mIndices[0]]) &&
                findVertex(vertexStruct[face.mIndices[1]], vertex[f.mIndices[1]]) &&
                findVertex(vertexStruct[face.mIndices[2]], vertex[f.mIndices[2]]) ) {

                //semantic vertex found
                mat = semantic->mMaterials[mesh->mMaterialIndex];
                mat->Get(AI_MATKEY_NAME, name);
                label = std::string(name.data);
                //std::cout << label << " "; //[0] << name->data[1] << " ";
                return true;
            }
        }
    }
    return false;
}


/**
 * Convert 4 channel representation of instance id to a unique scalar
 */
int instanceRGBtoID(cv::Vec4b v){
    return v[0] + v[1] * 256 + v[2] * 256 * 4;
}


/**
 * Convert scalar representation of instance id toa unique 4 channel representation
 */
cv::Vec4b instanceIDtoRGB(unsigned short id){
    cv::Vec4b v;
    v[2] = id / (256 * 4);
    int temp = id % (256 * 4);
    v[1] = temp / 256;
    v[0] = temp % 256;
    v[3] = 255;
    //std::cout << (ushort) v[0] << " " << (ushort) v[1] << " " <<(ushort) v[2] << " " <<(ushort) v[3] << " " <<id << std::endl;

    return v;
}


/**
 * Create glm percpective matrix from the camera intrinsics, near and far planes and image size
 */
glm::mat4 perspective_glm(double fx, double fy, double cx, double cy, double n, double f, double w, double h) {
    assert(f > n);

    glm::mat4 res = glm::mat4();

    res[0] = glm::vec4(2 * fx / w,         0,             -(2*(cx/w) - 1),                0);
    res[1] = glm::vec4(0,               - 2 * fy / h,       2*(cy/h) - 1,                 0);
    res[2] = glm::vec4(0,               0,                -(f+n)/(f-n),      -2*f*n/(f-n));
    res[3] = glm::vec4(0,               0,                -1,                0);

    // glm is column-major order
    return glm::transpose(res);
}


glm::vec3 convertLabel(const std::string label, std::string &room, std::map<int, std::string> &instance2room){

    // list of semantic classes from the official Stanford 2D-3D S dataset
    std::string semanticClasses[14] = {"chair", "door", "floor", "ceiling", "wall",
                                       "beam",   "column", "window", "table", "sofa",
                                       "bookcase", "board", "clutter", "<UNK>"};

    std::string labelSemantic, labelInstance, roomName, roomNumber;

    int index = -1;

    std::stringstream slabel(label);

    std::getline(slabel, labelSemantic, '_');
    std::getline(slabel, labelInstance, '_');
    std::getline(slabel, roomName, '_');
    std::getline(slabel, roomNumber, '_');

    room = roomName + "_" + roomNumber;

    std::stringstream ins(labelInstance);
    int x;
    ins >> x;

    for(int i = 0; i < 14; i++){
        if(semanticClasses[i] == std::string(labelSemantic)) {
            index = i;
            break;
        }
    }
    if(index == -1){
        std::cout << "problem";
    }

    glm::vec3 instance;
    instance.x = index;
    instance.y = x / 256;
    instance.z = x % 256;

    int inst = instance.z + instance.y * 256 + instance.x * 256 * 4;

    instance2room.insert(std::pair<int, std::string> (inst, room));

    return instance;
}

/**
 * Utility function to save print a glm matrix - for debugging
 */
void print_glm(glm::mat4 m){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            std::cout << m[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/**
 * read RGBA image from the opengl buffer and store in opencv format
 * optionally, save the image
 */
cv::Mat getColor(bool save, std::string area, std::string name){

    std::string nfile = name;

    cv::Mat image(SCR_HEIGHT, SCR_WIDTH, CV_8UC4);
    GLubyte *data = new GLubyte[SCR_HEIGHT * SCR_WIDTH * 4];
    int channels = 4;

    //glReadPixels( 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    for(int i=0; i<SCR_HEIGHT; i++){
        for(int j=0; j<SCR_WIDTH; j++){
            for(int c=0; c<channels-1; c++){
                image.at<cv::Vec4b>(SCR_HEIGHT - i - 1,j)[2-c] = data[channels*i*SCR_WIDTH + channels*j + c];
            }
            image.at<cv::Vec4b>(SCR_HEIGHT - i - 1,j)[3] = data[channels*i*SCR_WIDTH + channels*j + 3];
        }
    }

    delete[] data;

    if(save){
        std::string filename = OUT_DIR + area + "/rgb/" + nfile + ".png";
        saveImage(filename, image);
    }

    return image;
}

/**
 * read depth map from the opengl buffer and store in opencv format
 * optionally, save the image
 */
cv::Mat getDepth(bool save, std::string area, std::string name){

    std::string nfile = name;

    cv::Mat depth(SCR_HEIGHT, SCR_WIDTH, CV_32F);
    GLubyte *data = new GLubyte[SCR_HEIGHT * SCR_WIDTH * 4];

    // utilized 4 channels in the shader, for precise depth
    int channels = 4;

    //glReadPixels( 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    for(int i=0; i<SCR_HEIGHT; i++){
        for(int j=0; j<SCR_WIDTH; j++){

            depth.at<float>(SCR_HEIGHT - i - 1, j) =   float(data[channels*i*SCR_WIDTH + channels*j + 2])
                                                     + 256. * float(data[channels*i*SCR_WIDTH + channels*j + 0]);

        }
    }

    delete[] data;

    depth.convertTo(depth, CV_16U);

    if(save){
        std::string filename = OUT_DIR + area + "/depth/" + nfile + ".png";
        saveImage(filename, depth);
    }

    return depth;
}

/**
 * read instance labels from the opengl buffer and store in opencv format
 * optionally, save the image
 */
cv::Mat getLabel(bool save, std::string area, std::string name){

    std::string nfile = name;

    cv::Mat label(SCR_HEIGHT, SCR_WIDTH, CV_32F);
    cv::Mat image(SCR_HEIGHT, SCR_WIDTH, CV_8UC4);

    GLubyte *data = new GLubyte[SCR_HEIGHT * SCR_WIDTH * 4];
    int channels = 4;

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    for(int i=0; i<SCR_HEIGHT; i++){
        for(int j=0; j<SCR_WIDTH; j++){
            for(int c=0; c<channels-1; c++){
                image.at<cv::Vec4b>(SCR_HEIGHT - i - 1,j)[2-c] = data[channels*i*SCR_WIDTH + channels*j + c];
            }
            image.at<cv::Vec4b>(SCR_HEIGHT - i - 1,j)[3] = data[channels*i*SCR_WIDTH + channels*j + 3];

            label.at<float>(SCR_HEIGHT - i - 1,j) = instanceRGBtoID(image.at<cv::Vec4b>(SCR_HEIGHT - i - 1,j)) ;
        }
    }

    delete[] data;

    label.convertTo(label, CV_16U);

    if(save){
        std::string filename = OUT_DIR + area + "/instance/" + nfile + ".png";
        saveImage(filename, label);
    }

    return label;
}

/**
* Save depth map layers
*/
void saveDepthLayers(std::string area, std::vector<cv::Mat> depth, int frame){

    for(int i=0; i<depth.size(); i++){
        std::string filename = OUT_DIR + area + "/depth/" + std::to_string(frame) + "_" + std::to_string(i) + ".png";
        saveImage(filename, depth[i]);
    }
}

/**
* Save RGBA layers
*/
void saveColorLayers(std::string area, std::vector<cv::Mat> color, int frame){

    for(int i=0; i<color.size(); i++){
        std::string filename = OUT_DIR + area + "/rgb/" + std::to_string(frame) + "_" + std::to_string(i) + ".png";
        saveImage(filename, color[i]);
    }
}

/**
* Save instance segmentation layers
*/
void saveLabelLayers(std::string area, std::vector<cv::Mat> label, int frame)
{

    for(int i=0; i<label.size(); i++){
        std::string filename = OUT_DIR + area + "/instance/" + std::to_string(frame) + "_" + std::to_string(i) + ".png";
        saveImage(filename, label[i]);
    }
}

void saveImage(std::string filename, cv::Mat content)
{
    char file[50];
    strcpy(file, filename.c_str());
    cv::imwrite(file, content);
}
