#pragma once

#include <vector>

using std::vector;

class histogram {
public:
	histogram();
	inline void addData(int value) { __data[value/4]++; __counter++; }
	void normalize();
	void __normalize(); // normalize using data from __data; must be called after loading image from file!
	void combine(const histogram &other);
	inline double get(int index) const { return data[index]; }
protected:
	double counter;
	double data[64];

	// this data is just mean for when the image is first being read
	// doubles are slow!  and we only need ints when first loading :)
	int __data[64];
	int __counter;
};
