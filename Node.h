#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

class Node {
  public:
   glm::vec3 position;
   bool used;
   Node(){};
   Node(float x, float y, float z){
     position = glm::vec3(x,y,z);
   }
   void print(){
     std::cout << position[0] << " " << position[1] << " " << position[2] << "\n";
   }
   void use(){ used = true;}
   void unuse(){ used = false;}
   float x(){return position.x;};
   float y(){return position.y;};
   float z(){return position.z;};
};
