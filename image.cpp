#include <limits>

#include "image.h"
#include "readfile.h"

array<image> image::images;

void image::openFile(string fname) {
	filename = fname;
	// write data directly into vars by passing by reference
	readPGM(fname, h, clusterNum);

	h.__normalize();
}

void image::setPerceptronEvalCacheSize(int size) {
	perceptronEvalCache.resize(size);
	for (unsigned int i=0; i<perceptronEvalCache.size(); i++)
		perceptronEvalCache[i] = std::numeric_limits<double>::max(); // no cached val yet
}
