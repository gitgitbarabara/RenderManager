#include "Layer.hpp"

Layer::Layer(const int _signature)
        : signature(
        _signature) {

    memory = 0;

    vertexData = nullptr;
    texcoordData = nullptr;
    colorData = nullptr;

    next = nullptr;

    fieldSize = 1.0f;
    landscapeRatio = 1.0f;
    portraitRatio = 1.0f;

    mesh = nullptr;
    trash = nullptr;
    shader = nullptr;

    meshCount = 0;
    needMaterialize = false;

    destroyed = false;

    uniformData = nullptr;
}

Layer::~Layer(){

    if (shader) {
        delete shader;
        shader = nullptr;
    }

    if (vertexData) {
        delete[] vertexData;
        vertexData = nullptr;
    }

    if (texcoordData) {
        delete[] texcoordData;
        texcoordData = nullptr;
    }

    if (colorData) {
        delete[] colorData;
        colorData = nullptr;
    }

    if (uniformData) {
        delete uniformData;
        uniformData = nullptr;
    }

    Mesh *temp = nullptr;

    while (mesh != nullptr) {
        temp = mesh->next;
        delete mesh;
        mesh = temp;
    }

    temp = nullptr;

    while (trash != nullptr) {
        temp = trash->next;
        delete trash;
        trash = temp;
    }
}

void Layer::SetOrthographic(float size, float windowWidth, float windowHeight, bool landscape) {

}

void Layer::Zoom(float size) {

}

Mesh * Layer::RegisterBack(const int signature) {
    return nullptr;
}

Mesh * Layer::RegisterFront(const int signature) {
    return nullptr;
}

void Layer::Trash(Mesh *_trash) {
    Mesh *temp = trash;
    _trash->next = nullptr;
    trash = _trash;
    trash->activated = false;
    trash->next = temp;
}

void Layer::ClearTrash() {
    Mesh *temp = nullptr;

    while (trash != nullptr) {
        temp = trash->next;
        delete trash;
        trash = temp;
    }
}

Layer * Layer::Destroy() {
    if (!destroyed) destroyed = true;
    return this;
}

bool Layer::isDestroyed() {
    return destroyed;
}


void Layer::Render(){}
void Layer::CheckMesh(){}
void Layer::Materialize(){}
void Layer::ConvertMatrix(){}
void Layer::ConvertTexcoord(){}
void Layer::ConvertColor(){}
void Layer::CreateShader(){}
