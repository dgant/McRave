#include "McRave.h"

// McRaveT Stuff: need to wall in all 3 
// TvP
// 2 Fact triple tank cheese
// 9 depot, 10 rax, 12 refinery, 15 depot, 18 fact, 20 fact, 23 depot, both addons asap, research vult speed/mines, build vultures after the tanks and push

// TvT
// 1 Fact FE into 2 base BC / yamato

// TvZ
// 2 port wraith

// McRaveZ Stuff:
// ZvP / ZvT
// 2 Hatch lurker
// 9 pool, 9 gas, 6 lings, lair, den, aspect, 4 hydras

// ZvZ
// 1H spire
// 9 pool, 9 gas, 6 lings, lair, spire

void BuildOrderTrackerClass::update()
{
	Display().startClock();
	updateDecision();
	updateBuild();
	Display().performanceTest(__FUNCTION__);
	return;
}

void BuildOrderTrackerClass::updateDecision()
{
	if (Broodwar->self()->getRace() == Races::Protoss)
	{
		// If we have a Core and 2 Gates, opener is done
		if (buildingDesired[UnitTypes::Protoss_Cybernetics_Core] >= 1 && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Gateway) >= 2 && getOpening)
		{
			// Put opener function here instead
			getOpening = false;
		}

		// If we have our tech unit, set to none
		if (Broodwar->self()->completedUnitCount(techUnit) > 0)
		{
			techUnit = UnitTypes::None;
		}

		// If production is saturated and none are idle or we need detection for some invis units, choose a tech
		if (Strategy().needDetection() || (!getOpening && !getTech && techUnit == UnitTypes::None && Production().isGateSat() && Production().getIdleHighProduction().size() == 0 && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Gateway) >= 2))
		{
			// Put tech function here instead
			getTech = true;
		}

		// If we are choosing an opening
		if (getOpening && opening != 1)
		{
			// PvZ - FFE always
			if (Players().getNumberZerg() > 0 && opening == 0)
			{
				opening = 1;
				return;
			}

			// PvP - 1 Gate Core
			else if (Players().getNumberProtoss() > 0)
			{				
				if (Strategy().isRush())
				{
					opening = 4;
				}
				else
				{
					opening = 2;
				}
			}
			// PvT - 1 Gate Nexus
			else if (Players().getNumberTerran() > 0)
			{
			/*	if (Strategy().isEnemyFastExpand())
				{
					opening = 2;
				}
				else
				{
					opening = 3;
				}*/
				opening = 3;
			}
			// PvR - 2 Gate Core
			else
			{
				opening = 4;
			}

			
		}		
	}
	else if (Broodwar->self()->getRace() == Races::Terran)
	{
		// If we have an Academy, opener is done
		if (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) >= 1)
		{
			getOpening = false;
		}

		if (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Command_Center) >= 2)
		{
			getTech = true;
		}

		// Only one opener for now
		opening = 0;
	}
	return;
}

void BuildOrderTrackerClass::updateBuild()
{
	// Protoss
	if (Broodwar->self()->getRace() == Races::Protoss)
	{
		protossSituational();
		protossTech();
		protossOpener();
	}

	// Terran
	else if (Broodwar->self()->getRace() == Races::Terran)
	{
		terranSituational();
		terranTech();
		terranOpener();
	}

	// Zerg
	else if (Broodwar->self()->getRace() == Races::Zerg)
	{
		zergSituational();
		zergTech();
		zergOpener();
	}
}

void BuildOrderTrackerClass::protossOpener()
{
	if (getOpening)
	{
		// FFE
		if (opening == 1)
		{
			buildingDesired[UnitTypes::Protoss_Forge] = Units().getSupply() >= 18;
			buildingDesired[UnitTypes::Protoss_Nexus] = 1 + (Units().getSupply() >= 28);
			buildingDesired[UnitTypes::Protoss_Photon_Cannon] = (Units().getSupply() >= 22) + (Units().getSupply() >= 24) + (Units().getSupply() >= 30);
			buildingDesired[UnitTypes::Protoss_Gateway] = (Units().getSupply() >= 32) + (Units().getSupply() >= 46);
			buildingDesired[UnitTypes::Protoss_Assimilator] = Units().getSupply() >= 36;
			buildingDesired[UnitTypes::Protoss_Cybernetics_Core] = Units().getSupply() >= 42;
		}
		// 1 Gate Core
		else if (opening == 2)
		{
			buildingDesired[UnitTypes::Protoss_Nexus] = 1;
			buildingDesired[UnitTypes::Protoss_Gateway] = (Units().getSupply() >= 20) + (Units().getSupply() >= 36);
			buildingDesired[UnitTypes::Protoss_Assimilator] = Units().getSupply() >= 24;
			buildingDesired[UnitTypes::Protoss_Cybernetics_Core] = Units().getSupply() >= 28;
		}
		// 14 Nexus
		else if (opening == 3)
		{
			buildingDesired[UnitTypes::Protoss_Nexus] = 1 + (Units().getSupply() >= 28) + Strategy().isEnemyFastExpand();
			buildingDesired[UnitTypes::Protoss_Gateway] = (Units().getSupply() >= 20) + (Units().getSupply() >= 32);
			buildingDesired[UnitTypes::Protoss_Assimilator] = Units().getSupply() >= 30;
			buildingDesired[UnitTypes::Protoss_Cybernetics_Core] = Units().getSupply() >= 30;
		}
		// 2 Gate Core
		else if (opening == 4)
		{			
			buildingDesired[UnitTypes::Protoss_Nexus] = 1;
			buildingDesired[UnitTypes::Protoss_Gateway] = (Units().getSupply() >= 20) + (Units().getSupply() >= 24);
			buildingDesired[UnitTypes::Protoss_Assimilator] = Units().getSupply() >= 48;
			buildingDesired[UnitTypes::Protoss_Cybernetics_Core] = Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Zealot) >= 4;
		}
		// 9/9 Gates
		else if (opening == 5)
		{
			buildingDesired[UnitTypes::Protoss_Nexus] = 1;
			buildingDesired[UnitTypes::Protoss_Gateway] = (Units().getSupply() >= 18) * 2;
			buildingDesired[UnitTypes::Protoss_Assimilator] = Units().getSupply() >= 48;
			buildingDesired[UnitTypes::Protoss_Cybernetics_Core] = Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Zealot) >= 6;
		}
	}
	return;
}

