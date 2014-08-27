/*! 
 * \file  MFrontNewtonRaphsonSolvers.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 22 août 2014
 */

#include "MFront/MFrontDebugMode.hxx"
#include "MFront/MechanicalBehaviourDescription.hxx"
#include "MFront/MFrontPerformanceProfiling.hxx"
#include "MFront/MFrontNewtonRaphsonSolvers.hxx"

namespace mfront{

  std::vector<std::string>
  MFrontNewtonRaphsonSolverBase::getReservedNames(void) const
  {
    using namespace std;
    vector<string> n;
    if(this->usesPowellDogLegAlgorithm()){
      const vector<string>& n2 =
	MFrontPowellDogLegAlgorithmBase::getReservedNames();
      n.insert(n.end(),n2.begin(),n2.end());
      n.push_back("tjacobian");
      n.push_back("tfzeros");
    }
    n.push_back("integrate_one_half");
    n.push_back("computeFdF_ok");
    return n;
  } // end of MFrontNewtonRaphsonSolverBase::getReservedNames

  bool
  MFrontNewtonRaphsonSolverBase::usesJacobian(void) const
  {
    return true;
  } // end of MFrontNewtonRaphsonSolverBase::usesJacobian

  bool
  MFrontNewtonRaphsonSolverBase::usesJacobianInvert(void) const
  {
    return false;
  } // end of MFrontNewtonRaphsonSolverBase::usesJacobianInvert

  bool
  MFrontNewtonRaphsonSolverBase::allowsJacobianInitialisation(void) const
  {
    return false;
  } // end of MFrontNewtonRaphsonSolverBase::allowsJacobianInitialisation

  bool
  MFrontNewtonRaphsonSolverBase::allowsJacobianInvertInitialisation(void) const
  {
    return false;
  } // end of MFrontNewtonRaphsonSolverBase::allowsJacobianInvertInitialisation

  bool
  MFrontNewtonRaphsonSolverBase::requiresJacobianToBeReinitialisedToIdentityAtEachIterations(void) const
  {
    return !this->requiresNumericalJacobian();
  } // end of MFrontNewtonRaphsonSolverBase::requiresJacobianToBeReinitialisedToIdentityAtEachIterations

  std::pair<bool,tfel::utilities::CxxTokenizer::TokensContainer::const_iterator>
  MFrontNewtonRaphsonSolverBase::treatSpecificKeywords(MechanicalBehaviourDescription& mb,
						       const std::string& key,
						       const tfel::utilities::CxxTokenizer::TokensContainer::const_iterator p,
						       const tfel::utilities::CxxTokenizer::TokensContainer::const_iterator pe) const
  {
    using namespace std;
    if(this->usesPowellDogLegAlgorithm()){
      return MFrontPowellDogLegAlgorithmBase::treatSpecificKeywords(mb,key,p,pe);
    }
    return make_pair(false,p);
  } // end of MFrontNewtonRaphsonSolverBase::treatSpecificKeywords

  void
  MFrontNewtonRaphsonSolverBase::endsInputFileProcessing(MechanicalBehaviourDescription& mb) const
  {
    using namespace std;
    if(this->usesPowellDogLegAlgorithm()){
      return MFrontPowellDogLegAlgorithmBase::endsInputFileProcessing(mb);
    }
  } // end of MFrontNewtonRaphsonSolverBase::endsInputFileProcessing

  void
  MFrontNewtonRaphsonSolverBase::writeSpecificInitializeMethodPart(std::ostream&,
								   const MechanicalBehaviourDescription&,
								   const tfel::material::ModellingHypothesis::Hypothesis) const
  {} // end of MFrontNewtonRaphsonSolverBase::writeSpecificMembers

  void
  MFrontNewtonRaphsonSolverBase::writeSpecificMembers(std::ostream&,
						      const MechanicalBehaviourDescription&,
						      const tfel::material::ModellingHypothesis::Hypothesis) const
  {} // end of MFrontNewtonRaphsonSolverBase::writeSpecificMembers

