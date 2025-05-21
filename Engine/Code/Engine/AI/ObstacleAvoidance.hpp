#pragma once
#include <vector>
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

struct NavMesh;
class NavMeshHeatMap;

struct AIAgent
{
	Vec3 m_position = Vec3::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	Vec3 m_preferredVelocity = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	float m_searchRadius = 0.f;
	float m_physicsRadius = 0.f;
	float m_moveSpeed = 0.f;
};

struct VO
{
	Vec3 m_apex; // The point from which the obstacle is viewed 
	Vec3 m_leftLeg; // Boundary of the obstacle
	Vec3 m_rightLeg; // Boundary of the obstacle

	VO() {}

	VO operator+(Vec3 const& other) const
	{
		VO result;
		result.m_apex = this->m_apex + other * 0.5f;
		result.m_leftLeg = this->m_leftLeg + other;
		result.m_rightLeg = this->m_rightLeg + other;

		return result;
	}
};

struct RVO
{
	Vec3 m_apex; // The point from which the obstacle is viewed 
	Vec3 m_leftLeg; // Boundary of the obstacle
	Vec3 m_rightLeg; // Boundary of the obstacle

	RVO() {}

	RVO(const VO& vo, const Vec3& averageVelocities)
	{
		// Initialize the RVO based on the VO and average velocity
		m_apex = vo.m_apex + averageVelocities;
		m_leftLeg = vo.m_leftLeg;
		m_rightLeg = vo.m_rightLeg;
	}
};

struct HRVO
{
	Vec3 m_apex; // The point from which the obstacle is viewed 
	Vec3 m_leftLeg; // Boundary of the obstacle
	Vec3 m_rightLeg; // Boundary of the obstacle

	HRVO() {}

	HRVO(const VO& vo, const Vec3& relativeVelocity)
	{
		// Initialize the HRVO based on the VO and relative velocity
		m_apex = vo.m_apex + (relativeVelocity * 0.5f);
		m_leftLeg = vo.m_leftLeg;
		m_rightLeg = vo.m_rightLeg;
	}
};

class ObstacleAvoidnace
{
public:
	ObstacleAvoidnace(NavMesh* mesh, NavMeshHeatMap* heatMap);
	ObstacleAvoidnace() = default;
	virtual ~ObstacleAvoidnace() = default;

public:
	// VO 
	VO CalculateVO(const AIAgent& agent, const AIAgent& other);
	void ComputeVO(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug = false);
	bool IsInsideVO(const Vec3& velocity, const VO& vo);
	Vec3 FindAlternativeVelocity(const AIAgent& agent, const VO& vo);

	// RVO
	void ComputeRVO(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug = false);
	bool IsInsideRVO(const Vec3& velocity, const RVO& rvo);

	// HRVO
	void ComputeHRVO(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug = false);
	bool IsInsideHRVO(const Vec3& velocity, const HRVO& hrvo);

	// ORCA
	void ComputeORCA(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug = false);
	bool IsInsideORCAConstraint(const Vec3& velocity, const Vec3& constraintPoint, const Vec3& normal);
	Vec3 FindAlternativeVelocity(const Vec3& preferredVelocity, const Vec3& halfPlaneNormal);

private:
	NavMesh* m_navMesh = nullptr;
	NavMeshHeatMap* m_heatMap = nullptr;
};