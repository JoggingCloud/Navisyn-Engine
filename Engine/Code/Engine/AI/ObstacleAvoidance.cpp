#include "Engine/AI/ObstacleAvoidance.hpp"
#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Renderer/NavMesh.hpp"

ObstacleAvoidnace::ObstacleAvoidnace(NavMesh* mesh, NavMeshHeatMap* heatMap)
	: m_navMesh(mesh), m_heatMap(heatMap)
{
}

VO ObstacleAvoidnace::CalculateVO(const AIAgent& agent, const AIAgent& other)
{
	VO vo;
	float combinedRadius = agent.m_physicsRadius + other.m_physicsRadius;
	float avoidanceFactor = 5.f; // Scale based on velocity

	vo.m_apex = agent.m_position; // Set apex at agent's position

	Vec3 relativeVelocity = other.m_velocity - agent.m_velocity;

	// Calculate forward direction based on agent velocity and scale it
	Vec3 forwardDir = agent.m_orientation.GetForwardVector().GetNormalized();
	float forwardScale = (relativeVelocity.GetLength() / agent.m_moveSpeed) * avoidanceFactor;
	forwardDir *= forwardScale;

	// Calculate left and right boundaries based on forward direction
	Vec3 perpendicularDir = CrossProduct3D(relativeVelocity, Vec3(0.f, 0.f, 1.f)).GetNormalized();

	// Scale the perpendicular direction by the combined radius
	// Maybe instead of 2.f make it be based on speed?!?!
	float dynamicMultiplier = 2.f * (agent.m_velocity.GetLength() + other.m_velocity.GetLength());
	Vec3 leftLeg = perpendicularDir * combinedRadius * dynamicMultiplier;
	Vec3 rightLeg = -perpendicularDir * combinedRadius * dynamicMultiplier;

	// Calculate final positions of left and right legs
	vo.m_leftLeg = vo.m_apex + leftLeg + forwardDir;
	vo.m_rightLeg = vo.m_apex + rightLeg + forwardDir;

	return vo;
}

void ObstacleAvoidnace::ComputeVO(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug)
{
	if (enableDebug)
	{
		DebugAddWorld3DRing(agent.m_position, searchRadius, 64, 0.1f, 0.f, Rgba8::LIGHT_GREEN, Rgba8::LIGHT_GREEN, DebugRenderMode::ALWAYS);
	}
	
	Vec3 newVelocity = agent.m_preferredVelocity;

	if (!nearbyActors.empty())
	{
		for (AIAgent* other : nearbyActors)
		{
			if (other == nullptr || other == &agent) continue; 

			VO vo = CalculateVO(agent, *other);

			if (enableDebug)
			{
				DebugAddWorldLine(vo.m_apex, agent.m_orientation.GetForwardVector() + vo.m_leftLeg, 0.05f, 0.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::ALWAYS);
				DebugAddWorldLine(vo.m_apex, agent.m_orientation.GetForwardVector() + vo.m_rightLeg, 0.05f, 0.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::ALWAYS);
			}

			if (IsInsideVO(newVelocity, vo))
			{
				Vec3 alternativeVelocity = FindAlternativeVelocity(agent, vo);

				// Calculate velocity blend factor
				float velocityAlignment = DotProduct3D(agent.m_velocity.GetNormalized(), other->m_velocity.GetNormalized());
				float velocityFactor = (1.f - velocityAlignment) * 0.5f; // Factor is higher when agents are moving toward each other

				// Calculate the speed blend factor
				float relativeSpeed = (other->m_velocity - agent.m_velocity).GetLength();
				float maxRelativeSpeed = agent.m_moveSpeed + other->m_moveSpeed;
				float speedFactor = relativeSpeed / maxRelativeSpeed;

				// Calculate the urgency factor (Urgency is higher the more direct the collision is (head on collision))
				Vec3 directionToOther = (other->m_position - agent.m_position).GetNormalized();
				float alignment = DotProduct3D(agent.m_velocity.GetNormalized(), directionToOther);
				float urgencyFactor = (alignment > 0.8f) ? 1.f : (alignment > 0.5f) ? 0.8f : 0.5f;

				// Combine factors to compute the final blend factor (bigger emphasis on the urgency factor)
				float blendFactor = (speedFactor * 0.2f) + (urgencyFactor * 0.7f) + (velocityFactor * 0.1f);

				newVelocity = Interpolate(newVelocity, alternativeVelocity, blendFactor);
				
				if (enableDebug)
				{
					DebugAddWorldArrow(vo.m_apex, newVelocity, 0.8f, 0.07f, 0.05f, 0.f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::ALWAYS);
				}
			}
		}
	}

	agent.m_velocity = newVelocity;
}

