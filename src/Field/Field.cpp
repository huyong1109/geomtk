#include "Field.h"

Field::Field(Mesh &mesh) {
    this->mesh = &mesh;
}

Field::~Field() {
}

const Mesh& Field::getMesh() {
    return *mesh;
}