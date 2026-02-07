//------------------------------------------------------------------------------------------------
class SCR_PlayerDataConfigs : Managed
{		
	
	protected static ref SCR_PlayerDataConfigs instance;
	
	/****************/
	//RANK AND LEVEL//
	/****************/
	
	float MODIFIERSPECIALIZATIONS = 0.2;
	const int XP_NEEDED_FOR_LEVEL = 10000;
	const int MAXEXP = 1000000;
		
	/*****************/
	//SPECIALIZATIONS//
	/*****************/
	
	int SPECIALIZATIONS_COUNT;
	
	string SPECIALIZATION_0_NAME = "#AR-CareerProfile_Specialization1";
	string SPECIALIZATION_1_NAME = "#AR-CareerProfile_Specialization2";
	string SPECIALIZATION_2_NAME = "#AR-CareerProfile_Specialization3";
	
	int SPECIALIZATION_0_COUNT;
	int SPECIALIZATION_1_COUNT;
	int SPECIALIZATION_2_COUNT;
	
	ref array<ref SCR_PlayerDataSpecializationDisplay> m_aSpecialization0 = {};
	ref array<ref SCR_PlayerDataSpecializationDisplay> m_aSpecialization1 = {};
	ref array<ref SCR_PlayerDataSpecializationDisplay> m_aSpecialization2 = {};
	
	const int SPECIALIZATIONMAX = 1000000;
	
	//0 - Infantry
	const float MODIFIERMETERSWALKED = 1;
	const float MODIFIERKILLS = 250;
	const float MODIFIERAIKILLS = 125;
	const float MODIFIERPRECISION = 300;
	
	//1 - Logistics
	const float MODIFIERMETERSDRIVEN = 1;
	const float MODIFIERDRIVEROFPLAYERS = 0.7;
	const float MODIFIERTRAVELTIMESUPPLYVEHICLE = 1;
	const float MODIFIERTRAVELEDDISTANCESUPPLYVEHICLE = 5;
	const int MODIFIERPLAYERSDIEDINVEHICLE = 500;
	
	//2 - Medical Support
	const float MODIFIERBANDAGESELF = 150;
	const float MODIFIERBANDAGEFRIENDLIES = 300;
	const float MODIFIERTOURNIQUETSELF = 150;
	const float MODIFIERTOURNIQUETFRIENDLIES = 300;
	const float MODIFIERSELINESELF = 150;
	const float MODIFIERSELINEFRIENDLIES = 300;
	const float MODIFIERMORPHINESELF = 150;
	const float MODIFIERMORPHINEFRIENDLIES = 300;
	
	
	//Config of Ranges
	const int INTERVALS_COUNT = 5;
	ref const array<int> INTERVALS_ORIGIN = {0, 160000, 600000, 750000, 900000};
	ref const array<int> INTERVALS_END = {160000, 600000, 750000, 900000, 1000000};
	ref const array<float> INTERVALS_C1 = {1.9449, 0.71, 8.1682, 6, 18};
	ref const array<float> INTERVALS_C2 = {0.9254, 0.9908, 0.9517, 0.9559, 0.9425};
	
	/************/
	//WAR CRIMES//
	/************/
	
	const float WARCRIMESPUNISHMENT = 0.25;
	
	const int MAXWARCRIMESVALUE = 250000;
	const int WARCRIMESDECREASEPERHOUR = 25000;
	const int WARCRIMEDIVIDERTOPERCENTAGE = 1000000;
	
	const float MODIFIERFRIENDLYKILLS = 7500;
	const float MODIFIERFRIENDLYAIKILLS = 5000;
	
	const float m_fSTDPointsQualityTime = 10000;
	
	
	/**********/
	//UI STUFF//
	/**********/
	
	const float SPPOINTS_CONVERSIONPERCENTAGE =	0.0001;
	
	//------------------------------------------------------------------------------------------------
	static SCR_PlayerDataConfigs GetInstance()
	{
		if (!instance)
			instance = new SCR_PlayerDataConfigs();
		
		return instance;
	}
	
	string GetSpecializationName(int n)
	{
		switch (n)
		{
			case 0: return SPECIALIZATION_0_NAME;
			case 1: return SPECIALIZATION_1_NAME;
			case 2: return SPECIALIZATION_2_NAME;
		}
		
		return "UNDEFINED_NAME";
	}
	
	int GetSpecializationStatsCount(int n)
	{
		switch (n)
		{
			case 0: return SPECIALIZATION_0_COUNT;
			case 1: return SPECIALIZATION_1_COUNT;
			case 2: return SPECIALIZATION_2_COUNT;
		}
		
		return -1;
	}
	
