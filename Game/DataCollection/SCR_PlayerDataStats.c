//------------------------------------------------------------------------------------------------
class SCR_PlayerDataStats : Managed
{
	/******/
	//RANK//
	/******/
	
	int m_iRank;
	int m_iRankExperience;
	
	/*****************/
	//SPECIALIZATIONS//
	/*****************/
	
	//To convert these into % divide them by 10,000. Ie. 90.35% is 903,500
	//Alternatively, to put them into the [0,1] range divide them by 1,000,000
	//Max value for each position = 1,000,000
	ref array<int> m_aSpPoints = {};
	
	/************/
	//WAR CRIMES//
	/************/
	
	float m_fWarCrimes;
	
	/**********************/
	//DATACOLLECTORMODULES//
	/**********************/
	
	//BasicActionsModule
	
	float m_fMetersWalked;
	float m_fSessionDuration;
			
	//ShootingModule
	int m_iKills;
	int m_iAIKills;
	int m_iShots;
	int m_iGrenadesThrown;
	int m_iFriendlyKills;
	int m_iFriendlyAIKills;
	int m_iDeaths;
	
	//DriverModule
	float m_fMetersDriven;
	int m_iPointsAsDriverOfPlayers;
	int m_iPlayersDiedInVehicle;
	
	int m_iRoadKills;
	int m_iFriendlyRoadKills;
	int m_iAIRoadKills;
	int m_iFriendlyAIRoadKills;
	
	float m_fMetersAsOccupant;
	
	//SupplyTruckDriverModule
	float m_fTraveledDistanceSupplyVehicle;
	int m_iTraveledTimeSupplyVehicle;
	
	void fillWithZeroes()
	{
		//RANK
		////////////////////////
		m_iRank = 0;
		m_iRankExperience = 0;
		////////////////////////
		
		//SPECIALIZATIONS
		////////////////////////
		m_aSpPoints.Insert(0);
		m_aSpPoints.Insert(0);
		m_aSpPoints.Insert(0);
		////////////////////////
		
		//WAR CRIMES
		////////////////////////
		m_fWarCrimes = 0;
		////////////////////////
		
		//DATACOLLECTORMODULES
		////////////////////////
		//BasicActionsModule
		m_fMetersWalked = 0;
		m_fSessionDuration = 0;
			
		//ShootingModule
		m_iKills = 0;
		m_iAIKills = 0;
		m_iShots = 0;
		m_iGrenadesThrown = 0;
		m_iFriendlyKills = 0;
		m_iFriendlyAIKills = 0;
		m_iDeaths = 0;
		
		//DriverModule
		m_fMetersDriven = 0;
		m_iPointsAsDriverOfPlayers = 0;
		m_iPlayersDiedInVehicle = 0;
		
		m_iRoadKills = 0;
		m_iFriendlyRoadKills = 0;
		m_iAIRoadKills = 0;
		m_iFriendlyAIRoadKills = 0;
		
		m_fMetersAsOccupant = 0;
		
		//SupplyTruckDriverModule
		m_fTraveledDistanceSupplyVehicle = 0;
		m_iTraveledTimeSupplyVehicle = 0;
		////////////////////////
	}
	
