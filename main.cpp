#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <iterator>
#include <algorithm> // for std::max
#include <thread>

#include "cluster.h"
#include "image.h"
#include "readfile.h"
#include "perceptron.h"
#include "typedef.h"
#include "array.h"

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;

// spawn the max threads supported by the systems processor
uint MAX_THREADS;

// loads training images and test images in parallel
void loadSetImages(vector<string> &trainingSet, vector<string> &testSet, // filenames
	                 array<image> &images,
									 array<int> &trainingSetIndices, array<int> &testSetIndices,
									 vector<int> &trainingSetClassNumbers) { // class numbers for training perceptrons
 	trainingSetIndices.resize(trainingSet.size());
 	testSetIndices.resize(testSet.size());
	// too big but we don't know how many duplicates that there are until all
	// of the images have been loaded (this alg is faster than the one that does
	// otherwise that I used to have)
	images.resize(trainingSet.size() + testSet.size());
	trainingSetClassNumbers.resize(trainingSet.size());

	// Time for some thread magic :)
	//
	// start up to numThreads number of threads for loading images
	//   if (MAX_THREADS >= trainingSet.size || MAX_THREADS >= testSet.size) then (numThreads == MAX_THREADS)
	//   ELSE (numTheads == MAX(trainingSet.size,testSet.size))

	uint numThreads = std::max( (MAX_THREADS <= trainingSet.size()) ? MAX_THREADS : trainingSet.size(),
															(MAX_THREADS <= testSet.size()) ? MAX_THREADS : testSet.size() );

	std::vector<std::thread> threads;
	threads.reserve(numThreads);

	for (uint id=0; id<numThreads; id++) {
		threads.push_back(std::thread([&](uint id) {
			// load training images
			for (uint imgIndex=id; imgIndex<trainingSet.size(); imgIndex+=numThreads) {
				images[imgIndex].openFile(trainingSet[imgIndex]);
				trainingSetIndices[imgIndex] = imgIndex;
				trainingSetClassNumbers[imgIndex] = images[imgIndex].getClusterNum();
			}

			// load test images
			for (uint imgIndex=id; imgIndex<testSet.size(); imgIndex+=numThreads) {
				images[imgIndex+trainingSet.size()].openFile(testSet[imgIndex]);
				testSetIndices[imgIndex] = imgIndex + trainingSet.size();
			}
		}, id));
	}

	// now wait for the threads to finish
	for (uint i=0; i<numThreads; i++)
		threads[i].join();
}


// trains the perceptrons in parallel
void trainPerceptrons(array<perceptron> &perceptrons,
											array<image> &images,
											array<int> &trainingSetIndices,
											vector<int> &classNums) {
	// set the size of the arrays
	perceptrons.resize(classNums.size());

	// Now for the threads

	// This works pretty much the same as loading images
	// the thread is offset by it's id plus a multiple of
	// numThreads to make sure that every perceptron is
	// trained and there cannot be any data races.

	// no more threads then the number of perceptrons or MAX_THREADS
	uint numThreads = std::min(MAX_THREADS,(uint)classNums.size());

	std::vector<std::thread> threads;
	threads.reserve(numThreads);

	for (uint id=0; id<numThreads; id++) {
		threads.push_back(std::thread([&](uint id) {
			// train the perceptron(s)
			for (uint percIndex=id; percIndex < classNums.size(); percIndex += numThreads) {
				// set the perceptronIndex (used internally for caching perviously computed values)
				perceptrons[percIndex].setPerceptronIndex(percIndex);

				// set the class number that perceptron will "accept" first
				perceptrons[percIndex].setClusterIndex(classNums[percIndex]);

				// now train it
				for (int k=0; k<100; k++)
				for (uint j=0; j<trainingSetIndices.size(); j++)
					perceptrons[percIndex].train(images[trainingSetIndices[j]]);

				// mark it as done so now it will begin caching when perceptron::evaluate is called
				perceptrons[percIndex].setDoneTraining(true);
			}
		}, id));
	}

	for (uint i=0; i<numThreads; i++)
		threads[i].join();
}

int main(int argc, const char* argv[]) {
	if (argc != 4) {
		cerr << "Usage: ./PA10 <training_set> <test_set> <num_clusters>" << endl;
		return -1;
	}

	// get number of threads the cpu supports
	MAX_THREADS = std::thread::hardware_concurrency();

	// read args
	vector<string> trainingSet;
	vector<string> testSet;
	readList(argv[1], trainingSet);
	readList(argv[2], testSet);
	int targetNumClusters = atoi(argv[3]);

	if (testSet.size() < 2) {
		cerr << "There must be at least 2 files to compare in " << argv[3] << endl;
		return -1;
	}
	if (testSet.size() < (uint)targetNumClusters) {
		cerr << "Number of clusters cannot be greater than the number of test set images." << endl;
		return -1;
	}

	// load images concurrently :D
	// array<image> images; // now a static var in image
	vector<int> trainingSetClasses;
	array<int> trainingSetIndices;
	array<int> testSetIndices;

	loadSetImages(trainingSet,testSet,image::images,trainingSetIndices,testSetIndices,trainingSetClasses);

	// get number of classes (and the classNums) in the training set
	std::set<int> trainingClassesSet(trainingSetClasses.data(),trainingSetClasses.data()+trainingSetClasses.size()-1);
	vector<int> trainingClasses;
	std::copy(trainingClassesSet.begin(), trainingClassesSet.end(), std::back_inserter(trainingClasses));

	// TODO put this into threads somehow
	// each image has a perceptronEvalCache vector
	// that needs to be set to the size of the num
	// of perceptrons
	for (uint i=0; i<image::images.size(); i++)
		image::images[i].setPerceptronEvalCacheSize(trainingClasses.size());

	if (trainingClasses.size() < 2) {
		cerr << "There must be at least 2 class types in the training set." << endl;
		return -1;
	}

	// now it is time to train the perceptrons
	array<perceptron> perceptrons;
	trainPerceptrons(perceptrons, image::images, trainingSetIndices, trainingClasses);

	// init clusters
	array<cluster> clusters(testSet.size());
	for (uint i=0; i<testSet.size(); i++) {
		clusters[i].addImage(testSetIndices[i]);
		clusters[i].setIndex(i);
	}

	// don't ever recalculate similarities
	array<array<double>> memorizedSimilarities(testSet.size());
	for (uint i=0; i<memorizedSimilarities.size(); i++)
		memorizedSimilarities[i].resize(testSet.size());

	cluster::precomputeSimilarityScores(clusters,perceptrons,memorizedSimilarities);

	// merge clusters
	int lastMergedIndex = -1;
	int numClusters = clusters.size();
	while (numClusters != targetNumClusters) {
		lastMergedIndex = cluster::mergeMostSimilar(clusters, perceptrons, memorizedSimilarities, lastMergedIndex);
		numClusters--;
	}

	// output clusters
	for (uint i=0; i<clusters.size(); i++)
		clusters[i].print();
	return 0;
}