	array<ref SCR_PlayerDataSpecializationDisplay> GetSpecializationArray(int n)
	{
		switch (n)
		{
			case 0: return m_aSpecialization0;
			case 1: return m_aSpecialization1;
			case 2: return m_aSpecialization2;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerDataConfigs()
	{
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.METERSWALKED, SCR_EDataStats.METERSWALKED, "#AR-CareerProfile_DistanceTravelled_ByFoot", "#AR-CareerProfile_KMs"));
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.PLAYERKILLS, SCR_EDataStats.KILLS, "#AR-CareerProfile_PlayersKilled", "#AR-CareerProfile_Enemies"));
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.AIKILLS, SCR_EDataStats.AIKILLS, "#AR-CareerProfile_AIKilled", "#AR-CareerProfile_Enemies"));
		m_aSpecialization0.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization0Display.SHOTS, SCR_EDataStats.SHOTS, "#AR-CareerProfile_BulletsShot", "#AR-CareerProfile_Rounds"));
		
		m_aSpecialization1.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization1Display.METERSDRIVEN, SCR_EDataStats.METERSDRIVEN, "#AR-CareerProfile_DistanceTravelled_AsDriver", "#AR-CareerProfile_KMs"));
		m_aSpecialization1.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization1Display.POINTSDRIVINGPEOPLE, SCR_EDataStats.METERSASOCCUPANT, "#AR-CareerProfile_DistanceTravelled_AsPassenger", "#AR-CareerProfile_KMs"));
		m_aSpecialization1.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization1Display.POINTSDRIVINGPEOPLE, SCR_EDataStats.POINTSASDRIVEROFPLAYERS, "#AR-CareerProfile_PointsDriverAllies", "#AR-CareerProfile_Points"));
		
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.BANDAGESELF, SCR_EDataStats.BANDAGESELF, "#AR-CareerProfile_BandagesSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.BANDAGEFRIENDLIES, SCR_EDataStats.BANDAGEFRIENDLIES, "#AR-CareerProfile_BandagesAllies", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.TOURNIQUETSELF, SCR_EDataStats.TOURNIQUETSELF, "#AR-CareerProfile_TourniquetsSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.TOURNIQUETFRIENDLIES, SCR_EDataStats.TOURNIQUETFRIENDLIES, "#AR-CareerProfile_TourniquetsAllies", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.SELINESELF, SCR_EDataStats.SELINESELF, "#AR-CareerProfile_SelinesSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.SELINEFRIENDLIES, SCR_EDataStats.SELINEFRIENDLIES, "#AR-CareerProfile_SelinesAllies", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.MORPHINESELF, SCR_EDataStats.MORPHINESELF, "#AR-CareerProfile_MorphinesSelf", "#AR-CareerProfile_Times"));
		m_aSpecialization2.Insert(new SCR_PlayerDataSpecializationDisplay(SCR_ESpecialization2Display.MORPHINEFRIENDLIES, SCR_EDataStats.MORPHINEFRIENDLIES, "#AR-CareerProfile_MorphinesAllies", "#AR-CareerProfile_Times"));
		
		typename tEnum = SCR_ECareerSp;
		SPECIALIZATIONS_COUNT = tEnum.GetVariableCount();
		
		tEnum = SCR_ESpecialization0Display;		
		SPECIALIZATION_0_COUNT = tEnum.GetVariableCount();
		
		tEnum = SCR_ESpecialization1Display;
		SPECIALIZATION_1_COUNT = tEnum.GetVariableCount();
		
		tEnum = SCR_ESpecialization2Display;
		SPECIALIZATION_2_COUNT = tEnum.GetVariableCount();
		
		if (m_aSpecialization0.Count() != SPECIALIZATION_0_COUNT)
			Print("Error in PlayerDataConfigs: Weird size on specialization0 array", LogLevel.ERROR);
		
		if (m_aSpecialization1.Count() != SPECIALIZATION_1_COUNT)
			Print("Error in PlayerDataConfigs: Weird size on specialization1 array", LogLevel.ERROR);
		
		if (m_aSpecialization2.Count() != SPECIALIZATION_2_COUNT)
			Print("Error in PlayerDataConfigs: Weird size on specialization2 array", LogLevel.ERROR);
	}
};

//Don't change the orders of any of these enums please, we are using their position as an id
//------------------------------------------------------------------------------------------------
enum SCR_ECareerSp
{
	INFANTRY,
	LOGISTICS,
	MEDICAL
};

//------------------------------------------------------------------------------------------------
enum SCR_ESpecialization0Display
{
	METERSWALKED,
	PLAYERKILLS,
	AIKILLS,
	SHOTS
};

//------------------------------------------------------------------------------------------------
enum SCR_ESpecialization1Display
{
	METERSDRIVEN,
	METERSASOCCUPANT,
	POINTSDRIVINGPEOPLE
};

//------------------------------------------------------------------------------------------------
enum SCR_ESpecialization2Display
{
	BANDAGESELF,
	BANDAGEFRIENDLIES,
	TOURNIQUETSELF,
	TOURNIQUETFRIENDLIES,
	SELINESELF,
	SELINEFRIENDLIES,
	MORPHINESELF,
	MORPHINEFRIENDLIES
};