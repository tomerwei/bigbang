#ifndef FAST_PROJ_H
#define FAST_PROJ_H

#include <zjucad/matrix/matrix.h>
#include <Eigen/Sparse>

namespace bigbang {

template <typename T>
class Functional;

template <typename T>
class Constraint;

using mati_t=zjucad::matrix::matrix<size_t>;
using matd_t=zjucad::matrix::matrix<double>;
using pfunc_t=std::shared_ptr<Functional<double>>;
using pcons_t=std::shared_ptr<Constraint<double>>;

struct inext_cloth_args {
  double density;
  double timestep;
  double ws, wb, wg;
};

class inext_cloth_solver
{
public:
  inext_cloth_solver(const mati_t &tris, const matd_t &nods);
  int initialize(const inext_cloth_args &args);
  void pin_down_vert(const size_t id, const double *pos);
  void release_vert(const size_t id);
  void apply_force(const size_t id, const double *f);
  void remove_force(const size_t id);
  int advance(double *x) const;
private:
  const size_t dim_;
  const mati_t &tris_;
  const matd_t &nods_;
  inext_cloth_args args_;
  mati_t edges_, diams_;

  std::vector<pfunc_t> ebf_;
  pfunc_t energy_;
  std::vector<pcons_t> cbf_;
  pcons_t constraint_;
};

}

#endif
