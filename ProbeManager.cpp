#include "ProbeManager.h"
#include "ResourceManager.h"
#include "TerrainManager.h"
#include "GridManager.h"
#include "UnitManager.h"
#include "StrategyManager.h"
#include "UnitUtil.h"
#include "CommandManager.h"

using namespace BWAPI;
using namespace std;

void ProbeTrackerClass::update()
{
	scoutProbe();
	enforceAssignments();
}

void ProbeTrackerClass::storeProbe(Unit unit)
{
	if (unit->exists() && unit->isCompleted())
	{
		myProbes[unit].setMiniTile(UnitUtil::Instance().getMiniTile(unit));
	}
	return;
}

void ProbeTrackerClass::removeProbe(Unit probe)
{
	if (ResourceTracker::Instance().getMyGas().find(myProbes[probe].getTarget()) != ResourceTracker::Instance().getMyGas().end())
	{
		ResourceTracker::Instance().getMyGas()[myProbes[probe].getTarget()].setGathererCount(ResourceTracker::Instance().getMyGas()[myProbes[probe].getTarget()].getGathererCount() - 1);
	}
	if (ResourceTracker::Instance().getMyMinerals().find(myProbes[probe].getTarget()) != ResourceTracker::Instance().getMyMinerals().end())
	{
		ResourceTracker::Instance().getMyMinerals()[myProbes[probe].getTarget()].setGathererCount(ResourceTracker::Instance().getMyMinerals()[myProbes[probe].getTarget()].getGathererCount() - 1);
	}
	myProbes.erase(probe);

	// If scouting probe died, assume where enemy is based on death location
	if (probe == scout && isScouting())
	{
		scouting = false;
		if (TerrainTracker::Instance().getEnemyBasePositions().size() == 0)
		{
			double closestD = 0.0;
			BaseLocation* closestB = getNearestBaseLocation(probe->getTilePosition());
			for (auto base : getStartLocations())
			{
				if (probe->getDistance(base->getPosition()) < closestD || closestD == 0.0)
				{
					closestB = base;
				}
			}
			TerrainTracker::Instance().getEnemyBasePositions().emplace(closestB->getPosition());
		}
	}
}

void ProbeTrackerClass::assignProbe(Unit probe)
{
	// Assign a task if none
	int cnt = 1;

	for (auto &gas : ResourceTracker::Instance().getMyGas())
	{
		if (gas.second.getType() == UnitTypes::Protoss_Assimilator && gas.first->isCompleted() && gas.second.getGathererCount() < 3)
		{
			gas.second.setGathererCount(gas.second.getGathererCount() + 1);
			myProbes[probe].setTarget(gas.first);
			return;
		}
	}

	// First checks if a mineral field has 0 Probes mining, if none, checks if a mineral field has 1 Probe mining. Assigns to 0 first, then 1. Spreads saturation.
	while (cnt <= 2)
	{
		for (auto &mineral : ResourceTracker::Instance().getMyMinerals())
		{
			if (mineral.second.getGathererCount() < cnt)
			{
				mineral.second.setGathererCount(mineral.second.getGathererCount() + 1);
				myProbes[probe].setTarget(mineral.first);
				return;
			}
		}
		cnt++;
	}
	return;
}

void ProbeTrackerClass::reAssignProbe(Unit probe)
{
	if (ResourceTracker::Instance().getMyGas().find(myProbes[probe].getTarget()) != ResourceTracker::Instance().getMyGas().end())
	{
		ResourceTracker::Instance().getMyGas()[myProbes[probe].getTarget()].setGathererCount(ResourceTracker::Instance().getMyGas()[myProbes[probe].getTarget()].getGathererCount() - 1);
	}
	if (ResourceTracker::Instance().getMyMinerals().find(myProbes[probe].getTarget()) != ResourceTracker::Instance().getMyMinerals().end())
	{
		ResourceTracker::Instance().getMyMinerals()[myProbes[probe].getTarget()].setGathererCount(ResourceTracker::Instance().getMyMinerals()[myProbes[probe].getTarget()].getGathererCount() - 1);
	}

	assignProbe(probe);
}

void ProbeTrackerClass::scoutProbe()
{
	for (auto &u : myProbes)
	{
		// Crappy scouting method
		if (!scout)
		{
			scout = u.first;
		}
		if (u.first == scout)
		{
			if (TerrainTracker::Instance().getEnemyBasePositions().size() == 0 && UnitTracker::Instance().getSupply() >= 18 && scouting)
			{
				for (auto start : getStartLocations())
				{
					if (Broodwar->isExplored(start->getTilePosition()) == false)
					{
						if (u.first->getOrderTargetPosition() != start->getPosition())
						{
							u.first->move(start->getPosition());
						}
						break;
					}
				}
			}
			if (TerrainTracker::Instance().getEnemyBasePositions().size() > 0)
			{
				WalkPosition start = u.second.getMiniTile();
				if (GridTracker::Instance().getEGroundGrid(start.x, start.y) > 0.0 || GridTracker::Instance().getEDistanceGrid(start.x, start.y) > 0.0)
				{
					CommandTracker::Instance().fleeTarget(u.first, u.first->getClosestUnit(Filter::IsEnemy));
				}
				else
				{
					exploreArea(u.first);
				}
			}
		}
	}
}