	//------------------------------------------------------------------------------------------------
	void FillWithProfile(array<int> playerProfile)
	{
		typename characterDataEnum = ECharacterDataStats;
		
		/*
		Print("Fill with profile. PlayerProfile: "+playerProfile.Count() + " positions");
		for (int i=0; i<playerProfile.Count(); i++)
			Print("Position" +i+": "+playerProfile[i]);
		*/
		
		if (!playerProfile)
		{
			Print ("SCR:PlayerDataStats:FillWithProfile: No playerProfile", LogLevel.ERROR);
			return;
		}
			
		
		if (playerProfile.Count() != characterDataEnum.GetVariableCount())
		{
			Print("SCR:PlayerDataStats:FillWithProfile: Couldn't fill profile due to playerProfile having a different size compared to the enum. Expected count: "+characterDataEnum.GetVariableCount()+", real count: "+playerProfile.Count(), LogLevel.ERROR);
			return;
		}
		
		//RANK
		////////////////////////
		m_iRank = playerProfile[ECharacterDataStats.m_iRank];
		m_iRankExperience = playerProfile[ECharacterDataStats.m_iRankExperience];
		////////////////////////
		
		//SPECIALIZATIONS
		////////////////////////
		m_aSpPoints.Insert(playerProfile[ECharacterDataStats.m_aSpPoints_0]);
		m_aSpPoints.Insert(playerProfile[ECharacterDataStats.m_aSpPoints_1]);
		m_aSpPoints.Insert(playerProfile[ECharacterDataStats.m_aSpPoints_2]);
		////////////////////////
		
		//WAR CRIMES
		////////////////////////
		m_fWarCrimes = playerProfile[ECharacterDataStats.m_fWarCrimes];
		////////////////////////
		
		//DATACOLLECTORMODULES
		////////////////////////
		//BasicActionsModule
		m_fMetersWalked = playerProfile[ECharacterDataStats.m_fMetersWalked];
		m_fWarCrimes = playerProfile[ECharacterDataStats.m_fWarCrimes];
		m_fSessionDuration = playerProfile[ECharacterDataStats.m_fSessionDuration];
			
		//ShootingModule
		m_iKills = playerProfile[ECharacterDataStats.m_iKills];
		m_iAIKills = playerProfile[ECharacterDataStats.m_iAIKills];
		m_iShots = playerProfile[ECharacterDataStats.m_iShots];
		m_iGrenadesThrown = playerProfile[ECharacterDataStats.m_iGrenadesThrown];
		m_iFriendlyKills = playerProfile[ECharacterDataStats.m_iFriendlyKills];
		m_iFriendlyAIKills = playerProfile[ECharacterDataStats.m_iFriendlyAIKills];
		m_iDeaths = playerProfile[ECharacterDataStats.m_iDeaths];
		
		//DriverModule
		m_fMetersDriven = playerProfile[ECharacterDataStats.m_fMetersDriven];
		m_iPointsAsDriverOfPlayers = playerProfile[ECharacterDataStats.m_iPointsAsDriverOfPlayers];
		m_iPlayersDiedInVehicle = playerProfile[ECharacterDataStats.m_iPlayersDiedInVehicle];
		
		m_iRoadKills = playerProfile[ECharacterDataStats.m_iRoadKills];
		m_iFriendlyRoadKills = playerProfile[ECharacterDataStats.m_iFriendlyRoadKills];
		m_iAIRoadKills = playerProfile[ECharacterDataStats.m_iAIRoadKills];
		m_iFriendlyAIRoadKills = playerProfile[ECharacterDataStats.m_iFriendlyAIRoadKills];
		
		m_fMetersAsOccupant = playerProfile[ECharacterDataStats.m_fMetersAsOccupant];
		
		//SupplyTruckDriverModule
		m_fTraveledDistanceSupplyVehicle = playerProfile[ECharacterDataStats.m_fTraveledDistanceSupplyVehicle];
		m_iTraveledTimeSupplyVehicle = playerProfile[ECharacterDataStats.m_iTraveledTimeSupplyVehicle];
		////////////////////////
	}
	
