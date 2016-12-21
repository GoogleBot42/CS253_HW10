#pragma once

#include <string>

#include "histogram.h"
#include "array.h"

using std::string;

class image {
public:
	image() {}
	void openFile(string filename);
	void combine(const image &b) { h.combine(b.h); }

	inline int getClusterNum() const { return clusterNum; }
	inline const histogram& getHistogram() const { return h; }
	inline void normalize() { h.normalize(); }
	inline const string& getFilename() const { return filename; }

	void setPerceptronEvalCacheSize(int size);
	// inline array<double>& getPerceptronEvalCache() { return perceptronEvalCache; }
	array<double> perceptronEvalCache;

	static array<image> images;
protected:
	// the cluster this image belongs in as was parsed in the filename that
	// passed to image::open
	int clusterNum;
	string filename;
	histogram h;
};