void ProbeTrackerClass::exploreArea(Unit probe)
{
	WalkPosition start = myProbes[probe].getMiniTile();
	vector<WalkPosition> possibilites;
	double closestD = 1000;
	recentExplorations[start] = Broodwar->getFrameCount();

	for (int x = start.x - 20; x < start.x + 20 + probe->getType().tileWidth(); x++)
	{
		for (int y = start.y - 20; y < start.y + 20 + probe->getType().tileHeight(); y++)
		{
			if (GridTracker::Instance().getAntiMobilityGrid(x, y) == 0 && GridTracker::Instance().getMobilityGrid(x, y) > 0 && GridTracker::Instance().getEGroundGrid(x, y) == 0.0 && GridTracker::Instance().getEDistanceGrid(x, y) == 0.0 && WalkPosition(x, y).isValid() && Broodwar->getFrameCount() - recentExplorations[WalkPosition(x, y)] > 10 && Position(WalkPosition(x, y)).getDistance(TerrainTracker::Instance().getEnemyStartingPosition()) < 320)
			{
				possibilites.push_back(WalkPosition(x, y));
			}
		}
	}

	if (possibilites.size() > 0)
	{
		int i = rand() % possibilites.size();
		probe->move(Position(possibilites.at(i)));
	}
	else
	{
		probe->move(TerrainTracker::Instance().getEnemyStartingPosition());
	}
}

void ProbeTrackerClass::enforceAssignments()
{
	// For each Probe mapped to gather minerals
	for (auto &u : myProbes)
	{
		if (ResourceTracker::Instance().getGasNeeded() > 0)
		{
			reAssignProbe(u.first);
			ResourceTracker::Instance().setGasNeeded(ResourceTracker::Instance().getGasNeeded() - 1);
		}
		// If no valid target, try to get a new one
		if (!u.second.getTarget())
		{
			assignProbe(u.first);
			// Any idle Probes can gather closest mineral field until they are assigned again
			if (u.first->isIdle() && u.first->getClosestUnit(Filter::IsMineralField))
			{
				u.first->gather(u.first->getClosestUnit(Filter::IsMineralField));
				continue;
			}
			continue;
		}

		// Attack units in mineral line
		if (GridTracker::Instance().getResourceGrid(u.first->getTilePosition().x, u.first->getTilePosition().y) > 0 && u.first->getUnitsInRadius(64, Filter::IsEnemy).size() > 0 && (u.first->getHitPoints() + u.first->getShields()) > 20)
		{
			u.first->attack(u.first->getClosestUnit(Filter::IsEnemy, 320));
		}
		else if (u.first->getLastCommand().getType() == UnitCommandTypes::Attack_Unit && (GridTracker::Instance().getResourceGrid(u.first->getTilePosition().x, u.first->getTilePosition().y) == 0 || (u.first->getHitPoints() + u.first->getShields()) <= 20))
		{
			u.first->stop();
		}

		// Else command probe
		if (u.first && u.first->exists())
		{
			// Draw on every frame
			if (u.first && u.second.getTarget())
			{
				Broodwar->drawLineMap(u.first->getPosition(), u.second.getTarget()->getPosition(), Broodwar->self()->getColor());
			}

			// Return if not latency frame
			if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
			{
				continue;
			}

			// If idle and carrying gas or minerals, return cargo			
			if (u.first->isIdle() && (u.first->isCarryingGas() || u.first->isCarryingMinerals()))
			{
				if (u.first->getLastCommand().getType() == UnitCommandTypes::Return_Cargo)
				{
					continue;
				}
				u.first->returnCargo();
				continue;
			}

			// If not scouting and there's boulders to remove	
			if (!scouting && ResourceTracker::Instance().getMyBoulders().size() > 0 && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus) >= 2)
			{
				for (auto b : ResourceTracker::Instance().getMyBoulders())
				{
					if (b.first && b.first->exists() && !u.first->isGatheringMinerals() && u.first->getDistance(b.second.getPosition()) < 512)
					{
						u.first->gather(b.first);
						break;
					}
				}
			}

			// If we have been given a command this frame already, continue
			if (!u.first->isIdle() && (u.first->getLastCommand().getType() == UnitCommandTypes::Move || u.first->getLastCommand().getType() == UnitCommandTypes::Build))
			{
				continue;
			}


			// If idle and not targeting the mineral field the Probe is mapped to
			if (u.first->isIdle() || (u.first->isGatheringMinerals() && !u.first->isCarryingMinerals() && u.first->getTarget() != u.second.getTarget()))
			{
				// If the Probe has a target
				if (u.first->getTarget())
				{
					// If the target has a resource count of 0 (mineral blocking a ramp), let Probe continue mining it
					if (u.first->getTarget()->getResources() == 0)
					{
						continue;
					}
				}
				// If the mineral field is in vision and no target, force to gather from the assigned mineral field
				if (u.second.getTarget() && u.second.getTarget()->exists())
				{
					u.first->gather(u.second.getTarget());
					continue;
				}
			}
		}
	}
}