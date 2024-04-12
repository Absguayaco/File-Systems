#include "Wad.h"

Wad::Wad(const std::string &path) : fileStream(path){
  if(!fileStream.is_open()){
    std::cerr << "Error: Could not open file " << path << std::endl;
  }
  fileStream.read(this->fileMagic, 4);

  fileStream.read(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));//check if this works without this

  fileStream.read(reinterpret_cast<char*>(&descriptorOffset), sizeof(descriptorOffset));

  //TODO:: Construct tree here
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
    std::smatch match;
    if(strName.find("_START") != std::string::npos){ // add E1M1 marker for this seperate if statement, maybe change to regex instead
      Node* node = new Node(offset, size, name, "namespace");
      tree->insert(curDirectory, node);
      curDirectory = node;
    }
    else if(std::regex_search(strName, match, pattern)){ //maybe add cstr to strName here
      Node* node = new Node(offset, size, name, "map");
      tree->insert(curDirectory, node);
      curDirectory = node;    
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