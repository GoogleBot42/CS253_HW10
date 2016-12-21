#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "readfile.h"

using std::cerr;
using std::endl;
using std::vector;
using std::string;

void readList(string filename, vector<string> &list) {
	list.resize(0);

	std::ifstream file(filename, std::ifstream::in);

	if (!file) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	string line;
	while (file >> line)
		list.push_back(line);
	file.close();
}

int getClassNum(const string &filename) {
	int uniqueNum;
	int clusterNum;
	if (sscanf(filename.c_str(), "class%d_%d.pgm", &clusterNum, &uniqueNum) != 2) {
		cerr << "Filename " << filename << " must take the form classN_#.pgm." << endl;
		exit(-1);
	}
	return clusterNum;
}


// v----------- For reading pgmimages ---------v

// ascii whitespace lookup table
// includes: ' ', '\n', and '\r'
bool isWhiteSpaceTable [256] = {
	false, false, false, false, false, false, false, false, false, false, true,  false, false, true,  false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	true,  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
};

int isNumTable [256] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};


inline bool isWhiteSpace(char c) {
	//return c == ' ' || c == '\n' || c == '\r';
	return isWhiteSpaceTable[(int)c];
}

inline bool toNum(char c, int &val) {
	return (val = isNumTable[(int)c]) != -1;
}

inline bool readThreeDigitNum(char c1, char c2, char c3, int &num) {
	num = 0;
	int n1,n2,n3;
	if (!toNum(c1,n1)) return false;
	if (isWhiteSpace(c2)) {
		// one digit num
		num += n1;
	} else if (isWhiteSpace(c3)) {
		// two digit num
		if (!toNum(c2,n2)) return false;
		num += n1 * 10;
		num += n2;
	} else {
		// three digit num
		if (!toNum(c2,n2) || !toNum(c3,n3)) return false;
		num += n1 * 100;
		num += n2 * 10;
		num += n3;
	}
	return true;
}

// increments the data pointer passed the current token
// this is very hackish and will only for for parsing tokens
// that are three chars or less if a char is bigger than that
// this function will error upon attempting to pass the token
// the next time it is run
bool increment(char *&data, int &bytesLeft, char &c1, char &c2, char &c3) {
	int count = 1;
	// get to a place that is whitespace, now we have passed the data that we just read
	while (!isWhiteSpace(data[count])) { count++; }

	// we should not have gone over anything bigger than three chars
	// that would be an error (I am taking advantage that no token will be longer than
	// three chars in a text file pgm image)
	if (count > 3) {
		cerr << "Unrecognized token in file." << endl; // too long
		exit(-1);
	}

	// now skip over all of the whitespace
	while (isWhiteSpace(data[count])) { count++; }

	bytesLeft -= count;

	if (bytesLeft <= 0) {
		// out of data
		return true;
	}

	// now we are at some data that is interesting
	// update each char so that together they
	// represent the token
	data += count;
	c1 = data[0];
	c2 = data[1];
	c3 = data[2];

	return false; // a least one more byte left to parse
}

//  entirely hand written, even faster pgm loader

/*   -----------------------------/ ^^^^^^^ \       */
/*   /                             |  | * * |  |    */
/*   / |   )                   |  ||\__/  @  \__/   */
/*   \/   \ / /----------\______/ \ //     '-'      */
/*    ||=|=                   ||=|=                 */

