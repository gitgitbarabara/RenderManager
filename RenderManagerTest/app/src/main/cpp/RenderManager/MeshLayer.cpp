#include "Layer.hpp"

MeshLayer::MeshLayer(const int signature, int _memory, std::string fileName,
                     Shader::Shader *_shader)
        : Layer(signature) {

    memory = _memory;

    vertexData = new float[memory * 18];
    texcoordData = new float[memory * 12];
    colorData = new float[memory * 24];

    next = nullptr;

    fieldSize = 1.0f;
    landscapeRatio = 1.0f;
    portraitRatio = 1.0f;

    meshCount = 0;

    mesh = nullptr;
    trash = nullptr;

    shader = _shader;
    uniformData = new Shader::UniformData();
    if (!shader) shader = new Shader::Default();
    shader->uniformData = uniformData;

    destroyed = false;
    needMaterialize = false;

    uniformData->texture = BDTLoad(fileName);
}

MeshLayer::~MeshLayer() {

}

void MeshLayer::SetOrthographic(float size, float windowWidth, float windowHeight, bool landscape) {
    //landscape
    //true = 가로넓이가 정값으로
    //false = 세로넓이가 정값으로

    fieldSize = size;

    if (landscape) {
        landscapeRatio = 1.0f;
        portraitRatio = windowHeight / windowWidth;
    } else {
        landscapeRatio = windowWidth / windowHeight;
        portraitRatio = 1.0f;
    }

    uniformData->ortho = vec::ortho(-fieldSize / 2.0f * landscapeRatio,
                                    fieldSize / 2.0f * landscapeRatio,
                                    -fieldSize / 2.0 * portraitRatio,
                                    fieldSize / 2.0 * portraitRatio, -10000.0,
                                    10000.0);
}

void MeshLayer::Zoom(float size) {

    fieldSize = size;

    uniformData->ortho = vec::ortho(-fieldSize / 2.0f * landscapeRatio,
                                    fieldSize / 2.0f * landscapeRatio,
                                    -fieldSize / 2.0 * portraitRatio,
                                    fieldSize / 2.0 * portraitRatio, -1000.0,
                                    1000.0);
}

Mesh *MeshLayer::RegisterBack(const int signature) {

    Mesh *temp = mesh;

    if (!mesh) {
        mesh = new Mesh(signature);
        return mesh;
    }

    while (temp->next) temp = temp->next;

    temp->next = new Mesh(signature);

    return temp->next;
}

Mesh *MeshLayer::RegisterFront(const int signature) {

    Mesh *temp = mesh;

    mesh = new Mesh(signature);
    mesh->next = temp;

    return mesh;
}

void MeshLayer::Render() {

    if (isDestroyed()) return;

    CheckMesh();
    Materialize();
    ClearTrash();
    CreateShader();

    if (!shader) return;
    if (meshCount <= 0) return;

    glBindVertexArray(shader->vao);
    glUseProgram(shader->shaderProgram);
    glEnable(GL_TEXTURE_2D);

    if (vertexData) {
        ConvertMatrix();
        glBindBuffer(GL_ARRAY_BUFFER, shader->positionVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 * meshCount, vertexData,
                     GL_STATIC_DRAW);
    }
    if (texcoordData) {
        ConvertTexcoord();
        glBindBuffer(GL_ARRAY_BUFFER, shader->texcoordVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12 * meshCount, texcoordData,
                     GL_STATIC_DRAW);
    }
    if (colorData) {
        ConvertColor();
        glBindBuffer(GL_ARRAY_BUFFER, shader->colorVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24 * meshCount, colorData,
                     GL_STATIC_DRAW);
    }

    if (uniformData) shader->LoadUniform();

    glDrawArrays(GL_TRIANGLES, 0, 6 * meshCount);
    glDisable(GL_TEXTURE_2D);
}

void MeshLayer::CheckMesh() {

    Mesh *prev = nullptr;
    Mesh *temp = mesh;

    while (temp != nullptr) {

        if (temp->isDestroyed()) {

            if (temp->activated) {
                temp->activated = false;
                needMaterialize = true;
            }

            if (!prev) {
                mesh = mesh->next;
                Trash(temp);
                temp = mesh;

            } else {
                prev->next = temp->next;
                Trash(temp);
                temp = prev->next;
            }
        } else {
            prev = temp;
            temp = temp->next;
        }
    }
}

