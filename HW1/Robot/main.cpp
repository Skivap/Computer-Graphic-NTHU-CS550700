/* Shared Library */
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/type_ptr.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
/* Standard Library */
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <list>
#include <cstdint>
#include <iomanip>
#include <unordered_map>
#include <algorithm>

/* Shared variables */
GLFWwindow* window;
/* Vertex Shader */

const char* vertexShaderSource = R"(
#version 410 core
layout (location = 0) in vec3 iv3vertex;
layout (location = 1) in vec2 iv2tex_coord;
layout (location = 2) in vec3 iv3normal;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform mat4 proj;
uniform mat4 view;

uniform mat4 translate;
uniform mat4 translateBack;
uniform mat4 rotation;
uniform mat4 translate2;
uniform mat4 translateBack2;
uniform mat4 rotation2;

void main()
{
    gl_Position = proj * view * translateBack2 * rotation2 * translate2 * translateBack * rotation * translate * vec4(iv3vertex, 1.0);
    vertexData.texcoord = iv2tex_coord;
}
)";

/* Fragment Shader */

const char* fragmentShaderSource = R"(
#version 410 core

layout(location = 0) out vec4 diffuseColor;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D myTexture;
//out vec4 FragColor;

void main(){
    vec4 texColor = texture(myTexture, vertexData.texcoord);
    diffuseColor = texColor;
    //diffuseColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
)";

const int SCREEN_HEIGHT = 800;
const int SCREEN_WIDTH = 800;
std::pair<double, double> mouseCursor;
float wdControl = 0.0f;
bool pause = false;

/* DEFINE */
#define MATH_PI 3.14159265358979323846

/* TEXTURE */

static std::unordered_map<std::string, GLuint> textures2D;

GLuint loadTextureImage(const char* path, int num_chanels) {

    /* If image texture already loaded, don't load it again */
    if (textures2D.find(path) != textures2D.end()) {
        return textures2D[path];
    }

    GLuint texture;

    int width;
    int height;
    int n;
    stbi_set_flip_vertically_on_load(true); // vertical mirror image data

    // [TODO] : Change the desired channel dynamically
    unsigned char* data = stbi_load(path, &width, &height, &n, num_chanels);

    if (!data) {
        std::cerr << "Failed to load image: " << path << '\n';
        std::exit(1);
    }

    GLenum format;
    if (n == 1) format = GL_RED;
    else if (n == 3) format = GL_RGB;
    else if (n == 4) format = GL_RGBA;
    else {
        std::cerr << "Image format not supported: " << path << std::endl;
        std::exit(1);
    }

    std::cout << "Loaded image with width: " << width << " height: " << height << " channels: " << n << std::endl;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Loaded image, " << path << '\n';
    textures2D[path] = texture;
    return texture;
}

// Default Loading texture image from MTL file
GLuint loadTextureImage(const tinyobj::material_t& mat, int num_chanels) {
    if (mat.diffuse_texname.empty()) {
        std::cout << "Failed to render Texture, use sample texture instead" << '\n';
        return loadTextureImage("texture/sample.jpg", num_chanels);
    }
    return loadTextureImage(mat.diffuse_texname.c_str(), num_chanels);
}

/* OBJECT */

class Object {
public:
    typedef struct _MeshData
    {
        // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
        // If no data return empty vector
        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<float> texcoords;
        std::vector<unsigned int> indices;
        std::vector<unsigned char> num_vertices;
        std::vector<int> material_ids; // per-face material ID
    } MeshData;

    typedef struct _MeshRender {
        GLuint vao, vbo, ebo;
        GLuint materialId, indexCount;
    } MeshRender;

    std::vector<GLuint> textureMap;
    std::vector<MeshRender> data;

    Object(const char* const objFilePath, GLuint shaderProgram);
    virtual void update(float deltaTime);
    virtual void draw();
};

/* ROBOT */

class Robot : public Object
{
public:
    /* Only for inner arms and leg */
    GLuint uniformRotate;
    GLuint uniformTranslate;
    GLuint uniformTranslateBack;
    /* Only for outer arms or leg */
    GLuint uniformRotate2;
    GLuint uniformTranslate2;
    GLuint uniformTranslateBack2;

    /* Only for inner arms and leg */
    std::vector<glm::mat4> translationMatrix;
    std::vector<glm::mat4> rotationMatrix;
    std::vector<glm::mat4> translationMatrixBack;
    /* Only for outer arms or leg */
    std::vector<glm::mat4> translationMatrix2;
    std::vector<glm::mat4> rotationMatrix2;
    std::vector<glm::mat4> translationMatrixBack2;

