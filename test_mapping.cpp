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
    Camera camera(glm::vec3(20.0f, 30.0f, 15.0f));
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
  vector<Triangle*> triangles = {};
  Node *curr_origin = nullptr;
  Node *curr_other = nullptr;
  Node *next_origin = nullptr;
  Node *next_other = nullptr;
  Node *third = nullptr;
  Node *init_origin = nullptr;
  Node *init_other = nullptr;
  vector<float> vertices;

void cleanup_vector(vector<Node*> &vec){
  std::cout << "cleaning up nodes\n";
  for(auto& it : vec){
    delete it;
  }
  std::cout << "done cleaning up nodes\n";
}

void cleanup_triangles(){
  std::cout << "cleaning up triangles\n";
  for(auto& it : triangles){
    delete it;
  }
  std::cout << "done cleaning up triangles\n";
}

void cleanup(){
  std::cout << "about to cleanup\n";
  /*
  //std::cout << "Curr address " << curr_origin << " " << &curr_origin << "\n";
  delete &curr_origin;
  //std::cout << "Curr address " << curr_origin << " " << &curr_origin << "\n";
  //std::cout << "curr_origin deleted\n";
  delete curr_other;
  //std::cout << "curr_other deleted\n";
  //std::cout << "Address " << next_origin << " " << &next_origin << "\n";
  //delete next_origin;
  //std::cout << "next_origin deleted\n";
  delete next_other;
  //std::cout << "next_other deleted\n";*/
  //delete this_level;
  cleanup_triangles();
  cleanup_vector(nodes);
  //cleanup_vector(this_level);
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
  //std::cout << "this_level deleted\n";
  //cleanup_vector(next_level);
  //std::cout << "next_level deleted\n";
  std::cout << "cleanup done\n";
}

void getSlice(ifstream &coords, vector<Node*>* &slice){
  std::cout << "*****************************Getting slice**********************************\n";
  slice = new vector<Node*>;
  Node* prev = nullptr;
  while(true){
    //std::cout << "\n/// Reading coords ///\n";
    getline(coords, coordX);
    //std::cout << "Coord X: " << coordX;
    if(coordX == "===") break;
    getline(coords, coordY);
    getline(coords, coordZ);
    //std::cout << ", Coord Y: " << coordY << ", Coord Z: " << coordZ << "\n";
    x = std::stof(coordX);
    y = std::stof(coordY);
    z = std::stof(coordZ);
    nodes.push_back(new Node(x,y,z));
    slice->push_back(nodes[nodes.size()-1]);
  }
	std::cout << "X: " << x << ", Y: " << y << ", Z: " << z << std::endl;
  int x = 0;
  for(auto& it : *slice){
    if(x+1 == slice->size()) break;
    //std::cout<<"-------\n" << &((*slice)[x]) << "\n";
    //std::cout << &it << "\n";
    //(&it)->print();
    (*slice)[x]->next = (*slice)[x+1];
    x += 1; 
  }
  (*slice)[slice->size()-1]->next = (*slice)[0];
  //std::cout << "**********\n";
  //for(auto& it : *slice){
    //(&it)->print();
    //(&it)->next->print();
    //x += 1; 
  //}
  //std::cout << "**********\n";
};
/*
    1. curr_same = init_node
    2. curr_other = mas cercano a curr_same de la otra capa
    2.1. init_other = curr_other
    # Empieza algoritmo
    3. Nodo más cercano a curr_same = next_same
    4. Nodo más cercano a curr_other = next_other
    5. Diagonal más pequeña entre (next_other -> curr_same) y (next_same -> curr_other) queda
    6. Dibujar triangulo
    7. curr_same = punta de llegada de diagonal
    8. curr_other = punta de salida
    Clausula de escape if ambos esan en current
 */
