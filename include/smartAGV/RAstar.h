/********************************************************************
 *   MIT License
 *
 *   Copyright (c) 2018 Shivang Patel
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *deal in the Software without restriction, including without limitation the
 *rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *IN THE SOFTWARE.
 ********************************************************************/

/** @file RAstar.h
 *  @brief Definition of Astar plugin RAstarPLannerRos.
 *
 *  This file contains declaration of class RAstarPLannerRos. Inherits needed
 *  methods from BaseGlobalPlanner and implements Astar algorithm.
 *
 *  @author Shivang Patel
 */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/** include ros libraries**********************/
#include <ros/ros.h>

#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>

#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/Twist.h>
#include <move_base_msgs/MoveBaseActionGoal.h>
#include <move_base_msgs/MoveBaseGoal.h>

#include <nav_msgs/GetPlan.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <netdb.h>
#include <tf/tf.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>

/** for global path planner interface */
#include <costmap_2d/costmap_2d.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <nav_core/base_global_planner.h>

#include <angles/angles.h>

// #include <pcl_conversions/pcl_conversions.h>
#include <base_local_planner/costmap_model.h>
#include <base_local_planner/world_model.h>

// Added for rviz
#include <pcl_ros/publisher.h>

#include "sensor_msgs/LaserScan.h"
#include "sensor_msgs/PointCloud2.h"
#include <string>
#include <vector>
#include <set>
#include <boost/foreach.hpp>

//  using namespace std;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::multiset;

#ifndef INCLUDE_SMARTAGV_RASTAR_H_
#define INCLUDE_SMARTAGV_RASTAR_H_

/**
 * @struct cells
 * @brief A struct used for a cell and its fCost.
 */
struct cells {
  int currentCell;
  float fCost;
};

/**
 *  @brief Class definition of global planner class
*/
namespace RAstar_planner {

class RAstarPlannerROS : public nav_core::BaseGlobalPlanner {
 public:
  explicit RAstarPlannerROS(ros::NodeHandle &);
  RAstarPlannerROS();
  RAstarPlannerROS(std::string name, costmap_2d::Costmap2DROS *costmap_ros);

  // Added for rviz
  ros::Publisher plan_pub_;

  ros::NodeHandle ROSNodeHandle;

  /** overriden classes from interface nav_core::BaseGlobalPlanner **/

  /**
       *   @brief  Initialize planner
       *
       *   @param name in string
       *   @param costmap from costmap_2s::Costmap2DROS
       *   @return none
      */
  void initialize(std::string name, costmap_2d::Costmap2DROS *costmap_ros);

  /**
     *   @brief  Create a plan given a start and goal
     *
     *   @param  start in geometry message/Posestamped
     *   @param  start in geometry message/Posestamped
     *   @param  plan in vector of geometry messages/Posestamped
     *   @return true/false
    */
  bool makePlan(const geometry_msgs::PoseStamped &start,
                const geometry_msgs::PoseStamped &goal,
                std::vector<geometry_msgs::PoseStamped> &plan);

  /**
      *   @brief  Get cooridnate based on x,y position and origin location
      *
      *   @param  x in float
      *   @param  y in float

      *   @return none
     */
  void getCorrdinate(float &x, float &y);

  /**
      *   @brief  Get cell index based on x,y and resolution
      *
      *   @param  x in float
      *   @param  y in float

      *   @return cell index in int
     */
  int convertToCellIndex(float x, float y);

  /**
        *   @brief  Convert to coordinate base on x,y, origin location, and id
   * index
        *
        *   @param  index in int
        *   @param  x in float
        *   @param  y in float
        *   @return none
       */
  void convertToCoordinate(int index, float &x, float &y);

  /**
        *   @brief  Check if cell is in the map based on resolution, width, and
     height of map
        *
        *   @param  float in x
        *   @param  float in y

        *   @return true/false
       */
  bool isCellInsideMap(float x, float y);

  /**
        *   @brief  Convert costmap to world coordinates based on origin and
   * resolution
        *
        *   @param  mx in double
        *   @param  my in double
        *   @param  wx in double
        *   @param wy in double
        *   @return none
       */
  void mapToWorld(double mx, double my, double &wx, double &wy);

