#include "histogram.h"

histogram::histogram() : __counter(0) {
	for (int i=0; i<64; i++)
		__data[i] = 0;
}

void histogram::normalize() {
	for (int i=0; i<64; i++)
		data[i] /= counter;
	counter = 1;
}

// write to data but read from __data this allows reading to be faster
void histogram::__normalize() {
	for (int i=0; i<64; i++)
		data[i] = (double)__data[i] / (double)__counter;
	counter = 1;
}

void histogram::combine(const histogram &other) {
	for (int i=0; i<64; i++)
		data[i] += other.data[i];
	counter += other.counter;
	normalize();
}
