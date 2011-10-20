#ifndef FAU_POT_BASE_H
#define FAU_POT_BASE_H

#include <faunus/common.h>
#include <faunus/point.h>
#include <faunus/inputfile.h>
#include <faunus/physconst.h>
#include <faunus/geometry.h>
#include <faunus/potentials.h>
#include <faunus/textio.h>

namespace Faunus {

  namespace Potential {

    PairPotentialBase::PairPotentialBase() {
      setScale(1);
    }
    
    void PairPotentialBase::setScale(double s)  {
      _tokT=s;
      _setScale(_tokT);  
    }
    
    void PairPotentialBase::_setScale(double s) {}
    
    double PairPotentialBase::tokT() { return _tokT; }

    string PairPotentialBase::brief() {
      return name + ": " + _brief();
    }
    string PairPotentialBase::_brief() {
      return string("This is potential base!");
    }

    Harmonic::Harmonic(double forceconst, double eqdist) : k(forceconst), req(eqdist) {
      name="Harmonic";
    }

    void Harmonic::_setScale(double s) {
      _tokT=s;
      k=k/_tokT;
    }

    double Harmonic::energy(const particle &a, const particle &b, double r2) const {
      double d=sqrt(r2)-req;
      return k*d*d;
    }

    string Harmonic::_brief() {
      using namespace Faunus::textio;
      std::ostringstream o;
      o << "k=" << k*tokT() << kT << "/" << angstrom << squared << " req=" << req << _angstrom; 
      return o.str();
    }

    HardSphere::HardSphere(double infinity) {
      name="Hardsphere";
      inf=infinity;
    }
    
    string HardSphere::_brief() {
      return name;
    }

    LennardJones::LennardJones() {
      name="Lennard-Jones";
    }
    
    LennardJones::LennardJones(InputMap &in) {
      name="Lennard-Jones";
      eps = 4*in.get<double>( "lj_eps", 0.04 );
    }
    
    string LennardJones::_brief() {
      std::ostringstream o;
      o << "eps=" << eps*tokT() << textio::kT;
      return o.str();
    }
    
    void LennardJones::_setScale(double s) {
      _tokT=1;
      eps=eps/_tokT;
    }

    string LennardJones::info(char w) {
      std::ostringstream o;
      return o.str();
    }

    SquareWell::SquareWell(InputMap &in, string prefix) {
      name="Square Well";
      threshold = in.get<double>(prefix+"_threshold", 0);
      depth     = in.get<double>(prefix+"_depth", 0);
    }
    
    void SquareWell::_setScale(double s) {
      _tokT=s;
      threshold=threshold/_tokT;
    }
    
    string SquareWell::_brief() {
      std::ostringstream o;
      o << "u=" << depth*tokT() << textio::kT << " r=" << threshold;
      return o.str();
    }

    string SquareWell::info(char w) {
      std::ostringstream o;
      o << pad(SUB,w,"Threshold") << threshold*tokT() << " "+angstrom << endl;
      o << pad(SUB,w,"Depth") << depth*tokT() << kT << endl;
      return o.str();
    }

    Coulomb::Coulomb(InputMap &in) {
      name="Coulomb";
      temp=in.get("temperature", 298.15);
      epsilon_r=in.get("epsilon_r",80.);
      pc::T = temp;
      lB=pc::lB( epsilon_r );
      setScale(lB);
    }
    
    void Coulomb::_setScale(double s) {
    }
    
    string Coulomb::_brief() {
      std::ostringstream o;
      o << "lB=" << lB << " eps_r=" << epsilon_r << " T=" << temp;
      return o.str();
    }

    string Coulomb::info(char w) {
      std::ostringstream o;
      o << pad(SUB,w,"Temperature") << temp << " K" << endl
        << pad(SUB,w,"Dielectric constant") << epsilon_r << endl
        << pad(SUB,w,"Bjerrum length") << lB << " "+angstrom << endl;
      return o.str();
    }

    DebyeHuckel::DebyeHuckel(InputMap &in) : Coulomb(in) {
      double I;
      const double zero=1e-10;
      name="Debye-Huckel";
      c=8 * lB * pc::pi * pc::Nav / 1e27;
      I=in.get<double>("dh_ionicstrength",0);  // [mol/l]
      k=sqrt( I*c );
      if (k<zero)
        k=1/in.get<double>("dh_debyelength", 1/zero); // [A]
      k=-k;
    }
    
    string DebyeHuckel::_brief() {
      std::ostringstream o;
      o << Coulomb::_brief() << " I=" << ionicStrength();
      return o.str();
    }

    double DebyeHuckel::ionicStrength() const {
      return k*k/c;
    } //!< in [mol/l]

    double DebyeHuckel::debyeLength() const {
      return -1/k;
    } //!< in [A]

    string DebyeHuckel::info(char w) {
      std::ostringstream o;
      o << Coulomb::info(w);
      o << pad(SUB,w,"Ionic strength") << ionicStrength() << " mol/l" << endl;
      o << pad(SUB,w,"Debye length, 1/\u03BA") << debyeLength() << " "+angstrom << endl;
      return o.str();
    }

  } //Potential namespace

} //Faunus namespace
#endif
