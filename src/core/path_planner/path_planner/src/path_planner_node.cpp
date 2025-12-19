/**
 * *********************************************************
 *
 * @file: path_planner_node.cpp
 * @brief: Contains the path planner ROS wrapper class
 * @author: Yang Haodong
 * @date: 2024-9-24
 * @version: 2.0
 *
 * Copyright (c) 2024, Yang Haodong.
 * All rights reserved.
 *
 * --------------------------------------------------------
 *
 * ********************************************************
 */
#include <tf2/utils.h>
#include <pluginlib/class_list_macros.h>
// path planner
#include "path_planner/path_planner_node.h"

// path processor
#include "path_planner/path_prune/ramer_douglas_peucker.h"

// generator
#include "trajectory_planner/trajectory_generation/cubic_bezier_generator.h"

#include "common/util/log.h"
#include "common/util/visualizer.h"
#include "common/math/math_helper.h"

PLUGINLIB_EXPORT_CLASS(rmp::path_planner::PathPlannerNode, nav_core::BaseGlobalPlanner)

namespace rmp
{
  namespace path_planner
  {
    using Visualizer = rmp::common::util::Visualizer;
    using namespace rmp::trajectory_generation;
    using namespace rmp::common::math;

    /**
 * @brief Construct a new Graph Planner object
 */
    PathPlannerNode::PathPlannerNode() : initialized_(false), g_planner_(nullptr)
    {
    }

    /**
 * @brief Construct a new Graph Planner object
 * @param name        planner name
 * @param costmap_ros the cost map to use for assigning costs to trajectories
 */
    PathPlannerNode::PathPlannerNode(std::string name, costmap_2d::Costmap2DROS *costmap_ros) : PathPlannerNode()
    {
      initialize(name, costmap_ros);
    }

    /**
 * @brief Planner initialization
 * @param name       planner name
 * @param costmapRos costmap ROS wrapper
 */
    void PathPlannerNode::initialize(std::string name, costmap_2d::Costmap2DROS *costmapRos)
    {
      costmap_ros_ = costmapRos;
      initialize(name);
    }

    /**
 * @brief Planner initialization
 * @param name     planner name
 * @param costmap  costmap pointer
 * @param frame_id costmap frame ID
 */
    void PathPlannerNode::initialize(std::string name)
    {
      if (!initialized_)
      {
        initialized_ = true;

        // initialize ROS node
        ros::NodeHandle private_nh("~/" + name);

        // costmap frame ID
        frame_id_ = costmap_ros_->getGlobalFrameID();

        private_nh.param("default_tolerance", tolerance_, 0.0);                 // error tolerance
        private_nh.param("outline_map", is_outline_, false);                    // whether outline the map or not
        private_nh.param("expand_zone", is_expand_, false);                     // whether publish expand zone or not
        private_nh.param("show_safety_corridor", show_safety_corridor_, false); // whether visualize safety corridor

        PathPlannerFactory::PlannerProps path_planner_props;
        if (!PathPlannerFactory::createPlanner(private_nh, costmap_ros_, path_planner_props))
        {
          R_ERROR << "Create path planner failed.";
        }

        g_planner_ = path_planner_props.planner_ptr;
        planner_type_ = path_planner_props.planner_type;

        pruner_ = std::make_shared<RDPPathProcessor>(0.25, 3.5);
        // pruner_ = std::make_shared<SavitzkyGolayPathProcessor>();

        generator_ = std::make_shared<CubicBezierGenerator>(costmap_ros_, 0.2, 0.3, true, true);

        // register planning publisher
        plan_pub_ = private_nh.advertise<nav_msgs::Path>("plan", 1);
        traj_pub_ = private_nh.advertise<nav_msgs::Path>("trajectory", 1);
        plan_opt_pub_ = private_nh.advertise<nav_msgs::Path>("plan_opt", 1);
        points_pub_ = private_nh.advertise<visualization_msgs::MarkerArray>("key_points", 1);
        lines_pub_ = private_nh.advertise<visualization_msgs::MarkerArray>("safety_corridor", 1);
        tree_pub_ = private_nh.advertise<visualization_msgs::MarkerArray>("random_tree", 1);
        particles_pub_ = private_nh.advertise<visualization_msgs::MarkerArray>("particles", 1);

        // register explorer visualization publisher
        expand_pub_ = private_nh.advertise<nav_msgs::OccupancyGrid>("expand", 1);

        // register planning service
        make_plan_srv_ = private_nh.advertiseService("make_plan", &PathPlannerNode::makePlanService, this);
      }
      else
      {
        ROS_WARN("This planner has already been initialized, you can't call it twice, doing nothing");
      }
    }

