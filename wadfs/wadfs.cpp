#define FUSE_USE_VERSION 26
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fuse/fuse.h"

#include "../libWad/Wad.h"


static int getattrA(const char *path, struct stat *stbuf) {
  memset(stbuf, 0 , sizeof(struct stat));
  Wad* wad = (Wad*)(fuse_get_context()->private_data);
  if(wad->isDirectory(path)){
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
  }
  if(wad->isContent(path)){
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = wad->getSize(path);
    return 0;
  }
  return -ENOENT;
}

static int mknodA(const char *path, mode_t mode, dev_t rdev){
  Wad* wad = (Wad*)(fuse_get_context()->private_data);
  wad->createFile(path);
  return 0;

}
static int mkdirA(const char *path, mode_t mode){
  Wad* wad = (Wad*)(fuse_get_context()->private_data);
  wad->createDirectory(path);
  return 0;
}

static int readA(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
  Wad* wad = (Wad*)(fuse_get_context()->private_data);
  if(wad->isContent(path)){
    int bytes = wad->getContents(path, buf, size, offset);
    if(bytes < 0){
      return -ENOENT;
    }
    return bytes;
  }
  return -ENOENT;
}

static int writeA(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info *info){
  Wad* wad = (Wad*)(fuse_get_context()->private_data);
  int bytes = wad->writeToFile(path, buf, size, offset);
  if(bytes < 0){
    return -ENOENT;
  }
  return bytes;
}

static int readdirA(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
    Wad* wad = (Wad*)(fuse_get_context()->private_data);
    std::vector<std::string> directory;
    wad->getDirectory(path, &directory);

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for(auto it = directory.begin(); it != directory.end(); it++){
      filler(buf, it->c_str(), NULL, 0);
    }

  return 0;
}


static struct fuse_operations wad_operations = {
  .getattr = getattrA,
  .mknod = mknodA,
  .mkdir = mkdirA,
  .read = readA,
  .write = writeA,
  .readdir = readdirA
};

int main(int argc, char* argv[]){
  if(argc < 3){
    fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
    exit(1);
  }
  std::string wadPath = argv[argc-2];
  if(wadPath.at(0) != '/'){
    wadPath = std::string(get_current_dir_name()) + "/" + wadPath;
  }
  Wad* myWad = Wad::loadWad(wadPath);

  argv[argc - 2] = argv[argc - 1];
  argc--;
  return fuse_main(argc, argv, &wad_operations, myWad);
}
