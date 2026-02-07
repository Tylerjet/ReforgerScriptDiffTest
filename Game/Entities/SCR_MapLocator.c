class SCR_MapLocatorClass : GenericEntityClass
{
};

class SCR_MapLocator : GenericEntity
{
	protected ResourceName m_sUIhintLayout = "{2229B9CC8917D3A3}UI/layouts/Map/MapLocationHint.layout";
	protected Widget m_wUIhintWidget;
	protected TextWidget m_wUIHintText;
	protected TextWidget m_wUIHintText2;

   	protected LocalizedString m_aWorldDirections[9] =
	{
		"#AR-MapLocationHint_EveronSouthWest",
		"#AR-MapLocationHint_EveronSouth",
		"#AR-MapLocationHint_EveronSouthEast",
		"#AR-MapLocationHint_EveronWest",
		"#AR-MapLocationHint_EveronCentral",
		"#AR-MapLocationHint_EveronEast",
		"#AR-MapLocationHint_EveronNorthWest",
		"#AR-MapLocationHint_EveronNorth",
		"#AR-MapLocationHint_EveronNorthEast"
    };
	protected int m_iGridSizeX;
	protected int m_iGridSizeY;
	
	protected const float angleA = 0.775;
	protected const float angleB = 0.325;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode()) 
			return;
		SCR_MapEntity.GetOnMapOpen().Insert(ShowMapHint);
		SCR_MapEntity.GetOnMapClose().Insert(HideMapHint);
		
		vector mins,maxs;
		GetGame().GetWorldEntity().GetWorldBounds(mins, maxs);
		
		m_iGridSizeX = maxs[0]/3;
		m_iGridSizeY = maxs[2]/3;
	}
	
	protected void ShowMapHint()
	{
		if (SCR_SelectSpawnPointSubMenu.GetInstance())
			return;
		m_wUIhintWidget = GetGame().GetWorkspace().CreateWidgets(m_sUIhintLayout);
		if (!m_wUIhintWidget)
			return;
		m_wUIHintText = TextWidget.Cast(m_wUIhintWidget.FindAnyWidget("MapLocationHint_Text"));
		m_wUIHintText2= TextWidget.Cast(m_wUIhintWidget.FindAnyWidget("MapLocationHint_Text2"));
		CalculateClosestLocation();
		GetGame().GetCallqueue().CallLater(CalculateClosestLocation,10000,true);
	}
	
	protected void HideMapHint()
	{
		if (m_wUIhintWidget)
			m_wUIhintWidget.RemoveFromHierarchy();
		m_wUIhintWidget = null;
		m_wUIHintText = null;
		m_wUIHintText2 = null;
		GetGame().GetCallqueue().Remove(CalculateClosestLocation);
	}
	
	protected void CalculateClosestLocation()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
	       if (!core)
	           return;
		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
		{
			if (m_wUIhintWidget)
			{
				m_wUIhintWidget.RemoveFromHierarchy();
				m_wUIhintWidget = null;
			}
			return;
		}
		vector posPlayer = player.GetOrigin();
		
		SCR_EditableEntityComponent nearest = core.FindNearestEntity(posPlayer, EEditableEntityType.COMMENT, EEditableEntityFlag.LOCAL);
		GenericEntity nearestLocation = nearest.GetOwner();
		SCR_MapDescriptorComponent mapDescr = SCR_MapDescriptorComponent.Cast(nearestLocation.FindComponent(SCR_MapDescriptorComponent));
		string closestLocationName;
		if (!mapDescr)
			return;
		MapItem item = mapDescr.Item();
		closestLocationName = item.GetDisplayName();

		vector lastLocationPos = nearestLocation.GetOrigin();
		float lastDistance = vector.DistanceSqXZ(lastLocationPos, posPlayer);
	
		LocalizedString closeLocationAzumith;
		vector result = posPlayer - lastLocationPos;
		result.Normalize();
	
		float angle1 = vector.DotXZ(result,vector.Forward);
		float angle2 = vector.DotXZ(result,vector.Right);
			
		if (angle2 > 0)
		{
			if (angle1 >= angleA) {closeLocationAzumith = "#AR-MapLocationHint_DirectionNorth"};
			if (angle1 < angleA && angle1 >= angleB ) {closeLocationAzumith = "#AR-MapLocationHint_DirectionNorthEast"};
			if (angle1 < angleB && angle1 >=-angleB) {closeLocationAzumith = "#AR-MapLocationHint_DirectionEast"};
			if (angle1 < -angleB && angle1 >=-angleA) {closeLocationAzumith = "#AR-MapLocationHint_DirectionSouthEast"};
			if (angle1 < -angleA) {closeLocationAzumith = "#AR-MapLocationHint_DirectionSouth"};
		} else {
			if (angle1 >= angleA) {closeLocationAzumith = "#AR-MapLocationHint_DirectionNorth"};
			if (angle1 < angleA && angle1 >= angleB ) {closeLocationAzumith = "#AR-MapLocationHint_DirectionNorthWest"};
			if (angle1 < angleB && angle1 >=-angleB) {closeLocationAzumith = "#AR-MapLocationHint_DirectionWest"};
			if (angle1 < -angleB && angle1 >=-angleA) {closeLocationAzumith = "#AR-MapLocationHint_DirectionSouthWest"};
			if (angle1 < -angleA) {closeLocationAzumith = "#AR-MapLocationHint_DirectionSouth"};
		};
		
		string positionAzimut;
		int playerGridPositionX = posPlayer[0]/m_iGridSizeX;
		int playerGridPositionY = posPlayer[2]/m_iGridSizeY;
		
		int playerGridID = GetGridIndex(playerGridPositionX,playerGridPositionY);
		m_wUIHintText.SetTextFormat("#AR-MapLocationHint_PlayerPosition", m_aWorldDirections[playerGridID]);
		if (lastDistance < 40000)
			m_wUIHintText2.SetTextFormat("%1.", closestLocationName);
		else
			m_wUIHintText2.SetTextFormat(closeLocationAzumith, closestLocationName);
	}
	
	protected int GetGridIndex(int x, int y)
	{
		return 3*y + x;
	}
	//------------------------------------------------------------------------------------------------
	void SCR_MapLocator(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
};
