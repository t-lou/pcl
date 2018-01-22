/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2010-2011, Willow Garage, Inc.
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
 * $Id: distances.cpp 527 2011-04-17 23:57:26Z rusu $
 *
 */
#include <pcl/common/common.h>
#include <pcl/console/print.h>

//////////////////////////////////////////////////////////////////////////////////////////////
void 
pcl::getMinMax (const pcl::PCLPointCloud2 &cloud, int,
                const std::string &field_name, float &min_p, float &max_p)
{
  min_p = FLT_MAX;
  max_p = -FLT_MAX;

  int field_idx = -1;
  for (size_t d = 0; d < cloud.fields.size (); ++d)
    if (cloud.fields[d].name == field_name)
      field_idx = static_cast<int>(d);

  if (field_idx == -1)
  {
    PCL_ERROR ("[getMinMax] Invalid field (%s) given!\n", field_name.c_str ());
    return;
  }

  for (unsigned int i = 0; i < cloud.fields[field_idx].count; ++i)
  {
    float data;
    // TODO: replace float with the real data type
    memcpy (&data, &cloud.data[cloud.fields[field_idx].offset + i * sizeof (float)], sizeof (float));
    min_p = (data > min_p) ? min_p : data; 
    max_p = (data < max_p) ? max_p : data; 
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void
pcl::getMeanStdDev (const std::vector<float> &values, double &mean, double &stddev)
{
  double sum = 0, sq_sum = 0;

  for (size_t i = 0; i < values.size (); ++i)
  {
    sum += values[i];
    sq_sum += values[i] * values[i];
  }
  mean = sum / static_cast<double>(values.size ());
  double variance = (sq_sum - sum * sum / static_cast<double>(values.size ())) / (static_cast<double>(values.size ()) - 1);
  stddev = sqrt (variance);
}

//////////////////////////////////////////////////////////////////////////////////////////////
Eigen::Vector4f
pcl::getPlaneBetweenPoints (const Eigen::Vector3f &point0,
                            const Eigen::Vector3f &point1)
{
  const Eigen::Vector3f normal = (point1 - point0).normalized ();
  Eigen::Vector4f plane;
  // (point1+point0)/2 is on plane, compute the offset of the plane
  plane << normal, -(point1 + point0).dot (normal) * 0.5f;
  return plane;
}

//////////////////////////////////////////////////////////////////////////////////////////////
Eigen::Vector3f
pcl::getCircumscribedCircleCentre (const Eigen::Vector3f &point0,
                                   const Eigen::Vector3f &point1,
                                   const Eigen::Vector3f &point2)
{
  // https://en.wikipedia.org/wiki/Circumscribed_circle#Cartesian_coordinates_from_cross-_and_dot-products
  const Eigen::Vector3f vec2 = point0 - point1;
  const Eigen::Vector3f vec0 = point1 - point2;
  const Eigen::Vector3f vec1 = point2 - point0;
  const float area = vec0.cross (vec1).norm ();
  const float determinator = 2.0f * area * area;

  const float alpha = vec0.dot (vec0) * vec2.dot (-vec1) / determinator;
  const float beta = vec1.dot (vec1) * -vec2.dot (vec0) / determinator;
  const float gamma = vec2.dot (vec2) * vec1.dot (-vec0) / determinator;

  return alpha * point0 + beta * point1 + gamma * point2;
}
