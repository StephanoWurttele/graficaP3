#include "Node.h"

class Triangle {
  public:
    Node* origin;
    Node* other;
    Node* third;
    glm::vec3 tangent, bitangent, edge1, edge2, nm;
    glm::vec2 uv1, uv2, uv3;
    Triangle(){};
    Triangle(Node* _origin, Node* _other, Node* _third){
      origin = _origin; other = _other; third = _third;
      uv1 = glm::vec2(0.0f, 1.0f); uv2 = glm::vec2(0.0f, 0.0f); uv3 = glm::vec2(1.0f, 0.0f);
      origin->use(); other->use(); third->use();

      edge1 = other->position - origin->position;
      edge2 = third->position - origin->position;
      glm::vec2 deltaUV1 = uv2 - uv1;
      glm::vec2 deltaUV2 = uv3 - uv1;

      float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
      tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
      tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
      tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

      bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
      bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
      bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

      nm = glm::normalize(glm::cross(edge1, edge2));

    }
    float orx(){ return origin->x();}
    float oty(){ return origin->y();}
    float z(){ return origin->z();}
    // insertion stuff
      void insertNormal(vector<float> &vec){
        vec.push_back(nm.x);
        vec.push_back(nm.y);
        vec.push_back(nm.z);
      }
      void insertTangent(vector<float> &vec){
        vec.push_back(tangent.x);
        vec.push_back(tangent.y);
        vec.push_back(tangent.z);
      }
      void insertBitangent(vector<float> &vec){
        vec.push_back(bitangent.x);
        vec.push_back(bitangent.y);
        vec.push_back(bitangent.z);
      }
      void insertOrigin(vector<float> &vec){
        vec.push_back(origin->x());
        vec.push_back(origin->y());
        vec.push_back(origin->z());
        insertNormal(vec);
        vec.push_back(uv1.x);
        vec.push_back(uv1.y);
        insertTangent(vec);
        insertBitangent(vec);
      }
      void insertOther(vector<float> &vec){
        vec.push_back(other->x());
        vec.push_back(other->y());
        vec.push_back(other->z());
        insertNormal(vec);
        vec.push_back(uv2.x);
        vec.push_back(uv2.y);
        insertTangent(vec);
        insertBitangent(vec);
      }
      void insertThird(vector<float> &vec){
        vec.push_back(third->x());
        vec.push_back(third->y());
        vec.push_back(third->z());
        insertNormal(vec);
        vec.push_back(uv3.x);
        vec.push_back(uv3.y);
        insertTangent(vec);
        insertBitangent(vec);
      }
      void insertObject(vector<float> &vec){
        insertOrigin(vec);
        origin->unuse();
        insertOther(vec);
        other->unuse();
        insertThird(vec);
        third->unuse();
      }
};
