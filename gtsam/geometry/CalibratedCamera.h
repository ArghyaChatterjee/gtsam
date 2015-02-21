/* ----------------------------------------------------------------------------

 * GTSAM Copyright 2010, Georgia Tech Research Corporation, 
 * Atlanta, Georgia 30332-0415
 * All Rights Reserved
 * Authors: Frank Dellaert, et al. (see THANKS for the full author list)

 * See LICENSE for the license information

 * -------------------------------------------------------------------------- */

/**
 * @file CalibratedCamera.h
 * @brief Calibrated camera for which only pose is unknown
 * @date Aug 17, 2009
 * @author Frank Dellaert
 */

#pragma once

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Point2.h>

namespace gtsam {

class GTSAM_EXPORT CheiralityException: public ThreadsafeException<
    CheiralityException> {
public:
  CheiralityException() :
      ThreadsafeException<CheiralityException>("Cheirality Exception") {
  }
};

/**
 * A pinhole camera class that has a Pose3, functions as base class for all pinhole cameras
 * @addtogroup geometry
 * \nosubgrouping
 */
class GTSAM_EXPORT PinholeBase {

private:

  Pose3 pose_; ///< 3D pose of camera

public:

  /// @name Static functions
  /// @{

  /**
   * Create a level pose at the given 2D pose and height
   * @param K the calibration
   * @param pose2 specifies the location and viewing direction
   * (theta 0 = looking in direction of positive X axis)
   * @param height camera height
   */
  static Pose3 LevelPose(const Pose2& pose2, double height);

  /**
   * Create a camera pose at the given eye position looking at a target point in the scene
   * with the specified up direction vector.
   * @param eye specifies the camera position
   * @param target the point to look at
   * @param upVector specifies the camera up direction vector,
   *        doesn't need to be on the image plane nor orthogonal to the viewing axis
   */
  static Pose3 LookatPose(const Point3& eye, const Point3& target,
      const Point3& upVector);

  /// @}
  /// @name Standard Constructors
  /// @{

  /** default constructor */
  PinholeBase() {
  }

  /** constructor with pose */
  explicit PinholeBase(const Pose3& pose) :
      pose_(pose) {
  }

  /// @}
  /// @name Advanced Constructors
  /// @{

  explicit PinholeBase(const Vector &v) :
      pose_(Pose3::Expmap(v)) {
  }

  /// @}
  /// @name Testable
  /// @{

  /// assert equality up to a tolerance
  bool equals(const PinholeBase &camera, double tol = 1e-9) const;

  /// print
  void print(const std::string& s = "PinholeBase") const;

  /// @}
  /// @name Standard Interface
  /// @{

  virtual ~PinholeBase() {
  }

  /// return pose, constant version
  const Pose3& pose() const {
    return pose_;
  }

  /// return pose, with derivative
  const Pose3& pose(OptionalJacobian<6, 6> H) const;

  /// @}
  /// @name Transformations and measurement functions
  /// @{

  /**
   * projects a 3-dimensional point in camera coordinates into the
   * camera and returns a 2-dimensional point
   * @param P A point in camera coordinates
   * @param Dpoint is the 2*3 Jacobian w.r.t. P
   */
  static Point2 project_to_camera(const Point3& P, //
      OptionalJacobian<2, 3> Dpoint = boost::none);

  /**
   * backproject a 2-dimensional point to a 3-dimension point
   */
  static Point3 backproject_from_camera(const Point2& p, const double scale);

  /**
   * Calculate range to a landmark
   * @param point 3D location of landmark
   * @param Dcamera the optionally computed Jacobian with respect to pose
   * @param Dpoint the optionally computed Jacobian with respect to the landmark
   * @return range (double)
   */
  double range(
      const Point3& point, //
      OptionalJacobian<1, 6> Dcamera = boost::none,
      OptionalJacobian<1, 3> Dpoint = boost::none) const {
    return pose_.range(point, Dcamera, Dpoint);
  }

  /**
   * Calculate range to another pose
   * @param pose Other SO(3) pose
   * @param Dcamera the optionally computed Jacobian with respect to pose
   * @param Dpose2 the optionally computed Jacobian with respect to the other pose
   * @return range (double)
   */
  double range(
      const Pose3& pose, //
      OptionalJacobian<1, 6> Dcamera = boost::none,
      OptionalJacobian<1, 6> Dpose = boost::none) const {
    return pose_.range(pose, Dcamera, Dpose);
  }

protected:

  /**
   * Calculate Jacobian with respect to pose
   * @param pn projection in normalized coordinates
   * @param d disparity (inverse depth)
   * @param Dpi_pn derivative of uncalibrate with respect to pn
   * @param Dpose Output argument, can be matrix or block, assumed right size !
   * See http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
   */
  template<typename Derived>
  static void calculateDpose(const Point2& pn, double d, const Matrix2& Dpi_pn,
      Eigen::MatrixBase<Derived> const & Dpose) {
    // optimized version of derivatives, see CalibratedCamera.nb
    const double u = pn.x(), v = pn.y();
    double uv = u * v, uu = u * u, vv = v * v;
    Matrix26 Dpn_pose;
    Dpn_pose << uv, -1 - uu, v, -d, 0, d * u, 1 + vv, -uv, -u, 0, -d, d * v;
    assert(Dpose.rows()==2 && Dpose.cols()==6);
    const_cast<Eigen::MatrixBase<Derived>&>(Dpose) = //
        Dpi_pn * Dpn_pose;
  }