    Robot(const char* const objFilePath, GLuint shaderProgram);
    void update(float deltaTime) override;
    void draw() override;
};

class PSYball : public Object
{
public:
    GLuint uniformRotate;
    GLuint uniformTranslate;
    GLuint uniformTranslateBack;
    GLuint uniformRotate2;
    GLuint uniformTranslate2;
    GLuint uniformTranslateBack2;

    glm::mat4 translationMatrix;
    glm::mat4 rotationMatrix;
    glm::mat4 identityMatrix;

    PSYball(const char* const objFilePath, GLuint shaderProgram);
    void update(float deltaTime) override;
    void draw() override;
};

/* ENGINE HPP */
class Camera
{
private:
    GLuint uniformView;
    GLuint uniformProj;

    GLuint program;

    glm::mat4 view;
    glm::mat4 projection;

    double height;

public:
    Camera();
    Camera(GLuint program);
    void update(float deltaTime);
    void draw();
};


class Engine {
private:
    GLuint shaderProgram;

    std::list<Object*> objects;

    Camera camera;

public:
    Engine();
    void Update(float deltaTime);
    void Render();
};

int main(void)
{

    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "Failed to initialize the library" << '\n';
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to load windows" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Initialize the glad library */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Engine engine;

    std::cout << "==================================\n";
    std::cout << "Start Rendering" << '\n';
    /* Loop until the user closes the window */

    float lastTime = glfwGetTime(); // Last time
    float frameTimeCounter = 0.0f;
    unsigned int frames = 0; // Variable to store frames
    
    // [TODO] : Update the frame limit
    // [Refs] : https://stackoverflow.com/questions/57800608/how-to-render-at-a-fixed-fps-in-a-glfw-window
    // [Note] : A variable FPS limit is declared in shared.hpp

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        frameTimeCounter += deltaTime;

        /* Mouse Position */
        glfwGetCursorPos(window, &mouseCursor.first, &mouseCursor.second);

        /* Update */
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) wdControl += deltaTime;
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) wdControl -= deltaTime;
        static int spaceState = GLFW_RELEASE;
        if (spaceState == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)pause = !pause;
        spaceState = glfwGetKey(window, GLFW_KEY_SPACE);
        wdControl = glm::clamp<float>(wdControl, -1.0f, 0.5f);
        engine.Update(deltaTime);

        /* Render */
        engine.Render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        frames++;

        // TODO: FIX FLUSH
        if (frameTimeCounter >= 1.0f) {
            std::cout << "\rFrames : " << frames << '\n' << std::flush;
            frameTimeCounter = 0.0f, frames = 0;
        }
    }

    glfwTerminate();
    return 0;
}

static GLuint createShaderProgram(const char** vertexSource, const char** fragmentSource);


/* Modify by your own preferences */
Engine::Engine() {
    /* Shader Program */
    shaderProgram = createShaderProgram(&vertexShaderSource, &fragmentShaderSource);

    /* Texture Uniform */
    glUniform1i(glGetUniformLocation(shaderProgram, "myTexture"), 0);

    /* Object Load */
    Object* robot = new Robot("myrobot.obj", shaderProgram);
    objects.push_back(robot);
    Object* Ball = new PSYball("obj/Sphere.obj", shaderProgram);
    objects.push_back(Ball);

    /* GL CLEAR COLOR */
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    /* Camera */
    camera = Camera(shaderProgram);
}

void Engine::Update(float deltaTime) {
    camera.update(deltaTime);
    for (Object* object : objects) {
        object->update(deltaTime);
    }
}

/* Modify by your own preferences */
void Engine::Render() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glEnable(GL_DEPTH_TEST);

    camera.draw();

    /* Every Objet */
    for (Object* object : objects) {
        object->draw();
    }

    glUseProgram(0);
}

static GLuint createShaderProgram(const char** vertexSource, const char** fragmentSource) {

    GLuint vertexShader, fragmentShader, shaderProgram;

    // create vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, vertexSource, NULL);
    glCompileShader(vertexShader);

    // create fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // create shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // delete vertex & fragment shader
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;

}