void BuildOrderTrackerClass::protossTech()
{
	if (getTech && techUnit == UnitTypes::None)
	{
		if (Strategy().needDetection())
		{
			techUnit = UnitTypes::Protoss_Observer;
		}
		else
		{
			double highest = 0.0;
			for (auto &tech : Strategy().getUnitScore())
			{
				if (tech.first == UnitTypes::Protoss_Dragoon || tech.first == UnitTypes::Protoss_Zealot || techList.find(tech.first) != techList.end())
				{
					continue;
				}
				if (tech.second > highest)
				{
					highest = tech.second;
					techUnit = tech.first;
				}
			}
		}

		// No longer need to choose a tech
		getTech = false;
		techList.insert(techUnit);
	}
	if (techUnit == UnitTypes::Protoss_Reaver)
	{
		buildingDesired[UnitTypes::Protoss_Robotics_Facility] = 1;
		buildingDesired[UnitTypes::Protoss_Robotics_Support_Bay] = min(1, Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Robotics_Facility));
		buildingDesired[UnitTypes::Protoss_Observatory] = min(1, buildingDesired[UnitTypes::Protoss_Observatory] + Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Reaver));
	}
	else if (techUnit == UnitTypes::Protoss_Observer)
	{
		buildingDesired[UnitTypes::Protoss_Robotics_Facility] = 1;
		buildingDesired[UnitTypes::Protoss_Observatory] = min(1, Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Robotics_Facility));
		buildingDesired[UnitTypes::Protoss_Robotics_Support_Bay] = min(1, Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Observer));
	}
	else if (techUnit == UnitTypes::Protoss_Corsair)
	{
		buildingDesired[UnitTypes::Protoss_Stargate] = 1;
		buildingDesired[UnitTypes::Protoss_Citadel_of_Adun] = min(1, Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Stargate));
		buildingDesired[UnitTypes::Protoss_Templar_Archives] = min(1, Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Citadel_of_Adun));
	}
	else if (techUnit == UnitTypes::Protoss_Scout)
	{
		buildingDesired[UnitTypes::Protoss_Stargate] = min(2, 1 + Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Stargate));
		buildingDesired[UnitTypes::Protoss_Fleet_Beacon] = min(1, Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Stargate));
	}
	else if (techUnit == UnitTypes::Protoss_Arbiter)
	{
		buildingDesired[UnitTypes::Protoss_Citadel_of_Adun] = 1;
		buildingDesired[UnitTypes::Protoss_Stargate] = Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Citadel_of_Adun);
		buildingDesired[UnitTypes::Protoss_Templar_Archives] = Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Citadel_of_Adun);
		buildingDesired[UnitTypes::Protoss_Arbiter_Tribunal] = Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives);
	}
	else if (techUnit == UnitTypes::Protoss_High_Templar)
	{
		buildingDesired[UnitTypes::Protoss_Citadel_of_Adun] = 1;
		buildingDesired[UnitTypes::Protoss_Templar_Archives] = min(1, Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Citadel_of_Adun));
	}
	return;
}

