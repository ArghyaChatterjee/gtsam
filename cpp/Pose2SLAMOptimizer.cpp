/*
 * Pose2SLAMOptimizer.cpp
 *
 *  Created on: Jan 22, 2010
 *      Author: dellaert
 */

#include "Pose2SLAMOptimizer.h"
#include "pose2SLAM.h"
#include "dataset.h"
#include "SubgraphPreconditioner-inl.h"

using namespace std;
namespace gtsam {

	using namespace pose2SLAM;

	/* ************************************************************************* */
	Pose2SLAMOptimizer::Pose2SLAMOptimizer(const string& dataset_name,
			const string& path) {

		static int maxID = 0;
		static bool addNoise = false;

		string filename;
		boost::optional<SharedDiagonal> noiseModel;
		boost::tie(filename, noiseModel) = dataset(dataset_name);

		// read graph and initial estimate
		boost::tie(graph_, theta_) = load2D(filename, maxID, noiseModel, addNoise);
		graph_->addPrior(theta_->begin()->first, theta_->begin()->second,
				noiseModel::Unit::Create(3));

		// initialize non-linear solver
		solver_.initialize(*graph_, *theta_);
	}

	/* ************************************************************************* */
	void Pose2SLAMOptimizer::update(const Vector& x) {
		VectorConfig X; // TODO
		update(X);
	}

	/* ************************************************************************* */
	void Pose2SLAMOptimizer::updatePreconditioned(const Vector& y) {
		Vector x;
		update(x);
	}

/* ************************************************************************* */
}
