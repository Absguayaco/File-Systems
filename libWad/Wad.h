#include <map>
#include <vector>
#include <unordered_map>
#include "tree.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

class Wad{
  private:
    std::fstream fileStream;
    Wad(const std::string &path);
    Tree* tree;
    char fileMagic[5] = {0};
    unsigned int numDescriptors = 0;
    unsigned int descriptorOffset = 0;

  public:
    ~Wad();
    static Wad* loadWad(const std::string &path);
    std::string getMagic();
    bool isContent(const std::string &path);
    bool isDirectory(const std::string &path);
    int getSize(const std::string &path);
    int getContents(const std::string &path, char *buffer, int length, int offset = 0);
    int getDirectory(const std::string &path, std::vector<std::string> *directory);
    void createDirectory(const std::string &path);
    void createFile(const std::string &path);
    int writeToFile(const std::string &path, const char *buffer, int length, int offset = 0);
    void searchWad(Node* node, Node* newNode, std::vector<std::string> path);
    void shiftWad(Node*node, Node* newNode, int length);

};
