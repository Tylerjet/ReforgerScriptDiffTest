//------------------------------------------------------------------------------------------------
class SCR_PlayerData : JsonApiStruct
{
	/********************/
	//BACKEND CONNECTION//
	/********************/
	
	protected int m_iPlayerID;
	
	//Stats that we keep updating
	protected ref array<float> m_aStats = {};
	
	//Original stats
	protected ref array<float> m_aPreviousStats = {};
	
	//We store in m_aEarnt the number of points provided by each of the fields to the specializations, and the total sum for each specialization in the corresponding field (sp0, sp1, etc)
	protected ref array<float> m_aEarnt = {};
	
	//Backend callbacks necessary to make the request to load and store the characterdata from the player's profile
	protected ref CharacterDataLoadingCallback m_CharacterDataCallback = new CharacterDataLoadingCallback(this);
	protected ref BackendCallback m_StoringCallback = new BackendCallback();
	
	//Invoker that notifies UI screens listening to it that the data object is ready
	protected ref ScriptInvoker m_OnDataReady = new ScriptInvoker();
	
	/*********************/
	//STATS CONFIGURATION//
	/*********************/
	
	protected ref SCR_PlayerDataConfigs m_Configs;
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerData (int id, bool HasPlayerBeenAuditted, bool requestFromBackend = true)
	{
		if (!GetGame().GetBackendApi())
			return;
		
		RegV("m_aStats");
		
		m_Configs = SCR_PlayerDataConfigs.GetInstance();
		m_iPlayerID = id;
		
		if (requestFromBackend)
			RequestLoadData(HasPlayerBeenAuditted);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Overrides previous stats array with the provided one
	//! To be used on clients only!
	void SetPreviousStats(array<float> previousStats)
	{
		m_aPreviousStats = previousStats;
	}
	
	//------------------------------------------------------------------------------------------------
	array<float> GetPreviousStats()
	{
		return m_aPreviousStats;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Overrides stats array with the provided one
	//! To be used on clients only!
	void SetStats(array<float> stats)
	{
		m_aStats = stats;
	}
	
	//------------------------------------------------------------------------------------------------
	array<float> GetStats()
	{
		return m_aStats;
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestLoadData(bool HasPlayerBeenAuditted = false)
	{
		BackendApi ba = GetGame().GetBackendApi();
		
		if (!ba)
		{
			Print("SCR_PlayerData:RequestLoadData: Backend api is null!", LogLevel.ERROR);
			return;
		}
			
		if (HasPlayerBeenAuditted)
		{
			ba.PlayerData(this, m_iPlayerID);
			BackendDataReady();
			return;
		}
		
		if (!m_CharacterDataCallback)
		{
			Print("SCR_PlayerData:RequestLoadData: The character data callback is null", LogLevel.ERROR);
			return;
		}
			
		
		if (ba.IsAuthenticated())
		{
				Print("Making menuCallback request for PlayerData", LogLevel.DEBUG);
				ba.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterGet, m_CharacterDataCallback, this, m_iPlayerID); //ID 0 refers to the local player
				//The callback has a reference to this instance so it will automatically call the BackendDataReady or the LoadEmptyProfile methods
		}
		else
		{
			Print ("SCR_PlayerData:RequestLoadData: Requesting data before player has been authenticated!", LogLevel.ERROR);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetDataReadyInvoker()
	{
		return m_OnDataReady;
	}
	
	//------------------------------------------------------------------------------------------------
	void BackendDataReady()
	{
		Print("Backend data is ready!", LogLevel.DEBUG); //To remove
		
		typename characterDataEnum = SCR_EDataStats;
		
		if (m_aStats.Count() != characterDataEnum.GetVariableCount())
		{
			Print("SCR:PlayerDataStats:FillWithProfile: m_aStats has a weird size compared to the enum. Expected count: "+characterDataEnum.GetVariableCount()+", real count: "+m_aStats.Count(), LogLevel.ERROR);
			LoadEmptyProfile();
		}
		else
		{
			m_aPreviousStats.InsertAll(m_aStats);
			m_OnDataReady.Invoke();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadEmptyProfile()
	{
		typename characterDataEnum = SCR_EDataStats;
		int count = characterDataEnum.GetVariableCount();
		
		for (int i = 0; i < count; i++)
		{
			if (i == SCR_EDataStats.LEVELEXPERIENCE)
				m_aStats.Insert(m_Configs.XP_NEEDED_FOR_LEVEL);
			else
				m_aStats.Insert(0);
		}
			
		m_aPreviousStats.InsertAll(m_aStats);
		m_OnDataReady.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	sealed void StoreProfile()
	{
		if (!m_CharacterDataCallback || !m_aStats || m_aStats.IsEmpty())
			return;
		
		BackendApi ba = GetGame().GetBackendApi();
		if (!ba)
			return;
		
		if (!IsDataProgressionReady())
			CalculateStatsChange();
		
		#ifndef WORKBENCH
			ba.PlayerRequest(EBackendRequest.EBREQ_GAME_CharacterUpdateS2S, m_StoringCallback, this, m_iPlayerID);
		#else
			ba.PlayerRequest(EBackendRequest.EBREQ_GAME_DevCharacterUpdate, m_StoringCallback, this, m_iPlayerID);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	sealed void DebugCalculateStats()
	{
		Print("SCR_PlayerData:DebugCalculateStats: This method calls calculateStatsChange providing fake stats for debugging purposes.", LogLevel.DEBUG);
		
		m_aStats[SCR_EDataStats.SESSIONDURATION] = m_aStats[SCR_EDataStats.SESSIONDURATION] + 7200;
		m_aStats[SCR_EDataStats.METERSWALKED] = m_aStats[SCR_EDataStats.METERSWALKED] + 20000;
		m_aStats[SCR_EDataStats.KILLS] = m_aStats[SCR_EDataStats.KILLS] + 20;
		m_aStats[SCR_EDataStats.AIKILLS] = m_aStats[SCR_EDataStats.AIKILLS] + 50;
		m_aStats[SCR_EDataStats.SHOTS] = m_aStats[SCR_EDataStats.SHOTS] + 800;
		m_aStats[SCR_EDataStats.GRENADESTHROWN] = m_aStats[SCR_EDataStats.GRENADESTHROWN] + 10;
		m_aStats[SCR_EDataStats.METERSDRIVEN] = m_aStats[SCR_EDataStats.METERSDRIVEN] + 50000;
		m_aStats[SCR_EDataStats.FRIENDLYKILLS] = m_aStats[SCR_EDataStats.FRIENDLYKILLS] + 30;
		
		m_aStats[SCR_EDataStats.BANDAGESELF] = m_aStats[SCR_EDataStats.BANDAGESELF] + 30;
		m_aStats[SCR_EDataStats.BANDAGEFRIENDLIES] = m_aStats[SCR_EDataStats.BANDAGEFRIENDLIES] + 10;
		m_aStats[SCR_EDataStats.TOURNIQUETSELF] = m_aStats[SCR_EDataStats.TOURNIQUETSELF] + 30;
		m_aStats[SCR_EDataStats.TOURNIQUETFRIENDLIES] = m_aStats[SCR_EDataStats.TOURNIQUETFRIENDLIES] + 10;
		m_aStats[SCR_EDataStats.SELINESELF] = m_aStats[SCR_EDataStats.SELINESELF] + 30;
		m_aStats[SCR_EDataStats.SELINEFRIENDLIES] = m_aStats[SCR_EDataStats.SELINEFRIENDLIES] + 10;
		m_aStats[SCR_EDataStats.MORPHINESELF] = m_aStats[SCR_EDataStats.MORPHINESELF] + 30;
		m_aStats[SCR_EDataStats.MORPHINEFRIENDLIES] = m_aStats[SCR_EDataStats.MORPHINEFRIENDLIES] + 10;
		if (!IsDataProgressionReady())
			CalculateStatsChange();
	}
	
	//------------------------------------------------------------------------------------------------
	sealed void CalculateStatsChange()
	{
		//We find the differences provided by this session
		//In order to calculate Rank, Specializations, and War Crime points
		array<float> StatsGain = CalculateStatsDifference();
		
		for (int i = StatsGain.Count()-1; i >= 0; i--)
			m_aEarnt.Insert(0);
		
		if (StatsGain[SCR_EDataStats.SESSIONDURATION] < 1)
			return;
		
		//Little assert
		if (StatsGain[SCR_EDataStats.SPPOINTS0] != 0 || StatsGain[SCR_EDataStats.SPPOINTS1] != 0 || StatsGain[SCR_EDataStats.SPPOINTS2] != 0|| StatsGain[SCR_EDataStats.RANK] != 0 || StatsGain[SCR_EDataStats.LEVELEXPERIENCE] != 0)
			Print("SCR_PlayerData:CalculateStatsChange: Specialization stats, experience or rank stats were modified prematurely", LogLevel.ERROR);
		
		//CALCULATE STATS ADDITIONS TO THE SPECIALIZATIONS, THEN WAR CRIMES, THEN SPECIALIZATION POINTS, AND THEN LEVEL OF EXPERIENCE AND RANK
		
		//Specializations: There are three: Infantry, Logistics and Medical
		int totalSpecializationPointsGain = 0;
		//
		//
		//Infantry
		if (m_aStats[SCR_EDataStats.SPPOINTS0] < m_Configs.SPECIALIZATIONMAX)
		{
			m_aEarnt[SCR_EDataStats.METERSWALKED] = StatsGain[SCR_EDataStats.METERSWALKED] * m_Configs.MODIFIERMETERSWALKED;
			m_aEarnt[SCR_EDataStats.KILLS] = StatsGain[SCR_EDataStats.KILLS] * m_Configs.MODIFIERKILLS;
			m_aEarnt[SCR_EDataStats.AIKILLS] = StatsGain[SCR_EDataStats.AIKILLS] * m_Configs.MODIFIERAIKILLS;
			
			if (StatsGain[SCR_EDataStats.SHOTS] <= 0)
				m_aEarnt[SCR_EDataStats.SHOTS] = 0;
			else
				//We should not use ROADKILLS yet
				m_aEarnt[SCR_EDataStats.SHOTS] = ( (StatsGain[SCR_EDataStats.KILLS] /*- StatsGain[SCR_EDataStats.ROADKILLS]*/ + StatsGain[SCR_EDataStats.AIKILLS] /*- StatsGain[SCR_EDataStats.AIROADKILLS]*/) / StatsGain[SCR_EDataStats.SHOTS]) * m_Configs.MODIFIERPRECISION;
			
			/* We should not use GRENADESTHROWN yet
			if (StatsGain[SCR_EDataStats.GRENADESTHROWN] <= 0)
				m_aEarnt[SCR_EDataStats.GRENADESTHROWN] = 0;
			else
				m_aEarnt[SCR_EDataStats.GRENADESTHROWN] = (StatsGain[SCR_EDataStats.KILLS] / StatsGain[SCR_EDataStats.GRENADESTHROWN]) * m_Configs.MODIFIERPRECISION;
			*/
			
			m_aEarnt[SCR_EDataStats.SPPOINTS0] = m_aEarnt[SCR_EDataStats.METERSWALKED] + m_aEarnt[SCR_EDataStats.KILLS] + m_aEarnt[SCR_EDataStats.AIKILLS] + m_aEarnt[SCR_EDataStats.SHOTS]; /*+ m_aEarnt[SCR_EDataStats.GRENADESTHROWN];*/
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS0] = 0;
		
		StatsGain[SCR_EDataStats.SPPOINTS0] = m_aEarnt[SCR_EDataStats.SPPOINTS0];
		totalSpecializationPointsGain += StatsGain[SCR_EDataStats.SPPOINTS0];
		
		//
		//
		//Logistics
		if (m_aStats[SCR_EDataStats.SPPOINTS1] < m_Configs.SPECIALIZATIONMAX)
		{
			m_aEarnt[SCR_EDataStats.METERSDRIVEN] = StatsGain[SCR_EDataStats.METERSDRIVEN] * m_Configs.MODIFIERMETERSDRIVEN;
			m_aEarnt[SCR_EDataStats.POINTSASDRIVEROFPLAYERS] = StatsGain[SCR_EDataStats.POINTSASDRIVEROFPLAYERS];
			
			if (StatsGain[SCR_EDataStats.PLAYERSDIEDINVEHICLE] > 5)
				StatsGain[SCR_EDataStats.PLAYERSDIEDINVEHICLE] = 5;
			
			m_aEarnt[SCR_EDataStats.PLAYERSDIEDINVEHICLE] = -1 * StatsGain[SCR_EDataStats.PLAYERSDIEDINVEHICLE] * m_Configs.MODIFIERPLAYERSDIEDINVEHICLE;
			
			m_aEarnt[SCR_EDataStats.SPPOINTS1] = m_aEarnt[SCR_EDataStats.METERSDRIVEN] + m_aEarnt[SCR_EDataStats.POINTSASDRIVEROFPLAYERS] + m_aEarnt[SCR_EDataStats.PLAYERSDIEDINVEHICLE];
			
			if (m_aEarnt[SCR_EDataStats.SPPOINTS1] <= 0)
				m_aEarnt[SCR_EDataStats.SPPOINTS1] = 0;
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS1] = 0;
		
		StatsGain[SCR_EDataStats.SPPOINTS1] = m_aEarnt[SCR_EDataStats.SPPOINTS1];
		totalSpecializationPointsGain += StatsGain[SCR_EDataStats.SPPOINTS1];
		
		//
		//
		//Medical
		if (m_aStats[SCR_EDataStats.SPPOINTS2] < m_Configs.SPECIALIZATIONMAX)
		{
			m_aEarnt[SCR_EDataStats.BANDAGESELF] = StatsGain[SCR_EDataStats.BANDAGESELF] * m_Configs.MODIFIERBANDAGESELF;
			m_aEarnt[SCR_EDataStats.BANDAGEFRIENDLIES] = StatsGain[SCR_EDataStats.BANDAGEFRIENDLIES] * m_Configs.MODIFIERBANDAGEFRIENDLIES;
			m_aEarnt[SCR_EDataStats.TOURNIQUETSELF] = StatsGain[SCR_EDataStats.TOURNIQUETSELF] * m_Configs.MODIFIERTOURNIQUETSELF;
			m_aEarnt[SCR_EDataStats.TOURNIQUETFRIENDLIES] = StatsGain[SCR_EDataStats.TOURNIQUETFRIENDLIES] * m_Configs.MODIFIERTOURNIQUETFRIENDLIES;
			m_aEarnt[SCR_EDataStats.SELINESELF] = StatsGain[SCR_EDataStats.SELINESELF] * m_Configs.MODIFIERSELINESELF;
			m_aEarnt[SCR_EDataStats.SELINEFRIENDLIES] = StatsGain[SCR_EDataStats.SELINEFRIENDLIES] * m_Configs.MODIFIERSELINEFRIENDLIES;
			m_aEarnt[SCR_EDataStats.MORPHINESELF] = StatsGain[SCR_EDataStats.MORPHINESELF] * m_Configs.MODIFIERMORPHINESELF;
			m_aEarnt[SCR_EDataStats.MORPHINEFRIENDLIES] = StatsGain[SCR_EDataStats.MORPHINEFRIENDLIES] * m_Configs.MODIFIERMORPHINEFRIENDLIES;
			
			m_aEarnt[SCR_EDataStats.SPPOINTS2] = m_aEarnt[SCR_EDataStats.BANDAGESELF] + m_aEarnt[SCR_EDataStats.BANDAGEFRIENDLIES] + m_aEarnt[SCR_EDataStats.TOURNIQUETSELF] + m_aEarnt[SCR_EDataStats.TOURNIQUETFRIENDLIES]+ m_aEarnt[SCR_EDataStats.SELINESELF] + m_aEarnt[SCR_EDataStats.SELINEFRIENDLIES] + m_aEarnt[SCR_EDataStats.MORPHINESELF] + m_aEarnt[SCR_EDataStats.MORPHINEFRIENDLIES];
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS2] = 0;
		
		StatsGain[SCR_EDataStats.SPPOINTS2] = m_aEarnt[SCR_EDataStats.SPPOINTS2];
		totalSpecializationPointsGain += StatsGain[SCR_EDataStats.SPPOINTS2];
		
		//NOW ADD INDIVIDUAL WAR CRIMES
		m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES] = m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES] + StatsGain[SCR_EDataStats.FRIENDLYKILLS] * m_Configs.MODIFIERFRIENDLYKILLS + StatsGain[SCR_EDataStats.FRIENDLYAIKILLS]  * m_Configs.MODIFIERFRIENDLYAIKILLS;
		
		//WE PUT THEM TOGETHER TO CALCULATE EARNT WAR CRIMES
		//There's only one war crime at the moment: WARCRIMEHARMINGFRIENDLIES
		m_aStats[SCR_EDataStats.WARCRIMES] = m_aStats[SCR_EDataStats.WARCRIMES] + m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES];
		
		//CALCULATE QUALITYTIME RATIO
		//qualityTimeRatio is calculated as the ratio between (ExpectedPointsPerHour * HoursPlayed) and ObtainedPoints.
		//For expected points, I use m_fSTDPointsQualityTime as the expected growth per specialization and 2.5 as the number of specializations with it as an accurate simplification
		float qualityTimeRatio = totalSpecializationPointsGain / (1+(m_Configs.m_fSTDPointsQualityTime * 2.5 * (StatsGain[SCR_EDataStats.SESSIONDURATION] / 3600)));
		
		//Little assert on the qualityTimeRatio to check player didn't perform abnormaly well
		if (qualityTimeRatio > 1.5)
			Print("SCR_PlayerData:CalculateStatsChange: totalSpecializationPointsGain is suspiciously high. Player with session id " + m_iPlayerID + " gained " + totalSpecializationPointsGain + " points on a session of " + (StatsGain[SCR_EDataStats.SESSIONDURATION] / 60) + " minutes, ending with a qualityTimeRatio of " + qualityTimeRatio, LogLevel.DEBUG);
		else
			Print("SCR_PlayerData:CalculateStatsChange: QualityTimeRatio is: " + qualityTimeRatio, LogLevel.DEBUG);
		
		//LOWER WAR CRIMES IF THERE'S ANY
		if (m_aStats[SCR_EDataStats.WARCRIMES] > 0)
		{
			// WarCrimes go down by m_fWarCrimesDecreaseRatePerHour * qualityTimeRatio * time
			int warCrimesDown = qualityTimeRatio * StatsGain[SCR_EDataStats.SESSIONDURATION]/3600 * m_Configs.WARCRIMESDECREASEPERHOUR;
			
			//All war crimes go down equitatively by amountToDecrease / numberOfNonZeroWarCrimes amount
			int amountToDecrease = (m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES]) * warCrimesDown / m_aStats[SCR_EDataStats.WARCRIMES];
			
			//Divide equitatively all the war crimes. In this case: only 1
			m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES] = m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES] - amountToDecrease / 1;
			m_aStats[SCR_EDataStats.WARCRIMES] = m_aStats[SCR_EDataStats.WARCRIMES] - warCrimesDown;
			
			if (m_aStats[SCR_EDataStats.WARCRIMES] > m_Configs.MAXWARCRIMESVALUE)
				m_aStats[SCR_EDataStats.WARCRIMES] = m_Configs.MAXWARCRIMESVALUE;
			else if (m_aStats[SCR_EDataStats.WARCRIMES] < 0)
				m_aStats[SCR_EDataStats.WARCRIMES] = 0;
		}
		
		//UPDATE THE SPECIALIZATION POINTS
		//HERE we update the specializationPoints
		//---------------------------------------
		
		int i, j;
		int currentSpPoints, gainedRawPoints;
		for (i = 0; i < m_Configs.SPECIALIZATIONS_COUNT; i++)
		{			
			j = 0;
			switch (i)
			{
				case SCR_ECareerSp.INFANTRY:
					gainedRawPoints = StatsGain[SCR_EDataStats.SPPOINTS0];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS0];
				break;
				case SCR_ECareerSp.LOGISTICS:
					gainedRawPoints = StatsGain[SCR_EDataStats.SPPOINTS1];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS1];
				break;
				case SCR_ECareerSp.MEDICAL:
					gainedRawPoints = StatsGain[SCR_EDataStats.SPPOINTS2];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS2];
				break;
				default: continue;
			}
			
			if (currentSpPoints >= m_Configs.SPECIALIZATIONMAX)
				continue;
			
			//Punish the specialization growth if there are war crimes
			if (m_aStats[SCR_EDataStats.WARCRIMES] > 0)
				gainedRawPoints *= (1 - m_Configs.WARCRIMESPUNISHMENT);
			
			while (j < m_Configs.INTERVALS_COUNT && m_Configs.INTERVALS_END[j] < currentSpPoints)
				j++;
			if (j >= m_Configs.INTERVALS_COUNT)
			{
				Print ("SCR_PlayerData:CalculateStatsChange: currentSpPoints outside of the accepted ranges.", LogLevel.ERROR);
				continue;
			}				
			
			//Formula: NewSpecializationPoints = CurrentSpecializationPoints + GainedPoints * c1 * (c2 ^ CurrentSpecializationPoints)
			//m_aStats.m_aSpPoints[i] = m_aStats.m_aSpPoints[i] + rawSpPoints[i] * m_Configs.INTERVALS_C1[j] * (Math.Pow(m_Configs.INTERVALS_C2[j], m_aStats.m_aSpPoints[i]));
			AddPointsToSpecialization(i, gainedRawPoints * m_Configs.INTERVALS_C1[j] * (Math.Pow(m_Configs.INTERVALS_C2[j], currentSpPoints / 10000)));
		}
		