Object::Object(const char* const objFilePath, GLuint shaderProgram) {

    /* OBJECT LOAD */
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objFilePath);

    // Error handling
    if (!err.empty()) {
        std::cerr << "Error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "Load Models ! Shapes size " << shapes.size() << " Material size " << materials.size() << '\n';

    std::vector<MeshData> meshes;
    for (int s = 0; s < shapes.size(); ++s) { // for each shape
        MeshData mesh;
        int index_offset = 0;
        for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) { // for each triangle
            int fv = shapes[s].mesh.num_face_vertices[f];
            for (int v = 0; v < fv; ++v) { // for each vertice
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                if (idx.vertex_index != -1) { // vertice positions
                    mesh.positions.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    mesh.positions.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    mesh.positions.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                }
                if (idx.texcoord_index != -1) { // vertice texcoord
                    mesh.texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    mesh.texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                }
                // TODO: num_vertices, normals, material_ids...
                if (idx.normal_index != -1) { // normals
                    mesh.normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    mesh.normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    mesh.normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                /* COLORS
                tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
                */

                mesh.indices.push_back(index_offset + v);
            }
            mesh.material_ids.push_back(shapes[s].mesh.material_ids[f]);
            index_offset += fv;

        }
        meshes.push_back(mesh);
    }

    std::cout << "Finished loading mesh, now compiling it..." << '\n';

    for (int i = 0; i < shapes.size(); i++)
    {
        MeshRender temp_data;
        glGenVertexArrays(1, &temp_data.vao); //Generate vao
        glBindVertexArray(temp_data.vao); //Bind vao

        glGenBuffers(1, &temp_data.vbo); //Generate vbo
        glBindBuffer(GL_ARRAY_BUFFER, temp_data.vbo); //Bind vbo

        size_t positionSize = meshes[i].positions.size() * sizeof(float);
        size_t textureSize = meshes[i].texcoords.size() * sizeof(float);
        size_t normalsSize = meshes[i].normals.size() * sizeof(float);
        size_t indicesSize = meshes[i].indices.size() * sizeof(unsigned int);

        std::cout << positionSize << " " << textureSize << " " << normalsSize << '\n';

        glBufferData(GL_ARRAY_BUFFER, (positionSize + textureSize + normalsSize), NULL, GL_STATIC_DRAW); //Get vbo space
        //				GL ARRAY BUFFER | INIT						| SIZE			| ARRAY
        glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, meshes[i].positions.data());
        glBufferSubData(GL_ARRAY_BUFFER, positionSize, textureSize, meshes[i].texcoords.data());
        glBufferSubData(GL_ARRAY_BUFFER, positionSize + textureSize, normalsSize, meshes[i].normals.data()); //Load data into buffer

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); //Set attribute of data
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(positionSize + textureSize));
        glEnableVertexAttribArray(2);

        glGenBuffers(1, &temp_data.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, temp_data.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, meshes[i].indices.data(), GL_STATIC_DRAW);

        /*
        std::cout << "===========================================\n";
        std::cout << "ARRAY" << meshes[i].positions.size() << " " << meshes[i].texcoords.size() << " " << meshes[i].normals.size() <<"\n";
        for (int j = 0; j < meshes[i].texcoords.size(); j+=2) {
            std::cout << meshes[i].texcoords[j] << " " << meshes[i].texcoords[j + 1] << '\n';
        }
        std::cout << "===========================================\n";
        std::vector<float> debug;
        debug.resize(positionSize);
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, debug.data());
        for (int j = 0; j < positionSize / sizeof(float); j += 3) {
            std::cout << debug[j] << " " << debug[j + 1] << " " << debug[j + 2] << '\n';
        }
        debug.clear();
        debug.resize(textureSize);
        glGetBufferSubData(GL_ARRAY_BUFFER, positionSize, textureSize, debug.data());
        for (int j = 0; j < textureSize / sizeof(float); j += 2) {
            std::cout << debug[j] << " " << debug[j + 1] << '\n';
        }
        debug.clear();
        debug.resize(normalsSize);
        glGetBufferSubData(GL_ARRAY_BUFFER, positionSize + textureSize, normalsSize, debug.data());
        for (int j = 0; j < normalsSize / sizeof(float); j += 3) {
            std::cout << debug[j] << " " << debug[j + 1] << " " << debug[j + 2] << '\n';
        }
        std::cout << "===========================================\n"; */

        //TextureLoader cube("cube.png");
        //TextureLoader cube("texture/sample.jpg");
        temp_data.materialId = shapes[i].mesh.material_ids[0]; // cube.texture; //
        temp_data.indexCount = shapes[i].mesh.indices.size();
        //std::cout << "MAT ID: " << shapes[i].mesh.material_ids[0] << '\n';

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        data.push_back(temp_data);
    }

    for (auto& tmp : data) {
        std::cout << tmp.materialId << std::endl;
    }

    /* Object with no MTL File */
    if (materials.size() == 0) {
        GLuint texture = loadTextureImage("texture/psy.png", 4);
        textureMap.push_back(texture);

        std::cout << "Finish Compiling Object" << '\n';
        for (auto& tmp : data) {
            tmp.materialId = 0;
        }
    }

    /* Object with MTL File */
    for (int i = 0; i < materials.size(); i++) {
        GLuint texture = loadTextureImage(materials[i], 3);
        textureMap.push_back(texture);
    }
}

