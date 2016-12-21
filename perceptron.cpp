#include <limits>

#include "perceptron.h"
#include "typedef.h"

perceptron::perceptron() : bias(0), doneTraining(false) {
	memset(weights,0,sizeof(weights));
}

void perceptron::train(image &img) {
	double y = evaluate(img);
  int d = (img.getClusterNum() == clusterIndex) ? 1 : -1;
  register double r = d-y;
  for (int i=0; i<64; i++)
    weights[i] += r * img.getHistogram().get(i);
  bias += r;
}

double perceptron::evaluate(image &img) const {
	// if the perceptron is training use a cached evaluate value
	// (if it exists)
	if (doneTraining) {
		double val = img.perceptronEvalCache[perceptronIndex];
		if (val != std::numeric_limits<double>::max())
			return val;
	}

	double sum = bias;
	for (int i=0; i<64; i++)
		sum += weights[i] * img.getHistogram().get(i);

	if (doneTraining)
		img.perceptronEvalCache[perceptronIndex] = sum;
	return sum;
}

double perceptron::similarity(array<perceptron> &p, image &img1, image &img2) {
	double sum = 0;
	for (uint i=0; i<p.size(); i++) {
		double exp = p[i].evaluate(img1) - p[i].evaluate(img2);
		exp *= exp;
		sum += 1 / ( exp + std::numeric_limits<double>::min() );
	}
	return sum;
}