void getClosestNode(Node* &origin, vector<Node*>* &opposite, Node* &closest){
  float minDist = 500.0;
  Node* temp = &(*closest);
  //std::cout << "GETTING CLOSEST\n";
  //std::cout << opposite << "\n";
  for(auto& it : *opposite){
    // if(it.used) continue;
    auto distance = glm::length(origin->position - it->position);
    //std::cout << "Comparing ";
    //it.print();
    // if(minDist <= distance) continue;
    if(minDist <= distance || it == origin) continue;
    minDist = distance;
    closest = it;
    // //std::cout << "Curr closest is ";
    //closest->print();
  }
  /*if(temp == closest){
    //std::cout << "temp is equal to closest\n";
    if(origin->position[2] == init_other->position[2]) closest = init_other;
    else closest = init_origin;
  }*/
}
void buildTriangle(){
  std::cout << "BUILDING TRIANGLE\n";
  //getClosestNode(curr_origin, this_level, next_origin);
  //std::cout << "next origin: "; next_origin->print();
  //getClosestNode(curr_other, next_level, next_other);
  std::cout << "next origin: "; next_origin->print();
  int alv;
  std::cout << "next other: "; next_other->print();
  curr_other->print();
  std::cout << bool(curr_origin->used == true) << " " << bool(curr_other->used == true) << "\n";
  auto distance_from_origin = glm::length(next_other->position - curr_origin->position);
  auto distance_from_other = glm::length(next_origin->position - curr_other->position);
  if(curr_origin->z() == 15.5 || curr_other->z() == 15.5){
    std::cout << curr_origin << " " << init_origin << "\n";
    curr_origin->print();
    std::cout << curr_origin->x() << " " << curr_origin->y() << "\n";
    std::cout << (curr_origin->x() == 28.4) << (curr_origin->y() == 29.3);
    cin >> alv;
  }
  if(curr_origin->x() == 29.2 && curr_origin->y() == 27.9){
    std::cout << curr_origin << " " << init_origin << "\n";
    cin >>alv;
  }
  if (curr_origin==init_origin && curr_origin->used) distance_from_origin = 500;
  if (curr_other==init_origin && curr_other->used) distance_from_other = 500;
  if (curr_other==init_other && curr_other->used) distance_from_other = 500;
  if (curr_origin==init_other && curr_origin->used) distance_from_origin = 500;
  std::cout << "comparing from origin " << distance_from_origin << " vs from other " << distance_from_other << "\n";
  if(distance_from_origin <= distance_from_other){
    third = next_other;
    triangles.push_back(new Triangle(curr_origin, curr_other, third));
    curr_other = curr_origin;
    vector<Node*>* temp = next_level;
    next_level = this_level;
    this_level = temp;
  }
  else{
    third = next_origin;
    triangles.push_back(new Triangle(curr_origin, curr_other, third));
  }
  curr_origin = third;
  std::cout << "Chosen as third: "; third->print();
}

void joinLevels(){
  curr_origin = (*this_level)[0];
  std::cout << "ORIGIN IS "; curr_origin->print();
  getClosestNode(curr_origin, next_level, curr_other);
  init_origin = curr_origin;
  init_other = curr_other;
  std::cout << "CHOSEN AS CLOSEST OTHER "; curr_other->print();
  std::cout << "Joining levels\n";
  int rounds = 0;int x;
  //((*next_level)[0]).print();
  do{
    std::cout << "================================\n";
    std::cout << "curr origin: "; curr_origin->print();
    //std::cout << triangles.size() << "\n";
    std::cout << "curr other: "; curr_other->print();
    std::cout << "Initial curr origin: "; init_origin->print();
    std::cout << "Initial curr other: "; init_other->print();
    //if(curr_origin != init_origin || rounds == 0)
      next_origin = curr_origin->next;
    //else next_origin = curr_origin;
    //if(curr_other != init_other || rounds == 0)
      next_other = curr_other->next;
    //else next_other = curr_other;
    buildTriangle();
    if(((curr_origin == init_origin && curr_other == init_other) || (curr_origin == init_other && curr_other == init_origin)) && rounds != 0) break;
    if(((curr_origin == init_origin && curr_other == init_other) || (curr_origin == init_other && curr_other == init_origin)) && triangles.size() > 0) {
      std::cout << "in if\n";
      cin >> x;
      break;
    }
    std::cout << "ASdasda;sldaskDJLASDJA" << curr_origin->z() << " " << curr_other->z() <<"\n";
    //cin >> x;
    //cin.ignore();
    if(curr_origin->z() == 15.5 || curr_other->z() == 15.5){
      //std::cout<<"este cin\n";
      //cin >> x;
    }
    //std::cout << "=============== Triangles size" << triangles.size() << "\n";
    //std::cout << "Curr origin: "; curr_origin->print();
    //std::cout << "Curr other: "; curr_other->print();
    rounds += 1;
  }while(true);
}

void renderQuad()
{
  std::cout << "Entering renderQuad\n";
  cleanup();
  vertices.clear();
  ifstream coords("../coords3.txt");
  //coords.seekg(0, ios::beg);
  int slice = 1;
  getSlice(coords, next_level);
  while(coords.peek() != EOF){
    ++slice;
    //std::cout << "in while\n";
    //std::cout << "Peek:" << coords.peek() << "\n";
    delete this_level;
    this_level = next_level;
    //std::cout << "\nAddresses are " << this_level << " " << next_level << "\n";
    getSlice(coords, next_level);
    std::cout << "Slices gotten\n";
    //std::cout << "\nAddresses are " << this_level << " " << next_level << "\n";
    std::cout << this_level->size() << " " << next_level->size() << "\n";
    joinLevels();
    for(auto& it: triangles) it->insertObject(vertices);
		std::cout << vertices.size() << "\n";
		std::cout << triangles.size() << "\n";
    std::cout << "Peeking: " << coords.peek() << " after slices " << slice-1 << " and " << slice << "\n";
  }
	//int x; cin >> x;
  std::cout << "######################### Levels joined, rendering #############################\n";
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
    glDrawArrays(GL_TRIANGLES, 0, 3*triangles.size());
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
