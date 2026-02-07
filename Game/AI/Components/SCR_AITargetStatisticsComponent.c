[ComponentEditorProps(category: "GameScripted/AI", description: "Component for statistics of aiming", color: "0 0 255 255")]
class SCR_AITargetStatisticsComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_AITargetStatisticsComponent : ScriptComponent
{
	ref array<vector> m_shots = {};
	
	private ref array<ref SCR_AITargetStatsScreen> m_aVisualizationManagers = {};
	private ref array<AIAgent> m_aAgents = {};
	private int m_iTargetNumber = 0;
	ref Color m_ColorStand = UIColors.CONFIRM;
	ref Color m_ColorCrouch = UIColors.CONTRAST_COLOR;
	ref Color m_ColorProne = UIColors.WARNING;
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.FRAME);
	}
	
	override event protected void EOnFrame(IEntity owner, float timeSlice)
	{
		foreach (SCR_AITargetStatsScreen screen : m_aVisualizationManagers)
		{
			screen.Update(timeSlice);
		}
	}
	
	private SCR_AITargetStatsScreen GetStatisticsScreen(AIAgent owner)
	{
		int i = m_aAgents.Find(owner);
		
		if (i > -1 && i < m_aVisualizationManagers.Count())
		{
			return m_aVisualizationManagers[i];
		}
		else
		{
			m_aAgents.Insert(owner);
			SCR_AITargetStatsScreen screen = new SCR_AITargetStatsScreen;
			m_aVisualizationManagers.Insert(screen);
			return screen;
		}
	}
	
	void AddShot(AIDangerEvent dangerEvent)
	{
		m_shots.Insert(dangerEvent.GetPosition());		
	}
	
	void ClearShots()
	{
		m_shots.Clear();
	}
	
	ECharacterStance GetStance(AIAgent owner)
	{
		CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_CharacterControllerComponent));
		if (!controller)
			return ECharacterStance.STAND;
		
		return controller.GetStance();
	}
	
	Color GetStanceColor(AIAgent owner)
	{
		ECharacterStance stance = GetStance(owner);
		if (stance == ECharacterStance.CROUCH)
			return m_ColorCrouch;
		if (stance == ECharacterStance.PRONE)
			return m_ColorProne;
		
		return m_ColorStand;
	}
	
	void VisualizeStatistics(AIAgent owner, vector origin, vector scale)
	{
		if (m_iTargetNumber == 8) 
		{
			m_iTargetNumber = 0;
		}
		
		VisualizeTarget(owner, origin, scale);
		VisualizeDistribution(owner, origin, GetStance(owner));
		m_iTargetNumber++;
	}
	
	void VisualizeTarget(AIAgent owner, vector origin, vector scale)
	{
		SCR_AITargetStatsScreen screen = GetStatisticsScreen(owner);
		if (!screen)
			return;
		
		vector point;
		for (int i = 0, length = m_shots.Count(); i < length; i++)
		{
			point[0] = (m_shots[i][1] - origin[1]) * scale[1];
			point[1] = (m_shots[i][2] - origin[2]) * scale[2];
			
			screen.DrawPoint(owner,point,m_iTargetNumber, GetStanceColor(owner));
		}
	}
	
	void VisualizeDistribution(AIAgent owner, vector origin, ECharacterStanceChange stance)
	{
		SCR_AITargetStatsScreen screen = GetStatisticsScreen(owner);
		if (!screen)
			return;
		
		int incidence[6] = {0, 0, 0, 0, 0, 0};		
		float distance;
		vector point;
		int length = m_shots.Count();
		
		// calculating target distribution: 20cm distances from origin, count hits in that region 
		for (int i = 0; i < length; i++)
		{
			distance = vector.Distance(m_shots[i],origin);
			if (distance > 0.6)
				incidence[5] = incidence[5] + 1;	 
			else
			{
				for (int k = 0; k < 5; k++)
				{
					if (distance < (k + 1) * 0.12)
					{
						incidence[k] = incidence[k]	+ 1;
						break;
					}	
				}
			}							
		}
		
		for (int k = 0; k < 6; k++)
		{
			point[1] = incidence[k] / length;
			point[0] = k;
			screen.DrawGraphDot(owner,point,m_iTargetNumber,GetStanceColor(owner), GetStance(owner));
		}
	}
};
