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
	
	/*TODO: Finish development of Proportionality principle
	//Possibly criminal stats - We use this to accumulate actions and afterwards decide whether they are criminal or not.
	protected ref array<float> m_aAccumulatedActions = {};
	protected float m_fTimerAccumulateActions = 0;
	*/

	//Stats difference - We need this variable because we need a pointer for it to be used on TD from C++ side
	protected ref array<float> m_aStatsGained = {};

	//DataEvent to send stats to the database for tracking purposes
	ref SCR_PlayerDataEvent dataEvent = new SCR_PlayerDataEvent;

	//We store in m_aEarnt the number of points provided by each of the fields to the specializations, and the total sum for each specialization in the corresponding field (sp0, sp1, etc)
	protected ref array<float> m_aEarnt = {};

	//Backend callbacks necessary to make the request to load and store the characterdata from the player's profile
	protected ref CharacterDataLoadingCallback m_CharacterDataCallback = new CharacterDataLoadingCallback(this);
	protected ref BackendCallback m_StoringCallback = new BackendCallback();

	//Starting session tick to calculate session duration
	protected int m_iSessionStartedTickCount;

	//Empty Profile
	protected bool m_bIsEmptyProfile;

	//Invoker that notifies UI screens listening to it that the data object is ready
	protected ref ScriptInvoker m_OnDataReady = new ScriptInvoker();

	/*********************/
	//STATS CONFIGURATION//
	/*********************/

	protected ref SCR_PlayerDataConfigs m_Configs;
	/* TODO: Finish rework of player data. These two will go to the DataCollector or to a Module from the data collector instead */
	protected bool m_bWarCrimesEnabled, m_bWarCrimesProportionalityPrincipleEnabled;

	//------------------------------------------------------------------------------------------------
	void SCR_PlayerData (int id, bool hasPlayerBeenAuditted, bool requestFromBackend = true)
	{
		if (!GetGame().GetBackendApi())
			return;

		RegV("m_aStats");

		m_Configs = SCR_PlayerDataConfigs.GetInstance();
		m_iPlayerID = id;
		
		m_iSessionStartedTickCount = System.GetTickCount();

		if (requestFromBackend)
			RequestLoadData(hasPlayerBeenAuditted);
		else if (!hasPlayerBeenAuditted)
			LoadEmptyProfile();
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
			m_bIsEmptyProfile = true;
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
			m_bIsEmptyProfile = true;
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
			Print("SCR_PlayerData:RequestLoadData: Requesting data before player has been authenticated!", LogLevel.ERROR);
			m_bIsEmptyProfile = true;
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
		typename characterDataEnum = SCR_EDataStats;

		if (m_aStats.Count() != characterDataEnum.GetVariableCount())
		{
			Print("SCR:PlayerDataStats:FillWithProfile: The CharacterData from this player seems incomplete or empty. A full profile should have "+characterDataEnum.GetVariableCount()+" stats, but this one has "+m_aStats.Count()+". Is it a new player?", LogLevel.WARNING);
			LoadEmptyProfile();
		}
		else
		{
			m_bIsEmptyProfile = false;
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
			if (i == SCR_EDataStats.LEVEL_EXPERIENCE)
				m_aStats.Insert(m_Configs.XP_NEEDED_FOR_LEVEL);
			else
				m_aStats.Insert(0);
		}

		m_bIsEmptyProfile = true;

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

		//SCR_PlayerDataEvent dataEvent = new SCR_PlayerDataEvent();
		//dataEvent.MetersWalked = 99999;
		//GetGame().GetStatsApi().PlayerEvent(m_iPlayerID, dataEvent);
		//Print("Data event sent from Store Profile!!", LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	Managed GetDataEventStats()
	{
		if (!IsDataProgressionReady() || m_aStatsGained.IsEmpty())
		{
			Print("Error: DataProgression is not ready but the TD event was requested", LogLevel.ERROR);
			return null;
		}

		dataEvent.amt_time_mission = m_aStatsGained[SCR_EDataStats.SESSION_DURATION];
		dataEvent.amt_distance_on_foot = m_aStatsGained[SCR_EDataStats.METERS_WALKED];
		dataEvent.amt_distance_vehicle = m_aStatsGained[SCR_EDataStats.METERS_DRIVEN];
		dataEvent.kills = m_aStatsGained[SCR_EDataStats.KILLS];
		//dataEvent.kills_from_over_25m;
		//dataEvent.kills_from_over_50m;
		//dataEvent.kills_from_over_100m;
		dataEvent.friendly_kills = m_aStatsGained[SCR_EDataStats.FRIENDLY_KILLS];
		//dataEvent.friendly_kills_from_over_25m;
		//dataEvent.friendly_kills_from_over_50m;
		//dataEvent.friendly_kills_from_over_100m;
		dataEvent.vehicle_kills = m_aStatsGained[SCR_EDataStats.ROADKILLS] + m_aStatsGained[SCR_EDataStats.AI_ROADKILLS];
		dataEvent.friendly_vehicle_kills = m_aStatsGained[SCR_EDataStats.FRIENDLY_ROADKILLS] + m_aStatsGained[SCR_EDataStats.FRIENDLY_AI_ROADKILLS];
		dataEvent.deaths = m_aStatsGained[SCR_EDataStats.DEATHS];
		//dataEvent.deaths_by_bleeding_out;
		//dataEvent.deaths_by_falling;
		dataEvent.shots = m_aStatsGained[SCR_EDataStats.SHOTS];
		//dataEvent.hits;
		//dataEvent.hits_from_over_25m;
		//dataEvent.hits_from_over_50m;
		//dataEvent.hits_from_over_100m;
		dataEvent.healing_allies = m_aStatsGained[SCR_EDataStats.BANDAGE_FRIENDLIES]; //Careful
		dataEvent.healing_self = m_aStatsGained[SCR_EDataStats.BANDAGE_SELF]; //Careful
		//dataEvent.weapon1_id;
		//dataEvent.weapon1_seconds;
		//dataEvent.weapon1_shots;
		//dataEvent.weapon1_hits;
		//dataEvent.weapon2_id;
		//dataEvent.weapon2_seconds;
		//dataEvent.weapon2_shots;
		//dataEvent.weapon2_hits;
		//dataEvent.weapon3_id;
		//dataEvent.weapon3_seconds;
		//dataEvent.weapon3_shots;
		//dataEvent.weapon3_hits;
		//dataEvent.setbase_coordinates;
		dataEvent.rank_points = m_aStatsGained[SCR_EDataStats.LEVEL_EXPERIENCE];
		dataEvent.rank_total_points = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE];
		dataEvent.warcrimes_points = m_aStatsGained[SCR_EDataStats.WARCRIMES];
		dataEvent.warcrimes_total_points = m_aStats[SCR_EDataStats.WARCRIMES];
		dataEvent.rank_level = m_aStatsGained[SCR_EDataStats.RANK];
		dataEvent.gt1_points = m_aStatsGained[SCR_EDataStats.SPPOINTS0];
		dataEvent.gt1_total_points = m_aStats[SCR_EDataStats.SPPOINTS0];
		dataEvent.gt2_points = m_aStatsGained[SCR_EDataStats.SPPOINTS1];
		dataEvent.gt2_total_points = m_aStats[SCR_EDataStats.SPPOINTS1];
		dataEvent.gt3_points = m_aStatsGained[SCR_EDataStats.SPPOINTS2];
		dataEvent.gt3_total_points = m_aStats[SCR_EDataStats.SPPOINTS2];
		dataEvent.gt_global_points = ( m_aStatsGained[SCR_EDataStats.SPPOINTS0] + m_aStatsGained[SCR_EDataStats.SPPOINTS1] + m_aStatsGained[SCR_EDataStats.SPPOINTS2] );

		return dataEvent;
	}

	//------------------------------------------------------------------------------------------------
	sealed void DebugCalculateStats()
	{
		Print("SCR_PlayerData:DebugCalculateStats: This method calls calculateStatsChange providing fake stats for debugging purposes.", LogLevel.DEBUG);

		m_aStats[SCR_EDataStats.SESSION_DURATION] = m_aStats[SCR_EDataStats.SESSION_DURATION] + 7200;
		m_aStats[SCR_EDataStats.METERS_WALKED] = m_aStats[SCR_EDataStats.METERS_WALKED] + 20000;
		m_aStats[SCR_EDataStats.KILLS] = m_aStats[SCR_EDataStats.KILLS] + 20;
		m_aStats[SCR_EDataStats.AI_KILLS] = m_aStats[SCR_EDataStats.AI_KILLS] + 50;
		m_aStats[SCR_EDataStats.SHOTS] = m_aStats[SCR_EDataStats.SHOTS] + 800;
		m_aStats[SCR_EDataStats.GRENADES_THROWN] = m_aStats[SCR_EDataStats.GRENADES_THROWN] + 10;
		m_aStats[SCR_EDataStats.METERS_DRIVEN] = m_aStats[SCR_EDataStats.METERS_DRIVEN] + 50000;
		m_aStats[SCR_EDataStats.FRIENDLY_KILLS] = m_aStats[SCR_EDataStats.FRIENDLY_KILLS] + 30;

		m_aStats[SCR_EDataStats.BANDAGE_SELF] = m_aStats[SCR_EDataStats.BANDAGE_SELF] + 30;
		m_aStats[SCR_EDataStats.BANDAGE_FRIENDLIES] = m_aStats[SCR_EDataStats.BANDAGE_FRIENDLIES] + 10;
		m_aStats[SCR_EDataStats.TOURNIQUET_SELF] = m_aStats[SCR_EDataStats.TOURNIQUET_SELF] + 30;
		m_aStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES] = m_aStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES] + 10;
		m_aStats[SCR_EDataStats.SALINE_SELF] = m_aStats[SCR_EDataStats.SALINE_SELF] + 30;
		m_aStats[SCR_EDataStats.SALINE_FRIENDLIES] = m_aStats[SCR_EDataStats.SALINE_FRIENDLIES] + 10;
		m_aStats[SCR_EDataStats.MORPHINE_SELF] = m_aStats[SCR_EDataStats.MORPHINE_SELF] + 30;
		m_aStats[SCR_EDataStats.MORPHINE_FRIENDLIES] = m_aStats[SCR_EDataStats.MORPHINE_FRIENDLIES] + 10;
		if (!IsDataProgressionReady())
			CalculateStatsChange();
	}

	//------------------------------------------------------------------------------------------------
	sealed void CalculateStatsChange()
	{
		//We find the differences provided by this session
		//In order to calculate Rank, Specializations, and War Crime points
		m_aStatsGained = CalculateStatsDifference();

		for (int i = m_aStatsGained.Count()-1; i >= 0; i--)
			m_aEarnt.Insert(0);

		if (m_aStatsGained[SCR_EDataStats.SESSION_DURATION] < 1)
			return;

		//Little assert
		if (m_aStatsGained[SCR_EDataStats.SPPOINTS0] != 0 || m_aStatsGained[SCR_EDataStats.SPPOINTS1] != 0 || m_aStatsGained[SCR_EDataStats.SPPOINTS2] != 0|| m_aStatsGained[SCR_EDataStats.RANK] != 0 || m_aStatsGained[SCR_EDataStats.LEVEL_EXPERIENCE] != 0)
			Print("SCR_PlayerData:CalculateStatsChange: Specialization stats, experience or rank stats were modified prematurely. The career data for this session might be compromised", LogLevel.ERROR);

		//CALCULATE STATS ADDITIONS TO THE SPECIALIZATIONS, THEN WAR CRIMES, THEN SPECIALIZATION POINTS, AND THEN LEVEL OF EXPERIENCE AND RANK

		//Specializations: There are three: Infantry, Logistics and Medical
		int totalSpecializationPointsGain = 0;
		//
		//
		//Infantry
		if (m_aStats[SCR_EDataStats.SPPOINTS0] < m_Configs.SPECIALIZATION_MAX)
		{
			m_aEarnt[SCR_EDataStats.METERS_WALKED] = m_aStatsGained[SCR_EDataStats.METERS_WALKED] * m_Configs.MODIFIER_METERS_WALKED;
			m_aEarnt[SCR_EDataStats.KILLS] = m_aStatsGained[SCR_EDataStats.KILLS] * m_Configs.MODIFIER_KILLS;
			m_aEarnt[SCR_EDataStats.AI_KILLS] = m_aStatsGained[SCR_EDataStats.AI_KILLS] * m_Configs.MODIFIER_AI_KILLS;

			if (m_aStatsGained[SCR_EDataStats.SHOTS] <= 0)
				m_aEarnt[SCR_EDataStats.SHOTS] = 0;
			else
				//We should not use ROADKILLS yet
				//Also this title is misleading: We are treating shots as "precision in killing" of sorts
				m_aEarnt[SCR_EDataStats.SHOTS] = ( (m_aStatsGained[SCR_EDataStats.KILLS] /*- m_aStatsGained[SCR_EDataStats.ROADKILLS]*/ + m_aStatsGained[SCR_EDataStats.AI_KILLS] /*- m_aStatsGained[SCR_EDataStats.AIROADKILLS]*/) / m_aStatsGained[SCR_EDataStats.SHOTS] ) * m_Configs.MODIFIER_PRECISION;

			/* We should not use GRENADESTHROWN yet
			if (m_aStatsGained[SCR_EDataStats.GRENADESTHROWN] <= 0)
				m_aEarnt[SCR_EDataStats.GRENADESTHROWN] = 0;
			else
				m_aEarnt[SCR_EDataStats.GRENADESTHROWN] = (m_aStatsGained[SCR_EDataStats.KILLS] / m_aStatsGained[SCR_EDataStats.GRENADESTHROWN]) * m_Configs.MODIFIERPRECISION;
			*/

			m_aEarnt[SCR_EDataStats.SPPOINTS0] = m_aEarnt[SCR_EDataStats.METERS_WALKED] + m_aEarnt[SCR_EDataStats.KILLS] + m_aEarnt[SCR_EDataStats.AI_KILLS] + m_aEarnt[SCR_EDataStats.SHOTS]; /*+ m_aEarnt[SCR_EDataStats.GRENADES_THROWN];*/
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS0] = 0;

		m_aStatsGained[SCR_EDataStats.SPPOINTS0] = m_aEarnt[SCR_EDataStats.SPPOINTS0];
		totalSpecializationPointsGain += m_aStatsGained[SCR_EDataStats.SPPOINTS0];

		//
		//
		//Logistics
		if (m_aStats[SCR_EDataStats.SPPOINTS1] < m_Configs.SPECIALIZATION_MAX)
		{
			m_aEarnt[SCR_EDataStats.METERS_DRIVEN] = m_aStatsGained[SCR_EDataStats.METERS_DRIVEN] * m_Configs.MODIFIER_METERS_DRIVEN;
			m_aEarnt[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] = m_aStatsGained[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS];

			m_aEarnt[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE] = -1 * Math.Min(m_aStatsGained[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE] * m_Configs.MODIFIER_PLAYERS_DIED_IN_VEHICLE, m_Configs.MAX_PLAYERS_DIED_IN_VEHICLE_PENALTY);

			m_aEarnt[SCR_EDataStats.SPPOINTS1] = m_aEarnt[SCR_EDataStats.METERS_DRIVEN] + m_aEarnt[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] + m_aEarnt[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE];

			if (m_aEarnt[SCR_EDataStats.SPPOINTS1] <= 0)
				m_aEarnt[SCR_EDataStats.SPPOINTS1] = 0;
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS1] = 0;

		m_aStatsGained[SCR_EDataStats.SPPOINTS1] = m_aEarnt[SCR_EDataStats.SPPOINTS1];
		totalSpecializationPointsGain += m_aStatsGained[SCR_EDataStats.SPPOINTS1];

		//
		//
		//Medical
		if (m_aStats[SCR_EDataStats.SPPOINTS2] < m_Configs.SPECIALIZATION_MAX)
		{
			m_aEarnt[SCR_EDataStats.BANDAGE_SELF] = m_aStatsGained[SCR_EDataStats.BANDAGE_SELF] * m_Configs.MODIFIER_BANDAGE_SELF;
			m_aEarnt[SCR_EDataStats.BANDAGE_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.BANDAGE_FRIENDLIES] * m_Configs.MODIFIER_BANDAGE_FRIENDLIES;
			m_aEarnt[SCR_EDataStats.TOURNIQUET_SELF] = m_aStatsGained[SCR_EDataStats.TOURNIQUET_SELF] * m_Configs.MODIFIER_TOURNIQUET_SELF;
			m_aEarnt[SCR_EDataStats.TOURNIQUET_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.TOURNIQUET_FRIENDLIES] * m_Configs.MODIFIER_TOURNIQUET_FRIENDLIES;
			m_aEarnt[SCR_EDataStats.SALINE_SELF] = m_aStatsGained[SCR_EDataStats.SALINE_SELF] * m_Configs.MODIFIER_SALINE_SELF;
			m_aEarnt[SCR_EDataStats.SALINE_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.SALINE_FRIENDLIES] * m_Configs.MODIFIER_SALINE_FRIENDLIES;
			m_aEarnt[SCR_EDataStats.MORPHINE_SELF] = m_aStatsGained[SCR_EDataStats.MORPHINE_SELF] * m_Configs.MODIFIER_MORPHINE_SELF;
			m_aEarnt[SCR_EDataStats.MORPHINE_FRIENDLIES] = m_aStatsGained[SCR_EDataStats.MORPHINE_FRIENDLIES] * m_Configs.MODIFIER_MORPHINE_FRIENDLIES;

			m_aEarnt[SCR_EDataStats.SPPOINTS2] = m_aEarnt[SCR_EDataStats.BANDAGE_SELF] + m_aEarnt[SCR_EDataStats.BANDAGE_FRIENDLIES] + m_aEarnt[SCR_EDataStats.TOURNIQUET_SELF] + m_aEarnt[SCR_EDataStats.TOURNIQUET_FRIENDLIES]+ m_aEarnt[SCR_EDataStats.SALINE_SELF] + m_aEarnt[SCR_EDataStats.SALINE_FRIENDLIES] + m_aEarnt[SCR_EDataStats.MORPHINE_SELF] + m_aEarnt[SCR_EDataStats.MORPHINE_FRIENDLIES];
		}
		else
			m_aEarnt[SCR_EDataStats.SPPOINTS2] = 0;

		m_aStatsGained[SCR_EDataStats.SPPOINTS2] = m_aEarnt[SCR_EDataStats.SPPOINTS2];
		totalSpecializationPointsGain += m_aStatsGained[SCR_EDataStats.SPPOINTS2];

		if (m_bWarCrimesEnabled)
		{
			//NOW ADD INDIVIDUAL WAR CRIMES
			m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES] = m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES] + m_aStatsGained[SCR_EDataStats.FRIENDLY_KILLS] * m_Configs.MODIFIER_FRIENDLY_KILLS + m_aStatsGained[SCR_EDataStats.FRIENDLY_AI_KILLS] * m_Configs.MODIFIER_FRIENDLY_AI_KILLS;

			//WE PUT THEM TOGETHER TO CALCULATE EARNT WAR CRIMES
			//There's only one war crime at the moment: WARCRIMEHARMINGFRIENDLIES
			m_aStats[SCR_EDataStats.WARCRIMES] = m_aStats[SCR_EDataStats.WARCRIMES] + m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES];
		}
		
		//CALCULATE QUALITYTIME RATIO
		//qualityTimeRatio is calculated as the ratio between (ExpectedPointsPerHour * HoursPlayed) and ObtainedPoints.
		//For expected points, I use m_fSTDPointsQualityTime as the expected growth per specialization and 2.5 as the number of specializations with it as an accurate simplification
		float qualityTimeRatio = totalSpecializationPointsGain / (1+(m_Configs.s_fSTDPointsQualityTime * 2.5 * (m_aStatsGained[SCR_EDataStats.SESSION_DURATION] / 3600)));

		//Little assert on the qualityTimeRatio to check player didn't perform abnormaly well
		if (qualityTimeRatio > 1.5)
			Print("SCR_PlayerData:CalculateStatsChange: totalSpecializationPointsGain is suspiciously high. That means player performed better than we expected which could point an an oversight from our side, or even a player cheating. Player with session id " + m_iPlayerID + " gained " + totalSpecializationPointsGain + " points on a session of " + (m_aStatsGained[SCR_EDataStats.SESSION_DURATION] / 60) + " minutes, ending with a qualityTimeRatio of " + qualityTimeRatio, LogLevel.WARNING);
		else
			Print("SCR_PlayerData:CalculateStatsChange: QualityTimeRatio (aka how well player performed) is: " + qualityTimeRatio, LogLevel.DEBUG);

		//LOWER WAR CRIMES IF THERE'S ANY
		if (m_bWarCrimesEnabled && m_aStats[SCR_EDataStats.WARCRIMES] > 0)
		{
			// WarCrimes go down by m_fWarCrimesDecreaseRatePerHour * qualityTimeRatio * time
			int warCrimesDown = qualityTimeRatio * m_aStatsGained[SCR_EDataStats.SESSION_DURATION]/3600 * m_Configs.WARCRIMES_DECREASE_PER_HOUR;

			//All war crimes go down equitatively by amountToDecrease / numberOfNonZeroWarCrimes amount
			int amountToDecrease = (m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES]) * warCrimesDown / m_aStats[SCR_EDataStats.WARCRIMES];

			//Divide equitatively all the war crimes. In this case: only 1
			m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES] = m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES] - amountToDecrease / 1;
			m_aStats[SCR_EDataStats.WARCRIMES] = m_aStats[SCR_EDataStats.WARCRIMES] - warCrimesDown;

			if (m_aStats[SCR_EDataStats.WARCRIMES] > m_Configs.MAX_WARCRIMES_VALUE)
				m_aStats[SCR_EDataStats.WARCRIMES] = m_Configs.MAX_WARCRIMES_VALUE;
			else if (m_aStats[SCR_EDataStats.WARCRIMES] < 0)
				m_aStats[SCR_EDataStats.WARCRIMES] = 0;
		}

		//UPDATE THE SPECIALIZATION POINTS
		//HERE we update the specializationPoints
		//------------------------------------------------------------------------------------------------

		int i, j;
		int currentSpPoints, gainedRawPoints;
		for (i = 0; i < m_Configs.SPECIALIZATIONS_COUNT; i++)
		{
			j = 0;
			switch (i)
			{
				case SCR_ECareerSp.INFANTRY:
					gainedRawPoints = m_aStatsGained[SCR_EDataStats.SPPOINTS0];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS0];
				break;
				case SCR_ECareerSp.LOGISTICS:
					gainedRawPoints = m_aStatsGained[SCR_EDataStats.SPPOINTS1];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS1];
				break;
				case SCR_ECareerSp.MEDICAL:
					gainedRawPoints = m_aStatsGained[SCR_EDataStats.SPPOINTS2];
					currentSpPoints = m_aStats[SCR_EDataStats.SPPOINTS2];
				break;
				default: continue;
			}

			if (currentSpPoints >= m_Configs.SPECIALIZATION_MAX)
				continue;

			//Punish the specialization growth if there are war crimes
			if (m_bWarCrimesEnabled && m_aStats[SCR_EDataStats.WARCRIMES] > 0)
				gainedRawPoints *= (1 - m_Configs.WARCRIMES_PUNISHMENT);

			while (j < m_Configs.INTERVALS_COUNT && m_Configs.INTERVALS_END[j] < currentSpPoints)
				j++;
			if (j >= m_Configs.INTERVALS_COUNT)
			{
				Print ("SCR_PlayerData:CalculateStatsChange: currentSpPoints outside of the accepted ranges.", LogLevel.ERROR);
				continue;
			}

			//Formula: NewSpecializationPoints = CurrentSpecializationPoints + GainedPoints * c1 * (c2 ^ CurrentSpecializationPoints)
			//m_aStats.m_aSpPoints[i] = m_aStats.m_aSpPoints[i] + rawSpPoints[i] * m_Configs.INTERVALS_C1[j] * (Math.Pow(m_Configs.INTERVALS_C2[j], m_aStats.m_aSpPoints[i]));
			AddPointsToSpecialization(i, gainedRawPoints * m_Configs.INTERVALS_C1[j] * (Math.Pow(m_Configs.INTERVALS_C2[j], currentSpPoints * m_Configs.XP_NEEDED_FOR_LEVEL_DIVIDER)));
		}

		//LAST BUT NOT LEAST,  INCREASE THE LEVEL OF EXPERIENCE
		//IF player has max level, skip this step
		if (m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] >= m_Configs.MAX_EXP)
		{
			m_aEarnt[SCR_EDataStats.SESSION_DURATION] = 0;
			m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] = 0;
			return;
		}

		//Calculate gains in level of experience. Currently, we are only using sessionduration and specializationgains
		m_aEarnt[SCR_EDataStats.SESSION_DURATION] = m_aStatsGained[SCR_EDataStats.SESSION_DURATION] * qualityTimeRatio;
		m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aEarnt[SCR_EDataStats.SESSION_DURATION] + totalSpecializationPointsGain * m_Configs.MODIFIER_SPECIALIZATIONS;

		if (m_bWarCrimesEnabled && m_aStats[SCR_EDataStats.WARCRIMES] > 0)
		{
			m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE] * (1 - m_Configs.WARCRIMES_PUNISHMENT);
		}
		m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] + m_aEarnt[SCR_EDataStats.LEVEL_EXPERIENCE];
		if (m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] > m_Configs.MAX_EXP)
			m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] = m_Configs.MAX_EXP;

		m_aStats[SCR_EDataStats.RANK] = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] * m_Configs.XP_NEEDED_FOR_RANK_DIVIDER;

		m_aStatsGained[SCR_EDataStats.RANK] = m_aStats[SCR_EDataStats.RANK] - m_aPreviousStats[SCR_EDataStats.RANK];
		m_aStatsGained[SCR_EDataStats.LEVEL_EXPERIENCE] = m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE] - m_aPreviousStats[SCR_EDataStats.LEVEL_EXPERIENCE];
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

			Print ("SCR_PlayerData:GetSpecializationPoints: WRONG SPECIALIZATION ID.", LogLevel.ERROR);
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

		Print ("SCR_PlayerData:GetSpecializationPoints: WRONG SPECIALIZATION ID.", LogLevel.ERROR);
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
				m_aStats[SCR_EDataStats.SPPOINTS0] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS0] + value, m_Configs.SPECIALIZATION_MAX);
				return;
			case SCR_ECareerSp.LOGISTICS:
				m_aStats[SCR_EDataStats.SPPOINTS1] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS1] + value, m_Configs.SPECIALIZATION_MAX);
				return;
			case SCR_ECareerSp.MEDICAL:
				m_aStats[SCR_EDataStats.SPPOINTS2] = Math.Min(m_aStats[SCR_EDataStats.SPPOINTS2] + value, m_Configs.SPECIALIZATION_MAX);
				return;
		}
		Print ("SCR_PlayerData:AddPointsToSpecialization: WRONG SPECIALIZATION ID.", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	//This method sets the SCR_PlayerDataSpecializationDisplay arrays from the PlayerDataConfigs. Typically the Career Menu will use this
	void PrepareSpecializationStatsDisplay()
	{
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.METERS_WALKED].SetValue(GetDistanceWalked() * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.PLAYER_KILLS].SetValue(GetPlayerKills());
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.AI_KILLS].SetValue(GetAIKills());
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.SHOTS].SetValue(GetBulletsShot());

		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERS_DRIVEN].SetValue(GetDistanceDriven() * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERS_AS_OCCUPANT].SetValue(GetDistanceAsOccupant() * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.POINTS_DRIVING_PEOPLE].SetValue(GetPointsAsDriverOfPlayers());

		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_SELF].SetValue(GetBandageSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_FRIENDLIES].SetValue(GetBandageFriends());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_SELF].SetValue(GetTourniquetSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_FRIENDLIES].SetValue(GetTourniquetFriends());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_SELF].SetValue(GetSelineSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_FRIENDLIES].SetValue(GetSelineFriends());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_SELF].SetValue(GetMorphineSelf());
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_FRIENDLIES].SetValue(GetMorphineFriends());
	}

	//This method sets the SCR_PlayerDataSpecializationDisplay arrays from the PlayerDataConfigs. Typically the Debrief screen will use this
	void PrepareSpecializationProgressionStatsDisplay()
	{
		if (m_aStatsGained.IsEmpty())
		{
			Print("Requesting statsDifference in PrepareSpecializationProgressionStatsDisplay but array is empty. Recalculating it!", LogLevel.DEBUG);
			m_aStatsGained = CalculateStatsDifference();
		}

		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.METERS_WALKED].SetValue(m_aStatsGained[SCR_EDataStats.METERS_WALKED] * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.PLAYER_KILLS].SetValue(m_aStatsGained[SCR_EDataStats.KILLS]);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.AI_KILLS].SetValue(m_aStatsGained[SCR_EDataStats.AI_KILLS]);
		m_Configs.m_aSpecialization0[SCR_ESpecialization0Display.SHOTS].SetValue(m_aStatsGained[SCR_EDataStats.SHOTS]);

		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERS_DRIVEN].SetValue(m_aStatsGained[SCR_EDataStats.METERS_DRIVEN] * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.METERS_AS_OCCUPANT].SetValue(m_aStatsGained[SCR_EDataStats.METERS_AS_OCCUPANT] * m_Configs.MetersToKilometersConversion);
		m_Configs.m_aSpecialization1[SCR_ESpecialization1Display.POINTS_DRIVING_PEOPLE].SetValue(m_aStatsGained[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS]);

		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_SELF].SetValue(m_aStatsGained[SCR_EDataStats.BANDAGE_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.BANDAGE_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.BANDAGE_FRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_SELF].SetValue(m_aStatsGained[SCR_EDataStats.TOURNIQUET_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.TOURNIQUET_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.TOURNIQUET_FRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_SELF].SetValue(m_aStatsGained[SCR_EDataStats.SALINE_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.SALINE_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.SALINE_FRIENDLIES]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_SELF].SetValue(m_aStatsGained[SCR_EDataStats.MORPHINE_SELF]);
		m_Configs.m_aSpecialization2[SCR_ESpecialization2Display.MORPHINE_FRIENDLIES].SetValue(m_aStatsGained[SCR_EDataStats.MORPHINE_FRIENDLIES]);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEmptyProfile()
	{
		return m_bIsEmptyProfile;
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO: THIS SHOULD GO TO THE DATA COLLECTOR OR ONE OF THE MODULES.
	void ActivateWarCrimesDetection(bool warcrimes, bool proportionalityPrinciple)
	{
		if (!warcrimes && proportionalityPrinciple)
			Print("War crimes are disabled but the Proportionality principle is enabled. That doesn't make a lot of sense...", LogLevel.WARNING);
		
		m_bWarCrimesEnabled = warcrimes;
		m_bWarCrimesProportionalityPrincipleEnabled = proportionalityPrinciple;
	}

	//m_aStats and m_aPreviousStats must not be accessed from outside of this class.
	//That is why there are getters and setters for their attributes, but not for their instances
	//
	//
	//
	//SETTERS:
	
	//------------------------------------------------------------------------------------------------
	void AddStat(SCR_EDataStats stat, int amount = 1)
	{
		m_aStats[stat] = m_aStats[stat] + amount;
		//TODO: DON'T ADD IT TO STATS, ADD IT TO TEMP_STATS AND MANAGE TEMP_STATS FROM DATACOLLECTOR EVERY X SECONDS
		//THAT CAN DECIDE TO NOTIFY THE ONCRIMEMODULE OR NOT AND WOULD ALSO ACT AS THE PROPORTIONALITY BUFFER
	}

	//GETTERS:
	
	/******************/
	//Rank and Generic//
	/******************/

	//------------------------------------------------------------------------------------------------
	float GetLevelExperience(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.LEVEL_EXPERIENCE];

		return m_aPreviousStats[SCR_EDataStats.LEVEL_EXPERIENCE];
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
			return m_aStats[SCR_EDataStats.SESSION_DURATION];

			return m_aPreviousStats[SCR_EDataStats.SESSION_DURATION];
	}

	//------------------------------------------------------------------------------------------------
	void CalculateSessionDuration()
	{
		m_aStats[SCR_EDataStats.SESSION_DURATION] = m_aStats[SCR_EDataStats.SESSION_DURATION] + (System.GetTickCount() - m_iSessionStartedTickCount)/1000;
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
			return m_aStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES];

		return m_aPreviousStats[SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES];
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
			return m_aStats[SCR_EDataStats.METERS_WALKED];

		return m_aPreviousStats[SCR_EDataStats.METERS_WALKED];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentDistanceWalked(bool newStats = true)
	{
		return m_aStats[SCR_EDataStats.METERS_WALKED] - m_aPreviousStats[SCR_EDataStats.METERS_WALKED];
	}

	/****************/
	//ShootingModule//
	/****************/

	//------------------------------------------------------------------------------------------------
	float GetDeaths(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.DEATHS];

		return m_aPreviousStats[SCR_EDataStats.DEATHS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentDeaths()
	{
		return m_aStats[SCR_EDataStats.DEATHS] - m_aPreviousStats[SCR_EDataStats.DEATHS];
	}

	//------------------------------------------------------------------------------------------------
	float GetPlayerKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.KILLS];

		return m_aPreviousStats[SCR_EDataStats.KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentPlayerKills()
	{
		return m_aStats[SCR_EDataStats.KILLS] - m_aPreviousStats[SCR_EDataStats.KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetAIKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.AI_KILLS];

		return m_aPreviousStats[SCR_EDataStats.AI_KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentAIKills()
	{
		return m_aStats[SCR_EDataStats.AI_KILLS] - m_aPreviousStats[SCR_EDataStats.AI_KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetFriendlyPlayerKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLY_KILLS];

		return m_aPreviousStats[SCR_EDataStats.FRIENDLY_KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentFriendlyPlayerKills()
	{
		return m_aStats[SCR_EDataStats.FRIENDLY_KILLS] - m_aPreviousStats[SCR_EDataStats.FRIENDLY_KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetFriendlyAIKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLY_AI_KILLS];

		return m_aPreviousStats[SCR_EDataStats.FRIENDLY_AI_KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentFriendlyAIKills()
	{
		return m_aStats[SCR_EDataStats.FRIENDLY_AI_KILLS] - m_aPreviousStats[SCR_EDataStats.FRIENDLY_AI_KILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetBulletsShot(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SHOTS];

		return m_aPreviousStats[SCR_EDataStats.SHOTS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentBulletsShot()
	{
		return m_aStats[SCR_EDataStats.SHOTS] - m_aPreviousStats[SCR_EDataStats.SHOTS];
	}

	//------------------------------------------------------------------------------------------------
	float GetGrenadesThrown(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.GRENADES_THROWN];

		return m_aPreviousStats[SCR_EDataStats.GRENADES_THROWN];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentGrenadesThrown()
	{
		return m_aStats[SCR_EDataStats.GRENADES_THROWN] - m_aPreviousStats[SCR_EDataStats.GRENADES_THROWN];
	}

	/****************/
	//DriverModule//
	/****************/

	//------------------------------------------------------------------------------------------------
	void AddPointsAsDriverOfPlayers(float meters, float players)
	{
		m_aStats[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] = m_aStats[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] + meters * players * m_Configs.MODIFIER_DRIVER_OF_PLAYERS;
	}

	//------------------------------------------------------------------------------------------------
	float GetPointsAsDriverOfPlayers(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS];

		return m_aPreviousStats[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentPointsAsDriverOfPlayers()
	{
		return m_aStats[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS] - m_aPreviousStats[SCR_EDataStats.POINTS_AS_DRIVER_OF_PLAYERS];
	}

	//------------------------------------------------------------------------------------------------
	float GetDistanceDriven(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.METERS_DRIVEN];

		return m_aPreviousStats[SCR_EDataStats.METERS_DRIVEN];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentDistanceDriven()
	{
		return m_aStats[SCR_EDataStats.METERS_DRIVEN] - m_aPreviousStats[SCR_EDataStats.METERS_DRIVEN];
	}

	//------------------------------------------------------------------------------------------------
	float GetDistanceAsOccupant(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.METERS_AS_OCCUPANT];

		return m_aPreviousStats[SCR_EDataStats.METERS_AS_OCCUPANT];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentDistanceAsOccupant()
	{
		return m_aStats[SCR_EDataStats.METERS_AS_OCCUPANT] - m_aPreviousStats[SCR_EDataStats.METERS_AS_OCCUPANT];
	}

	//------------------------------------------------------------------------------------------------
	float GetRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.ROADKILLS];

		return m_aPreviousStats[SCR_EDataStats.ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentRoadKills()
	{
		return m_aStats[SCR_EDataStats.ROADKILLS] - m_aPreviousStats[SCR_EDataStats.ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetAIRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.AI_ROADKILLS];

		return m_aPreviousStats[SCR_EDataStats.AI_ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentAIRoadKills()
	{
		return m_aStats[SCR_EDataStats.AI_ROADKILLS] - m_aPreviousStats[SCR_EDataStats.AI_ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetFriendlyRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLY_ROADKILLS];

		return m_aPreviousStats[SCR_EDataStats.FRIENDLY_ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentFriendlyRoadKills(bool newStats = true)
	{
		return m_aStats[SCR_EDataStats.FRIENDLY_ROADKILLS] - m_aPreviousStats[SCR_EDataStats.FRIENDLY_ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetFriendlyAIRoadKills(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.FRIENDLY_AI_ROADKILLS];

		return m_aPreviousStats[SCR_EDataStats.FRIENDLY_AI_ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentFriendlyAIRoadKills()
	{
		return m_aStats[SCR_EDataStats.FRIENDLY_AI_ROADKILLS] - m_aPreviousStats[SCR_EDataStats.FRIENDLY_AI_ROADKILLS];
	}

	//------------------------------------------------------------------------------------------------
	float GetPlayersDiedInVehicle(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE];

		return m_aPreviousStats[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentPlayersDiedInVehicle()
	{
		return m_aStats[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE] - m_aPreviousStats[SCR_EDataStats.PLAYERS_DIED_IN_VEHICLE];
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
		m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE] = m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE] + distance;
	}

	//------------------------------------------------------------------------------------------------
	float GetTraveledDistanceSupplyVehicle(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE];

		return m_aPreviousStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentTraveledDistanceSupplyVehicle()
	{
		return m_aStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE] - m_aPreviousStats[SCR_EDataStats.TRAVELEDDISTANCESUPPLYVEHICLE];
	}

	//------------------------------------------------------------------------------------------------
	float GetTraveledTimeSupplyVehicle(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE];

		return m_aPreviousStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE];
	}

	//------------------------------------------------------------------------------------------------
	float GetCurrentTraveledTimeSupplyVehicle()
	{
		return m_aStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE] - m_aPreviousStats[SCR_EDataStats.TRAVELEDTIMESUPPLYVEHICLE];
	}

	*/

	/********************/
	//HealingItemsModule//
	/********************/

	//------------------------------------------------------------------------------------------------
	void AddBandageUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.BANDAGE_SELF] = m_aStats[SCR_EDataStats.BANDAGE_SELF]+1;
		else
			m_aStats[SCR_EDataStats.BANDAGE_FRIENDLIES] = m_aStats[SCR_EDataStats.BANDAGE_FRIENDLIES] + 1;
	}

	//------------------------------------------------------------------------------------------------
	void AddTourniquetUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.TOURNIQUET_SELF] = m_aStats[SCR_EDataStats.TOURNIQUET_SELF] + 1;
		else
			m_aStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES] = m_aStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES] + 1;
	}

	//------------------------------------------------------------------------------------------------
	void AddSalineUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.SALINE_SELF] = m_aStats[SCR_EDataStats.SALINE_SELF] + 1;
		else
			m_aStats[SCR_EDataStats.SALINE_FRIENDLIES] = m_aStats[SCR_EDataStats.SALINE_FRIENDLIES] + 1;
	}

	//------------------------------------------------------------------------------------------------
	void AddMorphineUse(bool selfTarget)
	{
		if (selfTarget)
			m_aStats[SCR_EDataStats.MORPHINE_SELF] = m_aStats[SCR_EDataStats.MORPHINE_SELF] + 1;
		else
			m_aStats[SCR_EDataStats.MORPHINE_FRIENDLIES] = m_aStats[SCR_EDataStats.MORPHINE_FRIENDLIES] + 1;
	}

	//------------------------------------------------------------------------------------------------
	int GetBandageFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.BANDAGE_FRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.BANDAGE_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentBandageFriends()
	{
		return m_aStats[SCR_EDataStats.BANDAGE_FRIENDLIES] - m_aPreviousStats[SCR_EDataStats.BANDAGE_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetBandageSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.BANDAGE_SELF];
		return m_aPreviousStats[SCR_EDataStats.BANDAGE_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentBandageSelf()
	{
		return m_aStats[SCR_EDataStats.BANDAGE_SELF] - m_aPreviousStats[SCR_EDataStats.BANDAGE_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetTourniquetFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentTourniquetFriends()
	{
		return m_aStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES] - m_aPreviousStats[SCR_EDataStats.TOURNIQUET_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetTourniquetSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.TOURNIQUET_SELF];
		return m_aPreviousStats[SCR_EDataStats.TOURNIQUET_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentTourniquetSelf()
	{
		return m_aStats[SCR_EDataStats.TOURNIQUET_SELF] - m_aPreviousStats[SCR_EDataStats.TOURNIQUET_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetSelineFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SALINE_FRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.SALINE_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentSelineFriends()
	{
		return m_aStats[SCR_EDataStats.SALINE_FRIENDLIES] - m_aPreviousStats[SCR_EDataStats.SALINE_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetSelineSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.SALINE_SELF];
		return m_aPreviousStats[SCR_EDataStats.SALINE_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentSelineSelf()
	{
		return m_aStats[SCR_EDataStats.SALINE_SELF] - m_aPreviousStats[SCR_EDataStats.SALINE_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetMorphineFriends(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.MORPHINE_FRIENDLIES];
		return m_aPreviousStats[SCR_EDataStats.MORPHINE_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentMorphineFriends()
	{
		return m_aStats[SCR_EDataStats.MORPHINE_FRIENDLIES] - m_aPreviousStats[SCR_EDataStats.MORPHINE_FRIENDLIES];
	}

	//------------------------------------------------------------------------------------------------
	int GetMorphineSelf(bool newStats = true)
	{
		if (newStats)
			return m_aStats[SCR_EDataStats.MORPHINE_SELF];
		return m_aPreviousStats[SCR_EDataStats.MORPHINE_SELF];
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentMorphineSelf()
	{
		return m_aStats[SCR_EDataStats.MORPHINE_SELF] - m_aPreviousStats[SCR_EDataStats.MORPHINE_SELF];
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
	LEVEL_EXPERIENCE,
	SESSION_DURATION,
	SPPOINTS0, //INFANTRY
	SPPOINTS1, //LOGISTICS
	SPPOINTS2, //MEDICAL
	WARCRIMES,
	METERS_WALKED,
	KILLS,
	AI_KILLS,
	SHOTS,
	GRENADES_THROWN,
	FRIENDLY_KILLS,
	FRIENDLY_AI_KILLS,
	DEATHS,
	METERS_DRIVEN,
	POINTS_AS_DRIVER_OF_PLAYERS,
	PLAYERS_DIED_IN_VEHICLE,
	ROADKILLS,
	FRIENDLY_ROADKILLS,
	AI_ROADKILLS,
	FRIENDLY_AI_ROADKILLS,
	METERS_AS_OCCUPANT,
	BANDAGE_SELF,
	BANDAGE_FRIENDLIES,
	TOURNIQUET_SELF,
	TOURNIQUET_FRIENDLIES,
	SALINE_SELF,
	SALINE_FRIENDLIES,
	MORPHINE_SELF,
	MORPHINE_FRIENDLIES,
	WARCRIME_HARMING_FRIENDLIES //Harming friendlies
};