    /**
 * @brief plan a path given start and goal in world map
 * @param start start in world map
 * @param goal  goal in world map
 * @param plan  plan
 * @return true if find a path successfully, else false
 */
    bool PathPlannerNode::makePlan(const geometry_msgs::PoseStamped &start, const geometry_msgs::PoseStamped &goal,
                                   std::vector<geometry_msgs::PoseStamped> &plan)
    {
      return makePlan(start, goal, tolerance_, plan);
    }

    /**
 * @brief Plan a path given start and goal in world map
 * @param start     start in world map
 * @param goal      goal in world map
 * @param plan      plan
 * @param tolerance error tolerance
 * @return true if find a path successfully, else false
 */
    bool PathPlannerNode::makePlan(const geometry_msgs::PoseStamped &start, const geometry_msgs::PoseStamped &goal,
                                   double tolerance, std::vector<geometry_msgs::PoseStamped> &plan)
    {
      // start thread mutex
      std::unique_lock<costmap_2d::Costmap2D::mutex_t> lock(*g_planner_->getCostMap()->getMutex());
      if (!initialized_)
      {
        R_ERROR << "This planner has not been initialized yet, but it is being used, please call initialize() before use";
        return false;
      }
      // clear existing plan
      plan.clear();

      // judege whether goal and start node in costmap frame or not
      if (goal.header.frame_id != frame_id_)
      {
        R_ERROR << "The goal pose passed to this planner must be in the " << frame_id_ << " frame. It is instead in the "
                << goal.header.frame_id << " frame.";
        return false;
      }

      if (start.header.frame_id != frame_id_)
      {
        R_ERROR << "The start pose passed to this planner must be in the " << frame_id_ << " frame. It is instead in the "
                << start.header.frame_id << " frame.";
        return false;
      }

      // visualization
      const auto &visualizer = rmp::common::util::VisualizerPtr::Instance();

      // outline the map
      if (is_outline_)
        g_planner_->outlineMap();

      // calculate path
      PathPlanner::Points3d origin_plan;
      PathPlanner::Points3d expand;
      bool path_found = false;

      // planning
      R_INFO << "Planning request: Start=(" << start.pose.position.x << ", " << start.pose.position.y << "), Goal=(" << goal.pose.position.x << ", " << goal.pose.position.y << ")";
      auto start_time = std::chrono::high_resolution_clock::now();
      path_found = g_planner_->plan({start.pose.position.x, start.pose.position.y, tf2::getYaw(start.pose.orientation)},
                                    {goal.pose.position.x, goal.pose.position.y, tf2::getYaw(goal.pose.orientation)},
                                    origin_plan, expand);
      auto finish_time = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> cal_time_jps = finish_time - start_time;
      R_INFO << "Calculation Time: " << cal_time_jps.count() << " s";
      R_INFO << "Path found flag: " << path_found << ", Path size: " << origin_plan.size() << ", Expanded nodes: " << expand.size();

      // convert path to ros plan
      if (path_found)
      {
        R_INFO << "Path found by planner, converting to ROS plan format...";
        if (_getPlanFromPath(origin_plan, plan))
        {
          R_INFO << "Successfully converted to ROS plan with " << plan.size() << " poses";
          auto calCurv = [](const PathPlanner::Point3d &pt1, const PathPlanner::Point3d &pt2,
                            const PathPlanner::Point3d &pt3)
          {
            // double ax = pt1.x();
            // double ay = pt1.y();
            // double bx = pt2.x();
            // double by = pt2.y();
            // double cx = pt3.x();
            // double cy = pt3.y();
            // double a = std::hypot(bx - cx, by - cy);
            // double b = std::hypot(cx - ax, cy - ay);
            // double c = std::hypot(ax - bx, ay - by);

            // double cosB = (a * a + c * c - b * b) / (2 * a * c);
            // double sinB = std::sin(std::acos(cosB));

            // double cross = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
            // return std::copysign(2 * sinB / b, cross);
            const double dx1 = pt2.x() - pt1.x();
            const double dy1 = pt2.y() - pt1.y();
            const double dx2 = pt3.x() - pt2.x();
            const double dy2 = pt3.y() - pt2.y();
            const double area = 0.5 * ((dx1 * dy2) - (dx2 * dy1));
            const double l1 = hypot(dx1, dy1);
            const double l2 = hypot(dx2, dy2);
            if (l1 < 1e-6 || l2 < 1e-6 || fabs(area) < 1e-6)
            {
              return 0.0;
            }

            return 4.0 * area / (l1 * l2 * (l1 + l2));
          };

          auto calAngle = [](const PathPlanner::Point3d &pt1, const PathPlanner::Point3d &pt2,
                             const PathPlanner::Point3d &pt3)
          {
            // rmp::common::geometry::Vec2d vec1(pt2.x() - pt1.x(), pt2.y() - pt1.y());
            // rmp::common::geometry::Vec2d vec2(pt3.x() - pt2.x(), pt3.y() - pt2.y());
            // return std::acos(vec1.innerProd(vec2) / (vec1.length() * vec2.length()));
            const double dx1 = pt2.x() - pt1.x();
            const double dy1 = pt2.y() - pt1.y();
            const double dx2 = pt3.x() - pt2.x();
            const double dy2 = pt3.y() - pt2.y();
            const double l1 = hypot(dx1, dy1);
            const double l2 = hypot(dx2, dy2);
            if (l1 < 1e-6 || l2 < 1e-6)
            {
              return 0.0;
            }

            const double cross = dx1 * dy2 - dy1 * dx2;
            const double dot = dx1 * dx2 + dy1 * dy2;
            return atan2(cross, dot);
          };

          double path_len = 0.0;
          double max_curv = 0.0;
          double avg_curv = 0.0;
          double max_angle = 0.0;
          double avg_angle = 0.0;

          for (int i = 2; i < origin_plan.size(); i++)
          {
            int n = i - 1;
            const auto &pt1 = origin_plan[i - 2];
            const auto &pt2 = origin_plan[i - 1];
            const auto &pt3 = origin_plan[i];
            path_len += std::hypot(pt1.x() - pt2.x(), pt1.y() - pt2.y());
            double curv = std::fabs(calCurv(pt1, pt2, pt3));
            if (std::isnan(curv))
            {
              continue;
            }
            max_curv = std::max(curv, max_curv);
            avg_curv = ((n - 1) * avg_curv + curv) / n;
            double angle = calAngle(pt1, pt2, pt3);
            if (std::isnan(angle))
            {
              continue;
            }
            if (angle > M_PI)
            {
              angle = M_PI * 2 - angle;
            }
            angle = std::abs(pi2pi(angle));
            max_angle = std::max(angle, max_angle);
            avg_angle = ((n - 1) * avg_angle + angle) / n;
          }
          path_len += std::hypot(origin_plan[origin_plan.size() - 1].x() - origin_plan[origin_plan.size() - 2].x(),
                                 origin_plan[origin_plan.size() - 1].y() - origin_plan[origin_plan.size() - 2].y());

          R_INFO << "path length: " << path_len;
          R_INFO << "max curvature: " << max_curv;
          R_INFO << "avg curvature: " << avg_curv;
          R_INFO << "max angle: " << max_angle;
          R_INFO << "avg angle: " << avg_angle;
          R_INFO << "---------------";

          geometry_msgs::PoseStamped goalCopy = goal;
          goalCopy.header.stamp = ros::Time::now();
          plan.pop_back();
          plan.push_back(goalCopy);
          plan[0].pose.orientation = start.pose.orientation;

          // path process
          PathPlanner::Points3d prune_plan;
          pruner_->process(origin_plan, prune_plan);

          // generation
          PathPlanner::Points3d origin_traj;
          generator_->setWaypoints(origin_plan);
          R_INFO << "origin plan size: " << origin_plan.size();
          auto start_time_2 = std::chrono::high_resolution_clock::now();
          if (generator_->generation())
          {
            auto finish_time_2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> cal_time_bezier = finish_time_2 - start_time_2;
            R_INFO << "[OPT] Calculation Time: " << cal_time_jps.count() + cal_time_bezier.count() << " s";

            rmp::common::structure::Trajectory3d traj;
            if (generator_->getTrajectory(traj))
            {
              for (const auto &pt : traj.position)
              {
                origin_traj.emplace_back(pt.x(), pt.y(), pt.theta());
              }
            }
            path_len = 0.0;
            max_curv = 0.0;
            avg_curv = 0.0;
            max_angle = 0.0;
            avg_angle = 0.0;

            for (int i = 2; i < origin_traj.size(); i++)
            {
              int n = i - 1;
              const auto &pt1 = origin_traj[i - 2];
              const auto &pt2 = origin_traj[i - 1];
              const auto &pt3 = origin_traj[i];
              path_len += std::hypot(pt1.x() - pt2.x(), pt1.y() - pt2.y());
              double curv = std::fabs(calCurv(pt1, pt2, pt3));
              if (std::isnan(curv))
              {
                continue;
              }
              max_curv = std::max(curv, max_curv);
              avg_curv = ((n - 1) * avg_curv + curv) / n;
              double angle = calAngle(pt1, pt2, pt3);
              if (std::isnan(angle))
              {
                continue;
              }
              angle = std::abs(pi2pi(angle));
              max_angle = std::max(angle, max_angle);
              avg_angle = ((n - 1) * avg_angle + angle) / n;
            }
            path_len += std::hypot(origin_traj[origin_traj.size() - 1].x() - origin_traj[origin_traj.size() - 2].x(),
                                   origin_traj[origin_traj.size() - 1].y() - origin_traj[origin_traj.size() - 2].y());

            R_INFO << "[OPT] path length: " << path_len;
            R_INFO << "[OPT] max curvature: " << max_curv;
            R_INFO << "[OPT] avg curvature: " << avg_curv;
            R_INFO << "[OPT] max angle: " << max_angle;
            R_INFO << "[OPT] avg angle: " << avg_angle;
            R_INFO << "---------------";
            visualizer->publishPlan(origin_traj, traj_pub_, frame_id_);
          }

          // publish visulization plan
          if (is_expand_)
          {
            if (planner_type_ == GRAPH_PLANNER)
            {
              // publish expand zone
              visualizer->publishExpandZone(expand, costmap_ros_->getCostmap(), expand_pub_, frame_id_);
            }
            else
            {
              R_WARN << "Unknown planner type.";
            }
          }
          visualizer->publishPlan(origin_plan, plan_pub_, frame_id_);
          visualizer->publishPoints(prune_plan, points_pub_, frame_id_, "key_points", Visualizer::PURPLE, 0.15);

        }
        else
        {
          R_ERROR << "Failed to get a plan from path when a legal path was found. This shouldn't happen.";
        }
      }
      else
      {
        R_ERROR << "Failed to get a path. path_found=" << path_found << ", origin_plan.size()=" << origin_plan.size() << ", expand.size()=" << expand.size();
      }
      return !plan.empty();
    }

