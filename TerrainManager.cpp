#include "TerrainManager.h"
#include "src\bwem.h"

bool analyzeHome = true;
bool analysis_just_finished = true;
int currentSize = 0;

void TerrainTrackerClass::update()
{
	// Only do this loop once if map analysis done
	if (analyzed)
	{
		//drawTerrainData();
		if (analyzeHome)
		{
			buildChokeNodes();
			analyzeHome = false;

			// Find player starting position and tile position		
			BaseLocation* playerStartingLocation = getStartLocation(Broodwar->self());
			playerStartingPosition = playerStartingLocation->getPosition();
			playerStartingTilePosition = playerStartingLocation->getTilePosition();
			nextExpansion.push_back(playerStartingTilePosition);
			activeExpansion.push_back(playerStartingTilePosition);
		}
		if (analysis_just_finished)
		{
			map <int, TilePosition> testBases;
			if (enemyBasePositions.size() > 0)
			{
				analysis_just_finished = false;
				for (auto base : getBaseLocations())
				{
					int distances = getGroundDistance2(base->getTilePosition(), playerStartingTilePosition) - getGroundDistance2(base->getTilePosition(), enemyStartingTilePosition) - base->getTilePosition().getDistance(TilePosition(Broodwar->mapWidth(), Broodwar->mapHeight()));
					if (base->isMineralOnly() && base->getPosition().getDistance(playerStartingPosition) < 2560)
					{
						distances += 1280;
					}
					if (base->getTilePosition() != playerStartingTilePosition && !base->isIsland() && base->getTilePosition() != enemyStartingTilePosition)
					{
						testBases.emplace(distances, base->getTilePosition());
					}
				}

				for (auto base : testBases)
				{
					nextExpansion.push_back(base.second);
					activeExpansion.push_back(base.second);
				}
			}
		}
		for (auto pos : enemyBasePositions)
		{
			if (Broodwar->isVisible(TilePosition(pos)) && Broodwar->getUnitsInRadius(pos, 128, Filter::IsEnemy).size() == 0)
			{
				enemyBasePositions.erase(find(enemyBasePositions.begin(), enemyBasePositions.end(), pos));
				break;
			}
		}
	}
	if (!analyzeHome)
	{
		if (territory.size() < 1)
		{
			allyTerritory.emplace(getRegion(playerStartingPosition));
			territory = BWTA::getRegions();
		}
		// In each ally territory there are chokepoints to defend, find and store those chokepoints
		if (allyTerritory.size() != currentSize && allyTerritory.size() > 0)
		{
			// For all regions
			for (auto *region : territory)
			{
				bool merge = false;
				// For each chokepoint
				for (auto Chokepoint : region->getChokepoints())
				{
					if (region == getRegion(playerStartingTilePosition))
					{
						// If chokepoint is really wide, such as Andromeda, this will add the Territory of the natural
						if (Chokepoint->getWidth() > 200)
						{
							allyTerritory.emplace(Chokepoint->getRegions().first);
							allyTerritory.emplace(Chokepoint->getRegions().second);
						}
					}
					// Check if every chokepoint is connected to an ally territory, in which case add this to our territory (connecting regions)
					if (allyTerritory.find(Chokepoint->getRegions().first) != allyTerritory.end() || allyTerritory.find(Chokepoint->getRegions().second) != allyTerritory.end())
					{
						merge = true;
						if (region->getChokepoints().size() == 1)
						{
							break;
						}
					}
					else
					{
						merge = false;
						break;
					}
				}
				if (merge == true && allyTerritory.find(region) == allyTerritory.end())
				{
					allyTerritory.emplace(region);
				}
			}
			defendHere.erase(defendHere.begin(), defendHere.end());
			currentSize = allyTerritory.size();
			// For each region that is ally territory
			for (auto *region : allyTerritory)
			{
				// For each chokepoint of each ally region				
				for (auto Chokepoint : region->getChokepoints())
				{
					// Check if both territories are ally
					if (allyTerritory.find(Chokepoint->getRegions().first) != allyTerritory.end() && allyTerritory.find(Chokepoint->getRegions().second) != allyTerritory.end())
					{
						// If both are ally, do nothing (we don't need to defend two ally regions)							
					}
					else
					{
						defendHere.push_back(Chokepoint->getCenter());
					}
				}
			}
		}
		// Draw territory boundaries
		for (auto position : defendHere)
		{
			Broodwar->drawCircleMap(position, 80, Broodwar->self()->getColor());
		}
	}
	//for (auto p : defendHere)
	//{
	//	// Remove blocking mineral patches (maps like Destination)
	//	if (boulders.size() > 0 && Broodwar->getUnitsInRadius(p, 64, Filter::IsMineralField && Filter::Resources == 0).size() > 0)
	//	{
	//		defendHere.erase(find(defendHere.begin(), defendHere.end(), p));
	//		break;
	//	}
	//}
	for (auto u : Broodwar->enemy()->getUnits())
	{
		if (u->getPlayer() == Broodwar->enemy())
		{
			if (u->getType().isBuilding())
			{
				/*if (scouting && Broodwar->enemy()->getRace() == Races::Terran && u->getDistance(getNearestChokepoint(unit->getPosition())->getCenter()) < 256)
				{
					wallIn = true;
				}*/
				if (enemyBasePositions.size() == 0)
				{
					// Find closest base location to building
					double distance = 5000;
					for (auto base : getStartLocations())
					{
						if (u->getDistance(base->getPosition()) < distance)
						{
							enemyStartingTilePosition = base->getTilePosition();
							enemyStartingPosition = base->getPosition();
							distance = u->getDistance(base->getPosition());
						}
					}

					enemyBasePositions.push_back(enemyStartingPosition);
					path = theMap.GetPath(playerStartingPosition, enemyStartingPosition);
				}
			}
		}
	}
}

void TerrainTrackerClass::setAnalyzed()
{
	analyzed = true;
}