		//LAST BUT NOT LEAST,  INCREASE THE LEVEL OF EXPERIENCE
		//IF player has max level, skip this step
		if (m_aStats[SCR_EDataStats.LEVELEXPERIENCE] >= m_Configs.MAXEXP)
		{
			m_aEarnt[SCR_EDataStats.SESSIONDURATION] = 0;
			m_aEarnt[SCR_EDataStats.LEVELEXPERIENCE] = 0;
			return;
		}
		
		//Calculate gains in level of experience. Currently, we are only using sessionduration and specializationgains
		m_aEarnt[SCR_EDataStats.SESSIONDURATION] = StatsGain[SCR_EDataStats.SESSIONDURATION] * qualityTimeRatio;
		m_aEarnt[SCR_EDataStats.LEVELEXPERIENCE] = m_aEarnt[SCR_EDataStats.SESSIONDURATION] + totalSpecializationPointsGain * m_Configs.MODIFIERSPECIALIZATIONS;
			
		if (m_aStats[SCR_EDataStats.WARCRIMES] > 0)
		{
			m_aEarnt[SCR_EDataStats.LEVELEXPERIENCE] = m_aEarnt[SCR_EDataStats.LEVELEXPERIENCE] * (1 - m_Configs.WARCRIMESPUNISHMENT);
		}
		m_aStats[SCR_EDataStats.LEVELEXPERIENCE] = m_aStats[SCR_EDataStats.LEVELEXPERIENCE] + m_aEarnt[SCR_EDataStats.LEVELEXPERIENCE];
		if (m_aStats[SCR_EDataStats.LEVELEXPERIENCE] > m_Configs.MAXEXP)
			m_aStats[SCR_EDataStats.LEVELEXPERIENCE] = m_Configs.MAXEXP;
			