void BuildOrderTrackerClass::protossSituational()
{
	// Pylon logic
	if (Strategy().isAllyFastExpand() && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Pylon) <= 0)
	{
		buildingDesired[UnitTypes::Protoss_Pylon] = Units().getSupply() >= 14;
	}
	else
	{
		buildingDesired[UnitTypes::Protoss_Pylon] = min(22, (int)floor((Units().getSupply() / max(14, (16 - Broodwar->self()->allUnitCount(UnitTypes::Protoss_Pylon))))));
	}

	// Expansion logic
	if (!getOpening && !Strategy().isRush() && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus) == Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Nexus) && ((Broodwar->self()->minerals() > 300 && Resources().isMinSaturated() && Production().isGateSat() && Production().getIdleLowProduction().size() == 0) || (Strategy().isAllyFastExpand() && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Nexus) == 1)))
	{
		buildingDesired[UnitTypes::Protoss_Nexus] = Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Nexus) + 1;
	}

	// Shield battery logic
	if (Strategy().isRush())
	{
		buildingDesired[UnitTypes::Protoss_Shield_Battery] = min(1, Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Cybernetics_Core));
	}

	// Gateway logic
	if (Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Gateway) >= 2 && (Production().getIdleLowProduction().size() == 0 && ((Broodwar->self()->minerals() - Production().getReservedMineral() - Buildings().getQueuedMineral() > 150) || (!Production().isGateSat() && Resources().isMinSaturated()))))
	{		
		buildingDesired[UnitTypes::Protoss_Gateway] = min(Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus) * 3, Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Gateway) + 1);
	}

	// Assimilator logic
	if (Resources().isMinSaturated() && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus) == buildingDesired[UnitTypes::Protoss_Nexus])
	{
		buildingDesired[UnitTypes::Protoss_Assimilator] = Resources().getTempGasCount();
	}

	// Forge logic
	if (Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus) >= 3 && Units().getSupply() > 120)
	{
		buildingDesired[UnitTypes::Protoss_Forge] = 1;
	}

	// Cannon logic
	if (!Strategy().isAllyFastExpand() && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Forge))
	{
		buildingDesired[UnitTypes::Protoss_Photon_Cannon] = Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Photon_Cannon);
		for (auto &base : Bases().getMyBases())
		{
			if (base.second.unit()->isCompleted() && Grids().getDefenseGrid(base.second.getTilePosition()) < 2 && Broodwar->hasPower(TilePosition(base.second.getPosition())))
			{
				buildingDesired[UnitTypes::Protoss_Photon_Cannon] += 2 - Grids().getDefenseGrid(base.second.getTilePosition());
			}
		}
	}

	// Additional cannon for FFE logic (add on at most 2 at a time)
	if (Strategy().isAllyFastExpand() && Units().getGlobalEnemyStrength() > Units().getGlobalAllyStrength() + Units().getAllyDefense())
	{
		buildingDesired[UnitTypes::Protoss_Photon_Cannon] = min(2 + Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Photon_Cannon), 1 + Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Photon_Cannon));
	}
}

void BuildOrderTrackerClass::terranOpener()
{
	if (getOpening)
	{
		// Two Rax Academy
		if (opening == 1)
		{
			buildingDesired[UnitTypes::Terran_Barracks] = (Units().getSupply() >= 18) + (Units().getSupply() >= 20) + (Units().getSupply() >= 42);
			buildingDesired[UnitTypes::Terran_Engineering_Bay] = (Units().getSupply() >= 36);
			buildingDesired[UnitTypes::Terran_Refinery] = (Units().getSupply() >= 36);
			buildingDesired[UnitTypes::Terran_Academy] = (Units().getSupply() >= 48);
		}

		// 1 Rax Fact
		if (opening == 2)
		{

		}
	}
	return;
}

void BuildOrderTrackerClass::terranTech()
{

}

void BuildOrderTrackerClass::terranSituational()
{
	// Supply Depot logic
	buildingDesired[UnitTypes::Terran_Supply_Depot] = min(22, (int)floor((Units().getSupply() / max(14, (16 - Broodwar->self()->allUnitCount(UnitTypes::Terran_Supply_Depot))))));

	// Bunker logic
	if (Strategy().isRush())
	{
		buildingDesired[UnitTypes::Terran_Bunker] = 1;
	}

	// Refinery logic
	if (Resources().isMinSaturated())
	{
		buildingDesired[UnitTypes::Terran_Refinery] = Resources().getMyGas().size();
	}

	// Barracks logic
	if (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Barracks) >= 3 && (Production().getIdleLowProduction().size() == 0 && ((Broodwar->self()->minerals() - Production().getReservedMineral() - Buildings().getQueuedMineral() > 200) || (!Production().isBarracksSat() && Resources().isMinSaturated()))))
	{
		buildingDesired[UnitTypes::Terran_Barracks] = min(Broodwar->self()->completedUnitCount(UnitTypes::Terran_Command_Center) * 3, Broodwar->self()->visibleUnitCount(UnitTypes::Terran_Barracks) + 1);
	}

	// CC logic
	if (Broodwar->self()->hasResearched(TechTypes::Stim_Packs))
	{
		buildingDesired[UnitTypes::Terran_Command_Center] = 2;
	}
}

void BuildOrderTrackerClass::zergOpener()
{

}

void BuildOrderTrackerClass::zergTech()
{

}

void BuildOrderTrackerClass::zergSituational()
{

}