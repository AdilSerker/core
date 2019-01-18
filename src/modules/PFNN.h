#ifndef PFNN_H
#define PFNN_H

#include <stdio.h>
#include <stdarg.h>
#include <eigen3/Eigen/Dense>
#include <iostream>
#include <vector>

using namespace Eigen;

enum { XDIM = 342, YDIM = 311, HDIM = 512 };
enum { MODE_CONSTANT, MODE_LINEAR, MODE_CUBIC };

class PFNN {

  public:
  ArrayXf  Xp, Yp;
  
  PFNN(int pfnnmode);
  void predict(float P);

  ArrayXf getYmean() const {
    return Ymean;
  }

  private:
  int mode;
  
  ArrayXf Xmean, Xstd;
  ArrayXf Ymean, Ystd;
  
  std::vector<ArrayXXf> W0, W1, W2;
  std::vector<ArrayXf>  b0, b1, b2;
  
  ArrayXf  H0,  H1;
  ArrayXXf W0p, W1p, W2p;
  ArrayXf  b0p, b1p, b2p;

  void load_weights(ArrayXXf &A, int rows, int cols, const char* fmt, ...);
  void load_weights(ArrayXf &V, int items, const char* fmt, ...);

  void load();

  void ELU(ArrayXf &x);

  void linear(ArrayXf  &o, const ArrayXf  &y0, const ArrayXf  &y1, float mu);
  void linear(ArrayXXf &o, const ArrayXXf &y0, const ArrayXXf &y1, float mu);
  
  void cubic(ArrayXf  &o, const ArrayXf &y0, const ArrayXf &y1, const ArrayXf &y2, const ArrayXf &y3, float mu);
  
  void cubic(ArrayXXf &o, const ArrayXXf &y0, const ArrayXXf &y1, const ArrayXXf &y2, const ArrayXXf &y3, float mu);
    
};

#endif /* PFNN_H */
