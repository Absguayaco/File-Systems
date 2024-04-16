#include "Wad.h"
#include <iostream>
void Wad::searchWad(Node* node, Node* newNode, std::vector<std::string> path){
  std::vector<std::string> matchString;
  matchString.push_back("/");

  unsigned int zero = 0;
  std::streampos currentPositionG = fileStream.tellg();
  std::streampos currentPositionP = fileStream.tellp();
//  std::cout << "Current position in searchWad write: " << currentPositionP << std::endl;
  fileStream.seekg(descriptorOffset);
  // std::cout << "Current position in searchWad Read: " << currentPositionG << std::endl;
  if(node->getName() == "/"){
    //std::cout << "writing in condition" << std::endl;
    fileStream.seekp(descriptorOffset + (numDescriptors * 16));
    currentPositionP = fileStream.tellp();
   // std::cout << "Current position in searchWad write2: " << currentPositionP << std::endl;
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
      fileStream.write(newNode->getName().c_str(), 8);//maybe add strcpy for these
      fileStream.flush();
      return;
   }
  return;
  }
  Node* curDirectory = tree->getRoot();
   for(size_t i = 0; i < path.size(); i++){
          std::cout << " Path: " << path[i] << std::endl;
   }
  for(unsigned int i = 0; i < numDescriptors; i++){
     std::cout << "=----------------------=" << std::endl;
     for(size_t i = 0; i < matchString.size(); i++){
          std::cout << "Matchstring: " << matchString[i] << std::endl;
     }
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
	  std::cout << "Matchstring: " << matchString[i] << "-- Path: " << path[i] << std::endl;
          if(path[i] != matchString[i]){
            areEqual = false;
          }
        }
      }
      else{
        areEqual = false;
      }
      if(strName == node->getName() + "_END" && areEqual){
 //       std::cout <<  "Original Node parent: " << node->getParent()->getName() << std::endl;
        //Copy and shift remaining forward
        fileStream.seekg(descriptorOffset + (i) * 16);
        char buffer[(numDescriptors - i) * 16];
        fileStream.read(buffer, sizeof(buffer));

        if(newNode->getType() == "namespace"){
	  fileStream.seekp(descriptorOffset + (i+2) * 16);
          fileStream.write(buffer, sizeof(buffer));
          fileStream.seekp(descriptorOffset + (i) * 16); //this line

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
          fileStream.seekp(descriptorOffset + (i) * 16); //this line

          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(reinterpret_cast<const char*>(&zero), 4);
          fileStream.write(newNode->getName().c_str(), 8);//maybe add strcpy for these
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
  fileStream.open(path, std::ios::in | std::ios::out);
  char nameArr[1] = {'/'};
  this->tree = new Tree(nameArr);
  if(!fileStream.is_open()){
    std::cerr << "Error: Could not open file " << path << std::endl;
  }
  fileStream.read(this->fileMagic, 4);

  fileStream.read(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
  std::cout << "num descriptors on creation: " << numDescriptors << std::endl;
  fileStream.read(reinterpret_cast<char*>(&descriptorOffset), sizeof(descriptorOffset));

  std::regex pattern("E[0-9]M[0-9]");
  std::regex patternStart(".{1,2}_START");
  std::regex patternEnd(".{1,2}_END");
  std::smatch match;
  int count = 0;

  fileStream.seekg(descriptorOffset);
  Node* curDirectory = tree->getRoot();
//  std::cout << "error after root settting" << std::endl;
  for(unsigned int i = 0; i < numDescriptors; i++){
  //   std::cout << "error with loop, Iteration: " << i << std::endl;
    unsigned int offset;
    fileStream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    //std::cout << "Error on first reaf" << std::endl;
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
    //std::cout << "Error after string name" << std::endl;
    if(std::regex_search(strName, match, patternStart)){
//	std::cout << "Error in pattern start" << std::endl;
      char arr[2];
      auto charPos = strName.find('_');
      strcpy(arr, strName.substr(0, charPos).c_str());
      Node* node = new Node(offset, size, arr, "namespace"); //if this works naem should onyl be first two
      tree->insert(curDirectory, node);
      curDirectory = node;
    }
    else if(std::regex_search(strName, match, pattern)){ //maybe add cstr to strName here
  //    std::cout << "Error in pattern" << std::endl;
      Node* node = new Node(offset, size, name, "map");
      tree->insert(curDirectory, node);
      curDirectory = node;
      count += 10;
    }
    else if (std::regex_search(strName, match, patternEnd)){
    //   std::cout << "Error on parent check" << std::endl;
      //Node* node = new Node(offset, size, name, "namespace"); //maybe dont create node here, maybe unecessary test after done
      //tree->insert(curDirectory, node);
//	std::cout << "Current directory before ahift" << curDirectory->getName() << std::endl;
      if(curDirectory->getName() == "/"){
        continue;
      }
      curDirectory = curDirectory->getParent();
  //    std::cout << "Error after parent check" << std::endl;
      if(curDirectory == nullptr){
    //    std::cout << "cause of error here parent null" << std::endl;
	 curDirectory = tree->getRoot();
      }
    }
    else if(count != 0){
      //std::cout << "Error in count check" << std::endl;
      count--;
      Node* node = new Node(offset, size, name);
      tree->insert(curDirectory, node);
      if(count == 0 || count % 10 == 0){
        curDirectory = curDirectory->getParent();
      }
    }
    else{
      //std::cout << "Error on node file" << std::endl;
      Node* node = new Node(offset, size, name);
      //std::cout << "Error after node creation" << std::endl;
      //std::cout << "curDirectory: " << curDirectory->getName() << std::endl;
      tree->insert(curDirectory, node);
      //std::cout << "Error in file" << std::endl;
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
//  for(int i = 0; i < splitString.size(); i++){
  //   std::cout << splitString[i] << std::endl;
  //}
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return false;
  }
  //std::cout << "Node type content: " << node ->getType() << std::endl;
  if(node->getType() != "file"){
    return false;
  }
  return true;
}


bool Wad::isDirectory(const std::string &path){
  //tree->printNodes(tree->getRoot());
  std::vector<std::string> splitString = intepretPath(path);
  if(path.empty()){
    return false;
  }
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  for(int i = 0; i < splitString.size(); i++){
    std::cout << splitString[i] << std::endl;
  }
  if(node == nullptr){
    return false;
  }
  //std::cout << "Node type Directory: " << node->getType() << std::endl;
  if(node->getType() == "map" || node->getType() == "namespace"){
    return true;
  }
  return false;
}

int Wad::getSize(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
     std::cout << "node is nullptr" << std::endl;
    return -1;
  }
  if(node->getType() != "file"){
    std::cout << "node is not file" << std::endl;
    return -1;
  }
  std::cout << "Node name: " << node->getName() << std::endl;
  std::cout << "node length" << node->getLength() << std::endl;
 // tree->printNodes(tree->getRoot());
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
  fileStream.seekg(node->getOffset() + offset); //TODO: check that this works properly
  int bytesRead = fileStream.read(buffer, length).gcount();
  return bytesRead;
}

int Wad::getDirectory(const std::string &path, std::vector<std::string> *directory){
  //tree->printNodes(tree->getRoot());
  std::vector<std::string> splitString = intepretPath(path);
  if(splitString.empty()){
    return -1;
  }
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    std::cout << "node in nullptr" << std::endl;
    return -1;
  }
  //std::cout << "Node get: " << node->getName() << std::endl;
  int count = 0;
  //std::cout << "Descriptor count: " << numDescriptors << std::endl;
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
  std::cout << "root name = " << tree->getRoot()->getName() << std::endl;
  if(splitString.back().size() > 2){
    std::cout << "string too long" << std::endl;
    std::cout << "root name = " << tree->getRoot()->getName() << std::endl;
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
  std::cout << "Node Name: " << node->getName() << std::endl;
  Node* newNode = new Node(0, 0, strcpy(arr, name.c_str()) , "namespace");
  std::cout << "NewNode Name: " << newNode->getName() << std::endl;
  tree->insert(node, newNode);
  searchWad(node, newNode, splitString);

  numDescriptors += 2;
  std::streampos currentPosition = fileStream.tellg();
  std::cout << "current Position" << currentPosition << std::endl;

  unsigned int size = 0;
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
  std::cout << "Descripot numbers before writing plz wokr: " << size << std::endl;
  fileStream.seekp(4);
  fileStream.write(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
  fileStream.flush();
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size),sizeof(size));
  std::cout << "Descripotr after write plzzzz: " << size << std::endl;
  fileStream.flush();

  fileStream.seekp(descriptorOffset);
  fileStream.seekg(descriptorOffset);
  //std::cout << "Node name is: " << node->getName() << std::endl;
  //std::cout << "Node parent is: " << node->getParent() << std::endl;
  for(unsigned int i = 0; i < numDescriptors; i++){
    unsigned int desOffset;
    fileStream.read(reinterpret_cast<char*>(&desOffset), sizeof(desOffset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
    std::cout << "String name is: " << strName << std::endl;
    //if(strName == node->getName() && node->getLength() == 0){
      //std::cout << "name found in Wad" << std::endl;
      //fileStream.seekp(newOffset + i * 16);
      //fileStream.write(reinterpret_cast<char*>(&descriptorOffset), sizeof(newOffset));
      //fileStream.write(reinterpret_cast<char*>(&bytesWritten), sizeof(bytesWritten));
      //fileStream.flush();
   // }
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
  std::cout << "Descripot numbers before writing plz wokr: " << size << std::endl;
  fileStream.seekp(4);
  fileStream.write(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
  fileStream.flush();
  fileStream.seekg(4);
  fileStream.read(reinterpret_cast<char*>(&size),sizeof(size));
  std::cout << "Descripotr after write plzzzz: " << size << std::endl;
  fileStream.flush();

    fileStream.seekp(descriptorOffset);                                                                                                  fileStream.seekg(descriptorOffset);
  //std::cout << "Node name is: " << node->getName() << std::endl;
  //std::cout << "Node parent is: " << node->getParent() << std::endl;
  //for(unsigned int i = 0; i < numDescriptors; i++){
    //unsigned int desOffset;
    //fileStream.read(reinterpret_cast<char*>(&desOffset), sizeof(desOffset));
    //unsigned int size;
    //fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    //char name[8];
    //fileStream.read(name, 8);
    //std::string strName(name);
    //std::cout << "String name is: " << strName << std::endl;
  //}
}

int Wad::writeToFile(const std::string &path, const char *buffer, int length, int offset){
  std::vector<std::string> splitString = intepretPath(path);
  //for(int i = 0; i < splitString.size(); i++){
   // std::cout << "Split Strinf elem: " << splitString[i] << std::endl;
  //}
  Node* node = tree->depthFirstSearch(tree->getRoot(), splitString);
  if(node == nullptr){
    return -1;
  }
  if(node->getType() != "file"){
    return -1;
  }
 // std::cout << "Node Name 12: " << node->getName() << std::endl;
  //std::cout << "Node lenggth 12: " << node->getLength() << std::endl;
  Node* tempNode = node;
  //while(tempNode->getName() != "/"){
    //std::cout << "Node PARENT: " << " " << tempNode->getParent()->getName() << std::endl;
    //tempNode = tempNode->getParent();
  //}
  if(node->getLength() != 0){
    std::cout << "this length error" << std::endl;
    //return node->getLength();
    return 0;
  }
  //TODO: write a lot here

  fileStream.seekg(descriptorOffset);

  //shifts descriptor list forward
  char readBuffer[numDescriptors * 16];
  fileStream.read(readBuffer, sizeof(readBuffer));
  //fileStream.seekp(descriptorOffset + length);
  //fileStream.write(readBuffer, sizeof(readBuffer));
  //fileStream.flush();

  //write lump data
  buffer += offset;
  fileStream.seekp(descriptorOffset);
  int beforeWrite = fileStream.tellp();
  std::cout << "Pointer bf write: " << beforeWrite << std::endl;

  int bytesWritten = fileStream.write(buffer, length).tellp();
   std::cout << "Pointer after write: " << fileStream.tellp() << std::endl;
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
  //std::cout << "Node name is: " << node->getName() << std::endl;
  //std::cout << "Node parent is: " << node->getParent() << std::endl;
  for(unsigned int i = 0; i < numDescriptors; i++){
    unsigned int desOffset;
    fileStream.read(reinterpret_cast<char*>(&desOffset), sizeof(desOffset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[8];
    fileStream.read(name, 8);
    std::string strName(name);
    std::cout << "String name is: " << strName << std::endl;
    if(strName == node->getName() && size == 0){
      std::cout << "name found in Wad" << std::endl;
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
