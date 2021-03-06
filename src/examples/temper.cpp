#include <faunus/faunus.h>
using namespace Faunus;
typedef Space<Geometry::Cuboid> Tspace;

template<class Tspace>
struct myenergy : public Energy::Energybase<Tspace> {  //custom energy class
  double Tscale;                               //reduced temperature
  double i_external(const p_vec &p, int i)  {  //pot. on particle
    double s=( 1+std::sin(2*pc::pi*p[i].x()) ) / Tscale;
    if (p[i].x() >=-2.00 && p[i].x() <=-1.25) return 1*s;
    if (p[i].x() >=-1.25 && p[i].x() <=-0.25) return 2*s;
    if (p[i].x() >=-0.25 && p[i].x() <= 0.75) return 3*s;
    if (p[i].x() >= 0.75 && p[i].x() <= 1.75) return 4*s;
    if (p[i].x() >= 1.75 && p[i].x() <= 2.00) return 5*s;
    return pc::infty;
  }
  double g_external(const p_vec &p, Group &g) {//pot. on group
    double u=0;
    for (auto i : g)
      u+=i_external(p, i);
    return u; // in kT
  }
  string _info() { return "myenergy"; }       //mandatory info 
};

int main() {
  Faunus::MPI::MPIController mpi;             //init MPI
  InputMap mcp(textio::prefix+"temper.input");//read input file
  MCLoop loop(mcp);                           //handle mc loops
  myenergy<Tspace> pot;                       //our custom potential!
  pot.Tscale = mcp.get("Tscale",1.0);         //temperature from input
  Tspace spc(mcp);                            //create simulation space
  spc.insert( PointParticle() );              //insert a single particle
  Group mygroup(0,0);                         //group with single particle
  spc.enroll(mygroup);                        //tell space about group

  Analysis::LineDistribution<> dst(.05);      //distribution func.
  Move::ParallelTempering<Tspace> pt(mcp,pot,spc,mpi);//temper move
  Move::AtomicTranslation<Tspace> trans(mcp,pot,spc); //translational move
  trans.setGroup(mygroup);                    //set translation group

  mpi.cout << spc.info() << loop.info();      //print initial info

  while ( loop.macroCnt() ) {                 //start markov chain
    while ( loop.microCnt() ) {
      trans.move();                           //translate particle
      dst(spc.p[0].x())++;                    //update histogram
    }
    pt.move();                                //do temper move
    mpi.cout << loop.timing();                //print progress
  }

  dst.save(textio::prefix+"dist");            //save histogram
  mpi.cout << trans.info() << pt.info();      //print final info
}
/**
  @page example_temper Example: Parallel Tempering

  This is an example of parallel tempering taken from the book

  - _Understanding Molecular Simulation_ by Frenkel and Smit, 2nd edition.
  p.391 - Case Study 21, _Parallel Tempering of a Single Particle_.

  We simulate a single particle in an oscillating
  potential and use tempering in temperature (you may use any
  parameter in the Hamiltonian) to overcome energy barriers.
  Below is the one-dimensional distribution function sampled with and without
  tempering, demonstrating how the high T replicas help low T systems to
  pass steep barriers.

  ![Particle distribution function in an oscillating field](temper.png)

  To run this example, make sure faunus is compiled with MPI enabled:

  ~~~
  $ cmake . -DENABLE_MPI=on
  $ make
  $ cd src/examples
  $ ./temper.run
  ~~~

  The tempering routine in Faunus is general and implemented in
  MPI where each replica has its own rank. The temper parameter(s) is
  given simply by giving different input files for each replica.
  The input files, `mpi$rank.temper.input`, for this example look like this:

  ~~~
  loop_macrosteps       2000 # number of temper moves
  loop_microsteps       10000# number of translational moves per temper move
  cuboid_len            4    # Box side length Angstrom
  temper_runfraction    1.0  # Set to one/zero to turn on/off tempering
  temper_format         XYZ  # Exchange only coordinates while tempering
  mv_particle_genericdp 0.5  # translational displacement [angstrom]
  Tscale                1.0  # Reduced temperature
  ~~~

  temper.cpp
  ==========

  @includelineno examples/temper.cpp

*/

