#pragma once

#include <vector>

#include "image.h"

using std::vector;

class perceptron {
public:
	perceptron();
	inline void setClusterIndex(int ci) { clusterIndex = ci; }
	inline void setPerceptronIndex(int pi) { perceptronIndex = pi; }
	inline void setDoneTraining(bool done) { doneTraining = done; }
	void train(image &h);
	double evaluate(image &h) const;
	static double similarity(array<perceptron> &p, image &img1, image &img2);
protected:
	double weights[64];
	double bias;

	// the clustering index (from pgm filename) that this perceptron will train for
	int clusterIndex;

	// once this is true this perceptron will start storing the result of perceptron.evaluate
	// into the image that it was evaluated with
 	bool doneTraining;

	// used for storing the result of perceptron.evaluate
	int perceptronIndex;
};
