#include "BdtInfo.h"

BDTImage::BDTImage(std::string fileName) {

    std::ifstream BmpFile;
    //BmpFile.open(FILENAME, std::ios::binary);
    BmpFile.open(Device::devicePath + "/" + fileName, std::ios::binary);

    width = 0;
    height = 0;
    component = GL_RGBA;
    format = GL_RGBA;

    BmpFile.read((char*)&bitcount, sizeof(int));
    BmpFile.read((char*)&width, sizeof(int));
    BmpFile.read((char*)&height, sizeof(int));
    BmpFile.read((char*)&bitsize, sizeof(int));

    int size = width * height * (bitcount / 8);
    bits = new unsigned char[size];
    BmpFile.read((char*)bits, size);
}
BDTImage::~BDTImage(){
    if (bits != nullptr) delete bits;
    bits = nullptr;
}

BDTImage* BDTLoad(std::string fileName) {
    return new BDTImage(fileName);
}