void Object::update(float deltaTime) {

}

void Object::draw() {

    for (int i = 0; i < data.size(); i++) {

        MeshRender& object = data[i];

        /* Texture*/
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId]);

        /* Bind */
        glBindVertexArray(object.vao);

        /* Templete to add: Set Uniform */

        /* TODO: DRAW */
        // glDrawArrays(GL_TRIANGLES, 0, object.indexCount);
        glDrawElements(GL_TRIANGLES, object.indexCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}


Camera::Camera() {

}

Camera::Camera(GLuint program) {
    this->program = program;
    uniformView = glGetUniformLocation(program, "view");
    uniformProj = glGetUniformLocation(program, "proj");
    height = 1.0;
}

void Camera::update(float deltaTime) {
    static float rotation = 0.0f;
    static const float distance = 2.0f;

    /* Mouse Function */
    static bool firstClick = true;
    static float sens = 10.0f;
    static std::pair<double, double> lastCur;
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstClick) {
            lastCur = mouseCursor;
            firstClick = false;
        }
        else {
            float deltaX = static_cast<float>(mouseCursor.first - lastCur.first);
            float deltaY = static_cast<float>(mouseCursor.second - lastCur.second);

            rotation += deltaX * sens * deltaTime;
            height = glm::clamp<float>(height + deltaY * sens * deltaTime, 0.5f - distance, 0.5f + distance);
            lastCur = mouseCursor;
        }
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        firstClick = true;
        rotation += deltaTime;
    }
    rotation = fmod(rotation, 2 * MATH_PI);

    /* Uniform Proj + View */
    view = glm::lookAt(glm::vec3(distance * cos(rotation), height, distance * sin(rotation)), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
}

void Camera::draw() {
    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uniformProj, 1, GL_FALSE, glm::value_ptr(projection));
}


static float lerp(float start, float finish, float progress) {
    // Progress from 0 to 1
    return start * (1.0f - progress) + finish * progress;
}

Robot::Robot(const char* const objFilePath, GLuint shaderProgram) : Object(objFilePath, shaderProgram) {
    /* Only for inner arms and leg */
    uniformRotate = glGetUniformLocation(shaderProgram, "rotation");
    uniformTranslate = glGetUniformLocation(shaderProgram, "translate");
    uniformTranslateBack = glGetUniformLocation(shaderProgram, "translateBack");
    /* Only for outer arms or leg */
    uniformRotate2 = glGetUniformLocation(shaderProgram, "rotation2");
    uniformTranslate2 = glGetUniformLocation(shaderProgram, "translate2");
    uniformTranslateBack2 = glGetUniformLocation(shaderProgram, "translateBack2");

    /* Resize Matrix */
    translationMatrix.resize(9);
    translationMatrixBack.resize(9);
    rotationMatrix.resize(9);
    translationMatrix2.resize(9);
    translationMatrixBack2.resize(9);
    rotationMatrix2.resize(9);

    /* Stay */
    for (int i = 0; i < 9; i++) {
        translationMatrix[i] =
            translationMatrixBack[i] =
            rotationMatrix[i] =
            translationMatrix2[i] =
            translationMatrixBack2[i] =
            rotationMatrix2[i] =
            glm::mat4(1.0f);
    }

}

