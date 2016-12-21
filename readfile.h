#include <vector>
#include <string>

#include "histogram.h"

using std::vector;
using std::string;

void readList(string filename, vector<string> &list);
void readPGM(const string &filename, histogram &h, int &clusterNum);
int getClassNum(const string &filename);
