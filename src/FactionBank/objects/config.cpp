class CfgPatches
{
	class YourModName
	{
		units[] = {"ExpansionFactionLocker"};
		requiredAddons[] = {"DZ_Structures_Furniture"};
	};
};

class CfgVehicles
{
	class ExpansionATMBase;  // base class from Expansion

	class ExpansionFactionLocker: ExpansionATMBase
	{
		scope = 2;  // Needed so it's spawnable
		displayName = "Faction Bank";
		model = "\DZ\structures\furniture\cases\locker\locker_open_blue_v1.p3d";
		rotationFlags = 12;
	};
};