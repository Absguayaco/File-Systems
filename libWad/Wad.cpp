#include "Wad.h"

Wad::Wad(const std::string &path) : fileStream(path){
  Tree* tempTree = new Tree("/");
  this->tree = tempTree;
  delete tempTree;
  if(!fileStream.is_open()){
    std::cerr << "Error: Could not open file " << path << std::endl;
  }
  fileStream.read(this->fileMagic, 4);

  fileStream.read(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));

  fileStream.read(reinterpret_cast<char*>(&descriptorOffset), sizeof(descriptorOffset));

  fileStream.ignore(descriptorOffset - 12); 
  Node* curDirectory = tree->root;
  for(int i = 0; i < numDescriptors; i++){
    unsigned int offset;
    fileStream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);

    std::regex pattern("E[0-9]M[0-9]");
    std::regex patternStart("[A-Z0-9]{1,2}_START");
    std::regex patternEnd("[A-Z0-9]{1,2}_END");
    std::smatch match;
    int count = 0;
    if(std::regex_search(strName, match, patternStart)){ 
      Node* node = new Node(offset, size, name, "namespace");
      tree->insert(curDirectory, node);
      curDirectory = node;
    }
    else if(std::regex_search(strName, match, pattern)){ //maybe add cstr to strName here
      Node* node = new Node(offset, size, name, "map");
      tree->insert(curDirectory, node);
      curDirectory = node;    
      count = 9;
    }
    else if (std::regex_search(strName, match, patternEnd)){
      Node* node = new Node(offset, size, name, "map"); //maybe dont create node here, maybe unecessary test after done
      tree->insert(curDirectory, node);
      curDirectory = curDirectory->getParent();  

    }
    else if(count != 0){
      count--;
      Node* node = new Node(offset, size, name);
      tree->insert(curDirectory, node);
      if(count == 0 || count % 10 == 0){
        curDirectory = curDirectory->getParent();
      }
    }
    else{
      Node* node = new Node(offset, size, name);
      tree->insert(curDirectory, node);
    }
  }
}

Wad* Wad::loadWad(const std::string &path){

  Wad* temp = new Wad(path);
  return temp;
}

std::string Wad::getMagic(){
  return fileMagic;
}

bool Wad::isContent(const std::string &path){
  //TODO: fix depth first search here, repeat for isDirectory
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return false;
  }
  if(node->getType() != "file"){
    return false;
  }
  return true;
}


bool Wad::isDirectory(const std::string &path){
  //TODO: fix depth first search here, repeat for isDirectory
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return false;
  }
  if(node->getType() == "map" || node->getType() == "namespace"){
    return true;
  }
  return false;
}

int Wad::getSize(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return -1;
  }
  return node->getLength();
}

int Wad::getContents(const std::string &path, char *buffer, int length, int offset){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return -1;
  }
  if(node->getType() != "file"){
    return -1;
  }

  fileStream.seekg(node->getOffset() + offset);
  int bytesRead = fileStream.read(buffer, length).gcount();
  return bytesRead;
}

int Wad::getDirectory(const std::string &path, std::vector<std::string> *directory){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return -1;
  }
  int count = 0;
  if(node->getType() == "map" || node->getType() == "namespace"){
    for(Node* child : node->getChildren()){
      directory->push_back(child->getName());
      count++;
  }
  }
  
  return count;
}