bool ObstacleAvoidnace::IsInsideVO(const Vec3& velocity, const VO& vo)
{
	Vec3 relativeVelocity = velocity - vo.m_apex;

	Vec3 leftCross = CrossProduct3D(vo.m_leftLeg, relativeVelocity);
	Vec3 rightCross = CrossProduct3D(vo.m_rightLeg, relativeVelocity);

	return leftCross.z < 0.f && rightCross.z > 0.f;
}

Vec3 ObstacleAvoidnace::FindAlternativeVelocity(const AIAgent& agent, const VO& vo)
{
	// Determine which leg (left or right) has a shorter length to prioritize movement along that leg.
	// This chooses the side where the agent can move with the least change in direction.
	Vec3 direction = (vo.m_leftLeg.GetLength() < vo.m_rightLeg.GetLength()) ? vo.m_leftLeg : vo.m_rightLeg;

	// Normalize the selected direction and scale it to match the agent's preferred movement speed.
	// This ensures that the alternative velocity maintains the agent’s speed but changes direction for avoidance.
	Vec3 alternativeVelocity = direction.GetNormalized() * agent.m_preferredVelocity.GetLength();

	alternativeVelocity.z = 0.f;

	// Return the computed alternative velocity, which offers a safer direction while maintaining speed.
	return alternativeVelocity;
}

void ObstacleAvoidnace::ComputeRVO(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug)
{
	if (enableDebug)
	{
		DebugAddWorld3DRing(agent.m_position, searchRadius, 64, 0.1f, 0.f, Rgba8::LIGHT_GREEN, Rgba8::LIGHT_GREEN, DebugRenderMode::ALWAYS);
	}
	
	Vec3 newVelocity = agent.m_preferredVelocity;

	if (!nearbyActors.empty())
	{
		for (AIAgent* other : nearbyActors)
		{
			if (other == nullptr || other == &agent) continue;

			VO vo = CalculateVO(agent, *other);

			if (enableDebug)
			{
				DebugAddWorldLine(vo.m_apex, agent.m_orientation.GetForwardVector() + vo.m_rightLeg, 0.05f, 0.f, Rgba8::LIGHT_BLUE, Rgba8::LIGHT_BLUE, DebugRenderMode::ALWAYS);
				DebugAddWorldLine(vo.m_apex, agent.m_orientation.GetForwardVector() + vo.m_leftLeg, 0.05f, 0.f, Rgba8::LIGHT_BLUE, Rgba8::LIGHT_BLUE, DebugRenderMode::ALWAYS);
			}

			Vec3 averageVelocities = (agent.m_velocity + other->m_velocity) * 0.5f;
			RVO rvoAB(vo, averageVelocities);

			if (enableDebug)
			{
				DebugAddWorldLine(rvoAB.m_apex, agent.m_orientation.GetForwardVector() + rvoAB.m_leftLeg, 0.05f, 0.f, Rgba8::LIGHT_ORANGE, Rgba8::LIGHT_ORANGE, DebugRenderMode::ALWAYS);
				DebugAddWorldLine(rvoAB.m_apex, agent.m_orientation.GetForwardVector() + rvoAB.m_rightLeg, 0.05f, 0.f, Rgba8::LIGHT_ORANGE, Rgba8::LIGHT_ORANGE, DebugRenderMode::ALWAYS);
			}

			if (IsInsideRVO(newVelocity, rvoAB))
			{
				Vec3 alternativeVelocity = FindAlternativeVelocity(agent, vo);

				// Calculate velocity blend factor
				float velocityAlignment = DotProduct3D(agent.m_velocity.GetNormalized(), other->m_velocity.GetNormalized());
				float velocityFactor = (1.f - velocityAlignment) * 0.5f; // Factor is higher when agents are moving toward each other

				// Calculate the speed blend factor
				float relativeSpeed = (other->m_velocity - agent.m_velocity).GetLength();
				float maxRelativeSpeed = agent.m_moveSpeed + other->m_moveSpeed;
				float speedFactor = relativeSpeed / maxRelativeSpeed;

				// Calculate the urgency factor (Urgency is higher the more direct the collision is (head on collision))
				Vec3 directionToOther = (other->m_position - agent.m_position).GetNormalized();
				float alignment = DotProduct3D(agent.m_velocity.GetNormalized(), directionToOther);
				float urgencyFactor = (alignment > 0.8f) ? 1.f : (alignment > 0.5f) ? 0.8f : 0.5f;

				// Combine factors to compute the final blend factor (bigger emphasis on the urgency factor)
				float blendFactor = (speedFactor * 0.2f) + (urgencyFactor * 0.7f) + (velocityFactor * 0.1f);

				newVelocity = Interpolate(newVelocity, alternativeVelocity, blendFactor);
				
				if (enableDebug)
				{
					DebugAddWorldArrow(vo.m_apex, newVelocity, 0.8f, 0.07f, 0.05f, 0.f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::ALWAYS);
				}
			}
		}
	}
	agent.m_velocity = newVelocity;
}

