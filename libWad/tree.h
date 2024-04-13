#include <vector>
#include <cstring> // Add this line to include the <cstring> header

class Node{
  private:
    Node* parent;
    std::vector<Node*> children;
    int offset = 0;
    int length = 0;
    char name[8];
    int data = 0;
    std::string type;
  public:
    Node(int data){
      this->data = data;
    }
    Node(int offset, int length, char data[], std::string type = "file"){
      this->offset = offset;
      this->length = length;
      strcpy(this->name, data); 
      this->type = type;
    }
    friend class Tree;
    Node* getParent(){
      return this->parent;
    }
    std::string getName(){
      return this->name;
    }
    std::string getType(){
      return this->type;
    }
    int getLength(){
      return this->length;
    }
    int getOffset(){
      return this->offset;
    }
    std::vector<Node*> getChildren(){
      return this->children;
  }
};

class Tree{
  public:
    Node* root;
    Tree(char data[]){
      root = new Node(0, 0, data, "directory");
    }
    void insert(Node* parent, Node* node){
      parent->children.push_back(node);
      node->parent = parent;
    }
    Node* depthFirstSearch(Node* node, std::vector<std::string> path){
      if(node->name == path[0]){
        return node;
      }
      for(Node* child : node->children){
        Node* result = depthFirstSearch(child, path);
        if(result != nullptr){
          return result;
        }
      }
      return nullptr;
    }
};

std::vector<std::string> intepretPath(std::string path){ 
  std::vector<std::string> splitString;
  std::string delimiter = "/";
  size_t pos = 0;
  std::string token;
  if(path[0] != '/'){
    return splitString;
  }
  while ((pos = path.find(delimiter)) != std::string::npos) {
    splitString.push_back(path.substr(0, pos));
    path.erase(0, pos + delimiter.length());
  }
}