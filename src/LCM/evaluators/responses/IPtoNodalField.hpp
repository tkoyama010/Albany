//*****************************************************************//
//    Albany 3.0:  Copyright 2016 Sandia Corporation               //
//    This Software is released under the BSD license detailed     //
//    in the file "license.txt" in the top-level Albany directory  //
//*****************************************************************//

#if !defined(LCM_IPtoNodalField_hpp)
#define LCM_IPtoNodalField_hpp

#include <Phalanx_Evaluator_WithBaseImpl.hpp>
#include <Phalanx_Evaluator_Derived.hpp>
#include <Phalanx_MDField.hpp>
#include <Phalanx_DataLayout.hpp>
#include <Teuchos_ParameterList.hpp>
#include "Albany_ProblemUtils.hpp"
#include "Albany_StateManager.hpp"

namespace LCM
{
class IPtoNodalFieldManager;

/// 
/// \brief Evaltuator to compute a nodal stress field
///
template<typename EvalT, typename Traits>
class IPtoNodalFieldBase:
    public PHX::EvaluatorWithBaseImpl<Traits>,
    public PHX::EvaluatorDerived<EvalT, Traits>
{
public:
  typedef typename EvalT::ScalarT ScalarT;
  typedef typename EvalT::MeshScalarT MeshScalarT;

  ///
  /// Constructor
  ///
  IPtoNodalFieldBase(Teuchos::ParameterList& p,
      const Teuchos::RCP<Albany::Layouts>& dl,
      const Albany::MeshSpecsStruct* mesh_specs);

  ///
  /// Phalanx method to allocate space
  ///
  void postRegistrationSetup(typename Traits::SetupData d,
      PHX::FieldManager<Traits>& vm);

  ///
  /// These functions are defined in the specializations
  ///
  void preEvaluate(typename Traits::PreEvalData d) = 0;
  void postEvaluate(typename Traits::PostEvalData d) = 0;
  void evaluateFields(typename Traits::EvalData d) = 0;

  Teuchos::RCP<const PHX::FieldTag> getEvaluatedFieldTag() const
  {
    return field_tag_;
  }

  Teuchos::RCP<const PHX::FieldTag> getResponseFieldTag() const
  {
    return field_tag_;
  }

protected:

  Teuchos::RCP<const Teuchos::ParameterList>
  getValidIPtoNodalFieldParameters() const;

  int number_of_fields_;

  std::vector<std::string> ip_field_names_;
  std::vector<std::string> ip_field_layouts_;
  std::vector<std::string> nodal_field_names_;

  std::string nodal_weights_name_;
    
  int num_pts_;
  int num_dims_;
  int num_nodes_;
  int num_vertices_;
    
  PHX::MDField<MeshScalarT,Cell,QuadPoint> weights_;
  std::vector<PHX::MDField<ScalarT>> ip_fields_;

  bool output_to_exodus_;
  bool output_node_data_;

  Teuchos::RCP<PHX::Tag<ScalarT>> field_tag_;
  Albany::StateManager* p_state_mgr_;

  Teuchos::RCP<IPtoNodalFieldManager> mgr_;
};

template<typename EvalT, typename Traits>
class IPtoNodalField
: public IPtoNodalFieldBase<EvalT, Traits> {
public:
  IPtoNodalField(Teuchos::ParameterList& p,
      const Teuchos::RCP<Albany::Layouts>& dl,
      const Albany::MeshSpecsStruct* mesh_specs) :
      IPtoNodalFieldBase<EvalT, Traits>(p, dl, mesh_specs) {}

  void preEvaluate(typename Traits::PreEvalData d) {}

  void postEvaluate(typename Traits::PostEvalData d) {}

  void evaluateFields(typename Traits::EvalData d) {}
};

// **************************************************************
// **************************************************************
// * Specializations
// **************************************************************
// **************************************************************

// **************************************************************
// Residual 
// **************************************************************

template<typename Traits>
class IPtoNodalField<PHAL::AlbanyTraits::Residual, Traits>
: public IPtoNodalFieldBase<PHAL::AlbanyTraits::Residual, Traits> {
public:
  IPtoNodalField(Teuchos::ParameterList& p,
      const Teuchos::RCP<Albany::Layouts>& dl,
      const Albany::MeshSpecsStruct* mesh_specs);
  void preEvaluate(typename Traits::PreEvalData d);
  void postEvaluate(typename Traits::PostEvalData d);
  void evaluateFields(typename Traits::EvalData d);
};
}

#endif  // IPtoNodalField.hpp