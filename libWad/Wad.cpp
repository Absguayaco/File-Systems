#include "Wad.h"
#include <iostream>
void Wad::searchWad(Node* node, Node* newNode, std::vector<std::string> path){
  std::vector<std::string> matchString;
  matchString.push_back("/");

  unsigned int zero = 0;
  std::streampos currentPositionG = fileStream.tellg();
  std::streampos currentPositionP = fileStream.tellp();
  fileStream.seekg(descriptorOffset);
  if(node->getName() == "/"){
    fileStream.seekp(descriptorOffset + (numDescriptors * 16));
    currentPositionP = fileStream.tellp();
    if(newNode->getType() == "namespace"){
      std::string tempString = newNode->getName() + "_START";
      std::string tempString2 = newNode->getName() + "_END";
      fileStream.write(reinterpret_cast<const char*>(&zero), 4);
      fileStream.write(reinterpret_cast<const char*>(&zero), 4);
      fileStream.write(tempString.c_str(), 8);

      fileStream.write(reinterpret_cast<const char*>(&zero), 4);
      fileStream.write(reinterpret_cast<const char*>(&zero), 4);
      fileStream.write(tempString2.c_str(), 8);
      fileStream.flush();
      return;
    }
   else if(newNode->getType() == "file"){
      fileStream.write(reinterpret_cast<const char*>(&zero), 4);
      fileStream.write(reinterpret_cast<const char*>(&zero), 4);
      fileStream.write(newNode->getName().c_str(), 8);
      fileStream.flush();
      return;
   }
  return;
  }
  Node* curDirectory = tree->getRoot();
  for(unsigned int i = 0; i < numDescriptors; i++){
    std::regex pattern("E[0-9]M[0-9]");
    std::regex patternStart(".{1,2}_START");
    std::regex patternEnd(".{1,2}_END");
    std::smatch match;
    unsigned int offset;
    fileStream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
    if(std::regex_search(strName, match, patternStart)){
      matchString.push_back(strName.substr(0, strName.find('_')));
    }
    if(std::regex_search(strName, match, patternEnd)){
      bool areEqual = true;
      if(matchString.size() == path.size()){
        for(size_t i = 0; i < path.size(); i++){
            if(path[i] != matchString[i]){
              areEqual = false;
            }
          }
      }
      else{
        areEqual = false;
      }
      if(strName == node->getName() + "_END" && areEqual){
        //Copy and shift remaining forward
        fileStream.seekg(descriptorOffset + (i) * 16);
        char buffer[(numDescriptors - i) * 16];
        fileStream.read(buffer, sizeof(buffer));

        if(newNode->getType() == "namespace"){
	  fileStream.seekp(descriptorOffset + (i+2) * 16);
          fileStream.write(buffer, sizeof(buffer));
          fileStream.seekp(descriptorOffset + (i) * 16); 

          std::string tempString = newNode->getName() + "_START";
          std::string tempString2 = newNode->getName() + "_END";
          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(tempString.c_str(), 8);

          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(tempString2.c_str(), 8);
	  fileStream.flush();
          break;
        }
        else if(newNode->getType() == "file"){
	  fileStream.seekp(descriptorOffset + (i+1) * 16);
          fileStream.write(buffer, sizeof(buffer));
          fileStream.seekp(descriptorOffset + (i) * 16); 

          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(newNode->getName().c_str(), 8);
          fileStream.flush();
          break;
        }
      }
      matchString.pop_back();
    }
  }
}

void Wad::shiftWad(Node* node, Node* newNode, int length){
    }