bool ObstacleAvoidnace::IsInsideRVO(const Vec3& velocity, const RVO& rvo)
{
	Vec3 relativeVelocity = velocity - rvo.m_apex;

	Vec3 leftCross = CrossProduct3D(rvo.m_leftLeg, relativeVelocity);
	Vec3 rightCross = CrossProduct3D(rvo.m_rightLeg, relativeVelocity);

	return leftCross.z < 0.f && rightCross.z > 0.f;
}

void ObstacleAvoidnace::ComputeHRVO(AIAgent& agent, float searchRadius, const std::vector<AIAgent*> nearbyActors, bool enableDebug)
{
	if (enableDebug)
	{
		DebugAddWorld3DRing(agent.m_position, searchRadius, 64, 0.1f, 0.f, Rgba8::LIGHT_GREEN, Rgba8::LIGHT_GREEN, DebugRenderMode::ALWAYS);
	}
	
	Vec3 newVelocity = agent.m_preferredVelocity;

	if (!nearbyActors.empty())
	{
		for (AIAgent* other : nearbyActors)
		{
			if (other == nullptr || other == &agent) continue;

			VO vo = CalculateVO(agent, *other);

			if (enableDebug)
			{
				DebugAddWorldLine(vo.m_apex, agent.m_orientation.GetForwardVector() + vo.m_leftLeg, 0.05f, 0.f, Rgba8::LIGHT_BLUE, Rgba8::LIGHT_BLUE, DebugRenderMode::ALWAYS);
				DebugAddWorldLine(vo.m_apex, agent.m_orientation.GetForwardVector() + vo.m_rightLeg, 0.05f, 0.f, Rgba8::LIGHT_BLUE, Rgba8::LIGHT_BLUE, DebugRenderMode::ALWAYS);
			}

			Vec3 averageVelocities = (agent.m_velocity + other->m_velocity) * 0.5f;
			RVO rvoAB(vo, averageVelocities);

			if (enableDebug)
			{
				DebugAddWorldLine(rvoAB.m_apex, agent.m_orientation.GetForwardVector() + rvoAB.m_leftLeg, 0.05f, 0.f, Rgba8::LIGHT_ORANGE, Rgba8::LIGHT_ORANGE, DebugRenderMode::ALWAYS);
				DebugAddWorldLine(rvoAB.m_apex, agent.m_orientation.GetForwardVector() + rvoAB.m_rightLeg, 0.05f, 0.f, Rgba8::LIGHT_ORANGE, Rgba8::LIGHT_ORANGE, DebugRenderMode::ALWAYS);
			}

			Vec3 relativeVelocity = other->m_velocity - agent.m_velocity;
			HRVO hrvoAB(vo, relativeVelocity);

			if (enableDebug)
			{
				DebugAddWorldLine(hrvoAB.m_apex, agent.m_orientation.GetForwardVector() + hrvoAB.m_leftLeg, 0.05f, 0.f, Rgba8::CYAN, Rgba8::CYAN, DebugRenderMode::ALWAYS);
				DebugAddWorldLine(hrvoAB.m_apex, agent.m_orientation.GetForwardVector() + hrvoAB.m_rightLeg, 0.05f, 0.f, Rgba8::CYAN, Rgba8::CYAN, DebugRenderMode::ALWAYS);
			}

			if (IsInsideHRVO(newVelocity, hrvoAB))
			{
				Vec3 alternativeVelocity = FindAlternativeVelocity(agent, vo);

				// Calculate velocity blend factor
				float velocityAlignment = DotProduct3D(agent.m_velocity.GetNormalized(), other->m_velocity.GetNormalized());
				float velocityFactor = (1.f - velocityAlignment) * 0.5f; // Factor is higher when agents are moving toward each other

				// Calculate the speed blend factor
				float relativeSpeed = (other->m_velocity - agent.m_velocity).GetLength();
				float maxRelativeSpeed = agent.m_moveSpeed + other->m_moveSpeed;
				float speedFactor = relativeSpeed / maxRelativeSpeed;

				// Calculate the urgency factor (Urgency is higher the more direct the collision is (head on collision))
				Vec3 directionToOther = (other->m_position - agent.m_position).GetNormalized();
				float alignment = DotProduct3D(agent.m_velocity.GetNormalized(), directionToOther);
				float urgencyFactor = (alignment > 0.8f) ? 1.f : (alignment > 0.5f) ? 0.8f : 0.5f;

				// Combine factors to compute the final blend factor (bigger emphasis on the urgency factor)
				float blendFactor = (speedFactor * 0.2f) + (urgencyFactor * 0.7f) + (velocityFactor * 0.1f);

				newVelocity = Interpolate(newVelocity, alternativeVelocity, blendFactor);

				if (enableDebug)
				{
					DebugAddWorldArrow(vo.m_apex, newVelocity, 0.8f, 0.07f, 0.05f, 0.f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::ALWAYS);
				}
			}
		}
	}
	agent.m_velocity = newVelocity;
}

