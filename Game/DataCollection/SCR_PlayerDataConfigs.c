//------------------------------------------------------------------------------------------------
class SCR_PlayerDataConfigs : Managed
{		
	
	protected static ref SCR_PlayerDataConfigs instance;
	
	/******/
	//RANK//
	/******/
	
	float m_fSpecializationPointsModifier = 0.2;
	
	/*****************/
	//SPECIALIZATIONS//
	/*****************/
	
	int m_iSpecialization0DisplayCount;
	int m_iSpecialization1DisplayCount;
	int m_iSpecialization2DisplayCount;
	
	//Config of Ranges
	int m_iRanges = 5;
	ref array<int> m_aRangesOrigin = {0, 160000, 600000, 750000, 900000};
	ref array<int> m_aRangesEnd = {160000, 600000, 750000, 900000, 1000000};
	ref array<float> m_aRangesC1 = {1.9449, 0.71, 8.1682, 6, 18};
	ref array<float> m_aRangesC2 = {0.9254, 0.9908, 0.9517, 0.9559, 0.9425};
	
	int m_iNumberOfSpecializations;
	
	//0 - Infantry
	const float m_fSpInfantryModifierPointsMetersWalked = 1;
	const float m_fKillModifierPoints = 250;
	const float m_fAIKillModifierPoints = 125;
	const float m_fPrecisionModifierPoints = 300;
	
	//1 - Logistics
	const float m_fMetersDrivenModifierPoints = 1;
	const float m_fDriverOfPlayersModifierPoints = 0.7;
	const float m_fTraveledTimeSupplyVehicleModifierPoints = 1;
	const float m_fTraveledDistanceSupplyVehicleModifierPoints = 5;
	
	const int m_iPlayersDiedInVehicleModifierPoints = 500;
	
	/************/
	//WAR CRIMES//
	/************/
	
	const float m_fMaxWarCrimes = 0.25;
	const float m_fWarCrimesDecreaseRatePerHour = 0.025;
	
	const float m_fFriendlyKillsModifierPoints = 0.75;
	const float m_fFriendlyAIKillsModifierPoints = 0.5;
	
	const float m_fSTDPointsQualityTime = 10000;
	
	static SCR_PlayerDataConfigs GetInstance()
	{
		if (!instance)
			instance = new SCR_PlayerDataConfigs();
		
		return instance;
	}
	
	void SCR_PlayerDataConfigs()
	{
		typename tEnum = ECareerSp;
		m_iNumberOfSpecializations = tEnum.GetVariableCount();
		
		tEnum = ESpecialization0Display;		
		m_iSpecialization0DisplayCount = tEnum.GetVariableCount();
		
		tEnum = ESpecialization1Display;
		m_iSpecialization1DisplayCount = tEnum.GetVariableCount();
		
		tEnum = ESpecialization2Display;
		m_iSpecialization2DisplayCount = tEnum.GetVariableCount();
	}
};

enum ECareerSp
{
	Infantry,
	Logistics,
	Medical
};

enum ESpecialization0Display
{
	MetersWalked,
	PlayerKills,
	AIKills,
	BulletsShot
};

enum ESpecialization1Display
{
	DistanceDrivingSupplyVehicle,
	TimeDrivingSupplyVehicle,
	PointsDrivingPeople
};

enum ESpecialization2Display
{
	BandagedSelf,
	BandagedAllies
};