void Robot::draw() {


    /* Robot Draw */
    for (int i = 0; i < data.size(); i++) {

        MeshRender& object = data[i];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId]);

        glBindVertexArray(object.vao);

        int idx;

        if (i == 0) idx = 3; // L1 Arm Left
        else if (i == 4) idx = 4; // L1 Arm Right

        else if (i == 7) idx = 5; // R2 Arm
        else if (i == 8) idx = 6; // Forget anymore

        else if (i == 1) idx = 2; // R2 Leg
        else if (i == 5) idx = 8;

        else if (i == 2) idx = 1; // R1 Leg
        else if (i == 6) idx = 7;

        else idx = 0;

        glUniformMatrix4fv(uniformTranslate, 1, GL_FALSE, glm::value_ptr(translationMatrix[idx]));
        glUniformMatrix4fv(uniformTranslateBack, 1, GL_FALSE, glm::value_ptr(translationMatrixBack[idx]));
        glUniformMatrix4fv(uniformRotate, 1, GL_FALSE, glm::value_ptr(rotationMatrix[idx]));
        glUniformMatrix4fv(uniformTranslate2, 1, GL_FALSE, glm::value_ptr(translationMatrix2[idx]));
        glUniformMatrix4fv(uniformTranslateBack2, 1, GL_FALSE, glm::value_ptr(translationMatrixBack2[idx]));
        glUniformMatrix4fv(uniformRotate2, 1, GL_FALSE, glm::value_ptr(rotationMatrix2[idx]));

        /* TODO: DRAW */
        // glDrawArrays(GL_TRIANGLES, 0, object.indexCount);
        glDrawElements(GL_TRIANGLES, object.indexCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}

void Robot::update(float deltaTime) {
    /* Matrix Update */
    static float counter = 0.0f;
    if (!pause)
        counter = fmod(counter + (deltaTime + std::max(wdControl * 0.01f, 0.0f)) * 3.0f, 2.0f);

    // Angle Mapping
        /* Legs */
    static const float r1LeftLegAngle[] = { 45.0f, -75.0f, 45.0f };
    static const float r2LeftLegAngle[] = { 90.0f, 0.0f, 90.0f };

    static const float r1RightLegAngle[] = { -75.0f, 45.0f, -75.0f };
    static const float r2RightLegAngle[] = { 0.0f, 90.0f, 0.0f };
    /* Arms */
    static const float r1LeftArmAngle[] = { 0.0f, -90.0f, 0.0f };
    static const float r2LeftArmAngle[] = { 0.0f, -45.0f, 0.0f };

    static const float r1RightArmAngle[] = { -90.0f, 0.0f, -90.0f };
    static const float r2RightArmAngle[] = { -45.0f, 0.0f, -45.0f };

    // Inner Leg --> idx[1]
    float legInnerAngleLeft = lerp(r1LeftLegAngle[(int)floorf(counter)], r1LeftLegAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[1] = glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.44f, 0.0f));
    translationMatrixBack[1] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.44f, 0.0f));
    rotationMatrix[1] = glm::rotate(glm::mat4(1.0f), glm::radians(legInnerAngleLeft), glm::vec3(1.0f, 0.0f, 0.0f));

    // Outer Leg 2 --> idx[2]
    float legOuterAngleLeft = lerp(r2LeftLegAngle[(int)floorf(counter)], r2LeftLegAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[2] = glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.25f, 0.0f));
    translationMatrixBack[2] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.25f, 0.0f));
    rotationMatrix[2] = glm::rotate(glm::mat4(1.0f), glm::radians(legOuterAngleLeft), glm::vec3(1.0f, 0.0f, 0.0f));
    translationMatrix2[2] = translationMatrix[1];
    translationMatrixBack2[2] = translationMatrixBack[1];
    rotationMatrix2[2] = rotationMatrix[1];

    // Inner Leg --> idx[7]
    float legInnerAngleRight = lerp(r1RightLegAngle[(int)floorf(counter)], r1RightLegAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[7] = glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.44f, 0.0f));
    translationMatrixBack[7] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.44f, 0.0f));
    rotationMatrix[7] = glm::rotate(glm::mat4(1.0f), glm::radians(legInnerAngleRight), glm::vec3(1.0f, 0.0f, 0.0f));

    // Outer Leg 2 --> idx[8]
    float legOuterAngleRight = lerp(r2RightLegAngle[(int)floorf(counter)], r2RightLegAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[8] = glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 0.25f, 0.0f));
    translationMatrixBack[8] = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.25f, 0.0f));
    rotationMatrix[8] = glm::rotate(glm::mat4(1.0f), glm::radians(legOuterAngleRight), glm::vec3(1.0f, 0.0f, 0.0f));
    translationMatrix2[8] = translationMatrix[7];
    translationMatrixBack2[8] = translationMatrixBack[7];
    rotationMatrix2[8] = rotationMatrix[7];

    // Left Inner Arm --> idx[3]
    float armInnerAngleLeft = lerp(r1LeftArmAngle[(int)floorf(counter)], r1LeftArmAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[3] = glm::translate(glm::mat4(1.0f), -glm::vec3(0.1f, 0.64f, 0.0f));
    translationMatrixBack[3] = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, 0.64f, 0.0f));
    rotationMatrix[3] = glm::rotate(glm::mat4(1.0f), glm::radians(-armInnerAngleLeft), glm::vec3(0.0f, 0.0f, 1.0f));

    // Left Outer Arm --> idx[5]
    float armOuterAngleLeft = lerp(r2LeftArmAngle[(int)floorf(counter)], r2LeftArmAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[5] = glm::translate(glm::mat4(1.0f), -glm::vec3(0.25f, 0.64f, 0.0f));
    translationMatrixBack[5] = glm::translate(glm::mat4(1.0f), glm::vec3(0.25f, 0.64f, 0.0f));
    rotationMatrix[5] = glm::rotate(glm::mat4(1.0f), glm::radians(-armOuterAngleLeft), glm::vec3(0.0f, 0.0f, 1.0f));
    translationMatrix2[5] = translationMatrix[3];
    translationMatrixBack2[5] = translationMatrixBack[3];
    rotationMatrix2[5] = rotationMatrix[3];

    // Right Inner Arm --> idx[4]
    float armInnerAngleRight = lerp(r1RightArmAngle[(int)floorf(counter)], r1RightArmAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[4] = glm::translate(glm::mat4(1.0f), -glm::vec3(-0.1f, 0.64f, 0.0f));
    translationMatrixBack[4] = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1f, 0.64f, 0.0f));
    rotationMatrix[4] = glm::rotate(glm::mat4(1.0f), glm::radians(armInnerAngleRight), glm::vec3(0.0f, 0.0f, 1.0f));

    // Right Outer Arm --> idx[6]
    float armOuterAngleRight = lerp(r2RightArmAngle[(int)floorf(counter)], r2RightArmAngle[(int)ceilf(counter)], fmod(counter, 1.0f));
    translationMatrix[6] = glm::translate(glm::mat4(1.0f), -glm::vec3(-0.25f, 0.64f, 0.0f));
    translationMatrixBack[6] = glm::translate(glm::mat4(1.0f), glm::vec3(-0.25f, 0.64f, 0.0f));
    rotationMatrix[6] = glm::rotate(glm::mat4(1.0f), glm::radians(armOuterAngleRight), glm::vec3(0.0f, 0.0f, 1.0f));
    translationMatrix2[6] = translationMatrix[4];
    translationMatrixBack2[6] = translationMatrixBack[4];
    rotationMatrix2[6] = rotationMatrix[4];

}

