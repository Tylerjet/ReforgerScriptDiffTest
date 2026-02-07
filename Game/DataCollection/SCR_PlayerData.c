//------------------------------------------------------------------------------------------------
class SCR_PlayerData : JsonApiStruct
{		
	//Only available in game modes
	protected int m_iPlayerID;
	
	/********************/
	//BACKEND CONNECTION//
	/********************/
	
	protected ref array<int> m_aData;
	protected static BackendApi s_BackendApi = null;
	protected ref BackendCallback m_Callback = new ref CampaignCallback();
	protected ref ProfileMenuLoadingCallback m_menuCallback = new ref ProfileMenuLoadingCallback(this);
	
	/*********************/
	//STATS CONFIGURATION//
	/*********************/
	
	protected ref SCR_PlayerDataConfigs m_Configs;
	
	/****************************************************/
	// Rank, Specializations, DataModules and War Crimes//
	/****************************************************/
	
	//Stats updated by data collectors
	protected ref SCR_PlayerDataStats m_Stats;
	
	//Original stats of this player at the beginning of the session
	protected ref SCR_PlayerDataStats m_PreviousStats;
	
	protected int m_iAttemptsCounter = 0; //To remove
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerData (int id = 0)
	{
		if (GetGame())
			s_BackendApi = GetGame().GetBackendApi();
		
		RegV("m_aData");
		
		m_aData = {};
		
		m_Stats = new SCR_PlayerDataStats();
		m_PreviousStats = new SCR_PlayerDataStats();
		m_Configs = SCR_PlayerDataConfigs.GetInstance();
		m_iPlayerID = id;
		
		if (m_menuCallback && s_BackendApi && s_BackendApi.IsAuthenticated())
		{
			Print("Making backend request for PlayerData", LogLevel.DEBUG);
			s_BackendApi.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterGet, m_menuCallback, this, id); //This should work either on main menu (with id 0) or ingame
			//Now I wait and OnSuccess in the callback I call BackendDataReady();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void BackendDataReady()
	{
		Print("Backend data is ready! Trying to use it to initialize the m_Stats and m_PreviousStats", LogLevel.DEBUG); //To remove
		m_Stats.FillWithProfile(m_aData);
		m_PreviousStats.FillWithProfile(m_aData);
	}
	
	void LoadEmptyProfile()
	{
		m_Stats.fillWithZeroes();
		m_PreviousStats.fillWithZeroes();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateCharacterData(array<int> UpdatedData)	
	{
		m_aData.Clear();
		m_aData.InsertAll(UpdatedData);
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreProfile()
	{
		if (!s_BackendApi || !m_Callback || !m_aData || m_aData.IsEmpty())
			return;
		
		SCR_GameModeCampaignMP campaign = SCR_GameModeCampaignMP.GetCampaign();
		if (!campaign)
			return;
		
		CalculateStatsChange();
		UpdateCharacterData(SCR_PlayerDataStats.DataStatsToArray(m_Stats));
		
		#ifndef WORKBENCH
			s_BackendApi.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterUpdateS2S,m_Callback,this,m_iPlayerID);
		#else
			s_BackendApi.PlayerRequest(EBackendRequest.EBREQ_GAME_DevCharacterUpdate,m_Callback,this,m_iPlayerID);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CalculateStatsChange()
	{
		//We find the differences provided by this session
		//In order to calculate Rank, Specializations, and War Crime points
		SCR_PlayerDataStats StatsGain = SCR_PlayerDataStats.CalculateStatsDifference(m_Stats, m_PreviousStats);
		
		array<int> rawSpPoints = {};
		rawSpPoints.Resize(m_Configs.m_iNumberOfSpecializations); //Allocate space for storing the raw specialization points gained
		
		//Specializations: There are three: Infantry, Logistics and Medical
		//
		//
		//Infantry
		rawSpPoints[ECareerSp.Infantry] = 0;
		rawSpPoints[ECareerSp.Infantry] = rawSpPoints[ECareerSp.Infantry] + StatsGain.m_fMetersWalked * m_Configs.m_fSpInfantryModifierPointsMetersWalked;
		rawSpPoints[ECareerSp.Infantry] = rawSpPoints[ECareerSp.Infantry] + StatsGain.m_iKills * m_Configs.m_fKillModifierPoints;
		rawSpPoints[ECareerSp.Infantry] = rawSpPoints[ECareerSp.Infantry] + StatsGain.m_iAIKills * m_Configs.m_fAIKillModifierPoints;
		rawSpPoints[ECareerSp.Infantry] = rawSpPoints[ECareerSp.Infantry] + (StatsGain.m_iKills / (1 + StatsGain.m_iShots + StatsGain.m_iGrenadesThrown)) * m_Configs.m_fPrecisionModifierPoints;
		//
		//
		//Logistics
		rawSpPoints[ECareerSp.Logistics] = 0;
		rawSpPoints[ECareerSp.Logistics] = rawSpPoints[ECareerSp.Logistics] + StatsGain.m_fMetersDriven * m_Configs.m_fMetersDrivenModifierPoints;
		rawSpPoints[ECareerSp.Logistics] = rawSpPoints[ECareerSp.Logistics] + StatsGain.m_iPointsAsDriverOfPlayers;
		rawSpPoints[ECareerSp.Logistics] = rawSpPoints[ECareerSp.Logistics] + StatsGain.m_fTraveledDistanceSupplyVehicle * m_Configs.m_fTraveledDistanceSupplyVehicleModifierPoints;
		rawSpPoints[ECareerSp.Logistics] = rawSpPoints[ECareerSp.Logistics] + StatsGain.m_iTraveledTimeSupplyVehicle * m_Configs.m_fTraveledTimeSupplyVehicleModifierPoints;
		int minusDiedPlayers = StatsGain.m_iPlayersDiedInVehicle;
		if (minusDiedPlayers>5) minusDiedPlayers = 5;
		rawSpPoints[ECareerSp.Logistics] = rawSpPoints[ECareerSp.Logistics] + -1*minusDiedPlayers*m_Configs.m_iPlayersDiedInVehicleModifierPoints;
		if (rawSpPoints[ECareerSp.Logistics] <= 0)
			rawSpPoints[ECareerSp.Logistics] = 0;
		//
		//Medical
		//--Not yet implemented--
		
		
		//HERE we update the specializationPoints
		//---------------------------------------
		int totalSpecializationPointsGain = 0;
		
		int i, j;
		for (i = 0; i < m_Configs.m_iNumberOfSpecializations; i++)
		{
			totalSpecializationPointsGain += rawSpPoints[i];
			
			j = 0;
			while (m_Configs.m_aRangesEnd[j] < m_Stats.m_aSpPoints[i] && j < m_Configs.m_iRanges)
				j++;
			if (j >= m_Configs.m_iRanges)
				continue;
			
			//Formula: NewSpecializationPoints = CurrentSpecializationPoints + GainedPoints * c1 * (c2 ^ CurrentSpecializationPoints)
			m_Stats.m_aSpPoints[i] = m_Stats.m_aSpPoints[i] + rawSpPoints[i] * m_Configs.m_aRangesC1[j]*(Math.Pow(m_Configs.m_aRangesC2[j], m_Stats.m_aSpPoints[i]));
		}
		
		//HERE we update the War Crimes
		float warCrimesPoints = 0;
		warCrimesPoints += StatsGain.m_iFriendlyKills * m_Configs.m_fFriendlyKillsModifierPoints;
		warCrimesPoints += StatsGain.m_iFriendlyAIKills * m_Configs.m_fFriendlyAIKillsModifierPoints;
		warCrimesPoints /= 100;
		
		// qualityTimeRatio is calculated as the ratio between expectedPoints per minute and obtainedPointsInAverage per minute.
		//For expected points, I use m_fSTDPointsQualityTime as the expected growth per specialization and 2.5 as the number of specializations with it as an accurate simplification
		float qualityTimeRatio = (totalSpecializationPointsGain/(StatsGain.m_fSessionDuration/60)) / (m_Configs.m_fSTDPointsQualityTime*2.5/60);
		
		// WarCrimes go down by m_fWarCrimesDecreaseRatePerHour * qualityTimeRatio * time
		float warCrimesDown = (qualityTimeRatio * StatsGain.m_fSessionDuration/3600) * m_Configs.m_fWarCrimesDecreaseRatePerHour;
		warCrimesPoints -= warCrimesDown;
		
		m_Stats.m_fWarCrimes += warCrimesPoints;
		
		if (m_Stats.m_fWarCrimes > m_Configs.m_fMaxWarCrimes)
			m_Stats.m_fWarCrimes = m_Configs.m_fMaxWarCrimes;
		else if (m_Stats.m_fWarCrimes < 0)
			m_Stats.m_fWarCrimes = 0;
		
		int rankPoints = totalSpecializationPointsGain * m_Configs.m_fSpecializationPointsModifier;
		rankPoints *= (1-m_Stats.m_fWarCrimes);
		
		m_Stats.m_iRankExperience += rankPoints;
		if (m_Stats.m_iRankExperience > 1000000)
			m_Stats.m_iRankExperience = 1000000;
		
		m_Stats.m_iRank = m_Stats.m_iRankExperience/100000;
	}
	
	//m_Stats and m_PreviousStats must not be accessed from outside of this class.
	//That is why there are getters and setters for their attributes, but not for their instances
	//
	//
	//
	//SETTERS AND GETTERS:
	
	/******************/
	//Rank and Generic//
	/******************/
	
	//------------------------------------------------------------------------------------------------
	bool IsPlayerDataAvailable()
	{
		return m_Stats != null && !m_Stats.m_aSpPoints.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRankExperience()
	{
		return m_Stats.m_iRankExperience;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRank()
	{
		return m_Stats.m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSessionDuration()
	{
		return m_Stats.m_fSessionDuration;
	}
	
	/*****************/
	//Specializations//
	/*****************/
	
	array<int> GetSpecializationPoints()
	{
		return m_Stats.m_aSpPoints;
	}
	
	int GetSpecializationCount(int n)
	{		
		switch (n)
		{
			case 0: return m_Configs.m_iSpecialization0DisplayCount;
			case 1: return m_Configs.m_iSpecialization1DisplayCount;
			case 2: return m_Configs.m_iSpecialization2DisplayCount;
			default: return -1;
		}
		return -1;
	}
	
	//
	//
	//
	//MODULES:
		
	/********************/
	//BasicActionsModule//
	/********************/
	
	float GetDistanceWalked()
	{
		return m_Stats.m_fMetersWalked;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddDistanceWalked(float meters)
	{
		m_Stats.m_fMetersWalked += meters;
	}
	
	void AddSessionDuration(float timeslice)
	{
		m_Stats.m_fSessionDuration += timeslice;
	}
	
	/****************/
	//ShootingModule//
	/****************/
		
	//------------------------------------------------------------------------------------------------
	void AddKill(bool friendly)
	{
		if (friendly)
			m_Stats.m_iFriendlyKills += 1;
		
		else
			m_Stats.m_iKills += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRoadKill(bool friendly)
	{
		if (friendly)
			m_Stats.m_iFriendlyRoadKills += 1;
		else
			m_Stats.m_iRoadKills += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAIKill(bool friendly)
	{
		if (friendly)
			m_Stats.m_iFriendlyAIKills += 1;
		else
			m_Stats.m_iAIKills += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAIRoadKill(bool friendly)
	{
		if (friendly)
			m_Stats.m_iFriendlyAIRoadKills += 1;
		
		else
			m_Stats.m_iAIRoadKills += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddDeath()
	{
		m_Stats.m_iDeaths += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddShot()
	{
		m_Stats.m_iShots += 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddShots(int n)
	{
		m_Stats.m_iShots += n;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddGrenadeThrown()
	{
		m_Stats.m_iGrenadesThrown += 1;
	}
	
	/****************/
	//DriverModule//
	/****************/
		
	//------------------------------------------------------------------------------------------------
	void AddMetersDriven(float meters)
	{
		m_Stats.m_fMetersDriven += meters;
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculatePointsAsDriver(float meters, int players)
	{
		m_Stats.m_iPointsAsDriverOfPlayers += meters * players * m_Configs.m_fDriverOfPlayersModifierPoints;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPlayersDiedInVehicle (int count)
	{
		m_Stats.m_iPlayersDiedInVehicle += count;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddMetersAsOccupant(float meters)
	{
		m_Stats.m_fMetersAsOccupant += meters;
	}
	
	/*************************/
	//SupplyTruckDriverModule//
	/*************************/
	
	//------------------------------------------------------------------------------------------------
	void AddTraveledTimeSupplyVehicle(int time)
	{
		m_Stats.m_iTraveledTimeSupplyVehicle += time;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddTraveledDistanceSupplyVehicle(float distance)
	{
		m_Stats.m_fTraveledDistanceSupplyVehicle += distance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPointsAsDriverOfPlayers()
	{
		return m_Stats.m_iPointsAsDriverOfPlayers;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDistanceDriven()
	{
		return m_Stats.m_fMetersDriven;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDistanceAsOccupant()
	{
		return m_Stats.m_fMetersAsOccupant;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDeaths()
	{
		return m_Stats.m_iDeaths;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayerKills()
	{
		return m_Stats.m_iKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAIKills()
	{
		return m_Stats.m_iAIKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFriendlyPlayerKills()
	{
		return m_Stats.m_iFriendlyKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFriendlyAIKills()
	{
		return m_Stats.m_iAIKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBulletsShot()
	{
		return m_Stats.m_iShots;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGrenadesThrown()
	{
		return m_Stats.m_iGrenadesThrown;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRoadKills()
	{
		return m_Stats.m_iRoadKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetAIRoadKills()
	{
		return m_Stats.m_iAIRoadKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFriendlyRoadKills()
	{
		return m_Stats.m_iFriendlyRoadKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFriendlyAIRoadKills()
	{
		return m_Stats.m_iFriendlyAIRoadKills;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPlayersDiedInVehicle()
	{
		return m_Stats.m_iPlayersDiedInVehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTraveledDistanceSupplyVehicle()
	{
		return m_Stats.m_fTraveledDistanceSupplyVehicle;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTraveledTimeSupplyVehicle()
	{
		return m_Stats.m_iTraveledTimeSupplyVehicle;
	}
};

//------------------------------------------------------------------------------------------------
class ProfileMenuLoadingCallback : BackendCallback
{
	
	SCR_PlayerData m_PlayerDataInstance;
	
	void ProfileMenuLoadingCallback(SCR_PlayerData instance)
	{
		m_PlayerDataInstance = instance;
	}
	
	override void OnDataReceive(string data, int size)
	{
		Print("[ProfileMenuLoadingCallback] Data received, size="+size, LogLevel.DEBUG);
		Print(data);
	}

	/**
	\brief Request finished with error result
	\param code Error code is type of EBackendError
	*/
	override void OnError(int code, int restCode, int apiCode)
	{
		Print("[ProfileMenuLoadingCallback] OnError: "+ GetGame().GetBackendApi().GetErrorCode(code), LogLevel.ERROR);
	}

	/**
	\brief Request finished with success result
	\param code Code is type of EBackendRequest
	*/
	override void OnSuccess(int code)
	{
		Print("[ProfileMenuLoadingCallback] OnSuccess.", LogLevel.DEBUG);
		m_PlayerDataInstance.BackendDataReady();
	}

	/**
	\brief Request not finished due to timeout
	*/
	override void OnTimeout()
	{
		Print("[ProfileMenuLoadingCallback] OnTimeout", LogLevel.ERROR);
	}
};