// This function has always been the slowest part of my program
// so I dedicate it my dog Daisy.  My favorite stubborn corgi. RIP
void readPGM(const string &filename, histogram &h, int &clusterNum) {
	int uniqueNum;
	if (sscanf(filename.c_str(), "class%d_%d.pgm", &clusterNum, &uniqueNum) != 2) {
		cerr << "Filename " << filename << " must take the form classN_#.pgm." << endl;
		exit(-1);
	}

	int FILE;
	char *src;
	struct stat statbuf;

	FILE = open(filename.c_str(), O_RDONLY);
	if (FILE < 0) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	// find the size of the file
	if (fstat(FILE,&statbuf) < 0) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	// map to memory
	src = (char *)mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, FILE, 0);
	if (src == (caddr_t) -1) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	// close file
	close(FILE);

	char *data = src;
	int size = statbuf.st_size;
	int bytesLeft = size;

	// make sure that there is at least room for a header
	// because we assume that below
	if (size < 2) {
		cerr << "No pgm image file header in " << filename;
		cerr << " (Needs a \"P2\")" << endl;
		exit(-1);
	}

	bool outOfMem = false;
	char c1, c2, c3;
	c1 = data[0];
	c2 = data[1];

	if (c1 != 'P' || c2 != '2') {
		cerr << "No pgm image file header in " << filename;
		cerr << " (Needs a \"P2\")" << endl;
		exit(-1);
	}

	// width, height, and image depth
	// all of which are static in the programming assignment
	// so I am taking advange so I can make it faster :D
	outOfMem = increment(data, bytesLeft, c1, c2, c3);
	if (outOfMem || c1 != '1' || c2 != '2' || c3 != '8') {
		cerr << "No valid width in " << filename << endl;
		exit(-1);
	}
	outOfMem = increment(data, bytesLeft, c1, c2, c3);
	if (outOfMem || c1 != '1' || c2 != '2' || c3 != '8') {
		cerr << "No valid height in " << filename << endl;
		exit(-1);
	}
	outOfMem = increment(data, bytesLeft, c1, c2, c3);
	if (outOfMem || c1 != '2' || c2 != '5' || c3 != '5') {
		cerr << filename << " Only 255 can be image depth." << endl;
		exit(-1);
	}

	// now for the actual data

	// how many we have parsed
	int count = 0;
	// when we are done cout should equal EXPECTED_COUNT
	const int EXPECTED_COUNT = 128*128;

	outOfMem = increment(data, bytesLeft, c1, c2, c3);

	while (!outOfMem) {
		int value;
		if (!readThreeDigitNum(c1,c2,c3,value)) {
			cerr << "Only numbers can be in " << filename << endl;
			exit(-1);
		}
		if (value < 0 || value > 255) {
			cerr << "\t" << c1 << c2 << c3 << "\t" << value << endl;
			cerr << "The numbers in " << filename << " can only range from [0-255]." << endl;
			exit(-1);
		}
		h.addData(value);
		count++;
		outOfMem = increment(data, bytesLeft, c1, c2, c3);
	}
	if (count != EXPECTED_COUNT) {
		cerr << "Width * Height and data values count mismatch in " << filename << endl;
		cerr << "Found " << count << " data values." << endl;
		exit(-1);
	}

	// close memory mapped file
	munmap(src,size);
}


// old versions of helper functions

/*inline bool isNum(char c) {
	return c >= '0' && c <= '9';
}

inline int toNum(char c) {
	return c - '0';
}

inline bool readThreeDigitNum(char c1, char c2, char c3, int &num) {
	num = 0;
	if (!isNum(c1)) return false;
	if (isWhiteSpace(c2)) {
		// one digit num
		num += toNum(c1);
	} else if (isWhiteSpace(c3)) {
		// two digit num
		if (!isNum(c2)) return false;
		num += toNum(c1) * 10;
		num += toNum(c2);
	} else {
		// three digit num
		if (!isNum(c2) || !isNum(c3)) return false;
		num += toNum(c1) * 100;
		num += toNum(c2) * 10;
		num += toNum(c3);
	}
	return true;
}*/








// major editions of the old versions for comparison

/*inline bool readThreeDigitNum(char *s, int &num) {
	num = 0;
	int length = strlen(s);
	if (length > 3) return false;
	for (int i=0; i<length; i++) {
		num *= 10;
		char c = s[i];
		if (c >= '0' && c <= '9')
			num += c - '0';
		else
			return false;
	}
	return true;
}*/