void MeshLayer::Materialize() {

    if (!needMaterialize) return;

    meshCount = 0;
    int i = 0;

    Mesh *temp = mesh;

    while (temp) {

        if (meshCount >= memory) break;

        if (!temp->activated) {
            temp = temp->next;
            continue;
        }

        vertexData[i * 18 + 0] = temp->matrix[0] * temp->width;
        vertexData[i * 18 + 1] = temp->matrix[1] * temp->height;
        vertexData[i * 18 + 2] = temp->depth;

        vertexData[i * 18 + 3] = temp->matrix[2] * temp->width;
        vertexData[i * 18 + 4] = temp->matrix[3] * temp->height;
        vertexData[i * 18 + 5] = temp->depth;

        vertexData[i * 18 + 6] = temp->matrix[4] * temp->width;
        vertexData[i * 18 + 7] = temp->matrix[5] * temp->height;
        vertexData[i * 18 + 8] = temp->depth;

        vertexData[i * 18 + 9] = temp->matrix[0] * temp->width;
        vertexData[i * 18 + 10] = temp->matrix[1] * temp->height;
        vertexData[i * 18 + 11] = temp->depth;

        vertexData[i * 18 + 12] = temp->matrix[4] * temp->width;
        vertexData[i * 18 + 13] = temp->matrix[5] * temp->height;
        vertexData[i * 18 + 14] = temp->depth;

        vertexData[i * 18 + 15] = temp->matrix[6] * temp->width;
        vertexData[i * 18 + 16] = temp->matrix[7] * temp->height;
        vertexData[i * 18 + 17] = temp->depth;

        switch (temp->cullingType) {

            case MeshType::Culling::CW:
                texcoordData[i * 12 + 0] = temp->texcoordData[0];
                texcoordData[i * 12 + 1] = temp->texcoordData[1];

                texcoordData[i * 12 + 2] = temp->texcoordData[2];
                texcoordData[i * 12 + 3] = temp->texcoordData[3];

                texcoordData[i * 12 + 4] = temp->texcoordData[4];
                texcoordData[i * 12 + 5] = temp->texcoordData[5];

                texcoordData[i * 12 + 6] = temp->texcoordData[0];
                texcoordData[i * 12 + 7] = temp->texcoordData[1];

                texcoordData[i * 12 + 8] = temp->texcoordData[4];
                texcoordData[i * 12 + 9] = temp->texcoordData[5];

                texcoordData[i * 12 + 10] = temp->texcoordData[6];
                texcoordData[i * 12 + 11] = temp->texcoordData[7];

                break;
            case MeshType::Culling::CCW:

                texcoordData[i * 12 + 0] = temp->texcoordData[6];
                texcoordData[i * 12 + 1] = temp->texcoordData[7];

                texcoordData[i * 12 + 2] = temp->texcoordData[4];
                texcoordData[i * 12 + 3] = temp->texcoordData[5];

                texcoordData[i * 12 + 4] = temp->texcoordData[2];
                texcoordData[i * 12 + 5] = temp->texcoordData[3];

                texcoordData[i * 12 + 6] = temp->texcoordData[6];
                texcoordData[i * 12 + 7] = temp->texcoordData[7];

                texcoordData[i * 12 + 8] = temp->texcoordData[2];
                texcoordData[i * 12 + 9] = temp->texcoordData[3];

                texcoordData[i * 12 + 10] = temp->texcoordData[0];
                texcoordData[i * 12 + 11] = temp->texcoordData[1];
                break;
        }

        for (int j = 0; j < 6; j++) {
            colorData[i * 24 + j * 4 + 0] = temp->colorData[0] * temp->brightness;
            colorData[i * 24 + j * 4 + 1] = temp->colorData[1] * temp->brightness;
            colorData[i * 24 + j * 4 + 2] = temp->colorData[2] * temp->brightness;
            colorData[i * 24 + j * 4 + 3] = temp->colorData[3] * temp->opacity;
        }

        i++;
        meshCount++;
        temp = temp->next;
    }

    needMaterialize = false;
}

