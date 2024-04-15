#include "Wad.h"
void Wad::searchWad(Node* node, Node* newNode){
  for(int i = 0; i < numDescriptors; i++){
    unsigned int offset;
    fileStream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[9];
    name[8] = '\O'; //not sure if nccessart
    fileStream.read(name, 8);
    std::string strName(name);
    if(strName == node->getName() + "_END"){
      //Copy and shift remaining forward
      fileStream.seekg(descriptorOffset + (i-1) * 16);
      char buffer[(numDescriptors - i) * 16];
      fileStream.read(buffer, sizeof(buffer)); 
      fileStream.seekp(descriptorOffset + (i+2) * 16);
      fileStream.write(buffer, sizeof(buffer)); 

      fileStream.seekp(descriptorOffset + (i-1) * 16); //this line
      unsigned int zero = 0;

      if(newNode->getType() == "namespace"){
        char arr[9];
        std::string tempString = newNode->getName() + "_START";
        std::string tempString2 = newNode->getName() + "_END";
        fileStream.write(reinterpret_cast<const char*>(&zero), 4);
        fileStream.write(reinterpret_cast<const char*>(&zero), 4);
        fileStream.write(tempString.c_str(), 8);

        fileStream.write(reinterpret_cast<const char*>(&zero), 4);
        fileStream.write(reinterpret_cast<const char*>(&zero), 4);
        fileStream.write(tempString2.c_str(), 8);
        break;
      }
      else if(newNode->getType() == "file"){
        char arr[8];
        fileStream.write(reinterpret_cast<const char*>(&zero), 4);
        fileStream.write(reinterpret_cast<const char*>(&zero), 4);
        fileStream.write(newNode->getName().c_str(), 8);//maybe add strcpy for these
        break;
      }
    }
  }
}

void Wad::shiftWad(Node* node, Node* newNode, int length){
    fileStream.seekg(descriptorOffset);

    char buffer[numDescriptors * 16];
    fileStream.read(buffer, sizeof(buffer));
    fileStream.seekp(descriptorOffset + length);
    fileStream.write(buffer, sizeof(buffer)); 


}
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

  std::regex pattern("E[0-9]M[0-9]");
  std::regex patternStart("[A-Z0-9]{1,2}_START");
  std::regex patternEnd("[A-Z0-9]{1,2}_END");
  std::smatch match;
  int count = 0;

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

    if(std::regex_search(strName, match, patternStart)){ 
      char arr[2];
      std::copy(strName.begin(), strName.begin() + 2, arr);
      Node* node = new Node(offset, size, arr, "namespace"); //if this works naem should onyl be first two
      tree->insert(curDirectory, node);
      curDirectory = node;
    }
    else if(std::regex_search(strName, match, pattern)){ //maybe add cstr to strName here
      Node* node = new Node(offset, size, name, "map");
      tree->insert(curDirectory, node);
      curDirectory = node;    
      count += 9;
    }
    else if (std::regex_search(strName, match, patternEnd)){
      //Node* node = new Node(offset, size, name, "namespace"); //maybe dont create node here, maybe unecessary test after done
      //tree->insert(curDirectory, node);
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

  fileStream.seekg(node->getOffset() + offset); //TODO: check that this works properly
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

void Wad::createDirectory(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  if(splitString.back().size() > 2){
    return;
  }
  std::string name = splitString.back();
  splitString.pop_back();

  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return;
  }
  char arr[name.length() + 1]; 
  Node* newNode = new Node(0, 0, strcpy(arr, name.c_str()) , "namespace");
  tree->insert(node, newNode);
  searchWad(node, newNode);

  numDescriptors += 2; 
  fileStream.seekp(4);
  fileStream.write(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors)); //TODO: make sure it works with root directory 
}

void Wad::createFile(const std::string &path){
  std::vector<std::string> splitString = intepretPath(path);
  std::regex pattern("E[0-9]M[0-9]");
  std::regex patternStart("[A-Z0-9]{1,2}_START");
  std::regex patternEnd("[A-Z0-9]{1,2}_END");
  std::smatch match;
  std::string name = splitString.back();
  splitString.pop_back();

  if(std::regex_search(name, match, patternStart) || std::regex_search(name, match, patternEnd) || std::regex_search(name, match, pattern)){ 
    return;
  }

  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return;
  }
  char arr[name.length() + 1]; 
  Node* newNode = new Node(0, 0, strcpy(arr, name.c_str()) , "file");
  tree->insert(node, newNode);
  searchWad(node, newNode);
 
numDescriptors += 1; 
fileStream.seekp(4);
fileStream.write(reinterpret_cast<char*>(&numDescriptors), sizeof(numDescriptors));
}

int Wad::writeToFile(const std::string &path, const char *buffer, int length, int offset){
  std::vector<std::string> splitString = intepretPath(path);
  Node* node = tree->depthFirstSearch(tree->root, splitString);
  if(node == nullptr){
    return -1;
  }
  if(node->getType() != "file"){
    return -1;
  }
  if(node->getLength() != 0){
    return 0;
  }
  //TODO: write a lot here

  fileStream.seekg(descriptorOffset);

  //shifts descriptor list forward
  char readBuffer[numDescriptors * 16];
  fileStream.read(readBuffer, sizeof(buffer));
  fileStream.seekp(descriptorOffset + length);
  fileStream.write(readBuffer, sizeof(readBuffer)); 
  
  //write lump data
  buffer += offset;
  fileStream.seekp(descriptorOffset);
  int beforeWrite = fileStream.tellp();
  int bytesWritten = fileStream.write(buffer, length).tellp();bytesWritten -= beforeWrite; 

  //Change Header
  fileStream.seekp(8);  
  int newOffset = fileStream.tellp();
  fileStream.write(reinterpret_cast<char*>(&newOffset), sizeof(newOffset));

  //Change descriptor
  node->setOffset(descriptorOffset);
  fileStream.seekp(newOffset);
  for(int i = 0; i < numDescriptors; i++){
    unsigned int desOffset;
    fileStream.read(reinterpret_cast<char*>(&desOffset), sizeof(desOffset));
    unsigned int size;
    fileStream.read(reinterpret_cast<char*>(&size), sizeof(size));
    char name[9];
    name[8] = '\O'; //not sure if nccessart
    fileStream.read(name, 8);
    std::string strName(name);
    if(strName == node->getName()){
      fileStream.seekp(descriptorOffset + i * 16);
      fileStream.write(reinterpret_cast<char*>(&descriptorOffset), sizeof(newOffset));
      fileStream.write(reinterpret_cast<char*>(&length), sizeof(length));
      break;
    }
  }

  descriptorOffset = newOffset;

  return bytesWritten;
}