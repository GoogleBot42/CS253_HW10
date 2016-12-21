#pragma once

#include <cstring> // memcpy
#include <algorithm> // std::min

#include "typedef.h"

// My own vector-like object.  It is super simple.
// IT DISABLES COPYING BECAUSE I JUST DON'T NEED IT

// The goal of this was to have a resizable array
// that doesn't initialize it's data unlike it's
// std::vector counterpart...

template <typename T>
class array {
public:
  array(uint size=0) : _size(size), data(new T[size]) {
   }
  void operator= (array<T> other) {
    data = other.data;
    _size = other._size;

    other.data = NULL;
  }
  ~array() { if (data) delete[] data; }

  void resize(uint s) {

    T *newdata = new T[s];

    // handle shrink or grow
    int amountToCopy = std::min(s,_size);

    memcpy(newdata,data,amountToCopy);

    delete[] data;

    data = newdata;
    _size = s;
  }

  void remove(uint index) { // removes element at i; only pretends to change array size for speed (hopefully)
    for (uint i=index; i<_size-1; i++)
      data[i] = data[i+1];
    _size--;
    // zero the tailing element (a workaround for a double delete bug)
    // i know that this probably isn't the right way to handle this :/
    memset(data+_size,0,sizeof(T));
  }

  inline uint size() const { return _size; }
  inline T& operator[] (uint index) { return data[index]; }
private:
  uint _size;
  T *data;
};