  void
  MFrontNewtonRaphsonSolverBase::writeResolutionAlgorithm(std::ostream& out,
							  const MechanicalBehaviourDescription& mb,
							  const tfel::material::ModellingHypothesis::Hypothesis h) const
  {
    
    using namespace std;
    const string btype = mb.getBehaviourTypeFlag();
    const MechanicalBehaviourData& d = mb.getMechanicalBehaviourData(h);
    VariableDescriptionContainer::const_iterator p;
    SupportedTypes::TypeSize n2;
    for(p=d.getIntegrationVariables().begin();p!=d.getIntegrationVariables().end();++p){
      n2 += mb.getTypeSize(p->type,p->arraySize);
    }
    if(mb.hasAttribute(h,MechanicalBehaviourData::compareToNumericalJacobian)){
      out << "tmatrix<" << n2 << "," << n2 << ",real> njacobian;\n";
    }
    out << "real error;\n";
    out << "bool converged=false;\n";
    out << "this->iter=0;\n";
    if(getDebugMode()){
      out << "cout << endl << \"" << mb.getClassName() << "::integrate() : beginning of resolution\" << endl;\n";
    }
    out << "while((converged==false)&&\n";
    out << "(this->iter<" << mb.getClassName() << "::iterMax)){\n";
    out << "++(this->iter);\n";
    if(mb.hasCode(h,MechanicalBehaviourData::ComputeStress)){
      out << "this->computeStress();\n";
    }
    out << "const bool computeFdF_ok = this->computeFdF();\n";
    out << "if(!computeFdF_ok){\n";
    out << "if(this->iter==1){\n";
    if(getDebugMode()){
      out << "cout << endl << \"" << mb.getClassName()
			  << "::integrate() : computFdF returned false on first iteration, abording...\" << endl;\n";
    }
    if(mb.useQt()){        
      out << "return MechanicalBehaviour<" << btype << ",hypothesis,Type,use_qt>::FAILURE;\n";
    } else {
      out << "return MechanicalBehaviour<" << btype << ",hypothesis,Type,false>::FAILURE;\n";
    }
    out << "} else {\n";
    if(getDebugMode()){
      out << "cout << endl << \"" << mb.getClassName()
			  << "::integrate() : computFdF returned false, dividing increment by two...\" << endl;\n";
    }
    out << "const real integrate_one_half = real(1)/real(2);\n";
    out << "this->zeros -= (this->zeros-this->zeros_1)*integrate_one_half;\n";
    out << "}\n";
    out << "} else {\n";
    out << "this->zeros_1  = this->zeros;\n";
    if(!this->requiresNumericalJacobian()){
      this->writeComparisonToNumericalJacobian(out,mb,h,"njacobian");
    }
    out << "error=norm(this->fzeros);\n";
    out << "converged = ((error)/(real(" << n2 << "))<";
    out << "(this->epsilon));\n";
    if(this->requiresNumericalJacobian()){
      // We compute the numerical jacobian even if we converged since
      // most of the time, this tangent operator will be computed
      // using the partial jacobian invert. We consider very unlikely
      // that a user may use a numerical jacobian and provide an
      // analytic definition of the tangent operator
      out << "if((!converged)||(smt!=NOSTIFFNESSREQUESTED)){" << endl
			  << "this->computeNumericalJacobian(this->jacobian);" << endl
			  << "}" << endl;
    }
    if(getDebugMode()){
      out << "cout << \"" << mb.getClassName()
			  << "::integrate() : iteration \" "
			  << "<< this->iter << \" : \" << (error)/(real(" << n2 << ")) << endl;\n";
    }
    out << "if(!converged){\n";
    if(this->usesPowellDogLegAlgorithm()){
      out << "tmatrix<" << n2 << "," << n2 << ",real> tjacobian(this->jacobian);\n";
      out << "tvector<" << n2 << ",real> tfzeros(this->fzeros);\n";
    }
    out << "try{" << endl;
    if(mb.getAttribute(MechanicalBehaviourData::profiling,false)){
      writeStandardPerformanceProfilingBegin(out,"TinyMatrixSolve","lu");
    }
    out << "TinyMatrixSolve<" << n2
	<< "," << "real>::exe(this->jacobian,this->fzeros);\n";
    if(mb.getAttribute(MechanicalBehaviourData::profiling,false)){
      writeStandardPerformanceProfilingEnd(out);
    }
    out << "}" << endl;
    out << "catch(LUException&){" << endl;
    if(mb.useQt()){        
      out << "return MechanicalBehaviour<" << btype << ",hypothesis,Type,use_qt>::FAILURE;\n";
    } else {
      out << "return MechanicalBehaviour<" << btype << ",hypothesis,Type,false>::FAILURE;\n";
    }
    out << "}" << endl;
    if(this->usesPowellDogLegAlgorithm()){
      this->writePowellDogLegStep(out,mb,h,"tjacobian","tfzeros","fzeros");
    } else {
      MFrontNonLinearSystemSolverBase::writeLimitsOnIncrementValues(out,mb,h,"fzeros");
      out << "this->zeros -= this->fzeros;\n";
    }
    MFrontNonLinearSystemSolverBase::writeLimitsOnIncrementValuesBasedOnStateVariablesPhysicalBounds(out,mb,h);
    MFrontNonLinearSystemSolverBase::writeLimitsOnIncrementValuesBasedOnIntegrationVariablesIncrementsPhysicalBounds(out,mb,h);
    out << "}\n";
    out << "}\n";
    out << "}\n";
  } // end of MFrontNewtonRaphsonSolverBase::writeResolutionAlgorithm