    /**
 * @brief Regeister planning service
 * @param req  request from client
 * @param resp response from server
 * @return true
 */
    bool PathPlannerNode::makePlanService(nav_msgs::GetPlan::Request &req, nav_msgs::GetPlan::Response &resp)
    {
      makePlan(req.start, req.goal, resp.plan.poses);
      resp.plan.header.stamp = ros::Time::now();
      resp.plan.header.frame_id = frame_id_;

      return true;
    }

    /**
 * @brief Calculate plan from planning path
 * @param path path generated by global planner
 * @param plan plan transfromed from path, i.e. [start, ..., goal]
 * @return bool true if successful, else false
 */
    bool PathPlannerNode::_getPlanFromPath(PathPlanner::Points3d &path, std::vector<geometry_msgs::PoseStamped> &plan)
    {
      if (!initialized_)
      {
        R_ERROR << "This planner has not been initialized yet, but it is being used, please call initialize() before use";
        return false;
      }
      plan.clear();

      for (const auto &pt : path)
      {
        // double wx, wy;
        // g_planner_->map2World(pt.x(), pt.y(), wx, wy);

        // coding as message type
        geometry_msgs::PoseStamped pose;
        pose.header.stamp = ros::Time::now();
        pose.header.frame_id = frame_id_;
        pose.pose.position.x = pt.x();
        pose.pose.position.y = pt.y();
        pose.pose.position.z = 0.0;
        tf2::Quaternion q;
        q.setRPY(0, 0, pt.theta());
        pose.pose.orientation.x = q.getX();
        pose.pose.orientation.y = q.getY();
        pose.pose.orientation.z = q.getZ();
        pose.pose.orientation.w = q.getW();
        plan.push_back(pose);
      }

      return !plan.empty();
    }
  } // namespace path_planner
} // namespace rmp