		m_aStats[SCR_EDataStats.RANK] = m_aStats[SCR_EDataStats.LEVELEXPERIENCE] / 100000;
	}
	
	//------------------------------------------------------------------------------------------------
	array<float> GetArrayEarntPoints()
	{
		return m_aEarnt;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDataReady()
	{
		return !m_aStats.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDataProgressionReady()
	{
		return m_aEarnt && !m_aEarnt.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	void FillArrayWithSpecializationPoints(inout array<float> SpPoints, bool newStats = true)
	{
		for (int i = 0; i < m_Configs.SPECIALIZATIONS_COUNT; i++)
		{
			SpPoints.Insert(GetSpecializationPoints(i, newStats));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	sealed array<float> CalculateStatsDifference()
	{
		array<float> StatsToReturn = {};
		int count = m_aStats.Count();
		
		for (int i = 0; i < count; i++)
		{
			StatsToReturn.Insert(m_aStats[i] - m_aPreviousStats[i]);
		}			
		
		return StatsToReturn;
	}	
	
	//------------------------------------------------------------------------------------------------
	float GetSpecializationPoints(int n, bool newStats = true)
	{
		if (newStats)
		{
			if (!m_aStats || m_aStats.IsEmpty())
				return 0;
			
			switch (n)
			{
				case SCR_ECareerSp.INFANTRY:
					return m_aStats[SCR_EDataStats.SPPOINTS0];
				case SCR_ECareerSp.LOGISTICS:
					return m_aStats[SCR_EDataStats.SPPOINTS1];
				case SCR_ECareerSp.MEDICAL:
					return m_aStats[SCR_EDataStats.SPPOINTS2];
			}
			
			return 0;
		}
		
		if (!m_aPreviousStats || m_aPreviousStats.IsEmpty())
			return 0;
		
		switch (n)
		{
			case SCR_ECareerSp.INFANTRY:
				return m_aPreviousStats[SCR_EDataStats.SPPOINTS0];
			case SCR_ECareerSp.LOGISTICS:
				return m_aPreviousStats[SCR_EDataStats.SPPOINTS1];
			case SCR_ECareerSp.MEDICAL:
				return m_aPreviousStats[SCR_EDataStats.SPPOINTS2];
		}
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddPointsToSpecialization(int n, float value)
	{
		if (!m_aStats || m_aStats.IsEmpty())
			return;
		
		switch (n)
		{
			case SCR_ECareerSp.INFANTRY:
				m_aStats[SCR_EDataStats.SPPOINTS0] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS0] + value, m_Configs.SPECIALIZATIONMAX);
				return;
			case SCR_ECareerSp.LOGISTICS:
				m_aStats[SCR_EDataStats.SPPOINTS1] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS1] + value, m_Configs.SPECIALIZATIONMAX);
				return;
			case SCR_ECareerSp.MEDICAL:
				m_aStats[SCR_EDataStats.SPPOINTS2] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS2] + value, m_Configs.SPECIALIZATIONMAX);
				return;
		}
		Print ("SCR_PlayerData:AddPointsToSpecialization: WRONG SPECIALIZATION ID.", LogLevel.ERROR);
	}
	
	//This method sets the SCR_PlayerDataSpecializationDisplay arrays from the PlayerDataConfigs. Typically the Career Menu will use this
	//------------------------------------------------------------------------------------------------
	void PrepareSpecializationStatsDisplay()
	{
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.METERSWALKED].SetValue(GetDistanceWalked() / 1000);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.PLAYERKILLS].SetValue(GetPlayerKills());
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.AIKILLS].SetValue(GetAIKills());
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.SHOTS].SetValue(GetBulletsShot());
		
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERSDRIVEN].SetValue(GetDistanceDriven() / 1000);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERSASOCCUPANT].SetValue(GetDistanceAsOccupant() / 1000);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.POINTSDRIVINGPEOPLE].SetValue(GetPointsAsDriverOfPlayers());
		
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGESELF].SetValue(GetBandageSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGEFRIENDLIES].SetValue(GetBandageFriends());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUETSELF].SetValue(GetTourniquetSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUETFRIENDLIES].SetValue(GetTourniquetFriends());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SELINESELF].SetValue(GetSelineSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SELINEFRIENDLIES].SetValue(GetSelineFriends());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINESELF].SetValue(GetMorphineSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINEFRIENDLIES].SetValue(GetMorphineFriends());		
	}
	
	//This method sets the SCR_PlayerDataSpecializationDisplay arrays from the PlayerDataConfigs. Typically the Debrief screen will use this
	void PrepareSpecializationProgressionStatsDisplay()
	{
		array<float> statsGain = CalculateStatsDifference();
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.METERSWALKED].SetValue(statsGain[SCR_EDataStats.METERSWALKED] / 1000);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.PLAYERKILLS].SetValue(statsGain[SCR_EDataStats.KILLS]);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.AIKILLS].SetValue(statsGain[SCR_EDataStats.AIKILLS]);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.SHOTS].SetValue(statsGain[SCR_EDataStats.SHOTS]);
		
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERSDRIVEN].SetValue(statsGain[SCR_EDataStats.METERSDRIVEN] / 1000);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERSASOCCUPANT].SetValue(statsGain[SCR_EDataStats.METERSASOCCUPANT] / 1000);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.POINTSDRIVINGPEOPLE].SetValue(statsGain[SCR_EDataStats.POINTSASDRIVEROFPLAYERS]);
		
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGESELF].SetValue(statsGain[SCR_EDataStats.BANDAGESELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGEFRIENDLIES].SetValue(statsGain[SCR_EDataStats.BANDAGEFRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUETSELF].SetValue(statsGain[SCR_EDataStats.TOURNIQUETSELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUETFRIENDLIES].SetValue(statsGain[SCR_EDataStats.TOURNIQUETFRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SELINESELF].SetValue(statsGain[SCR_EDataStats.SELINESELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SELINEFRIENDLIES].SetValue(statsGain[SCR_EDataStats.SELINEFRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINESELF].SetValue(statsGain[SCR_EDataStats.MORPHINESELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINEFRIENDLIES].SetValue(statsGain[SCR_EDataStats.MORPHINEFRIENDLIES]);
	}
	
	//m_aStats and m_aPreviousStats must not be accessed from outside of this class.
	//That is why there are getters and setters for their attributes, but not for their instances
	//
	//
	//
	//SETTERS AND GETTERS:
	
	/******************/
	//Rank and Generic//
	/******************/
	
	//------------------------------------------------------------------------------------------------
	float GetLevelExperience(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.LEVELEXPERIENCE];
		
		return m_aPreviousStats[SCR_EDataStats.LEVELEXPERIENCE];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRank(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.RANK];
		
		return m_aPreviousStats[SCR_EDataStats.RANK];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSessionDuration(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SESSIONDURATION];
		
			return m_aPreviousStats[SCR_EDataStats.SESSIONDURATION];
	}
	
	/*****************/
	//Specializations//
	/*****************/
	
	//------------------------------------------------------------------------------------------------
	float GetSpecializationCount(int n)
	{		
		switch (n)
		{
			case 0: return m_Configs.SPECIALIZATION_0_COUNT;
			case 1: return m_Configs.SPECIALIZATION_1_COUNT;
			case 2: return m_Configs.SPECIALIZATION_2_COUNT;
		}
		return -1;
	}
	
	/************/
	//War Crimes//
	/************/
	
	//------------------------------------------------------------------------------------------------
	float GetWarCrimesAmount(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.WARCRIMES];
		
		return m_aPreviousStats[SCR_EDataStats.WARCRIMES]; 
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetWarCrimes(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.WARCRIMES] != 0;
		
		return m_aPreviousStats[SCR_EDataStats.WARCRIMES] != 0; 
	}
	
	//------------------------------------------------------------------------------------------------
	int GetWarCrimesHarmingFriendlies(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES];
		
		return m_aPreviousStats[SCR_EDataStats.WARCRIMEHARMINGFRIENDLIES];
	}
	
	/**********************/
	//DATACOLLECTORMODULES//
	/**********************/
		
	/********************/
	//BasicActionsModule//
	/********************/
	
	//------------------------------------------------------------------------------------------------
	int GetDistanceWalked(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.METERSWALKED];
		
		return m_aPreviousStats[SCR_EDataStats.METERSWALKED];
	}
	
	//------------------------------------------------------------------------------------------------
	void AddDistanceWalked(float meters)
	{
		m_aStats[SCR_EDataStats.METERSWALKED] = m_aStats[SCR_EDataStats.METERSWALKED] + meters;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddSessionDuration(float timeslice)
	{
		m_aStats[SCR_EDataStats.SESSIONDURATION] = m_aStats[SCR_EDataStats.SESSIONDURATION] + timeslice;
	}
	
	/****************/
	//ShootingModule//
	/****************/
		
	//------------------------------------------------------------------------------------------------
	void AddKill(bool friendly)
	{
		if (friendly)
			m_aStats[SCR_EDataStats.FRIENDLYKILLS] = m_aStats[SCR_EDataStats.FRIENDLYKILLS] + 1;
		
		else
			m_aStats[SCR_EDataStats.KILLS] = m_aStats[SCR_EDataStats.KILLS] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddRoadKill(bool friendly)
	{
		if (friendly)
			m_aStats[SCR_EDataStats.FRIENDLYROADKILLS] = m_aStats[SCR_EDataStats.FRIENDLYROADKILLS] + 1;
		
		else
			m_aStats[SCR_EDataStats.ROADKILLS] = m_aStats[SCR_EDataStats.ROADKILLS] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAIKill(bool friendly)
	{
		if (friendly)
			m_aStats[SCR_EDataStats.FRIENDLYAIKILLS] = m_aStats[SCR_EDataStats.FRIENDLYAIKILLS] + 1;
		
		else
			m_aStats[SCR_EDataStats.AIKILLS] = m_aStats[SCR_EDataStats.AIKILLS] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddAIRoadKill(bool friendly)
	{
		if (friendly)
			m_aStats[SCR_EDataStats.FRIENDLYAIROADKILLS] = m_aStats[SCR_EDataStats.FRIENDLYAIROADKILLS] + 1;
		
		else
			m_aStats[SCR_EDataStats.AIROADKILLS] = m_aStats[SCR_EDataStats.AIROADKILLS] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddDeath()
	{
		m_aStats[SCR_EDataStats.DEATHS] = m_aStats[SCR_EDataStats.DEATHS] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddShot()
	{
		m_aStats[SCR_EDataStats.SHOTS] = m_aStats[SCR_EDataStats.SHOTS] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddShots(float n)
	{
		m_aStats[SCR_EDataStats.SHOTS] = m_aStats[SCR_EDataStats.SHOTS] + n;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddGrenadeThrown()
	{
		m_aStats[SCR_EDataStats.GRENADESTHROWN] = m_aStats[SCR_EDataStats.GRENADESTHROWN] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDeaths(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.DEATHS];
		
		return m_aPreviousStats[SCR_EDataStats.DEATHS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPlayerKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.KILLS];
		
		return m_aPreviousStats[SCR_EDataStats.KILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAIKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.AIKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.AIKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFriendlyPlayerKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLYKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.FRIENDLYKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFriendlyAIKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLYAIKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.FRIENDLYAIKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetBulletsShot(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SHOTS];
		
		return m_aPreviousStats[SCR_EDataStats.SHOTS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetGrenadesThrown(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.GRENADESTHROWN];
		
		return m_aPreviousStats[SCR_EDataStats.GRENADESTHROWN];
	}
	
	/****************/
	//DriverModule//
	/****************/
		
	//------------------------------------------------------------------------------------------------
	void AddMetersDriven(float meters)
	{
		m_aStats[SCR_EDataStats.METERSDRIVEN] = m_aStats[SCR_EDataStats.METERSDRIVEN] + meters;
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculatePointsAsDriver(float meters, float players)
	{
		m_aStats[SCR_EDataStats.POINTSASDRIVEROFPLAYERS] = m_aStats[SCR_EDataStats.POINTSASDRIVEROFPLAYERS] + meters * players * m_Configs.MODIFIERDRIVEROFPLAYERS;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddPlayersDiedInVehicle (float count)
	{
		m_aStats[SCR_EDataStats.PLAYERSDIEDINVEHICLE] = m_aStats[SCR_EDataStats.PLAYERSDIEDINVEHICLE] + count;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddMetersAsOccupant(float meters)
	{
		m_aStats[SCR_EDataStats.METERSASOCCUPANT] = m_aStats[SCR_EDataStats.METERSASOCCUPANT] + meters;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPointsAsDriverOfPlayers(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.POINTSASDRIVEROFPLAYERS];
		
		return m_aPreviousStats[SCR_EDataStats.POINTSASDRIVEROFPLAYERS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDistanceDriven(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.METERSDRIVEN];
		
		return m_aPreviousStats[SCR_EDataStats.METERSDRIVEN];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDistanceAsOccupant(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.METERSASOCCUPANT];
		
		return m_aPreviousStats[SCR_EDataStats.METERSASOCCUPANT];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.ROADKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.ROADKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetAIRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.AIROADKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.AIROADKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFriendlyRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLYROADKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.FRIENDLYROADKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetFriendlyAIRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLYAIROADKILLS];
		
		return m_aPreviousStats[SCR_EDataStats.FRIENDLYAIROADKILLS];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetPlayersDiedInVehicle(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.PLAYERSDIEDINVEHICLE];
		
		return m_aPreviousStats[SCR_EDataStats.PLAYERSDIEDINVEHICLE];
	}
	
	/*************************/
	//SupplyTruckDriverModule//
	/*************************/
	
	/*
	//------------------------------------------------------------------------------------------------
	void AddTraveledTimeSupplyVehicle(float time)
	{
		m_aStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE] = m_aStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE] + time;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddTraveledDistanceSupplyVehicle(float distance)
	{
		m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE]  = m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE] + distance;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTraveledDistanceSupplyVehicle(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE];
		
		return m_aPreviousStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE];
	}
	
	//------------------------------------------------------------------------------------------------
	float GetTraveledTimeSupplyVehicle(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE];
		
		return m_aPreviousStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE];
	}
	
	*/
	
	/********************/
	//HealingItemsModule//
	/********************/
	
	//------------------------------------------------------------------------------------------------
	void AddBandageUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.BANDAGESELF] = m_aStats[SCR_EDataStats.BANDAGESELF]+1;
		else
			m_aStats[SCR_EDataStats.BANDAGEFRIENDLIES] = m_aStats[SCR_EDataStats.BANDAGEFRIENDLIES] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddTourniquetUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.TOURNIQUETSELF] = m_aStats[SCR_EDataStats.TOURNIQUETSELF] + 1;
		else
			m_aStats[SCR_EDataStats.TOURNIQUETFRIENDLIES] = m_aStats[SCR_EDataStats.TOURNIQUETFRIENDLIES] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddSalineUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.SELINESELF] = m_aStats[SCR_EDataStats.SELINESELF] + 1;
		else
			m_aStats[SCR_EDataStats.SELINEFRIENDLIES] = m_aStats[SCR_EDataStats.SELINEFRIENDLIES] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddMorphineUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.MORPHINESELF] = m_aStats[SCR_EDataStats.MORPHINESELF] + 1;
		else
			m_aStats[SCR_EDataStats.MORPHINEFRIENDLIES] = m_aStats[SCR_EDataStats.MORPHINEFRIENDLIES] + 1;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBandageFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.BANDAGEFRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.BANDAGEFRIENDLIES];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetBandageSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.BANDAGESELF];
		return m_aPreviousStats[SCR_EDataStats.BANDAGESELF];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTourniquetFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TOURNIQUETFRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.TOURNIQUETFRIENDLIES];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTourniquetSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TOURNIQUETSELF];
		return m_aPreviousStats[SCR_EDataStats.TOURNIQUETSELF];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSelineFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SELINEFRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.SELINEFRIENDLIES];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSelineSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SELINESELF];
		return m_aPreviousStats[SCR_EDataStats.SELINESELF];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMorphineFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.MORPHINEFRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.MORPHINEFRIENDLIES];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMorphineSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.MORPHINESELF];
		return m_aPreviousStats[SCR_EDataStats.MORPHINESELF];
	}
};

//------------------------------------------------------------------------------------------------
class CharacterDataLoadingCallback : BackendCallback
{
	
	SCR_PlayerData m_PlayerDataInstance;
	
	void CharacterDataLoadingCallback(SCR_PlayerData instance)
	{
		m_PlayerDataInstance = instance;
	}
	
	/**
	\brief Request finished with error result
	\param code Error code is type of EBackendError
	*/
	override void OnError(int code, int restCode, int apiCode)
	{
		Print("[CharacterDataLoadingCallback] OnError: "+ GetGame().GetBackendApi().GetErrorCode(code), LogLevel.ERROR);
		m_PlayerDataInstance.LoadEmptyProfile();
	}

	/**
	\brief Request finished with success result
	\param code Code is type of EBackendRequest
	*/
	override void OnSuccess(int code)
	{
		Print("[CharacterDataLoadingCallback] OnSuccess.", LogLevel.DEBUG);
		m_PlayerDataInstance.BackendDataReady();
	}

	/**
	\brief Request not finished due to timeout
	*/
	override void OnTimeout()
	{
		Print("[CharacterDataLoadingCallback] OnTimeout", LogLevel.ERROR);
		m_PlayerDataInstance.LoadEmptyProfile();
	}
};

//DO NOT CHANGE THE ORDER OF THE ITEMS OF THE ENUM. (!!!) WE USE THE INDEX AS AN ID IN THE PLAYERPROFILE'S CHARACTERDATA
//------------------------------------------------------------------------------------------------
enum SCR_EDataStats
{
	RANK,
	LEVELEXPERIENCE,
	SESSIONDURATION,
	SPPOINTS0, //INFANTRY
	SPPOINTS1, //LOGISTICS
	SPPOINTS2, //MEDICAL
	WARCRIMES,
	METERSWALKED,
	KILLS,
	AIKILLS,
	SHOTS,
	GRENADESTHROWN,
	FRIENDLYKILLS,
	FRIENDLYAIKILLS,
	DEATHS,
	METERSDRIVEN,
	POINTSASDRIVEROFPLAYERS,
	PLAYERSDIEDINVEHICLE,
	ROADKILLS,
	FRIENDLYROADKILLS,
	AIROADKILLS,
	FRIENDLYAIROADKILLS,
	METERSASOCCUPANT,
	BANDAGESELF,
	BANDAGEFRIENDLIES,
	TOURNIQUETSELF,
	TOURNIQUETFRIENDLIES,
	SELINESELF,
	SELINEFRIENDLIES,
	MORPHINESELF,
	MORPHINEFRIENDLIES,
	WARCRIMEHARMINGFRIENDLIES //Harming friendlies
};