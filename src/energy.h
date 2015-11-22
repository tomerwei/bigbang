#ifndef ENERGY_H
#define ENERGY_H

#include <zjucad/matrix/matrix.h>
#include <Eigen/Sparse>
#include <unordered_map>

#include "def.h"

using mati_t=zjucad::matrix::matrix<size_t>;
using matd_t=zjucad::matrix::matrix<double>;

namespace bigbang {

class momentum_potential : public Functional<double>
{
public:
  ~momentum_potential() {}
  virtual size_t Nx() const = 0;
  virtual int Val(const double *x, double *val) const = 0;
  virtual int Gra(const double *x, double *gra) const = 0;
  virtual int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const = 0;
  virtual void Update(const double *x) = 0;
  virtual double QueryKineticEnergy() const = 0;
  virtual const Eigen::SparseMatrix<double>& MassMatrix() const = 0;
  virtual const Eigen::VectorXd& CurrVelocity() const = 0;
  virtual double timestep() const = 0;
};

class momentum_potential_imp_euler : public momentum_potential
{
public:
  momentum_potential_imp_euler(const mati_t &cell, const matd_t &nods, const double rho, const double h, const double w=1.0);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
  void Init(const double *x0, const double *v0);
  void Update(const double *x);
  double QueryKineticEnergy() const;
  const Eigen::SparseMatrix<double>& MassMatrix() const { return M_; }
  const Eigen::VectorXd& CurrVelocity() const { return vn_; }
  double timestep() const { return h_; }
private:
  const double rho_, h_;
  const size_t dim_;
  double w_;
  Eigen::SparseMatrix<double> M_;
  Eigen::VectorXd xn_, vn_;
};

class momentum_potential_bdf2 : public momentum_potential
{
public:
  momentum_potential_bdf2(const mati_t &cell, const matd_t &nods, const double rho, const double h, const double w=1.0);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
  void Init(const double *x0, const double *v0);
  void Update(const double *x);
  double QueryKineticEnergy() const;
  const Eigen::SparseMatrix<double>& MassMatrix() const { return M_; }
  const Eigen::VectorXd& CurrVelocity() const { return vn_; }
  double timestep() const { return h_; }
private:
  const double rho_, h_;
  const size_t dim_;
  double w_;
  Eigen::SparseMatrix<double> M_;
  Eigen::VectorXd xn_, vn_, xnn_, vnn_;
};

class elastic_potential : public Functional<double>
{
public:
  enum Material {
    LINEAR,
    STVK,
    COROTATIONAL,
    NEOHOOKEAN
  };
  elastic_potential(const mati_t &tets, const matd_t &nods, Material type, const double Ym, const double Pr, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
private:
  const size_t dim_;
  double w_;
  Material type_;
  const mati_t &tets_;
  matd_t vol_;
  double lam_, miu_;
  matd_t Dm_;
};

class voxel_elastic_potential : public Functional<double>
{
public:
  enum Material {
    LINEAR,
    STVK
  };
  voxel_elastic_potential(const mati_t &cube, const matd_t &nods, Material type,
                          const double Ym, const double Pr, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
private:
  const size_t dim_;
  double w_;
  Material type_;
  const mati_t &cube_;
  double h_, lam_, miu_;
};

class gravitational_potential : public Functional<double>
{
public:
  gravitational_potential(const mati_t &cell, const matd_t &nods, const double rho, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const { return __LINE__; }
private:
  const size_t dim_;
  double w_;
  Eigen::SparseMatrix<double> M_;
};

class positional_potential : public Functional<double>
{
public:
  positional_potential(const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
  int Pin(const size_t id, const double *pos);
  int Release(const size_t id);
private:
  const size_t dim_;
  double w_;
  std::unordered_map<size_t, Eigen::Vector3d> fixed_;
};

class spring_potential : public Functional<double>
{
public:
  spring_potential(const mati_t &edge, const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
  void ResetWeight(const double w) { w_ = w; }
private:
  const size_t dim_;
  double w_;
  const mati_t &edge_;
  matd_t len_;
};

class fast_mass_spring : public Functional<double>
{
public:
  fast_mass_spring(const mati_t &edge, const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
  void LocalSolve(const double *x);
private:
  const size_t dim_;
  double w_;
  const mati_t &edge_;
  matd_t len_;
  matd_t d_;
};

class line_bending_potential : public Functional<double>
{
public:
  line_bending_potential(const mati_t &edge, const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
private:
  const size_t dim_;
  double w_;
  const mati_t &edge_;
  matd_t len_;
};

class surf_bending_potential : public Functional<double>
{
public:
  surf_bending_potential(const mati_t &diams, const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
  void ResetWeight(const double w) { w_ = w; }
private:
  const size_t dim_;
  double w_;
  const mati_t &diams_;
  matd_t len_, area_, angle_;
};

class ext_force_energy : public Functional<double>
{
public:
  ext_force_energy(const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const { return __LINE__; }
  int ApplyForce(const size_t id, const double *f);
  int RemoveForce(const size_t id);
private:
  const size_t dim_;
  double w_;
  matd_t force_;
};

class isometric_bending : public Functional<double>
{
public:
  isometric_bending(const mati_t &diams, const matd_t &nods, const double w);
  size_t Nx() const;
  int Val(const double *x, double *val) const;
  int Gra(const double *x, double *gra) const;
  int Hes(const double *x, std::vector<Eigen::Triplet<double>> *hes) const;
private:
  const size_t dim_;
  const mati_t &diams_;
  double w_;
  matd_t cotv_, area_;
};

/// !@todo check
class position_constraint : public Constraint<double>
{
public:
  position_constraint(const matd_t &nods, const double w=1.0);
  size_t Nx() const { return dim_; }
  size_t Nf() const { return 3*fixed_.size(); }
  int Val(const double *x, double *val) const;
  int Jac(const double *x, const size_t off, std::vector<Eigen::Triplet<double>> *jac) const;
  int Pin(const size_t pid, const double *pos);
  int Release(const size_t pid);
private:
  double w_;
  const size_t dim_;
  std::unordered_map<size_t, Eigen::Vector3d> fixed_;
};

/// !@todo finish
class inextensible_constraint : public Constraint<double>
{
public:
  inextensible_constraint(const mati_t &edge, const matd_t &nods, const double w=1.0);
  size_t Nx() const { return dim_; }
  size_t Nf() const { return edges_.size(2); }
  int Val(const double *x, double *val) const;
  int Jac(const double *x, const size_t off, std::vector<Eigen::Triplet<double>> *jac) const;
private:
  double w_;
  const size_t dim_;
  const mati_t &edges_;
  matd_t len_;
};

}

#endif