Wad::Wad(const std::string &path) {
  fileStream.open(path, std::ios::in | std::ios::out |std::ios::binary);
  char nameArr[1] = {'/'};
  this->tree = new Tree(nameArr);
  if(!fileStream.is_open()){
  }
  fileStream.read(this->fileMagic, 4);

  fileStream.read(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
  fileStream.read(reinterpret_cast<char*>(&descriptorOffset), sizeof(descriptorOffset));

  std::regex pattern("E[0-9]M[0-9]");
  std::regex patternStart(".{1,2}_START");
  std::regex patternEnd(".{1,2}_END");
  std::smatch match;
  int count = 0;

  fileStream.seekg(descriptorOffset);
  Node* curDirectory = tree->getRoot();
  for(unsigned int i = 0; i < numDescriptors; i++){
    unsigned int offset;
    fileStream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
    if(std::regex_search(strName, match, patternStart)){
      char arr[2];
      auto charPos = strName.find('_');
      strcpy(arr, strName.substr(0, charPos).c_str());
      Node* node = new Node(offset, size, arr, "namespace");
      tree->insert(curDirectory, node);
      curDirectory = node;
    }
    else if(std::regex_search(strName, match, pattern)){ 
      Node* node = new Node(offset, size, name, "map");
      tree->insert(curDirectory, node);
      curDirectory = node;
      count += 10;
    }
    else if (std::regex_search(strName, match, patternEnd)){
      if(curDirectory->getName() == "/"){
        continue;
      }
      curDirectory = curDirectory->getParent();
      if(curDirectory == nullptr){
	 curDirectory = tree->getRoot();
      }
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

Wad::~Wad(){
  fileStream.close();
  delete tree;
}
Wad* Wad::loadWad(const std::string &path){
  Wad* temp = new Wad(path);
  return temp;
}

std::string Wad::getMagic(){
  return fileMagic;
}

bool Wad::isContent(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return false;
  }
  if(node->getType() != "file"){
    return false;
  }
  return true;
}


bool Wad::isDirectory(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  if(path.empty()){
    return false;
  }
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
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
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return -1;
  }
  if(node->getType() != "file"){
    return -1;
  }
  return node->getLength();
}

int Wad::getContents(const std::string &path, char *buffer, int length, int offset){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return -1;
  }
  if(node->getType() != "file"){
    return -1;
  }

  if(length + offset > node->getLength()){
    length = node->getLength() - offset;
  }
  fileStream.seekg(node->getOffset() + offset); 
  int bytesRead = fileStream.read(buffer, length).gcount();
  return bytesRead;
}

int Wad::getDirectory(const std::string &path, std::vector<std::string> *directory){
  std::vector<std::string> splitString = intepretPath(path);
  if(splitString.empty()){
    return -1;
  }
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
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
  else{
    return -1;
  }
  
  return count;
}

void Wad::createDirectory(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  if(splitString.back().size() > 2){
    return;
  }
  std::string name = splitString.back();
  if(name.size() > 2){
    return;
  }
  splitString.pop_back();

  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return;
  }
  char arr[name.length() + 1];
  Node* newNode = new Node(0, 0, strcpy(arr, name.c_str()) , "namespace");
  tree->insert(node, newNode);
  searchWad(node, newNode, splitString);

  numDescriptors += 2;
  std::streampos currentPosition = fileStream.tellg();

  unsigned int size = 0;
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
  fileStream.seekp(4);
  fileStream.write(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
  fileStream.flush();
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size),sizeof(size));
  fileStream.flush();

  fileStream.seekp(descriptorOffset);
  fileStream.seekg(descriptorOffset);

  for(unsigned int i = 0; i < numDescriptors; i++){
    unsigned int desOffset;
    fileStream.read(reinterpret_cast<char*>(&desOffset), sizeof(desOffset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
  }
}

void Wad::createFile(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  std::regex pattern("E[0-9]M[0-9]");
  std::regex patternStart(".{1,2}_START");
  std::regex patternEnd(".{1,2}_END");
  std::smatch match;
  if(splitString.back().size() > 8){
   return;
  }
  std::string name = splitString.back();
  splitString.pop_back();

  if(std::regex_search(name, match, patternStart) || std::regex_search(name, match, patternEnd) || std::regex_search(name, match, pattern)){ 
    return;
  }

  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return;
  }
  if(node->getType() == "map" || node->getType() == "file"){
    return;
  }
  char arr[name.length() + 1]; 
  Node* newNode = new Node(0, 0, strcpy(arr, name.c_str()) , "file");
  tree->insert(node, newNode);
  searchWad(node, newNode, splitString);

  numDescriptors += 1;
  unsigned int size = 0;
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
  fileStream.seekp(4);
  fileStream.write(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
  fileStream.flush();
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size),sizeof(size));
  fileStream.flush();

    fileStream.seekp(descriptorOffset);                                                                                                  fileStream.seekg(descriptorOffset);
}

int Wad::writeToFile(const std::string &path, const char *buffer, int length, int offset){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return -1;
  }
  if(node->getType() != "file"){
    return -1;
  }
  Node* tempNode = node;
  if(node->getLength() != 0){
    return 0;
  }
  fileStream.seekg(descriptorOffset);

  //shifts descriptor list forward
  char readBuffer[numDescriptors * 16];
  fileStream.read(readBuffer, sizeof(readBuffer));

  //write lump data
  buffer += offset;
  fileStream.seekp(descriptorOffset);
  int beforeWrite = fileStream.tellp();

  int bytesWritten = fileStream.write(buffer, length).tellp();
  bytesWritten -= beforeWrite;
  int newOffset = descriptorOffset + bytesWritten;
  fileStream.flush();

  //write data back
  fileStream.write(readBuffer, sizeof(readBuffer));
  fileStream.flush();

  //Change Header
  fileStream.seekp(8);
  fileStream.write(reinterpret_cast<char*>(&newOffset), sizeof(newOffset));
  fileStream.flush();

  //Change descriptor
  fileStream.seekp(newOffset);
  fileStream.seekg(newOffset);
  for(unsigned int i = 0; i < numDescriptors; i++){
    unsigned int desOffset;
    fileStream.read(reinterpret_cast<char*>(&desOffset), sizeof(desOffset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
    if(strName == node->getName() && size == 0){
      fileStream.seekp(newOffset + i * 16);
      fileStream.write(reinterpret_cast<char*>(&descriptorOffset), sizeof(newOffset));
      fileStream.write(reinterpret_cast<char*>(&bytesWritten), sizeof(bytesWritten));
      fileStream.flush();
      break;
    }
  }
  node->setOffset(descriptorOffset);
  node->setLength(bytesWritten);

  descriptorOffset = newOffset;

  return bytesWritten;
}