bool ObstacleAvoidnace::IsInsideHRVO(const Vec3& velocity, const HRVO& hrvo)
{
	Vec3 relativeVelocity = velocity - hrvo.m_apex;

	Vec3 leftCross = CrossProduct3D(hrvo.m_leftLeg, relativeVelocity);
	Vec3 rightCross = CrossProduct3D(hrvo.m_rightLeg, relativeVelocity);

	return leftCross.z < 0.f && rightCross.z > 0.f;
}

void ObstacleAvoidnace::ComputeORCA(AIAgent& agent, float searchRadius, std::vector<AIAgent*> nearbyActors, bool enableDebug)
{
	if (enableDebug)
	{
		DebugAddWorld3DRing(agent.m_position, searchRadius, 32, 0.05f, 0.f, Rgba8::LIGHT_GREEN, Rgba8::LIGHT_GREEN, DebugRenderMode::ALWAYS);
	}
	
	Vec3 newVelocity = agent.m_preferredVelocity;
	std::vector<Vec3> halfPlaneNormals; // Accumulate half-plane normals

	// Step 1: Find the closest agent nearby 
	for (AIAgent* other : nearbyActors)
	{
		if (other == &agent) continue;

		if (enableDebug)
		{
			// Original velocity vector
			Vec3 clampedPreferred = agent.m_preferredVelocity;
			if (clampedPreferred.GetLength() > agent.m_moveSpeed * agent.m_moveSpeed)
			{
				clampedPreferred = clampedPreferred.GetNormalized() * agent.m_moveSpeed;
			}
			DebugAddWorldLine(agent.m_position, agent.m_position + clampedPreferred, 0.05f, 0.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::ALWAYS);
		}

		// Step 2: Calculate relative properties
		float nearbyActorDistanceSq = GetDistanceSquared3D(agent.m_position, other->m_position);
		float nearbyActorRelativeSpeed = (agent.m_velocity - other->m_velocity).GetLength();
		if (nearbyActorRelativeSpeed <= 0.f)
		{
			nearbyActorRelativeSpeed = 0.1f; // Prevent division by zero
		}

		float combinedRadius = agent.m_physicsRadius + other->m_physicsRadius;
		float tau = (nearbyActorDistanceSq - combinedRadius) / nearbyActorRelativeSpeed; // Predict the time to collision
		float minTimeHorizon = 0.1f;
		// Mess with value. 
		// Higher value will allow agents to make smoother and gradual adjustments. 
		// Lower value means more frequent changes and react only when they're very close to collisions.
		float maxTimeHorizon = 5.f; 
		float t = GetClamped(tau, minTimeHorizon, maxTimeHorizon); // Safety metric for determining time before collision

		Vec3 relativePosition = (other->m_position - agent.m_position) / t;
		Vec3 sharedVelocity = (other->m_velocity + agent.m_velocity) * 0.5f;

		// Step 3: Calculate the outward normal from relative position (iBasis)
		Vec3 outwardNormal = relativePosition.GetNormalized();
		halfPlaneNormals.emplace_back(outwardNormal); // store all the accumulated half-plane normals

		Vec3 jBasis = CrossProduct3D(outwardNormal, Vec3(0.f, 0.f, 1.f)).GetNormalized();

		if (enableDebug)
		{
			// Draw lines in the i and j directions to represent the plane boundary
			DebugAddWorldLine(agent.m_position - outwardNormal, agent.m_position + outwardNormal, 0.05f, 0.f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::ALWAYS);
			DebugAddWorldLine(agent.m_position - jBasis, agent.m_position + jBasis, 0.025f, 0.f, Rgba8::DARK_GREEN, Rgba8::DARK_GREEN, DebugRenderMode::ALWAYS);
		}

		// Step 4: Project the outward normal onto the shared velocity of the nearby agent
		Vec3 currentProjectedVelocity = GetProjectedOnto3D(sharedVelocity, outwardNormal);

		if (enableDebug)
		{
			// Projected point
			DebugAddWorldPoint(agent.m_position + currentProjectedVelocity, 0.1f, 16, 0.f, Rgba8::LIGHT_ORANGE, Rgba8::LIGHT_ORANGE, DebugRenderMode::ALWAYS);
		}

		// Step 5: Check if the other agent is static
		if (other->m_velocity.GetLengthSquared() <= 0.f)
		{
			// If its not on the safe side of the region
			if (!IsInsideORCAConstraint(newVelocity, currentProjectedVelocity, outwardNormal))
			{
				Vec3 accumulatedVelocity = Vec3::ZERO; // Initialize the accumulated velocities 
				int validAlternatives = 0; // Initialize the number of valid alternative velocity directions 

				for (const Vec3& halfPlaneNormal : halfPlaneNormals) // Loop through the number of stored half-plane normals
				{
					Vec3 alternativeVelocity = FindAlternativeVelocity(agent.m_preferredVelocity, halfPlaneNormal); // Find the alternative velocity for each half-plane normal
					
					if (m_navMesh && m_navMesh->m_heatMap)
					{
						int currentTri = m_navMesh->GetContainingTriangleIndex(agent.m_position);
						if (currentTri != -1)
						{
							const NavMeshTri& triangle = *m_navMesh->GetNavMeshTriangle(currentTri);
							float heatValueAtIndex = m_heatMap->GetValue(currentTri);
							float maxHeatDiff = 0.f;
							float bestAlignment = -1.f;

							Vec3 bestDirection = Vec3::ZERO;

							for (int neighborIndex : triangle.m_neighborTriIndexes)
							{
								if (neighborIndex == -1) continue;
								float neighborHeat = m_heatMap->GetValue(neighborIndex);

								if (neighborHeat > heatValueAtIndex)
								{
									float heatDiff = neighborHeat - heatValueAtIndex;
									Vec3 dirToCooler = (m_navMesh->GetTriangleCentroid(neighborIndex) - m_navMesh->GetTriangleCentroid(currentTri)).GetNormalized();
									float alignment = DotProduct3D(agent.m_preferredVelocity.GetNormalized(), dirToCooler); // reward direction close to preferred

									// Prioritize cooler triangle that's well-aligned with preferred movement
									if (heatDiff > maxHeatDiff || (heatDiff == maxHeatDiff && alignment > bestAlignment))
									{
										maxHeatDiff = heatDiff;
										bestAlignment = alignment;
										bestDirection = dirToCooler;
									}
								}
							}

							if (maxHeatDiff > 0.f)
							{
								// Higher heat -> lower influence
								float influenceStrength = 0.25f; // #TODO mess with value, test and see what feels right
								Vec3 influencedVelocity = alternativeVelocity + bestDirection * maxHeatDiff * influenceStrength;
								
								DebugAddWorldArrow(agent.m_position, agent.m_position + bestDirection * maxHeatDiff, 0.4f, 0.03f, 0.03f, 0.03f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::ALWAYS);
								alternativeVelocity = Interpolate(alternativeVelocity, influencedVelocity, 0.5f);
							}
						}
					}
					
					if (enableDebug)
					{
						DebugAddWorldLine(agent.m_position, agent.m_position + alternativeVelocity, 0.025f, 0.f, Rgba8::LIGHT_GRAY, Rgba8::LIGHT_GRAY, DebugRenderMode::ALWAYS);
					}

					accumulatedVelocity += alternativeVelocity; // Increase the number of accumulated velocities
					validAlternatives++; // Increase the number of valid alternatives 
				}

				if (validAlternatives > 0)
				{
					accumulatedVelocity /= static_cast<float>(validAlternatives);

					// Calculate the speed blend factor
					float relativeSpeed = (other->m_velocity - agent.m_velocity).GetLength();
					float maxRelativeSpeed = agent.m_moveSpeed + other->m_moveSpeed;
					float speedFactor = relativeSpeed / maxRelativeSpeed;

					// Calculate the urgency factor (Urgency is higher the more direct the collision is (head on collision))
					Vec3 directionToOther = (other->m_position - agent.m_position).GetNormalized();
					float alignment = DotProduct3D(agent.m_velocity.GetNormalized(), directionToOther);
					float urgencyFactor = (alignment > 0.8f) ? 1.f : (alignment > 0.5f) ? 0.8f : 0.5f;

					// Combine factors to compute the final blend factor (bigger emphasis on the urgency factor)
					float blendFactor = (speedFactor * 0.4f) + (urgencyFactor * 0.6f);

					newVelocity = Interpolate(newVelocity, accumulatedVelocity, blendFactor);
				}
			}
		}
		else
		{
			// Step 6: Check if the other agent is within the ORCA constraint.
			// Do this if other agent is non static (meaning other agent is moving)
			// If its not on the safe side of the region
			if (!IsInsideORCAConstraint(newVelocity, currentProjectedVelocity, outwardNormal))
			{
				Vec3 minimalViolationVelocity = agent.m_preferredVelocity; // Set minimal velocity 
				float minimumPenetration = std::numeric_limits<float>::max();

				Vec3 accumulatedVelocity = Vec3::ZERO; // Initialize the accumulated velocities 
				int validAlternatives = 0; // Initialize the number of valid alternative velocity directions 

				for (const Vec3& halfPlaneNormal : halfPlaneNormals) // Loop through the number of stored half-plane normals
				{
					Vec3 alternativeVelocity = FindAlternativeVelocity(agent.m_preferredVelocity, halfPlaneNormal); // Find the alternative velocity for each half-plane normal
					
					if (m_navMesh && m_navMesh->m_heatMap)
					{
						int currentTri = m_navMesh->GetContainingTriangleIndex(agent.m_position);
						if (currentTri != -1)
						{
							const NavMeshTri& triangle = *m_navMesh->GetNavMeshTriangle(currentTri);
							float heatValueAtIndex = m_heatMap->GetValue(currentTri);
							float maxHeatDiff = 0.f;
							float bestAlignment = -1.f;

							Vec3 bestDirection = Vec3::ZERO;

							for (int neighborIndex : triangle.m_neighborTriIndexes)
							{
								if (neighborIndex == -1) continue;
								float neighborHeat = m_heatMap->GetValue(neighborIndex);

								if (neighborHeat > heatValueAtIndex)
								{
									float heatDiff = neighborHeat - heatValueAtIndex;
									Vec3 dirToCooler = (m_navMesh->GetTriangleCentroid(neighborIndex) - m_navMesh->GetTriangleCentroid(currentTri)).GetNormalized();
									float alignment = DotProduct3D(agent.m_preferredVelocity.GetNormalized(), dirToCooler); // reward direction close to preferred

									// Prioritize cooler triangle that's well-aligned with preferred movement
									if (heatDiff > maxHeatDiff || (heatDiff == maxHeatDiff && alignment > bestAlignment))
									{
										maxHeatDiff = heatDiff;
										bestAlignment = alignment;
										bestDirection = dirToCooler;
									}
								}
							}

							if (maxHeatDiff > 0.f)
							{
								// Higher heat -> lower influence
								float influenceStrength = 0.25f; // #TODO mess with value, test and see what feels right
								Vec3 influencedVelocity = alternativeVelocity + bestDirection * maxHeatDiff * influenceStrength;

								if (enableDebug)
								{
									DebugAddWorldArrow(agent.m_position, agent.m_position + bestDirection * maxHeatDiff, 0.4f, 0.03f, 0.03f, 0.03f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::ALWAYS);
								}
								alternativeVelocity = Interpolate(alternativeVelocity, influencedVelocity, 0.5f);
							}
						}
					}

					minimalViolationVelocity = alternativeVelocity; // Set the minimal velocity change to be the alternative velocity 
					
					if (enableDebug)
					{
						DebugAddWorldLine(agent.m_position, agent.m_position + alternativeVelocity, 0.025f, 0.f, Rgba8::LIGHT_GRAY, Rgba8::LIGHT_GRAY, DebugRenderMode::ALWAYS);
					}

					// Calculate how much this velocity penetrates the half-plane
					float penetrationDistance = DotProduct3D(alternativeVelocity - newVelocity, halfPlaneNormal);
					if (penetrationDistance < minimumPenetration)
					{
						minimumPenetration = penetrationDistance; // Set the minimum Penetration distance 
						minimalViolationVelocity = alternativeVelocity; // Keep the velocity with the minimal penetration
					}

					accumulatedVelocity += alternativeVelocity; // Increase the number of accumulated velocities
					validAlternatives++; // Increase the number of valid alternatives 
				}

				if (validAlternatives > 0)
				{
					accumulatedVelocity /= static_cast<float>(validAlternatives);

					// Calculate velocity blend factor
					float velocityAlignment = DotProduct3D(agent.m_velocity.GetNormalized(), other->m_velocity.GetNormalized());
					float velocityFactor = (1.f - velocityAlignment) * 0.5f; // Factor is higher when agents are moving toward each other

					// Calculate the speed blend factor
					float relativeSpeed = (other->m_velocity - agent.m_velocity).GetLength();
					float maxRelativeSpeed = agent.m_moveSpeed + other->m_moveSpeed;
					float speedFactor = relativeSpeed / maxRelativeSpeed;

					// Calculate the urgency factor (Urgency is higher the more direct the collision is (head on collision))
					Vec3 directionToOther = (other->m_position - agent.m_position).GetNormalized();
					float alignment = DotProduct3D(agent.m_velocity.GetNormalized(), directionToOther);
					float urgencyFactor = (alignment > 0.8f) ? 1.f : (alignment > 0.5f) ? 0.8f : 0.5f;

					// Combine factors to compute the final blend factor (bigger emphasis on the urgency factor)
					float blendFactor = (speedFactor * 0.2f) + (urgencyFactor * 0.7f) + (velocityFactor * 0.1f);

					// First blend towards minimalViolationVelocity to ensure safety, then blend based on the blend factor
					newVelocity = Interpolate(newVelocity, minimalViolationVelocity, 1.f);
					newVelocity = Interpolate(newVelocity, accumulatedVelocity, blendFactor);

					// Check if the distance between the two agents is less than eight-tenths of the search radius (try with .5)
					if (GetDistanceSquared3D(agent.m_velocity, other->m_velocity) < searchRadius * 0.75f)
					{
						// Handle dense area when ORCA fails
						for (Vec3& halfPlaneNormal : halfPlaneNormals)
						{
							// Calculate the distance of current agent velocity to edge of half plane
							Vec3 projectedVelocity = GetProjectedOnto3D(agent.m_velocity, halfPlaneNormal);

							// Calculate a perpendicular direction using the crossproduct 
							Vec3 perpendicularDirection = CrossProduct3D(projectedVelocity, halfPlaneNormal);

							// Expand the half-plane boundary
							// Adjust value for testing but might want to make it dynamic so it keeps expanding until we find a valid velocity
							// #TODO try dynamic
							float expansionFactor = GetClamped(1.f - (nearbyActorDistanceSq / (searchRadius)), 0.05f, 0.5f); 
							Vec3 expandedHalfPlaneNormal = halfPlaneNormal + (expansionFactor * perpendicularDirection);
							expandedHalfPlaneNormal = expandedHalfPlaneNormal.GetNormalized();

							// Update the half-plane constraint with the expanded normal
							halfPlaneNormal = expandedHalfPlaneNormal;

							if (enableDebug)
							{
								DebugAddWorldLine(agent.m_position, agent.m_position + expandedHalfPlaneNormal, 0.025f, 0.f, Rgba8::PURPLE, Rgba8::PURPLE, DebugRenderMode::ALWAYS);
							}

							// Recheck velocity adjustments after expanding half-plane boundaries
							if (!IsInsideORCAConstraint(newVelocity, projectedVelocity, halfPlaneNormal))
							{
								Vec3 alternativeVelocity = FindAlternativeVelocity(agent.m_preferredVelocity, halfPlaneNormal);

								if (m_navMesh && m_navMesh->m_heatMap)
								{
									int currentTri = m_navMesh->GetContainingTriangleIndex(agent.m_position);
									if (currentTri != -1)
									{
										const NavMeshTri& triangle = *m_navMesh->GetNavMeshTriangle(currentTri);
										float heatValueAtIndex = m_heatMap->GetValue(currentTri);
										float maxHeatDiff = 0.f;
										float bestAlignment = -1.f;

										Vec3 bestDirection = Vec3::ZERO;

										for (int neighborIndex : triangle.m_neighborTriIndexes)
										{
											if (neighborIndex == -1) continue;
											float neighborHeat = m_heatMap->GetValue(neighborIndex);

											if (neighborHeat > heatValueAtIndex)
											{
												float heatDiff = neighborHeat - heatValueAtIndex;
												Vec3 dirToCooler = (m_navMesh->GetTriangleCentroid(neighborIndex) - m_navMesh->GetTriangleCentroid(currentTri)).GetNormalized();
												float aligned = DotProduct3D(agent.m_preferredVelocity.GetNormalized(), dirToCooler); // reward direction close to preferred

												// Prioritize cooler triangle that's well-aligned with preferred movement
												if (heatDiff > maxHeatDiff || (heatDiff == maxHeatDiff && aligned > bestAlignment))
												{
													maxHeatDiff = heatDiff;
													bestAlignment = alignment;
													bestDirection = dirToCooler;
												}
											}
										}

										if (maxHeatDiff > 0.f)
										{
											// Higher heat -> lower influence
											float influenceStrength = 0.25f; // #TODO mess with value, test and see what feels right
											Vec3 influencedVelocity = alternativeVelocity + bestDirection * maxHeatDiff * influenceStrength;

											if (enableDebug)
											{
												DebugAddWorldArrow(agent.m_position, agent.m_position + bestDirection * maxHeatDiff, 0.4f, 0.03f, 0.03f, 0.03f, Rgba8::MAGENTA, Rgba8::MAGENTA, DebugRenderMode::ALWAYS);
											}
											alternativeVelocity = Interpolate(alternativeVelocity, influencedVelocity, 0.5f);
										}
									}
								}
								newVelocity = Interpolate(newVelocity, alternativeVelocity, 0.75f);
							}
						}
					}
				}
			}
		}
	}
	
	if (newVelocity.GetLengthSquared() > agent.m_moveSpeed * agent.m_moveSpeed)
	{
		newVelocity = newVelocity.GetNormalized() * agent.m_moveSpeed;
	}
	agent.m_velocity = newVelocity;

	if (enableDebug)
	{
		// Newly adjusted Velocity
		DebugAddWorldLine(agent.m_position, agent.m_position + agent.m_velocity, 0.05f, 0.f, Rgba8::BROWN, Rgba8::BROWN, DebugRenderMode::ALWAYS);
	}
}

