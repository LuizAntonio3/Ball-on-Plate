Matrix<2, 2> Qx = {
  0.7316, 0.3238,
  0.3238, 0.2868
};

Matrix<2, 2> Qe = {
  583.1531, 273.1843,
  273.1843, 127.9761
};

Matrix<2, 2> P = {
  68.1078, 25.2748,
  25.2748, 13.8430
};

Matrix<1, 2> K = {
  3.3657, 1.5767
};

bool ETM(Matrix<2, 1> *x, Matrix<2, 1> *x_lt, float phi, float lambda, float theta, float *eta, float *Gamma) {
  
  Matrix<2, 1> e = (*x) - (*x_lt);
  *eta = (1 - lambda) * (*eta) + (*Gamma); // lambda is a chosen (0,1);
  // here the next 'eta' is calculated with n(k-1) and Gamma(k-1) values
  *Gamma = (~(*x) * Qx * (*x) - ~e * Qe * e)(0);

  float trigger = (*eta) + theta * (*Gamma);
  if (trigger > 0)
    return false;

  // allow transmission
  return true;
}

void ETM_UpdateLt(Matrix<2, 1> *x, Matrix<2, 1> *x_lt, float *Gamma) {
  *x_lt = *x;
  *Gamma = (~(*x) * Qx * (*x))(0);
}
