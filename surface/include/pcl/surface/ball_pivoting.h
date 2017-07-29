/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2014-, Open Perception, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */
  
#ifndef PCL_BALL_PIVOTING_H_
#define PCL_BALL_PIVOTING_H_

#include <memory>
#include <vector>
#include <map>
#include <set>
  
#include "pcl/surface/boost.h"
#include "pcl/surface/reconstruction.h"
#include "pcl/surface/ball_pivoting_front.h"

namespace pcl
{  
  /**
   * This class is an implementation of ball pivoting algorithm(BPA) with some 
   * different details. BPA exhibits linear time complexity (besides searching 
   * complexity) and robustness to noise in real scanned data. The output mesh 
   * is a manifold of an alpha-shape of the point set. BPA requires that the 
   * points are distributed over the entire surface with a spatial frequency 
   * greater than or equal to a certain minimum value (scale of pivoted ball).
   *
   * algorithm from
   * Bernardini F., Mittleman J., Rushmeier H., Silva C., Taubin G., 
   * "The ball-pivoting algorithm for surface reconstruction", TVCG'99
   *
   * author Tongxi Lou
   * email tongxi.lou@tum.de (tongxi.lou@gmx.de if first one is canceled after graduation)
   *
   * \ingroup surface
   */

  template<typename PointNT>
  class BallPivoting: public SurfaceReconstruction<PointNT>
  {
  public:
    typedef boost::shared_ptr<BallPivoting<PointNT> > Ptr;
    typedef boost::shared_ptr<const BallPivoting<PointNT> > ConstPtr;

    using SurfaceReconstruction<PointNT>::input_;
    using SurfaceReconstruction<PointNT>::tree_;
    
    typedef typename pcl::PointCloud<PointNT>::Ptr PointCloudPtr;

    typedef typename pcl::KdTree<PointNT> KdTree;
    typedef typename pcl::KdTree<PointNT>::Ptr KdTreePtr;
  
  protected:
    /** is_used_[id] indicates whether point input_[id] is used for meshing */
    std::vector<bool> is_used_;
    /** edge manager */
    bpa::Front front_;
    /** search radius. default value is -1, radius would be guessed if it is non-positive */
    double radius_;
    /** whether balls on the back surface would be considered */
    bool is_allow_back_ball_;
    /** whether the pivoting ball could flip from front face to back face or vice versa */
    bool is_allow_flip_;
    /** threshold for avoiding collinear points */
    double threshold_collinear_cos_;
    /** threshold for avoiding too near points */
    double threshold_distance_near_;
  
    /**
     * find one starting triangle for ball pivoting
     * @param seed the index of triangle vertices
     * @param center the center of ball
     * @param is_back_ball whether the found seed is on back surface
     * @return whether one triangle is found
     */
    bool
    findSeed (pcl::Vertices::Ptr &seed, Eigen::Vector3f &center, bool &is_back_ball);
  
    /**
     * pivoting around edge
     * @param edge edge for pivoting
     * @param id_extended index of the hitting vertice, which would bring more edges for pivoting
     * @param center_new
     * @param is_back_ball
     * @return whether pivoting is successful, if not, it is a bounday
     */
    bool
    pivot (const bpa::Front::Edge &edge, uint32_t &id_extended, 
    	   Eigen::Vector3f &center_new, bool &is_back_ball) const;
  
    /**
     * pivot until the front has no edge to pivot
     * @param polygons
     */
    void
    proceedFront (std::vector<pcl::Vertices> &polygons);
  
    /**
     * find the center of newly pivoted ball, if empty, then pivoting is not successful
     * @param is_back_first whether to consider assumed center on back surface first. it changes the result if
     *        flipping between front surface and back surface is enabled (setAllowFlip)
     * @param index index of triangle vertices, which are all one ball surface
     * @param is_back_ball whether the pivoted ball is on back surface
     * @return
     */
    boost::shared_ptr<Eigen::Vector3f>
    getBallCenter (const bool is_back_first, std::vector<uint32_t> &index, bool &is_back_ball) const;
  
  public:
    BallPivoting ();
  
    ~BallPivoting ();
  
    /**
     * set the radius of ball
     * @param radius
     */
    void
    setSearchRadius (const double radius)
    { radius_ = radius; }
  
    /**
     * get the radius of ball
     * @return
     */
    double
    getSearchRadius () const
    { return radius_; }
  
    /**
     * set whether ball on the back surface is allowed. if this value is set to true, the ball only pivots on the
     * direction of the normal vectors
     * @param is_allow_back_ball
     */
    void
    setAllowBackBall (const bool is_allow_back_ball)
    { is_allow_back_ball_ = is_allow_back_ball; }
  
    /**
     * get whether ball on the back surface is allowed
     * @return
     */
    bool
    getAllowBackBall () const
    { return is_allow_back_ball_; }
  
    /**
     * set whether flipping between front surface and back surface is allow. if not, the ball would only pivot
     * so it stays on one side of the surface, depending on where the center of seed ball was found
     * @param is_allow_flip
     */
    void
    setAllowFlip (const bool is_allow_flip)
    { is_allow_flip_ = is_allow_flip; }
  
    /**
     * get whether flipping between front surface and back surface is allow
     * @return
     */
    bool
    getAllowFlip () const
    { return is_allow_flip_; }
  
    /**
     * Create the surface.
     * @param output the resultant polygonal mesh
     */
    void
    performReconstruction (pcl::PolygonMesh &output);
  
    /**
     * Create the surface.
     * @param points the vertex positions of the resulting mesh
     * @param polygons the connectivity of the resulting mesh
     */
    virtual void
    performReconstruction (pcl::PointCloud<PointNT> &points,
                           std::vector<pcl::Vertices> &polygons);

    /**
     * estimate one radius and set it as ball radius. num_sample_point points would be selected randomly in cloud,
     * as least ratio_success of the sample points must have at least num_point_in_radius neighbors within
     * the estimated radius.
     * @param num_sample_point
     * @param num_point_in_radius
     * @param ratio_success
     */
    virtual void
    setEstimatedRadius (const int num_sample_point, const int num_point_in_radius, const float ratio_success);


  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  };
} // namespace pcl

#ifdef PCL_NO_PRECOMPILE
#include <pcl/surface/impl/ball_pivoting.hpp>
#endif
  
#endif // PCL_BALL_PIVOTING_H_
