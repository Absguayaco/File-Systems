#include <vector>
#include <cstring> // Add this line to include the <cstring> header

class Node{
    Node* parent;
    std::vector<Node*> children;
    int offset = 0;
    int length = 0;
    char charData[8];
    int data = 0;
    std::string type;
    public:
      Node(int data){
        this->data = data;
      }
      Node(int offset, int length, char data[], std::string type = "file"){
        this->offset = offset;
        this->length = length;
        strcpy(this->charData, data); 
        this->type = type;
      }
    friend class Tree;
  };

class Tree{
  public:
    Node* root;
    Tree(int data){
      root = new Node(data);
    }
    void insert(Node* parent, Node* node){
      parent->children.push_back(node);
      node->parent = parent;
    }
    Node* depthFirstSearch(Node* node, int data){
      if(node->data == data){
        return node;
      }
      for(Node* child : node->children){
        Node* result = depthFirstSearch(child, data);
        if(result != nullptr){
          return result;
        }
      }
      return nullptr;
    }
};