void MeshLayer::ConvertMatrix() {

    int i = 0;
    Mesh *temp = mesh;

    float distance, direction;

    while (temp) {

        if (!temp->activated) {
            temp = temp->next;
            continue;
        }

        if (i >= meshCount) break;

        float lookAt[2] = {0.0f, 0.0f};

        if (temp->camera) {
            lookAt[0] = temp->camera->lookAtPosition[0];
            lookAt[1] = temp->camera->lookAtPosition[1];
        }

        if (temp->convertMatrix) {

            for (int j = 0; j < 4; j++) {

                distance =
                        Tools::Distance(temp->axis[0], temp->axis[1], temp->vertexData[2 * j],
                                        temp->vertexData[2 * j + 1]) *
                        temp->size;
                direction = atan2(temp->vertexData[2 * j + 1] - temp->axis[1],
                                  temp->vertexData[2 * j] - temp->axis[0]) *
                            57.295780f;

                temp->matrix[2 * j] =
                        temp->axis[0] +
                        (cos((direction + (temp->angle + temp->valve)) * 0.017453f) * distance);
                temp->matrix[2 * j + 1] =
                        temp->axis[1] +
                        (sin((direction + (temp->angle + temp->valve)) * 0.017453f) * distance);

            }

            if (temp->camera) {
                lookAt[0] = temp->camera->tempPosition[0];
                lookAt[1] = temp->camera->tempPosition[1];
            }

            temp->convertMatrix = false;
        }


        switch (temp->cullingType) {

            case MeshType::Culling::CW:

                vertexData[i * 18 + 0] =
                        temp->matrix[0] * temp->width - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 1] =
                        temp->matrix[1] * temp->height - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 2] = temp->depth;

                vertexData[i * 18 + 3] =
                        temp->matrix[2] * temp->width - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 4] =
                        temp->matrix[3] * temp->height - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 5] = temp->depth;

                vertexData[i * 18 + 6] =
                        temp->matrix[4] * temp->width - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 7] =
                        temp->matrix[5] * temp->height - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 8] = temp->depth;

                vertexData[i * 18 + 9] =
                        temp->matrix[0] * temp->width - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 10] =
                        temp->matrix[1] * temp->height - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 11] = temp->depth;

                vertexData[i * 18 + 12] =
                        temp->matrix[4] * temp->width - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 13] =
                        temp->matrix[5] * temp->height - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 14] = temp->depth;

                vertexData[i * 18 + 15] =
                        temp->matrix[6] * temp->width - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 16] =
                        temp->matrix[7] * temp->height - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 17] = temp->depth;

                break;
            case MeshType::Culling::CCW:

                vertexData[i * 18 + 0] = -temp->matrix[6] - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 1] = temp->matrix[7] - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 2] = temp->depth;


                vertexData[i * 18 + 3] =
                        -temp->matrix[4] - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 4] = temp->matrix[5] - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 5] = temp->depth;

                vertexData[i * 18 + 6] =
                        -temp->matrix[2] - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 7] = temp->matrix[3] - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 8] = temp->depth;


                vertexData[i * 18 + 9] =
                        -temp->matrix[6] - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 10] =
                        temp->matrix[7] - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 11] = temp->depth;


                vertexData[i * 18 + 12] =
                        -temp->matrix[2] - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 13] =
                        temp->matrix[3] - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 14] = temp->depth;


                vertexData[i * 18 + 15] =
                        -temp->matrix[0] - lookAt[0] + temp->position[0];
                vertexData[i * 18 + 16] =
                        temp->matrix[1] - lookAt[1] + temp->position[1];
                vertexData[i * 18 + 17] = temp->depth;


                break;
        }

        i++;
        temp = temp->next;
    }
}

void MeshLayer::ConvertTexcoord() {

    int i = 0;
    Mesh *temp = mesh;

    while (temp) {

        if (!temp->activated) {
            temp = temp->next;
            continue;
        }

        if (i >= meshCount) break;

        if (temp->convertTexcoord) {

            switch (temp->cullingType) {

                case MeshType::Culling::CW:
                    texcoordData[i * 12 + 0] = temp->texcoordData[0];
                    texcoordData[i * 12 + 1] = temp->texcoordData[1];

                    texcoordData[i * 12 + 2] = temp->texcoordData[2];
                    texcoordData[i * 12 + 3] = temp->texcoordData[3];

                    texcoordData[i * 12 + 4] = temp->texcoordData[4];
                    texcoordData[i * 12 + 5] = temp->texcoordData[5];

                    texcoordData[i * 12 + 6] = temp->texcoordData[0];
                    texcoordData[i * 12 + 7] = temp->texcoordData[1];

                    texcoordData[i * 12 + 8] = temp->texcoordData[4];
                    texcoordData[i * 12 + 9] = temp->texcoordData[5];

                    texcoordData[i * 12 + 10] = temp->texcoordData[6];
                    texcoordData[i * 12 + 11] = temp->texcoordData[7];

                    break;
                case MeshType::Culling::CCW:

                    texcoordData[i * 12 + 0] = temp->texcoordData[6];
                    texcoordData[i * 12 + 1] = temp->texcoordData[7];

                    texcoordData[i * 12 + 2] = temp->texcoordData[4];
                    texcoordData[i * 12 + 3] = temp->texcoordData[5];

                    texcoordData[i * 12 + 4] = temp->texcoordData[2];
                    texcoordData[i * 12 + 5] = temp->texcoordData[3];

                    texcoordData[i * 12 + 6] = temp->texcoordData[6];
                    texcoordData[i * 12 + 7] = temp->texcoordData[7];

                    texcoordData[i * 12 + 8] = temp->texcoordData[2];
                    texcoordData[i * 12 + 9] = temp->texcoordData[3];

                    texcoordData[i * 12 + 10] = temp->texcoordData[0];
                    texcoordData[i * 12 + 11] = temp->texcoordData[1];
                    break;
            }
        }

        i++;
        temp->convertTexcoord = false;
        temp = temp->next;
    }
}

void MeshLayer::ConvertColor() {

    int i = 0;
    Mesh *temp = mesh;

    while (temp) {

        if (!temp->activated) {
            temp = temp->next;
            continue;
        }

        if (i >= meshCount) break;

        if (temp->convertColor) {

            for (int j = 0; j < 6; j++) {
                colorData[i * 24 + j * 4 + 0] = temp->colorData[0] * temp->brightness;
                colorData[i * 24 + j * 4 + 1] = temp->colorData[1] * temp->brightness;
                colorData[i * 24 + j * 4 + 2] = temp->colorData[2] * temp->brightness;
                colorData[i * 24 + j * 4 + 3] = temp->colorData[3] * temp->opacity;
            }
        }

        i++;
        temp->convertColor = false;
        temp = temp->next;
    }
}

void MeshLayer::CreateShader() {
    if (!shader) return;
    if (shader->activated) return;

    shader->Activate();
    shader->activated = true;
}