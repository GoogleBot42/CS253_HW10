#include <iostream>

#include "cluster.h"
#include "typedef.h"

using std::cout;
using std::endl;

void cluster::addImage(const int i) {
	images.push_back(i);
	flatten();
}

void cluster::combine(const cluster &c) {
	for (uint i=0; i<c.images.size(); i++)
		images.push_back(c.images[i]);
	flatten();
}

// combines or "flatten" all images in the cluster into one so clusters of images can be compared
// this should be called whenever the images in the cluster's images changes
void cluster::flatten() {
	if (combinedImage.perceptronEvalCache.size() == 0)
		combinedImage.setPerceptronEvalCacheSize(image::images[images[0]].perceptronEvalCache.size());
	for (uint i=0; i<images.size(); i++)
		combinedImage.combine(image::images[images[i]]);

	combinedImage.normalize();
}

void cluster::print() const {
	for (uint i=0; i<images.size(); i++)
		cout << image::images[images[i]].getFilename() << " ";
	cout << endl;
}

void cluster::findMostSimilarScores(array<double> &scores, array<int> &mostSimilarIndexes, int &index1, int &index2) {
	double maxScore = 0;
	int maxIndex = 0;
	for (uint i=0; i<scores.size(); i++) {
		if (scores[i] > maxScore) {
			maxScore = scores[i];
			maxIndex = i;
		}
	}
	index1 = maxIndex;
	index2 = mostSimilarIndexes[index1];
}

// fills in the similarity scores memorization table at the begining
// this make cluster merging an n^2 operation instead of an n^3 one
// this allows for updating only the entries of the sim table that need to
// change when one cluster has been merged.  There is no need to recompute
// between images that haven't changed.
// Also see cluster::calculateSimilarityScores
void cluster::precomputeSimilarityScores(array<cluster> &clusters, array<perceptron> &p, array<array<double>> &memtable) {
	for (uint i=0; i<clusters.size(); i++)
		for (uint j=i; j<clusters.size(); j++) {
			if (i == j) continue;
			memtable[i][j] = perceptron::similarity(p,clusters[i].combinedImage,clusters[j].combinedImage);
		}
}

array<double> cluster::calculateSimilarityScores(array<cluster> &clusters, array<int> &mostSimilarIndexes, array<perceptron> &p, array<array<double>> &memtable, int mergeIndex) {
	array<double> scores(clusters.size());

	// recalculate the similarities between images if there was a merge
	if (mergeIndex != -1) {
		int memtableIndex = clusters[mergeIndex].getIndex();
		for (uint i=0; i<clusters.size(); i++) {
			if (i == (uint)mergeIndex) continue;
			double sim = perceptron::similarity(p,clusters[mergeIndex].combinedImage,clusters[i].combinedImage);
			int index = clusters[i].getIndex();
			memtable[index][memtableIndex] = sim;
			memtable[memtableIndex][index] = sim;
		}
	}

	// find what image each image matches best with
	for (uint i=0; i<clusters.size(); i++) {
		double maximum = 0;
		int maxIndex = 0;
		for (uint j=0; j<clusters.size(); j++) {
			if (i == j) continue;
			double sim = memtable[clusters[i].getIndex()][clusters[j].getIndex()];
			if (sim > maximum) {
				maximum = sim;
				maxIndex = j;
			}
		}
		scores[i] = maximum;
		mostSimilarIndexes[i] = maxIndex;
	}
	return scores;
}

int cluster::mergeMostSimilar(array<cluster> &clusters, array<perceptron> &p, array<array<double>> &memtable, int lastMergedIndex) {
	// compare clusters and get scores
	array<int> mostSimilarIndexes(clusters.size());
	array<double> scores = calculateSimilarityScores(clusters, mostSimilarIndexes, p, memtable, lastMergedIndex);

	// find most similar clusters
	int index1, index2;
	findMostSimilarScores(scores, mostSimilarIndexes, index1, index2);

	// combine them into one and remove the other
	clusters[index1].combine(clusters[index2]);
	// clusters.erase(clusters.begin() + index2);
	clusters.remove(index2);
	return index1;
}