  MFrontNewtonRaphsonSolverBase::~MFrontNewtonRaphsonSolverBase()
  {} // end of  MFrontNewtonRaphsonSolverBase::~MFrontNewtonRaphsonSolverBase()

  bool
  MFrontNewtonRaphsonSolver::requiresNumericalJacobian(void) const
  {
    return false;
  }

  bool
  MFrontNewtonRaphsonSolver::usesPowellDogLegAlgorithm(void) const
  {
    return false;
  }

  MFrontNewtonRaphsonSolver::~MFrontNewtonRaphsonSolver()
  {} // end of MFrontNewtonRaphsonSolver::~MFrontNewtonRaphsonSolver

  bool
  MFrontNewtonRaphsonNumericalJacobianSolver::requiresNumericalJacobian(void) const
  {
    return true;
  }

  bool
  MFrontNewtonRaphsonNumericalJacobianSolver::usesPowellDogLegAlgorithm(void) const
  {
    return false;
  }

  MFrontNewtonRaphsonNumericalJacobianSolver::~MFrontNewtonRaphsonNumericalJacobianSolver()
  {} // end of MFrontNewtonRaphsonSolver::~MFrontNewtonRaphsonSolver

  bool
  MFrontPowellDogLegNewtonRaphsonSolver::requiresNumericalJacobian(void) const
  {
    return false;
  }

  bool
  MFrontPowellDogLegNewtonRaphsonSolver::usesPowellDogLegAlgorithm(void) const
  {
    return true;
  }

  MFrontPowellDogLegNewtonRaphsonSolver::~MFrontPowellDogLegNewtonRaphsonSolver()
  {} // end of MFrontPowellDogLegNewtonRaphsonSolver::~MFrontPowellDogLegNewtonRaphsonSolver

  bool
  MFrontPowellDogLegNewtonRaphsonNumericalJacobianSolver::requiresNumericalJacobian(void) const
  {
    return true;
  }

  bool
  MFrontPowellDogLegNewtonRaphsonNumericalJacobianSolver::usesPowellDogLegAlgorithm(void) const
  {
    return true;
  }

  MFrontPowellDogLegNewtonRaphsonNumericalJacobianSolver::~MFrontPowellDogLegNewtonRaphsonNumericalJacobianSolver()
  {} // end of MFrontPowellDogLegNewtonRaphsonSolver::~MFrontPowellDogLegNewtonRaphsonSolver
  
} // end of namespace mfront