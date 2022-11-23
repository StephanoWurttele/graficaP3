#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <learnopengl/filesystem.h>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "Triangle.h"

#include <iostream>
#include <fstream>
#include <vector>

// general settings 
  // methods definition
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void processInput(GLFWwindow *window);
    unsigned int loadTexture(const char *path);
    void renderQuad();

  // settings
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

  // camera
    Camera camera(glm::vec3(30.0f, 30.0f, 80.0f));
    float lastX = (float)SCR_WIDTH / 2.0;
    float lastY = (float)SCR_HEIGHT / 2.0;
    bool firstMouse = true;

  // timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

void cleanup();

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        //std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        //std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader("../4.normal_mapping.vs", "../4.normal_mapping.fs");

    // load textures
    // -------------
    unsigned int diffuseMap = loadTexture("../resources/textures/brickwall.jpg");
    unsigned int normalMap  = loadTexture("../resources/textures/brickwall_normal.jpg");

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseMap", 0);
    shader.setInt("normalMap", 1);

    // lighting info
    // -------------
    glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // configure view/projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // render normal-mapped quad
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("lightPos", lightPos);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        renderQuad();

        // render light source (simply re-renders a smaller plane at the light's position for debugging/visualization)
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.1f));
        shader.setMat4("model", model);
        renderQuad();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    cleanup();
    return 0;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
  unsigned int quadVAO = 0;
  unsigned int quadVBO;
// triangulation build
  string coordX, coordY, coordZ;
  float x, y, z;
  vector<Node*>* this_level = nullptr;
  vector<Node*>* next_level = nullptr;
  vector<Node*> nodes = {};
  vector<float> vertices;
  vector<Triangle*> triangles = {};
  Node *curr_origin = nullptr;
  Node *curr_other = nullptr;
  Node *next_origin = nullptr;
  Node *next_other = nullptr;
  Node *third = nullptr;
  Node *init_origin = nullptr;
  Node *init_other = nullptr;

void cleanup_vector(vector<Node*> &vec){
  for(auto& it : vec){
    delete it;
  }
}
void cleanup_triangles(){
  for(auto& it : triangles){
    delete it;
  }
}
void cleanup(){
  cleanup_triangles();
  cleanup_vector(nodes);
  delete this_level;
  delete next_level;
  this_level = nullptr;
  next_level = nullptr;
  nodes = {};
  triangles = {};
  curr_origin = nullptr;
  curr_other = nullptr;
  next_origin = nullptr;
  next_other = nullptr;
  third = nullptr;
  init_origin = nullptr;
  init_other = nullptr;
}
void getSlice(ifstream &coords, vector<Node*>* &slice){
  //std::cout << "****************Getting slice*******************\n";
  slice = new vector<Node*>;
  Node* prev = nullptr;
  int xtemp = 0;
  while(true){
    //std::cout << "\n/// Reading coords ///\n";
    getline(coords, coordX);
    if(coordX == "===") break;
    getline(coords, coordY);
    getline(coords, coordZ);
    x = std::stof(coordX);
    y = std::stof(coordY);
    z = std::stof(coordZ);
    nodes.push_back(new Node(x,y,z));
    slice->push_back(nodes[nodes.size()-1]);
  }
	//std::cout << "X: " << x << ", Y: " << y << ", Z: " << z << std::endl;
  for(auto& it : *slice){
    if(xtemp+1 == slice->size()) break;
    (*slice)[xtemp]->next = (*slice)[xtemp+1];
    xtemp += 1; 
  }
  (*slice)[slice->size()-1]->next = (*slice)[0];
};
void getClosestNode(Node* &origin, vector<Node*>* &opposite, Node* &closest){
  float minDist = 500.0;
  Node* temp = &(*closest);
  for(auto& it : *opposite){
    auto distance = glm::length(origin->position - it->position);
    if(minDist <= distance || it == origin) continue;
    minDist = distance;
    closest = it;
  }
}
void buildTriangle(){
  int alv;
  auto distance_from_origin = glm::length(next_other->position - curr_origin->position);
  auto distance_from_other = glm::length(next_origin->position - curr_other->position);
  if (curr_origin==init_origin && curr_origin->used) distance_from_other = 500;
  if (curr_other==init_other && curr_other->used) distance_from_origin = 500;
  if(distance_from_origin <= distance_from_other){
    third = next_other;
    triangles.push_back(new Triangle(curr_origin, curr_other, third));
    curr_other->use();
    curr_other = next_other;
  }
  else{
    third = next_origin;
    triangles.push_back(new Triangle(curr_origin, curr_other, third));
    curr_origin->use();
    curr_origin = next_origin;
  }
  //std::cout << "Chosen as third: "; third->print();
}

void joinLevels(){
  curr_origin = (*this_level)[0];
  getClosestNode(curr_origin, next_level, curr_other);
  init_origin = curr_origin;
  init_other = curr_other;
  int rounds = 0;int x;
  do{
    next_origin = curr_origin->next;
    next_other = curr_other->next;
    buildTriangle();
    if(((curr_origin == init_origin && curr_other == init_other)) && rounds != 0) break;
    rounds += 1;
  }while(true);
}

void renderQuad()
{
  //std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!Entering renderQuad!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
  cleanup();
  vertices.clear();
  ifstream coords("../coords5.txt");
  int slice = 1;
  int xtemp = 0;
  getSlice(coords, next_level);
  while(coords.peek() != EOF){
    ++slice;
    delete this_level;
    this_level = next_level;
    getSlice(coords, next_level);
    //std::cout <<"Levels sizes: " << this_level->size() << " " << next_level->size() << "\n";
    joinLevels();
    for(auto& it: triangles) it->insertObject(vertices);
  }
  //std::cout << "######################### Levels joined, rendering #############################\n";
    if (quadVAO == 0)
    {
      float* quadVertices =  &vertices[0];
      // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    std::cout << "Triangles size for render: " << triangles.size() << "\n";
    std::cout << "Vertices  size for render: " << vertices.size() << "\n";
    std::cout << "--------------------------\n";
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()/14);
    glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        //std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