// This function is a rewrite. The old one is below
// This one runs in about half the time because up
// until the rewrite, this function was  litterally
// 95% of my runtime.
/*void readPGM(string filename, histogram &h, int &clusterNum, char *buffer) {
	int uniqueNum;
	if (sscanf(filename.c_str(), "class%d_%d.pgm", &clusterNum, &uniqueNum) != 2) {
		cerr << "Filename " << filename << " must take the form classN_#.pgm." << endl;
		exit(-1);
	}

	// not os dependent but slower?
	// FILE *f = fopen(filename.c_str(), "r");
	//
	// if (f == NULL) {
	// 	cerr << "Cannot open " << filename << endl;
	// 	exit(-1);
	// }
	//
	// fseek(f, 0, SEEK_END);
	// long fsize = ftell(f);
	// fseek(f, 0, SEEK_SET);
	// fread(buffer, fsize, 1, f);
	// fclose(f);
	//
	// buffer[fsize] = '\0';

	int FILE;
	char *src;
	struct stat statbuf;

	FILE = open(filename.c_str(), O_RDONLY);
	if (FILE < 0) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	// find the size of the file
	if (fstat(FILE,&statbuf) < 0) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	src = (char *)mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, FILE, 0);
	if (src == (caddr_t) -1) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	memcpy(buffer, src, statbuf.st_size);
	buffer[statbuf.st_size] = 0;

	// create tokenizer
	const char *sep = " \r\n";
	char *saveptr; // need for the threadsafe version of strtok
	char *next = strtok_r(buffer, sep, &saveptr);

	if (next == NULL || strcmp(next,"P2") != 0) {
		cerr << "No pgm image file header in " << filename;
		cerr << " (Needs a \"P2\")" << endl;
		exit(-1);
	}

	// width, height, and image depth
	// all of which are static in the programming assignment
	// so I am taking advange so I can make it faster :D
	next = strtok_r(NULL, sep, &saveptr);
	if (next == NULL || strcmp(next, "128")) {
		cerr << "No valid width in " << filename << endl;
		exit(-1);
	}
	next = strtok_r(NULL, sep, &saveptr);
	if (next == NULL || strcmp(next, "128")) {
		cerr << "No valid height in " << filename << endl;
		exit(-1);
	}
	next = strtok_r(NULL, sep, &saveptr);
	if (next == NULL || strcmp(next, "255")) {
		cerr << filename << " Only 255 can be image depth." << endl;
		exit(-1);
	}

	// now for the actual data

	// how many we have parsed
	int count = 0;
	// when we are done cout should equal EXPECTED_COUNT
	const int EXPECTED_COUNT = 128*128;

	next = strtok_r(NULL, sep, &saveptr);

	while (next != NULL) {
		int value;
		if (!readThreeDigitNum(next, value)) {
			cerr << "Only numbers can be in " << filename << endl;
			cerr << "Found \"" << next << "\" instead." << endl;
			exit(-1);
		}
		// too slow
		char *end;
		// long value = strtol(next, &end, 10);
		// if (end == next || *end != '\0' || errno == ERANGE) {
		// 	cerr << "Only numbers can be in " << filename << endl;
		// 	cerr << "Found \"" << next << "\" instead." << endl;
		// 	exit(-1);
		// }
		if (value < 0 || value > 255) {
			cerr << "The numbers in " << filename << " can only range from [0-255]." << endl;
			exit(-1);
		}
		h.addData(value);
		count++; next = strtok_r(NULL, sep, &saveptr);
	}
	if (count != EXPECTED_COUNT) {
		cerr << "Width * Height and data values count mismatch in " << filename << endl;
		cerr << "Found " << count << " data values." << endl;
		exit(-1);
	}
}*/

// old version just for comparison

/*void readPGM(string filename, histogram &h, int &clusterNum, char *buffer) {
	int width;
	int height;

	int uniqueNum;
	if (sscanf(filename.c_str(), "class%d_%d.pgm", &clusterNum, &uniqueNum) != 2) {
		cerr << "Filename " << filename << " must take the form classN_#.pgm." << endl;
		exit(-1);
	}

	std::ifstream inputFile;
	inputFile.open(filename.c_str());

	if (!inputFile) {
		cerr << "Cannot open " << filename << endl;
		exit(-1);
	}

	std::stringstream file;
	file << inputFile.rdbuf();
	inputFile.close();

	char header[128];
	file >> header;
	if (strcmp(header,"P2")) {
		cerr << "No pgm image file header in " << filename;
		cerr << " (Needs a \"P2\")" << endl;
		exit(-1);
	}

	file >> width;
	file >> height;

	if (file.fail()) {
		cerr << "No width and height in " << filename << endl;
		exit(-1);
	}

	if (width != 128 && height != 128) {
		cerr << "Width and height must be both 128 in " << filename << endl;
		exit(-1);
	}

	int tmp;
	file >> tmp;
	if (file.fail()) {
		cerr << "No max image depth defined in " << filename << endl;
		exit(-1);
	}
	if (tmp != 255) {
		cerr << filename << " Only 255 can be the max image width." << endl;
		exit(-1);
	}

	int num;
	int count = 0;
	while (true) {
		file >> num;
		if (file.eof() && file.fail()) break;
		if (file.fail()) {
			cerr << "Only numbers can be in " << filename << endl;
			exit(-1);
		}
		if (num < 0 || num >= 256) {
			cerr << "The numbers in " << filename << " can only range from [0-255]." << endl;
			exit(-1);
		}
		count++;
		h.addData(num);
	}
	if (count == 0) {
		cerr << filename << " is empty." << endl;
		exit(-1);
	}

	if (width != abs(width) || height != abs(height)) {
		cerr << "Error in " << filename << " cannot have a negative width or height" << endl;
		exit(-1);
	}

	if (count != width * height) {
		cerr << "Width * Height and data values count mismatch in " << filename << endl;
		cerr << "Found " << count << " data values." << endl;
		exit(-1);
	}
}*/
