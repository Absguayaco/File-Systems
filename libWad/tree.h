#include <vector>
#include <iostream>
#include <string>
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
      strncpy(this->name, data, sizeof(name));
      //this->name[sizeof(name-1] = '\0';
      this->type = type;
      if(type == "namespace"){

	}
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
  void setOffset(int offset){
    this->offset = offset;
  }
   void setName(std::string name){
	std::strcpy(this->name, name.c_str());
   }
   void setLength(int length){
     this->length = length;
   }
};

class Tree{
  private:
    Node* root;
  public:
    Tree(char data[]): root(new Node(0, 0, data, "namespace")){
    }
    void printNodes(Node* node){
    if(node == nullptr){
        return;
      }
    for(Node* child: node ->getChildren()){
        printNodes(child);
      }
    }
    
    void insert(Node* parent, Node* node){
	  if(parent == nullptr){
		  return;
	  }
	  if(node == nullptr){
		  return;
	  }
    if(root->getName() != "/"){
      root->setName("/");
    }
    parent->children.push_back(node);
    node->parent = parent;
    }

    Node* depthFirstSearch(Node* node, std::vector<std::string> path, size_t index = 0){
      if(node == nullptr || index >= path.size()){
        return nullptr;
      }
      if(path[path.size()-1] == node->name){
	      return node;
      }
      if(node->name == path[index]){
        if(index == path.size()-1){
	        return node;
	      }
	      if(!node->children.empty()){
          for(Node* child : node->children){
            if(child->name == path[index +1]){
              Node* result = depthFirstSearch(child, path, index + 1);
              if(result != nullptr){
                return result;
              }
	         }
	        }
        }
      }
      return nullptr;
    }
    void setRoot(Node* root){
	this->root = root;
     }
     Node* getRoot(){
	return root;
     }
};

static std::vector<std::string> intepretPath(std::string path){ 
  std::vector<std::string> splitString;
  std::string delimiter = "/";
  size_t pos = 0;
  std::string token;
  if(path.empty()){
   return splitString;
  }
  splitString.push_back("/");
  if(path == "/"){
    return splitString;
  }
  size_t start = 1;
  pos = path.find(delimiter);
  while (pos != std::string::npos) {
    if(pos > start)
    splitString.push_back(path.substr(start, pos - start));
    start = pos + 1;
    pos  = path.find(delimiter, start);
  }
  if(start < path.length()){
    splitString.push_back(path.substr(start));
  }
  return splitString;
}
