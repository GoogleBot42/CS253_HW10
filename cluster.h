#pragma once

#include <vector>

#include "image.h"
#include "perceptron.h"

class cluster {
	// combines list of images into a single one
	void flatten();
public:
	void addImage(const int i);
	void combine(const cluster &c);
	void print() const;

	inline int getIndex() const { return clusterIndex; }
	inline void setIndex(int index) { clusterIndex = index; }

	static void precomputeSimilarityScores(array<cluster> &clusters, array<perceptron> &p, array<array<double>> &memtable);
	static void findMostSimilarScores(array<double> &scores, array<int> &mostSimilarIndexes, int &index1, int &index2);
	static array<double> calculateSimilarityScores(array<cluster> &clusters, array<int> &mostSimilarIndexes, array<perceptron> &p, array<array<double>> &memtable, int mergeIndex);
	static int mergeMostSimilar(array<cluster> &clusters, array<perceptron> &p, array<array<double>> &memtable, int lastMergedIndex);
protected:
	// this var is used for memorization so the compare function is only called when needed
	// the index of the cluster in the cluster array changes but this stays the same to
	// avoid comparisons whenever possible
	int clusterIndex;

	image combinedImage;
	vector<int> images;
};
