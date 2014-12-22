/**
 * @file	NonlinearInequalityFactorGraph.h
 * @author 	Krunal Chande
 * @date	Dec 22, 2014
 */

#pragma once
#include <gtsam/nonlinear/NonlinearFactorGraph.h>

namespace gtsam {
class NonlinearInequalityFactorGraph : public FactorGraph<NonlinearFactor> {

public:
  /// default constructor
  NonlinearInequalityFactorGraph() {
  }

  /// linearize to a LinearEqualityFactorGraph
  LinearInequalityFactorGraph::shared_ptr linearize(
      const Values& linearizationPoint) const {
    LinearInequalityFactorGraph::shared_ptr linearGraph(
        new LinearInequalityFactorGraph());
    BOOST_FOREACH(const NonlinearFactor::shared_ptr& factor, *this){
      JacobianFactor::shared_ptr jacobian = boost::dynamic_pointer_cast<JacobianFactor>(
          factor->linearize(linearizationPoint));
      NonlinearConstraint::shared_ptr constraint = boost::dynamic_pointer_cast<NonlinearConstraint>(factor);
      linearGraph->add(LinearInequality(*jacobian, constraint->dualKey()));
    }
    return linearGraph;
  }

  /**
   * Return true if the all errors are <= 0.0
   */
  bool checkFeasibilityAndComplimentary(const Values& values, const VectorValues& duals, double tol) const {
    BOOST_FOREACH(const NonlinearFactor::shared_ptr& factor, *this){
      NoiseModelFactor::shared_ptr noiseModelFactor = boost::dynamic_pointer_cast<NoiseModelFactor>(
          factor);
      Vector error = noiseModelFactor->unwhitenedError(values);

      // Primal feasibility condition: all constraints need to be <= 0.0
      if (error[0] > tol) {
        return false;
      }

      // Complimentary condition: errors of active constraints need to be 0.0
      NonlinearConstraint::shared_ptr constraint = boost::dynamic_pointer_cast<NonlinearConstraint>(
          factor);
      Key dualKey = constraint->dualKey();
      if (!duals.exists(dualKey)) continue;  // if dualKey doesn't exist, it is an inactive constraint!
      if (fabs(error[0]) > tol) // for active constraint, the error should be 0.0
        return false;

    }
    return true;
  }
};
}