PSYball::PSYball(const char* const objFilePath, GLuint shaderProgram) : Object(objFilePath, shaderProgram) {
    /* Only for inner arms and leg */
    uniformRotate = glGetUniformLocation(shaderProgram, "rotation");
    uniformTranslate = glGetUniformLocation(shaderProgram, "translate");
    uniformTranslateBack = glGetUniformLocation(shaderProgram, "translateBack");
    /* Only for outer arms or leg */
    uniformRotate2 = glGetUniformLocation(shaderProgram, "rotation2");
    uniformTranslate2 = glGetUniformLocation(shaderProgram, "translate2");
    uniformTranslateBack2 = glGetUniformLocation(shaderProgram, "translateBack2");

    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, -1.5f));
    rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    identityMatrix = glm::mat4(1.0f);
}

void PSYball::update(float deltaTime) {
    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, (-1.5f + wdControl)));
}

void PSYball::draw() {
    /* Robot Draw */
    for (int i = 0; i < data.size(); i++) {

        MeshRender& object = data[i];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureMap[object.materialId]);
        glBindVertexArray(object.vao);

        /* Uniform */
        glUniformMatrix4fv(uniformTranslate, 1, GL_FALSE, glm::value_ptr(identityMatrix));
        glUniformMatrix4fv(uniformTranslateBack, 1, GL_FALSE, glm::value_ptr(translationMatrix));
        glUniformMatrix4fv(uniformRotate, 1, GL_FALSE, glm::value_ptr(rotationMatrix));
        glUniformMatrix4fv(uniformTranslate2, 1, GL_FALSE, glm::value_ptr(identityMatrix));
        glUniformMatrix4fv(uniformTranslateBack2, 1, GL_FALSE, glm::value_ptr(identityMatrix));
        glUniformMatrix4fv(uniformRotate2, 1, GL_FALSE, glm::value_ptr(identityMatrix));

        glDrawElements(GL_TRIANGLES, object.indexCount, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }
}