bool ObstacleAvoidnace::IsInsideORCAConstraint(const Vec3& velocity, const Vec3& constraintPoint, const Vec3& normal)
{
	// Calculate the vector from the constraint point to the given velocity
	Vec3 toVelocity = velocity - constraintPoint;

	// Return true if the velocity is on the safe side of the constraint (i.e., inside or on the half-plane)
	//
	// If the dot product is positive, the velocity lies on the "forbidden" side of the half-plane,
	// meaning the agent is violating the constraint and could potentially collide.
	//
	// If the dot product is zero or negative, the velocity is either on the boundary or inside the allowed region,
	// meaning the agent is in a safe zone and not violating any active constraints.
	return DotProduct3D(toVelocity, normal) <= 0.f; 
}

Vec3 ObstacleAvoidnace::FindAlternativeVelocity(const Vec3& preferredVelocity, const Vec3& halfPlaneNormal)
{
	// Calculate the length of the preferred velocity (the desired speed)
	float agentDesiredSpeed = preferredVelocity.GetLength();

	// Project the preferred velocity onto the half-plane boundary normal
	Vec3 projectedVelocity = GetProjectedOnto3D(preferredVelocity, halfPlaneNormal);

	// Calculate the alternative velocity by adjusting the preferred velocity
	Vec3 alternativeVelocity = preferredVelocity - projectedVelocity;

	// If the alternative velocity is zero, perturb in an orthogonal direction
	if (alternativeVelocity.GetLengthSquared() < 0.001f)
	{
		Vec3 orthogonalDirection = CrossProduct3D(halfPlaneNormal, Vec3(0.f, 0.f, 1.f)).GetNormalized();
		alternativeVelocity = orthogonalDirection * agentDesiredSpeed * 0.1f; // Small perturbation
	}

	// Normalize the alternative velocity and set it to the desired speed
	alternativeVelocity = alternativeVelocity.GetNormalized() * agentDesiredSpeed;

	alternativeVelocity.z = 0.f;

	return alternativeVelocity;
}
