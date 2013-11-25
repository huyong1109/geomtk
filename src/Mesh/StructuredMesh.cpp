#include "StructuredMesh.h"

StructuredMesh::StructuredMesh(Domain &domain) : Mesh(domain) {
    fullCoords = new vec[domain.getNumDim()];
    halfCoords = new vec[domain.getNumDim()];
    fullIntervals = new vec[domain.getNumDim()];
    halfIntervals = new vec[domain.getNumDim()];
}

StructuredMesh::~StructuredMesh() {
    delete [] fullCoords;
    delete [] halfCoords;
    delete [] fullIntervals;
    delete [] halfIntervals;
}

void StructuredMesh::setCoords(int dim, int size, double *full, double *half) {
    // sanity check
    if (dim >= domain->getNumDim()) {
        REPORT_ERROR("Argument dim (" << dim << ") exceeds domain dimension (" << domain->getNumDim() << ")!")
    }
    if (domain->getAxisStartBndType(dim) == PERIODIC) {
        fullCoords[dim].resize(size+2);
        halfCoords[dim].resize(size+2);
        fullIntervals[dim].resize(size+1);
        halfIntervals[dim].resize(size+1);
        for (int i = 0; i < size-1; ++i) {
            fullIntervals[dim](i+1) = half[i+1]-half[i];
            halfIntervals[dim](i+1) = full[i+1]-full[i];
        }
        for (int i = 0; i < size; ++i) {
            fullCoords[dim](i+1) = full[i];
            halfCoords[dim](i+1) = half[i];
        }
        // add virtual grids at two ends for periodic boundary condition
        if (full[0] == domain->getAxisStart(dim)) {
            fullIntervals[dim](size) = domain->getAxisEnd(dim)-full[size-1];
            fullIntervals[dim](0) = fullIntervals[dim](size);
            halfIntervals[dim](0) = domain->getAxisSpan(dim)-half[size-1]+half[0];
            halfIntervals[dim](size) = halfIntervals[dim](0);
            fullCoords[dim](0) = domain->getAxisStart(dim)-fullIntervals[dim](size);
            fullCoords[dim](size+1) = domain->getAxisEnd(dim);
            halfCoords[dim](0) = half[0]-halfIntervals[dim](0);
            halfCoords[dim](size+1) = half[size-1]+halfIntervals[dim](size);
        } else if (half[0] == domain->getAxisStart(dim)) {
            fullIntervals[dim](0) = domain->getAxisSpan(dim)-full[size-1]+full[0];
            fullIntervals[dim](size) = fullIntervals[dim](0);
            halfIntervals[dim](size) = domain->getAxisEnd(dim)-half[size-1];
            halfIntervals[dim](0) = halfIntervals[dim](size);
            fullCoords[dim](0) = full[0]-fullIntervals[dim](0);
            fullCoords[dim](size+1) = full[size-1]+fullIntervals[dim](size);
            halfCoords[dim](0) = domain->getAxisStart(dim)-halfIntervals[dim](size);
            halfCoords[dim](size+1) = domain->getAxisEnd(dim);
        } else {
            REPORT_ERROR("Don't know how to handle input grid coordinates of dimension " << dim << "!")
        }
    } else {
        REPORT_ERROR("Under construction!")
        fullCoords[dim].resize(size);
        halfCoords[dim].resize(size);
        fullIntervals[dim].resize(size-1);
        halfIntervals[dim].resize(size-1);
    }
}