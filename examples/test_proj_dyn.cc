#include <iostream>
#include <fstream>
#include <jtflib/mesh/io.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "src/vtk.h"
#include "src/io.h"
#include "src/proj_dynamics.h"

using namespace std;
using namespace bigbang;
using namespace zjucad::matrix;
namespace po = boost::program_options;

namespace test_proj_dyn {
struct argument {
  string input_mesh;
  string input_cons;
  string output_folder;
  size_t total_frame;
  proj_dyn_args proj_args;
};
}

#define APPLY_FORCE(frame, id, f)    \
  if ( i == frame )                  \
    solver.apply_force(id, f);

#define REMOVE_FORCE(frame, id)      \
  if ( i == frame )                  \
    solver.remove_force(id);

#define RELEASE_VERT(frame, id)      \
  if ( i == frame )                  \
    solver.release_vert(id);

int main(int argc, char *argv[])
{
  po::options_description desc("Available options");
  desc.add_options()
      ("help,h", "produce help message")
      ("input_mesh,i", po::value<string>(), "input mesh")
      ("input_cons,c", po::value<string>(), "input positional constraints")
      ("output_folder,o", po::value<string>(), "output folder")
      ("frames,n", po::value<size_t>()->default_value(300), "frame number")
      ("method", po::value<int>()->default_value(0), "choose the method")
      ("spectral_radius", po::value<double>()->default_value(0.0), "spectral radius")
      ("density,d", po::value<double>()->default_value(1.0), "density")
      ("timestep,t", po::value<double>()->default_value(0.01), "timestep")
      ("maxiter,m", po::value<size_t>()->default_value(10000), "maximum iteration")
      ("tolerance,e", po::value<double>()->default_value(1e-8), "converge tolerance")
      ("ws", po::value<double>()->default_value(1e4), "stretch weight")
      ("wb", po::value<double>()->default_value(1e0), "bending weight")
      ("wg", po::value<double>()->default_value(1.0), "gravity weight")
      ("wp", po::value<double>()->default_value(1e3), "position weight")
      ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  if ( vm.count("help") ) {
    cout << desc << endl;
    return __LINE__;
  }
  test_proj_dyn::argument args; {
    args.input_mesh = vm["input_mesh"].as<string>();
    args.input_cons = vm["input_cons"].as<string>();
    args.output_folder = vm["output_folder"].as<string>();
    args.total_frame = vm["frames"].as<size_t>();
    args.proj_args.rho = vm["density"].as<double>();
    args.proj_args.h = vm["timestep"].as<double>();
    args.proj_args.maxiter = vm["maxiter"].as<size_t>();
    args.proj_args.method = vm["method"].as<int>();
    args.proj_args.sr = vm["spectral_radius"].as<double>();
    args.proj_args.eps = vm["tolerance"].as<double>();
    args.proj_args.ws = vm["ws"].as<double>();
    args.proj_args.wb = vm["wb"].as<double>();
    args.proj_args.wg = vm["wg"].as<double>();
    args.proj_args.wp = vm["wp"].as<double>();
  }

  if ( !boost::filesystem::exists(args.output_folder) )
    boost::filesystem::create_directories(args.output_folder);

  // load input
  mati_t tris; matd_t nods;
  jtf::mesh::load_obj(args.input_mesh.c_str(), tris, nods);
  vector<size_t> fixed;
  read_fixed_verts(args.input_cons.c_str(), fixed);

  // init the solver
  proj_dyn_spring_solver solver(tris, nods);
  solver.initialize(args.proj_args);

  // initial boudary conditions
  for (auto &elem : fixed)
    solver.pin_down_vert(elem, &nods(0, elem));

  // precompute
  solver.precompute();

  char outfile[256];
  double f[3] = {-200, 0, -200};
  for (size_t i = 0; i < args.total_frame; ++i) {
    cout << "[info] frame " << i << endl;
    sprintf(outfile, "%s/frm_%04zu.vtk", args.output_folder.c_str(), i);
    ofstream os(outfile);
    tri2vtk(os, &nods[0], nods.size(2), &tris[0], tris.size(2));
    os.close();
    
//    APPLY_FORCE(0, 3, f);
//    REMOVE_FORCE(40, 3);
//    RELEASE_VERT(160, 2);

    solver.advance(&nods[0]);

    // sprintf(outfile, "%s/rotation_method%d_frame%zu.vtk", args.output_folder.c_str(), args.proj_args.method, i);
    // solver.vis_rot(outfile);
    // sprintf(outfile, "%s/trajectory_method%d_frame%zu.vtk", args.output_folder.c_str(), args.proj_args.method, i);
    // solver.draw_trajectory(outfile);
  }

  cout << "[info] all done\n";
  return 0;
}
