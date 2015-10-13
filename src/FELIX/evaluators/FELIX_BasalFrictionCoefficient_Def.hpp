//*****************************************************************//
//    Albany 2.0:  Copyright 2012 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#include "Teuchos_TestForException.hpp"
#include "Teuchos_VerboseObject.hpp"
#include "Phalanx_DataLayout.hpp"
#include "Intrepid_FunctionSpaceTools.hpp"
#include "Albany_Layouts.hpp"

#include "FELIX_HomotopyParameter.hpp"

//uncomment the following line if you want debug output to be printed to screen
#define OUTPUT_TO_SCREEN

namespace FELIX
{

template<typename EvalT, typename Traits>
BasalFrictionCoefficient<EvalT, Traits>::BasalFrictionCoefficient (const Teuchos::ParameterList& p,
                                                                   const Teuchos::RCP<Albany::Layouts>& dl) :
  beta        (p.get<std::string> ("Basal Friction Coefficient Variable Name"), dl->side_qp_scalar)
{
#ifdef OUTPUT_TO_SCREEN
  Teuchos::RCP<Teuchos::FancyOStream> output(Teuchos::VerboseObjectBase::getDefaultOStream());
#endif

  Teuchos::ParameterList& beta_list = *p.get<Teuchos::ParameterList*>("Parameter List");
  Teuchos::ParameterList& physics = *p.get<Teuchos::ParameterList*>("FELIX Physical Parameters");

  std::string betaType = (beta_list.isParameter("Type") ? beta_list.get<std::string>("Type") : "From File");

  basalSideName = p.get<std::string>("Side Set Name");
  if (betaType == "Given Constant")
  {
#ifdef OUTPUT_TO_SCREEN
    *output << "Given constant and uniform beta, value loaded from xml input file.\n";
#endif
    beta_type = GIVEN_CONSTANT;
    beta_given_val = beta_list.get<double>("Constant Given Beta Value");
  }
  else if (betaType == "Given Field")
  {
#ifdef OUTPUT_TO_SCREEN
    *output << "Given constant beta field, loaded from mesh or file.\n";
#endif
    beta_type = GIVEN_FIELD;

    beta_given_field = PHX::MDField<ScalarT,Cell,Node>(p.get<std::string> ("Basal Friction Coefficient Variable Name"), dl->node_scalar);

    this->addDependentField (beta_given_field);
  }
  else if (betaType == "Power Law")
  {
    beta_type = POWER_LAW;

    mu    = beta_list.get<double>("Coulomb Friction Coefficient");
    power = beta_list.get("Power Exponent",1.0);
    TEUCHOS_TEST_FOR_EXCEPTION(power<-1.0, Teuchos::Exceptions::InvalidParameter,
        std::endl << "Error in FELIX::BasalFrictionCoefficient: \"Power Exponent\" must be greater than (or equal to) -1.\n");

#ifdef OUTPUT_TO_SCREEN
    *output << "Velocity-dependent beta (power law):\n\n"
            << "      beta = mu * N * |u|^p \n\n"
            << "  with N being the effective pressure, |u| the sliding velocity, and\n"
            << "    - mu (Coulomb Friction Coefficient): " << mu << "\n"
            << "    - p  (Power Exponent): " << power << "\n";
#endif

    N      = PHX::MDField<ScalarT,Cell,Side,QuadPoint>(p.get<std::string> ("Effective Pressure Side QP Variable Name"), dl->side_qp_scalar);
    u_norm = PHX::MDField<ScalarT,Cell,Side,QuadPoint>(p.get<std::string> ("Sliding Velocity Side QP Variable Name"), dl->side_qp_scalar);
    this->addDependentField (u_norm);
    this->addDependentField (N);
  }
  else if (betaType == "Regularized Coulomb")
  {
    beta_type = REGULARIZED_COULOMB;

    mu     = beta_list.get<double>("Coulomb Friction Coefficient");
    power  = beta_list.get("Power Exponent",1.0);
    lambda = beta_list.get("Bed Roughness",1e-4);
    if (beta_list.isParameter("Constant Flow Factor A"))
    {
      A = beta_list.get<double>("Constant Flow Factor A");
    }
    else
    {
      TEUCHOS_TEST_FOR_EXCEPTION (true, std::logic_error, "Error! The case with variable flow factor has not been implemented yet.\n");
      //this->addDependentField (flowFactorA);
    }
#ifdef OUTPUT_TO_SCREEN
    *output << "Velocity-dependent beta (regularized coulomb law):\n\n"
            << "      beta = mu * N * |u|^{p-1} / [|u| + lambda*A*N^(1/p)]^p\n\n"
            << "  with N being the effective pressure, |u| the sliding velocity, and\n"
            << "    - mu (Coulomb Friction Coefficient): " << mu << "\n"
            << "    - lambda (Bed Roughness or Regularization Parameter): " << lambda << "\n"
            << "    - A (Flow Factor A): " << A << "\n"
            << "    - p  (Power Exponent): " << power << "\n";
#endif

    N           = PHX::MDField<ScalarT,Cell,Side,QuadPoint>(p.get<std::string> ("Effective Pressure Side QP Variable Name"), dl->side_qp_scalar);
    u_norm      = PHX::MDField<ScalarT,Cell,Side,QuadPoint>(p.get<std::string> ("Sliding Velocity Side QP Variable Name"), dl->side_qp_scalar);
//    flowFactorA = PHX::MDField<ScalarT,Cell>(p.get<std::string> ("Flow Factor Variable Name"), dl->cell_scalar2);

    this->addDependentField (u_norm);
    this->addDependentField (N);
  }
  else
  {
    TEUCHOS_TEST_FOR_EXCEPTION(true, Teuchos::Exceptions::InvalidParameter,
        std::endl << "Error in FELIX::BasalFrictionCoefficient:  \"" << betaType << "\" is not a valid parameter for Beta Type\n");
  }

  if (beta_type==GIVEN_FIELD)
  {
    BF = PHX::MDField<RealType,Cell,Side,Node,QuadPoint>(p.get<std::string> ("BF Side Variable Name"), dl->side_node_qp_scalar);
    this->addDependentField (BF);

    numSideNodes = dl->side_node_scalar->dimension(2);

    // Index of the nodes on the sides in the numeration of the cell
    Teuchos::RCP<shards::CellTopology> cellType;
    cellType = p.get<Teuchos::RCP <shards::CellTopology> > ("Cell Type");
    int numSides = dl->side_qp_scalar->dimension(1);
    int sideDim  = dl->side_qp_gradient->dimension(3);
    sideNodes.resize(numSides);
    for (int side=0; side<numSides; ++side)
    {
      sideNodes[side].resize(numSideNodes);
      for (int node=0; node<numSideNodes; ++node)
        sideNodes[side][node] = cellType->getNodeMap(sideDim,side,node);
    }
  }

  numSideQPs = dl->side_qp_scalar->dimension(2);

  this->addEvaluatedField(beta);

  this->setName("BasalFrictionCoefficient"+PHX::typeAsString<EvalT>());
}

//**********************************************************************
template<typename EvalT, typename Traits>
void BasalFrictionCoefficient<EvalT, Traits>::
postRegistrationSetup (typename Traits::SetupData d,
                       PHX::FieldManager<Traits>& fm)
{
  this->utils.setFieldData(beta,fm);

  switch (beta_type)
  {
    case GIVEN_CONSTANT:
      for (int cell=0; cell<beta.fieldTag().dataLayout().dimension(0); ++cell)
        for (int side=0; side<beta.fieldTag().dataLayout().dimension(1); ++side)
          for (int qp=0; qp<numSideQPs; ++qp)
            beta(cell,side,qp) = beta_given_val;
      break;
    case GIVEN_FIELD:
      this->utils.setFieldData(BF,fm);
      this->utils.setFieldData(beta_given_field,fm);
      break;
    case POWER_LAW:
      this->utils.setFieldData(N,fm);
      this->utils.setFieldData(u_norm,fm);
      break;
    case REGULARIZED_COULOMB:
      this->utils.setFieldData(N,fm);
      this->utils.setFieldData(u_norm,fm);
//      this->utils.setFieldData(flowFactorA,fm);
  }
/*
  if (A>0)
  {
    const int numCells = flowFactorA.fieldTag().dataLayout().dimension(0);
    for (int cell=0; cell<numCells; ++cell)
      flowFactorA(cell) = A;
  }
*/
}

//**********************************************************************
template<typename EvalT, typename Traits>
void BasalFrictionCoefficient<EvalT, Traits>::evaluateFields (typename Traits::EvalData workset)
{
  if (workset.sideSets->find(basalSideName)==workset.sideSets->end())
    return;

  ScalarT homotopyParam = FELIX::HomotopyParameter<EvalT>::value;
  ScalarT ff = 0;
  if (homotopyParam!=0)
    ff = pow(10.0, -10.0*homotopyParam);

  const std::vector<Albany::SideStruct>& sideSet = workset.sideSets->at(basalSideName);
  for (auto it_side : sideSet)
  {
    // Get the local data of side and cell
    const int cell = it_side.elem_LID;
    const int side = it_side.side_local_id;

    switch (beta_type)
    {
      case GIVEN_CONSTANT:
        return;   // We can save ourself some useless iterations

      case GIVEN_FIELD:
        for (int qp=0; qp<numSideQPs; ++qp)
        {
          beta(cell,side,qp) = 0.;
          for (int node=0; node<numSideNodes; ++node)
          {
            beta(cell,side,qp) += BF(cell,side,node,qp)*beta_given_field(cell,sideNodes[side][node]);
          }
        }
        break;

      case POWER_LAW:
        for (int qp=0; qp<numSideQPs; ++qp)
        {
          beta(cell,side,qp) = mu * N(cell,side,qp) * std::pow (u_norm(cell,side,qp), power);
        }
        break;

      case REGULARIZED_COULOMB:
      {
        for (int qp=0; qp<numSideQPs; ++qp)
        {
          beta(cell,side,qp) = mu * N(cell,side,qp) * std::pow (u_norm(cell,side,qp),power-1)
//                             / std::pow( u_norm(cell,side,qp) + lambda*flowFactorA(cell)*std::pow(N(cell,side,qp),1./power), power);
                             / std::pow( std::pow(u_norm(cell,side,qp),1-ff) + lambda*A*std::pow(N(cell,side,qp),1./power), power);
        }
        break;
      }
    }
  }
}

} // Namespace FELIX