  /**
        *   @brief  Find best path from start to goal and time how long it took
        *
        *   @param  startCell in int
        *   @param  goalCell in int

        *   @return bestPath in vector int
       */
  vector<int> RAstarPlanner(int startCell, int goalCell);

  /**
        *   @brief  Run through algorithm to find path to goal, update open list
   * and cost scores
        *
        *   @param  startCell in int
        *   @param  goalCell in int
        *   @param  g_score in float
        *   @return bestPath in vector int
       */
  vector<int> findPath(int startCell, int goalCell, float g_score[]);

  /**
        *   @brief  Construct the path given start, goal, and gscores from
   * findPath
        *
        *   @param  startCell in int
        *   @param  goalCell in int
        *   @param  g_score in float
        *   @return bestPath in vector int
       */
  vector<int> constructPath(int startCell, int goalCell, float g_score[]);

  /**
        *   @brief  Calculate Heuristic cost
        *
        *   @param  cellID in int
        *   @param  goalCell in int

        *   @return float Hueristic cost
       */
  float calculateHCost(int cellID, int goalCell);

  /**
        *   @brief  Add a cell to the Open List
        *
        *   @param  OPL in multiset cells
        *   @param  neighborCell in int
        *   @param  goalCell in int
        *   @param  g_score in float
        *   @return none
       */
  void addNeighborCellToOpenList(multiset<cells> &OPL, int neighborCell,
                                 int goalCell, float g_score[]);

  /**
        *   @brief  Find free neighbors of current cell
        *
        *   @param  cellID in int
        *   @return Free neighbors cells in vector int
       */
  vector<int> findFreeNeighborCell(int CellID);

  /**
        *   @brief  Checks if start and goal are valid cells
        *
        *   @param  startCell in int
        *   @param  goalCell in int
        *   @return true/false
       */
  bool isStartAndGoalCellsValid(int startCell, int goalCell);

  /**
        *   @brief  Get the cost given two cell Ids. Get points and use other
   * getMoveCost function
        *
        *   @param  CellID1 in int
        *   @param  cellID2 in int
        *   @return moveCost in float
       */
  float getMoveCost(int CellID1, int CellID2);

  /**
        *   @brief  Get the move cost given two cells, four points (x,y)
        *
        *   @param  i1 in int
        *   @param  j1 in int
        *   @param  i2 in int
        *   @param  j2 in int
        *   @return moveCost in float
       */
  float getMoveCost(int i1, int j1, int i2, int j2);

  /**
        *   @brief  Check if a cell is free from cell ID
        *
        *   @param  CellID in int
        *   @return true/false
       */
  bool isFree(int CellID);  // returns true if the cell is Free

  /**
        *   @brief  Check if a cell is free from i,j point
        *
        *   @param  i in int
        *   @param  j in int
        *   @return true/false
       */
  bool isFree(int i, int j);

  /**
        *   @brief  Get cell index from point i,j and width
        *
        *   @param  i in int
        *   @param  j in int
        *   @return cell index in int
       */
  int getCellIndex(int i, int j);  // get index cell to be used in Path

  /**
        *   @brief  Get row id for cell given index, using width
        *
        *   @param  index in int
        *   @return cell row id in int
       */
  int getCellRowID(int index);  // get the row ID from cell index

  /**
        *   @brief  Get column id for cell given index, using width
        *
        *   @param  index in int
        *   @return cell column id in int
       */
  int getCellColID(int index);  // get colunm ID from cell index

  /**
        *   @brief  Publish the final path
        *
        *   @param  path in geometry messages/poseStamped
        *   @param  r in double
        *   @param  g in double
        *   @param  b in double
        *   @param  a in double
        *   @return none
       */
  void publishPlan(const std::vector<geometry_msgs::PoseStamped> &path,
                   double r, double g, double b, double a);

  float originX;                           // x origin
  float originY;                           // y origin
  float resolution;                        // resolution
  costmap_2d::Costmap2DROS *costmap_ros_;  // costmap
  double step_size_, min_dist_from_robot_;  // stepsize, dist from robot
  costmap_2d::Costmap2D *costmap_;         // costmap
  bool initialized_;                       // initialize var
  int width;                               // width
  int height;                              // height
  bool *OGM;                               // checking if cell is free
};

};     // namespace RAstar_planner
#endif  // INCLUDE_SMARTAGV_RASTAR_H_
