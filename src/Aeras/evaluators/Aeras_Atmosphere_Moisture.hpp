//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#ifndef AERAS_ATMOSPHERE_MOISTURE_HPP
#define AERAS_ATMOSPHERE_MOISTURE_HPP

#include "Phalanx_ConfigDefs.hpp"
#include "Phalanx_Evaluator_WithBaseImpl.hpp"
#include "Phalanx_Evaluator_Derived.hpp"
#include "Phalanx_MDField.hpp"

#include "Aeras_Layouts.hpp"

#include "Teuchos_ParameterList.hpp"
#include "Epetra_Vector.h"

namespace Aeras {

template<typename EvalT, typename Traits> 
class Atmosphere_Moisture : public PHX::EvaluatorWithBaseImpl<Traits>,
                            public PHX::EvaluatorDerived<EvalT, Traits>  {
  
public:
  typedef typename EvalT::ScalarT ScalarT;
  typedef typename EvalT::MeshScalarT MeshScalarT;
  
  Atmosphere_Moisture(Teuchos::ParameterList& p,
                      const Teuchos::RCP<Aeras::Layouts>& dl);
  
  void postRegistrationSetup(typename Traits::SetupData d,
                      PHX::FieldManager<Traits>& vm);
  
  void evaluateFields(typename Traits::EvalData d);
  
private:
  PHX::MDField<MeshScalarT,Cell,Node,QuadPoint> wBF;
  PHX::MDField<MeshScalarT,Cell,Vertex,Dim> coordVec;

  PHX::MDField<ScalarT,Cell,Node,VecDim> Velx;
  PHX::MDField<ScalarT,Cell,Node,VecDim> Temp;
  PHX::MDField<ScalarT,Cell,Node,VecDim> VelxResid;
  PHX::MDField<ScalarT,Cell,Node,VecDim> TempResid;

  std::map<std::string, PHX::MDField<ScalarT,Cell,Node> > TracerIn;
  std::map<std::string, PHX::MDField<ScalarT,Cell,Node> > TracerResid;


  const Teuchos::ArrayRCP<std::string> tracerNames;
  const Teuchos::ArrayRCP<std::string> tracerResidNames;
  std::map<std::string, std::string>   namesToResid;
 
  const int numNodes;
  const int numQPs;
  const int numDims;
  const int numLevels;
};
}

#endif
