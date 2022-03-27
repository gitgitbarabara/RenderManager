#include "Layer.hpp"

Collider::Collider() {
    collided = false;
    velocity = 0.0f;
    maxDistance = 0.0f;
}

Collider::~Collider() {

}

void Collider::SetCollisionVertex(float width, float height) {
    vertex[0] = -width / 2.0f;
    vertex[1] = -height / 2.0f;

    vertex[2] = -width / 2.0f;
    vertex[3] = height / 2.0f;

    vertex[4] = width / 2.0f;
    vertex[5] = height / 2.0f;

    vertex[6] = width / 2.0f;
    vertex[7] = -height / 2.0f;

    maxDistance = Tools::Distance(0.0f, 0.0f, width / 2.0f, height / 2.0f);
}

void Collider::Collide(Mesh *mesh, Collider *collider, Mesh *targetMesh, bool slide) {

    vec::vec2 destination;

    vec::vec2 targetPosition; //OBJECT
    vec::vec2 targetOrigin; //OBJECT
    vec::vec2 targetPoints; //OBJECT
    vec::vec2 targetNormalVector;
    float targetVertex[8];

    vec::vec2 vector1;
    vec::vec2 vector2;

    vec::vec2 meet;

    float dot;
    float space;

    velocity = mesh->velocity;

    if(velocity <= 0.0f) return;

    position[0] = mesh->position[0];
    position[1] = mesh->position[1];

    direction[0] = mesh->direction[0];
    direction[1] = mesh->direction[1];

    destination[0] = position[0] + (velocity * direction[0]);
    destination[1] = position[1] + (velocity * direction[1]);

    targetPosition[0] = targetMesh->position[0];
    targetPosition[1] = targetMesh->position[1];

    float distance = Tools::Distance(position[0], position[1], targetPosition[0],
                                     targetPosition[1]);

    if (distance - (maxDistance + collider->maxDistance + velocity) > 0.001f) { //첫번째 최적화
        velocity = mesh->velocity;
        return;
    }

    for(int i = 0; i < 4; i++){
        targetVertex[2 * i] = collider->vertex[2 * i] + vertex[2 * i];
        targetVertex[2 * i + 1] = collider->vertex[2 * i + 1] + vertex[2 * i + 1];
    }

    for (int i = 0; i < 4; i++) {

        targetOrigin[0] = targetVertex[2 * i] + targetPosition[0];
        targetOrigin[1] = targetVertex[2 * i + 1] + targetPosition[1];

        if (i == 3) {
            targetPoints[0] = targetVertex[0] + targetPosition[0];
            targetPoints[1] = targetVertex[1] + targetPosition[1];
        } else {
            targetPoints[0] = targetVertex[2 * i + 2] + targetPosition[0];
            targetPoints[1] = targetVertex[2 * i + 3] + targetPosition[1];
        }


        targetNormalVector = Tools::NormalVector(targetOrigin, targetPoints);
        dot = Tools::Dot(direction, targetNormalVector);
        if (dot >= 0.0f) continue;

        vector1 = Tools::Normalization(targetOrigin, position);
        dot = Tools::Dot(targetNormalVector, vector1);
        if (dot < -0.001f) continue;

        vector1 = Tools::Normalization(targetOrigin, destination);
        dot = Tools::Dot(targetNormalVector, vector1);
        if (dot > 0.001f) continue;

        vector1 = Tools::NormalVector(position, targetOrigin);
        dot = Tools::Dot(vector1, direction);
        if (dot <= 0.001f){
            vector1 = Tools::Normalization(targetOrigin, targetPosition);
            dot = Tools::Dot(vector1, direction);
            if(dot < 0.001f) continue;
        }

        vector1 = Tools::NormalVector(position, targetPoints);
        dot = Tools::Dot(vector1, direction);
        if (dot >= -0.001f){
            vector1 = Tools::Normalization(targetPoints, targetPosition);
            dot = Tools::Dot(vector1, direction);
            if(dot < 0.001f) continue;
        }

        vector1 = Tools::Normalization(targetOrigin, position);
        vector2 = Tools::Normalization(targetOrigin, targetPoints);
        dot = Tools::Dot(vector1, vector2);
        space = Tools::Distance(targetOrigin, position) * (1.0f - dot);

        vector1 = Tools::ReverseVector(targetNormalVector);
        dot = Tools::Dot(vector1, direction);

        space = space / dot;

        collided = true;
        if (velocity - space > 0.001f) velocity = space - 0.001f;
        if (velocity <= 0.001f) velocity = 0.0f;

        if (slide) {
            if(velocity > 0.0f) continue;

            vector1 = Tools::Normalization(targetOrigin, targetPosition);
            vector2 = Tools::Normalization(position, targetOrigin);
            dot = Tools::Dot(vector1, vector2);
            if(dot >= 0.999f) continue;

            vector1 = Tools::Normalization(targetPoints, targetPosition);
            vector2 = Tools::Normalization(position, targetPoints);
            dot = Tools::Dot(vector1, vector2);
            if(dot >= 0.999f) continue;

            velocity = mesh->velocity;

            vector1 = Tools::ProjectionVector(direction, velocity);
            dot = Tools::Dot(vector1, targetNormalVector);

            vector2 = Tools::ProjectionVector(targetNormalVector, dot);
            vector1 = Tools::SubtractionVector(vector1, vector2);

            vector2 = Tools::ReverseVector(vector1);
            vector1 = Tools::Normalization(vector2);
            velocity = Tools::Distance(vector2);
            direction = vector1;
        }
    }
}

void Collider::Look(Mesh *mesh, Mesh *targetMesh) {

    vec::vec2 targetPosition; //OBJECT

    position[0] = mesh->position[0];
    position[1] = mesh->position[1];

    targetPosition[0] = targetMesh->position[0];
    targetPosition[1] = targetMesh->position[1];

    direction = Tools::Normalization(position, targetPosition);

}

void Collider::InitCollider() {
    collided = false;
}

bool Collider::isCollided() {
    return collided;
}