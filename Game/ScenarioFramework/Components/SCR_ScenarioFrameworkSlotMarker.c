[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotMarkerClass : SCR_ScenarioFrameworkSlotBaseClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotMarker : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Marker Type", category: "Map Marker")]
	protected ref SCR_ScenarioFrameworkMarkerType m_MapMarkerType;
	
	protected ref SCR_MapMarkerBase m_MapMarker;
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkMarkerType GetMapMarkerType()
	{
		return m_MapMarkerType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMapMarkerType(SCR_ScenarioFrameworkMarkerType type)
	{
		m_MapMarkerType = type;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MapMarkerBase GetMapMarker()
	{
		return m_MapMarker;
	}
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned()
	{
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : m_aActivationConditions)
		{
			//If just one condition is false, we don't continue and interrupt the init
			if (!activationCondition.Init(GetOwner()))
			{
				super.AfterAllChildrenSpawned();
				return;
			}
		}
		
		if (!m_MapMarker)
			CreateMapMarker();
		
		super.AfterAllChildrenSpawned();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveMapMarker()
	{
		if (!m_MapMarker)
			return;
		
		SCR_MapMarkerManagerComponent markerMgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerMgr)
			return;
		
		int markerID = m_MapMarker.GetMarkerID();
		
		SCR_MapMarkerBase marker = markerMgr.GetStaticMarkerByID(markerID);
		if (!marker)
			return;
		
		markerMgr.OnRemoveSynchedMarker(markerID);	//Remove server side
		markerMgr.OnAskRemoveStaticMarker(markerID); //Remove client side
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateMapMarker()
	{
		vector worldPos = GetOwner().GetOrigin();
		m_MapMarker = new SCR_MapMarkerBase();
		
		m_MapMarker.SetWorldPos(worldPos[0], worldPos[2]);
		m_MapMarker.SetMarkerFactionKey(m_sFactionKey);
		
		SCR_ScenarioFrameworkMarkerCustom mapMarkerCustom = SCR_ScenarioFrameworkMarkerCustom.Cast(m_MapMarkerType);
		if (mapMarkerCustom)
		{
			m_MapMarker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
			m_MapMarker.SetIconEntry(mapMarkerCustom.m_eMapMarkerIcon);
			m_MapMarker.SetRotation(mapMarkerCustom.m_iMapMarkerRotation);
			m_MapMarker.SetColorEntry(mapMarkerCustom.m_eMapMarkerColor);
			m_MapMarker.SetCustomText(mapMarkerCustom.m_sMapMarkerText);
		}
		else
		{
			SCR_ScenarioFrameworkMarkerMilitary mapMarkerMilitary = SCR_ScenarioFrameworkMarkerMilitary.Cast(m_MapMarkerType);
			if (!mapMarkerMilitary)
				return;
		
			m_MapMarker.SetType(SCR_EMapMarkerType.PLACED_MILITARY);
			m_MapMarker.SetMarkerConfigID(mapMarkerMilitary.m_eMapMarkerIcon);
		}
		
		SCR_MapMarkerManagerComponent mapMarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		if (mapMarkerMgr)
		{
			mapMarkerMgr.OnAddSynchedMarker(m_MapMarker); //Call for server
			mapMarkerMgr.OnAskAddStaticMarker(m_MapMarker); //Call for clients
		}
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerType : ScriptAndConfig
{
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerCustom : SCR_ScenarioFrameworkMarkerType
{
	[Attribute("0", UIWidgets.ComboBox, "Marker Icon", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkMarkerCustom), category: "Map Marker")]
	SCR_EScenarioFrameworkMarkerCustom m_eMapMarkerIcon;
	
	[Attribute("0", UIWidgets.ComboBox, "Marker Color", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkMarkerCustomColor), category: "Map Marker")]
	SCR_EScenarioFrameworkMarkerCustomColor m_eMapMarkerColor;
	
	[Attribute(desc: "Text which will be displayed for the Map Marker", category: "Map Marker")];
	string m_sMapMarkerText;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Rotation of the Map Marker", params: "-180 180 1", category: "Map Marker")]
	int m_iMapMarkerRotation;
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkMarkerMilitary : SCR_ScenarioFrameworkMarkerType
{
	[Attribute("0", UIWidgets.ComboBox, "Marker Icon", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkMarkerMilitary), category: "Map Marker")]
	SCR_EScenarioFrameworkMarkerMilitary m_eMapMarkerIcon;
};

enum SCR_EScenarioFrameworkMarkerCustom
{
	CIRCLE = 0,
	CIRCLE2,
	CROSS,
	CROSS2,
	DOT,
	DOT2,
	DROP_POINT,
	DROP_POINT2,
	ENTRY_POINT,
	ENTRY_POINT2,
	FLAG,
	FLAG2,
	FLAG3,
	FORTIFICATION,
	FORTIFICATION2,
	MARK_EXCLAMATION,
	MARK_EXCLAMATION2,
	MARK_EXCLAMATION3,
	MARK_QUESTION,
	MARK_QUESTION2,
	MARK_QUESTION3,
	MINE_FIELD,
	MINE_FIELD2,
	MINE_FIELD3,
	MINE_SINGLE,
	MINE_SINGLE2,
	MINE_SINGLE3,
	OBJECTIVE_MARKER,
	OBJECTIVE_MARKER2,
	OBSERVATION_POST,
	OBSERVATION_POST2,
	PICK_UP,
	PICK_UP2,
	POINT_OF_INTEREST,
	POINT_OF_INTEREST2,
	POINT_OF_INTEREST3,
	POINT_SPECIAL,
	POINT_SPECIAL2,
	RECON_OUTPOST,
	RECON_OUTPOST2,
	WAYPOINT,
	WAYPOINT2,
	DEFEND,
	DEFEND2,
	DESTROY,
	DESTROY2,
	HEAL,
	HELP,
	HELP2,
	ATTACK,
	ATTACK_MAIN,
	CONTAIN,
	CONTAIN2,
	CONTAIN3,
	RETAIN,
	RETAIN2,
	STRONG_POINT,
	STRONG_POINT2,
	TARGET_REFERENCE_POINT,
	TARGET_REFERENCE_POINT2,
	AMBUSH,
	AMBUSH2,
	RECONNAISSANCE,
	SEARCH_AREA,
	DIRECTION_OF_ATTACK,
	DIRECTION_OF_ATTACK_MAIN,
	DIRECTION_OF_ATTACK_PLANNED,
	FOLLOW_AND_SUPPORT,
	FOLLOW_AND_SUPPORT2,
	JOIN,
	JOIN2,
	JOIN3,
	ARROW_LARGE,
	ARROW_LARGE2,
	ARROW_LARGE3,
	ARROW_MEDIUM,
	ARROW_MEDIUM2,
	ARROW_MEDIUM3,
	ARROW_SMALL,
	ARROW_SMALL2,
	ARROW_SMALL3,
	ARROW_CURVE_LARGE,
	ARROW_CURVE_LARGE2,
	ARROW_CURVE_LARGE3,
	ARROW_CURVE_MEDIUM,
	ARROW_CURVE_MEDIUM2,
	ARROW_CURVE_MEDIUM3,
	ARROW_CURVE_SMALL,
	ARROW_CURVE_SMALL2,
	ARROW_CURVE_SMALL3
};

enum SCR_EScenarioFrameworkMarkerCustomColor
{
	WHITE = 0,
	REFORGER_ORANGE,
	ORANGE,
	RED,
	OPFOR,
	INDEPENDENT,
	GREEN,
	BLUE,
	BLUFOR,
	DARK_BLUE,
	MAGENTA,
	CIVILIAN,
	DARK_PINK
};

enum SCR_EScenarioFrameworkMarkerMilitary
{
	LAND_INFANTRY_MOTORIZED = 0,
	LAND_INFANTRY_ARMOR,
	LAND_ARMOR,
	LAND_MORTAR,
	LAND_ARTILLERY,
	LAND_MEDICAL,
	LAND_ROTARY_WING,
	LAND_MOBILEHQ,
	LAND_MAINTENANCE,
	LAND_SUPPLY,
	LAND_RECON,
	LAND_MACHINEGUN,
	LAND_SNIPER,
	LAND_ANTITANK,
	INSTALLATION_NA
};