  /**
   * Calculate Jacobian with respect to point
   * @param pn projection in normalized coordinates
   * @param d disparity (inverse depth)
   * @param Dpi_pn derivative of uncalibrate with respect to pn
   * @param Dpoint Output argument, can be matrix or block, assumed right size !
   * See http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
   */
  template<typename Derived>
  static void calculateDpoint(const Point2& pn, double d, const Matrix3& R,
      const Matrix2& Dpi_pn, Eigen::MatrixBase<Derived> const & Dpoint) {
    // optimized version of derivatives, see CalibratedCamera.nb
    const double u = pn.x(), v = pn.y();
    Matrix23 Dpn_point;
    Dpn_point << //
        R(0, 0) - u * R(0, 2), R(1, 0) - u * R(1, 2), R(2, 0) - u * R(2, 2), //
    /**/R(0, 1) - v * R(0, 2), R(1, 1) - v * R(1, 2), R(2, 1) - v * R(2, 2);
    Dpn_point *= d;
    assert(Dpoint.rows()==2 && Dpoint.cols()==3);
    const_cast<Eigen::MatrixBase<Derived>&>(Dpoint) = //
        Dpi_pn * Dpn_point;
  }

private:

  /** Serialization function */
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar & BOOST_SERIALIZATION_NVP(pose_);
  }

};
// end of class PinholeBase

/**
 * A Calibrated camera class [R|-R't], calibration K=I.
 * If calibration is known, it is more computationally efficient
 * to calibrate the measurements rather than try to predict in pixels.
 * @addtogroup geometry
 * \nosubgrouping
 */
class GTSAM_EXPORT CalibratedCamera: public PinholeBase {

public:

  enum {
    dimension = 6
  };

  /// @name Standard Constructors
  /// @{

  /// default constructor
  CalibratedCamera() {
  }

  /// construct with pose
  explicit CalibratedCamera(const Pose3& pose) :
      PinholeBase(pose) {
  }

  /// @}
  /// @name Named Constructors
  /// @{

  /**
   * Create a level camera at the given 2D pose and height
   * @param pose2 specifies the location and viewing direction
   * @param height specifies the height of the camera (along the positive Z-axis)
   * (theta 0 = looking in direction of positive X axis)
   */
  static CalibratedCamera Level(const Pose2& pose2, double height);

  /**
   * Create a camera at the given eye position looking at a target point in the scene
   * with the specified up direction vector.
   * @param eye specifies the camera position
   * @param target the point to look at
   * @param upVector specifies the camera up direction vector,
   *        doesn't need to be on the image plane nor orthogonal to the viewing axis
   */
  static CalibratedCamera Lookat(const Point3& eye, const Point3& target,
      const Point3& upVector);

  /// @}
  /// @name Advanced Constructors
  /// @{

  /// construct from vector
  explicit CalibratedCamera(const Vector &v) :
      PinholeBase(v) {
  }

  /// @}
  /// @name Standard Interface
  /// @{

  /// destructor
  virtual ~CalibratedCamera() {
  }

  /// @}
  /// @name Manifold
  /// @{

  /// move a cameras pose according to d
  CalibratedCamera retract(const Vector& d) const;

  /// Return canonical coordinate
  Vector localCoordinates(const CalibratedCamera& T2) const;

  /// Lie group dimensionality
  inline size_t dim() const {
    return 6;
  }

  /// Lie group dimensionality
  inline static size_t Dim() {
    return 6;
  }

  /// @}
  /// @name Transformations and mesaurement functions
  /// @{
  /**
   * This function receives the camera pose and the landmark location and
   * returns the location the point is supposed to appear in the image
   * @param point a 3D point to be projected
   * @param Dpose the optionally computed Jacobian with respect to pose
   * @param Dpoint the optionally computed Jacobian with respect to the 3D point
   * @return the intrinsic coordinates of the projected point
   */
  Point2 project(const Point3& point,
      OptionalJacobian<2, 6> Dpose = boost::none,
      OptionalJacobian<2, 3> Dpoint = boost::none) const;

  /**
   * Calculate range to another camera
   * @param camera Other camera
   * @param H1 optionally computed Jacobian with respect to pose
   * @param H2 optionally computed Jacobian with respect to the 3D point
   * @return range (double)
   */
  double range(const CalibratedCamera& camera, OptionalJacobian<1, 6> H1 =
      boost::none, OptionalJacobian<1, 6> H2 = boost::none) const {
    return pose().range(camera.pose(), H1, H2);
  }

  /// @}

private:

  /// @name Advanced Interface
  /// @{

  /** Serialization function */
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version) {
    ar
        & boost::serialization::make_nvp("PinholeBase",
            boost::serialization::base_object<PinholeBase>(*this));
  }

  /// @}
};

template<>
struct traits<CalibratedCamera> : public internal::Manifold<CalibratedCamera> {
};

template<>
struct traits<const CalibratedCamera> : public internal::Manifold<
    CalibratedCamera> {
};

}

