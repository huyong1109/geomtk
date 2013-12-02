#include "RLLVelocityField.h"

PolarRing::PolarRing() {
    vr = NULL;
}

PolarRing::~PolarRing() {
    if (vr != NULL) {
        for (int i = 0; i < this->mesh->getNumGrid(0, CENTER, true); ++i) {
            delete [] vr[i];
        }
        delete [] vr;
    }
}

void PolarRing::create(Mesh &mesh) {
    this->mesh = static_cast<RLLMesh*>(&mesh);
    vr = new TimeLevels<SphereVelocity, 2>*[this->mesh->getNumGrid(0, CENTER, true)];
    for (int i = 0; i < this->mesh->getNumGrid(0, CENTER, true); ++i) {
        vr[i] = new TimeLevels<SphereVelocity, 2>[this->mesh->getNumGrid(2, CENTER)];
        for (int k = 0; k < this->mesh->getNumGrid(2, CENTER); ++k) {
            for (int l = 0; l < vr[i][k].getNumLevel(); ++l) {
                vr[i][k].get(l).setNumDim(mesh.getDomain().getNumDim());
            }
        }
    }
}

void PolarRing::update(int timeLevel, Pole pole, TimeLevels<cube, 2> *data) {
    // ring variable is at A-grids
    int nx = mesh->getNumGrid(0, CENTER, true);
    int j = pole == SOUTH_POLE ? 1 : mesh->getNumGrid(1, CENTER)-2; // off the Pole
    for (int i = 1; i < mesh->getNumGrid(0, CENTER, true)-1; ++i) {
        for (int k = 0; k < mesh->getNumGrid(2, CENTER); ++k) {
            vr[i][k].get(timeLevel)(0) =
                (data[0].get(timeLevel)(i-1, j, k)+
                 data[0].get(timeLevel)(i,   j, k))*0.5;
        }
    }
    // periodic boundary condition
    for (int k = 0; k < mesh->getNumGrid(2, CENTER); ++k) {
        vr[0][k].get(timeLevel)(0) = vr[nx-2][k].get(timeLevel)(0);
        vr[nx-1][k].get(timeLevel)(0) = vr[1][k].get(timeLevel)(0);
    }
    j = pole == SOUTH_POLE ? 0 : mesh->getNumGrid(1, EDGE)-2;
    for (int i = 0; i < mesh->getNumGrid(0, CENTER, true); ++i) {
        for (int k = 0; k < mesh->getNumGrid(2, CENTER); ++k) {
            vr[i][k].get(timeLevel)(1) =
                (data[1].get(timeLevel)(i, j,   k)+
                 data[1].get(timeLevel)(i, j+1, k))*0.5;
        }
    }
    if (mesh->getDomain().getNumDim() == 3) {
        for (int i = 0; i < mesh->getNumGrid(0, CENTER, true); ++i) {
            for (int k = 0; k < mesh->getNumGrid(2, CENTER); ++k) {
                vr[i][k].get(timeLevel)(2) =
                    (data[2].get(timeLevel)(i, j, k)+
                     data[2].get(timeLevel)(i, j, k+1))*0.5;
            }
        }
    }
    // transform velocity
    double cosLon, sinLon, sinLat, sinLat2;
    j = pole == SOUTH_POLE ? 1 : mesh->getNumGrid(1, CENTER)-2;
    sinLat = mesh->getSinLat(CENTER, j);
    sinLat2 = mesh->getSinLat2(CENTER, j);
    for (int i = -1; i < mesh->getNumGrid(0, CENTER)+1; ++i) {
        for (int k = 0; k < mesh->getNumGrid(2, CENTER); ++k) {
            cosLon = mesh->getCosLon(CENTER, i);
            sinLon = mesh->getSinLon(CENTER, i);
            vr[i+1][k].get(timeLevel).transformToPS(sinLat, sinLat2, cosLon, sinLon);
        }
    }
}

double PolarRing::getOriginalData(int timeLevel, int dim, int i, int k) const {
    return vr[i+1][k].get(timeLevel)(dim);
}

double PolarRing::getTransformedData(int timeLevel, int dim, int i, int k) const {
    return vr[i+1][k].get(timeLevel)(dim);
}

// -----------------------------------------------------------------------------

RLLVelocityField::RLLVelocityField(Mesh &mesh) : RLLVectorField(mesh) {
    rings = new PolarRing[2];
}

RLLVelocityField::~RLLVelocityField() {
    delete [] rings;
}

void RLLVelocityField::applyBndCond(int timeLevel) {
    RLLVectorField::applyBndCond(timeLevel);
    rings[0].update(timeLevel, SOUTH_POLE, data);
    rings[1].update(timeLevel, NORTH_POLE, data);
}

void RLLVelocityField::create(StaggerType uLonStaggerType,
                              StaggerType uLatStaggerType,
                              StaggerType vLonStaggerType,
                              StaggerType vLatStaggerType) {
    RLLVectorField::create(uLonStaggerType, uLatStaggerType,
                           vLonStaggerType, vLatStaggerType);
    rings[0].create(*mesh); rings[1].create(*mesh);
}

void RLLVelocityField::create(StaggerType uLonStaggerType,
                              StaggerType uLatStaggerType,
                              StaggerType uLevStaggerType,
                              StaggerType vLonStaggerType,
                              StaggerType vLatStaggerType,
                              StaggerType vLevStaggerType,
                              StaggerType wLonStaggerType,
                              StaggerType wLatStaggerType,
                              StaggerType wLevStaggerType) {
    RLLVectorField::create(uLonStaggerType, uLatStaggerType, uLevStaggerType,
                           vLonStaggerType, vLatStaggerType, vLevStaggerType,
                           wLonStaggerType, wLatStaggerType, wLevStaggerType);
    rings[0].create(*mesh); rings[1].create(*mesh);
}

const PolarRing& RLLVelocityField::getPolarRing(Pole pole) const {
    return rings[pole];
}