	//------------------------------------------------------------------------------------------------
	static array<int> DataStatsToArray(SCR_PlayerDataStats s)
	{
		array<int> toRet = {};
		
		toRet.Insert(s.m_iRank);
		toRet.Insert(s.m_iRankExperience);
		toRet.Insert(s.m_aSpPoints[0]);
		toRet.Insert(s.m_aSpPoints[1]);
		toRet.Insert(s.m_aSpPoints[2]);
		toRet.Insert(s.m_fWarCrimes);
		toRet.Insert(s.m_fMetersWalked);
		toRet.Insert(s.m_fSessionDuration);
		toRet.Insert(s.m_iKills);
		toRet.Insert(s.m_iAIKills);
		toRet.Insert(s.m_iShots);
		toRet.Insert(s.m_iGrenadesThrown);
		toRet.Insert(s.m_iFriendlyKills);
		toRet.Insert(s.m_iFriendlyAIKills);
		toRet.Insert(s.m_iDeaths);
		toRet.Insert(s.m_fMetersDriven);
		toRet.Insert(s.m_iPointsAsDriverOfPlayers);
		toRet.Insert(s.m_iPlayersDiedInVehicle);
		toRet.Insert(s.m_iRoadKills);
		toRet.Insert(s.m_iFriendlyRoadKills);
		toRet.Insert(s.m_iAIRoadKills);
		toRet.Insert(s.m_iFriendlyAIRoadKills);
		toRet.Insert(s.m_fMetersAsOccupant);
		toRet.Insert(s.m_fTraveledDistanceSupplyVehicle);
		toRet.Insert(s.m_iTraveledTimeSupplyVehicle);
		
		return toRet;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerDataStats CalculateStatsDifference(SCR_PlayerDataStats s1, SCR_PlayerDataStats s2)
	{
		SCR_PlayerDataStats StatsToReturn = new SCR_PlayerDataStats();
		
		//RANK
		////////////////////////
		StatsToReturn.m_iRank = 0;
		StatsToReturn.m_iRankExperience = 0;
		////////////////////////
		
		//SPECIALIZATIONS
		////////////////////////
		////////////////////////
		
		//WAR CRIMES
		////////////////////////
		StatsToReturn.m_fWarCrimes = 0;
		////////////////////////
		
		//DATACOLLECTORMODULES
		////////////////////////
		//BasicActionsModule
		StatsToReturn.m_fMetersWalked = s1.m_fMetersWalked - s2.m_fMetersWalked;
		StatsToReturn.m_fSessionDuration = s1.m_fSessionDuration - s2.m_fSessionDuration;
			
		//ShootingModule
		StatsToReturn.m_iKills = s1.m_iKills - s2.m_iKills;
		StatsToReturn.m_iAIKills = s1.m_iAIKills - s2.m_iAIKills;
		StatsToReturn.m_iShots = s1.m_iShots - s2.m_iShots;
		StatsToReturn.m_iGrenadesThrown = s1.m_iGrenadesThrown - s2.m_iGrenadesThrown;
		StatsToReturn.m_iFriendlyKills = s1.m_iFriendlyKills - s2.m_iFriendlyKills;
		StatsToReturn.m_iFriendlyAIKills = s1.m_iFriendlyAIKills - s2.m_iFriendlyAIKills;
		StatsToReturn.m_iDeaths = s1.m_iDeaths - s2.m_iDeaths;
		
		//DriverModule
		StatsToReturn.m_fMetersDriven = s1.m_fMetersDriven - s2.m_fMetersDriven;
		StatsToReturn.m_iPointsAsDriverOfPlayers = s1.m_iPointsAsDriverOfPlayers - s2.m_iPointsAsDriverOfPlayers;
		StatsToReturn.m_iPlayersDiedInVehicle = s1.m_iPlayersDiedInVehicle - s2.m_iPlayersDiedInVehicle;
		
		StatsToReturn.m_iRoadKills = s1.m_iRoadKills - s2.m_iRoadKills;
		StatsToReturn.m_iFriendlyRoadKills = s1.m_iFriendlyRoadKills - s2.m_iFriendlyRoadKills;
		StatsToReturn.m_iAIRoadKills = s1.m_iAIRoadKills - s2.m_iAIRoadKills;
		StatsToReturn.m_iFriendlyAIRoadKills = s1.m_iFriendlyAIRoadKills - s2.m_iFriendlyAIRoadKills;
		
		StatsToReturn.m_fMetersAsOccupant = s1.m_fMetersAsOccupant - s2.m_fMetersAsOccupant;
		
		//SupplyTruckDriverModule
		StatsToReturn.m_fTraveledDistanceSupplyVehicle = s1.m_fTraveledDistanceSupplyVehicle - s2.m_fTraveledDistanceSupplyVehicle;
		StatsToReturn.m_iTraveledTimeSupplyVehicle = s1.m_iTraveledTimeSupplyVehicle - s2.m_iTraveledTimeSupplyVehicle;
		////////////////////////			
		
		return StatsToReturn;
	}	
};

//DO NOT CHANGE THE ORDER OF THE ITEMS OF THE ENUM. (!!!) WE USE THE INDEX AS AN ID
//------------------------------------------------------------------------------------------------
enum ECharacterDataStats
{
	m_iRank,
	m_iRankExperience,
	m_fSessionDuration,
	m_aSpPoints_0,
	m_aSpPoints_1,
	m_aSpPoints_2,
	m_fWarCrimes,
	m_fMetersWalked,
	m_iKills,
	m_iAIKills,
	m_iShots,
	m_iGrenadesThrown,
	m_iFriendlyKills,
	m_iFriendlyAIKills,
	m_iDeaths,
	m_fMetersDriven,
	m_iPointsAsDriverOfPlayers,
	m_iPlayersDiedInVehicle,
	m_iRoadKills,
	m_iFriendlyRoadKills,
	m_iAIRoadKills,
	m_iFriendlyAIRoadKills,
	m_fMetersAsOccupant,
	m_fTraveledDistanceSupplyVehicle,
	m_iTraveledTimeSupplyVehicle
};