#ifndef PURE_H
#define PURE_H

double pureWrapDiff(double x, double l) {
  int image = int(floor(x/l));
  x -= image*l;
  if (x >= 0.5*l) x -= l;
  return x;
}

Vector3 pureWrapDiff(Vector3 r, Matrix3 basis, Matrix3 basisInv, int nx, int ny, int nz) {
  Vector3 l = basisInv.transform(r);
  l.x = pureWrapDiff(l.x, nx);
  l.y = pureWrapDiff(l.y, ny);
  l.z = pureWrapDiff(l.z, nz);
  return basis.transform(l);
}


// ------------------------

inline double pureWrapReal(double x, double l) {
  int image = int(floor(x/l));
  return x - image*l;
}

double pureWrap(double x, double r0, double dl) {
  return pureWrapReal(x-r0, dl) + r0;
}

double pureComputeGrad(double d, bool periodic, double r0, double dl, double dr, int n, double v1[], double v2[], double v3[]) {
  int home;

  if (periodic) {
    d = pureWrap(d, r0, dl);
    home = int(floor((d - r0)/dr));
  } else {
    home = int(floor((d - r0)/dr));

    if (home < 0) return 0.0;
    if (home >= n) return 0.0;
  }

  double homeR = home*dr + r0;
  double w = (d - homeR)/dr;
  double w2 = w*w;

  // Interpolate.
  return (3.0*v3[home]*w2 + 2.0*v2[home]*w + v1[home])/dr;
}



#endif