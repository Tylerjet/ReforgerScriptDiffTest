[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation16 : SCR_BaseCampaignTutorialArlandStage
{	
	bool m_bPositionFound;
	int m_iPlacedMarkers;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt)
			return;
		
		SCR_MapMarkersUI mapMarkersUI = SCR_MapMarkersUI.Cast(mapEnt.GetMapUIComponent(SCR_MapMarkersUI));
		if (!mapMarkersUI)
			return;
		
		mapMarkersUI.GetOnCustomMarkerPlaced().Insert(MarkerPlaced);
		
		PlaySoundSystem("Navigation_OrientationLocationMarker");
		HintOnVoiceOver();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHint()
	{
		string text;
		
		if (m_iPlacedMarkers >= 5 && m_iPlacedMarkers < 10)
		{
			text = "5";
			m_TutorialComponent.ShowMapDescriptor("LighthousePos", true);
		}
		
		if (m_iPlacedMarkers >= 10 && m_iPlacedMarkers < 15)
		{
			text = "10";
			m_TutorialComponent.ShowMapDescriptor("ChurchPos", true);
		}
		
		if (m_iPlacedMarkers >= 15 && m_iPlacedMarkers < 20)
		{
			text = "15";
			m_TutorialComponent.ShowMapDescriptor("TowerPos", true);
		}
		
		if (m_iPlacedMarkers >= 20)
		{
			text = "20";
			m_TutorialComponent.ShowMapDescriptor("NavigationPos", true)
		}
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage(), text));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void MarkerPlaced(int posX, int posY, bool isLocal)
	{
		vector position = m_Player.GetOrigin();
		
		position[0] = position[0] - posX;
		position[2] = position[2] - posY;
		
		m_iPlacedMarkers++;
		
		if (m_iPlacedMarkers == 5)
			UpdateHint();
		
		if (m_iPlacedMarkers == 10)
		{
			ScriptInvokerVoid invoker =  m_System.GetOnFinished();
			if (invoker)
				invoker.Insert(UpdateHint);
			
			PlaySoundSystem("Navigation_OrientationStruggle2", true);
		}
		
		if (m_iPlacedMarkers == 15)
			PlaySoundSystem("Navigation_OrientationStruggle3");
		
		if (m_iPlacedMarkers == 20)
		{
			PlaySoundSystem("Navigation_OrientationStruggle4", true);
			
			m_fDelay = 14;
			m_bPositionFound = true;
		}
				
		if (position[0] > 150 || position[0] < -150)
			return;
			
		if (position[2] > 150 || position[2] < -150)
			return;
		
		m_bPositionFound = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		return m_bPositionFound;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialNavigation16()
	{
		if (!m_System)
			return;
		
		ScriptInvokerVoid invoker =  m_System.GetOnFinished();
		if (invoker)
			invoker.Remove(UpdateHint);
	}
};