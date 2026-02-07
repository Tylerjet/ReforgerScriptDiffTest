class SCR_AIGetTarget: AITaskScripted
{
	[Attribute("100", UIWidgets.EditBox, "Filter target by distance", "")]
	float m_maxDistance;
	
	[Attribute("1", UIWidgets.EditBox, "Filter target by how long ago it was seen", "")]
	float m_lastSeenMax;
	
	[Attribute("3", UIWidgets.ComboBox, "Wanted target type", "", ParamEnumArray.FromEnum(ETargetCategory) )]
	int m_targetType;

	private float m_lastSeenMaxMillis;
	private ref Shape m_Shape;
	private PerceptionComponent perceptComp;
	private SCR_AIConfigComponent confComp;
	private SCR_AICombatComponent m_CombatComponent;
	
	private ref SCR_AITargetInfo m_targetInfo;
	private ref SCR_AITargetInfo m_targetOut;

	override bool VisibleInPalette()
    {
        return true;
    }
	
	protected override string GetNodeMiddleText()
	{
		string enumToString;
		switch (m_targetType)
		{
			case ETargetCategory.UNKNOWN : {enumToString = "Unknown"; break;}
			case ETargetCategory.DETECTED : {enumToString = "Detected"; break;}
			case ETargetCategory.FRIENDLY : {enumToString = "Friendly"; break;}
			case ETargetCategory.ENEMY : {enumToString = "Enemy"; break;}
			case ETargetCategory.FACTIONLESS : {enumToString = "Factionless"; break;}
			case ETargetCategory.STATIC : {enumToString = "Static"; break;}
		}
		return "MaxDistance:" + m_maxDistance.ToString() + "\n" + "LastSeenMax: " + m_lastSeenMax.ToString() + "\n" + "TargetCategory: " + enumToString;
	}
	
	override void OnInit(AIAgent owner)
	{
		GenericEntity ent = GenericEntity.Cast(owner.GetControlledEntity());
		if (!ent)
			return;
		
		perceptComp = PerceptionComponent.Cast(ent.FindComponent(PerceptionComponent));
		
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
		
		confComp = SCR_AIConfigComponent.Cast(owner.FindComponent(SCR_AIConfigComponent));	
		
		m_targetInfo = new ref SCR_AITargetInfo();
	}
		
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!confComp)
			return ENodeResult.FAIL;
		
		if (perceptComp && confComp.m_EnablePerception)
		{
			if (!owner.GetControlledEntity())
				return ENodeResult.FAIL;
			
			vector ownerLocation = owner.GetControlledEntity().GetOrigin();
			
			//m_CombatComponent.m_Enemies.Clear();
			BaseTarget target = perceptComp.GetClosestTarget(m_targetType, m_lastSeenMax);
			if (target)
			{
				m_targetInfo.m_TargetEntity = target.GetTargetEntity();
				m_targetInfo.m_vLastSeenPosition = target.GetLastSeenPosition();
				m_targetInfo.m_fLastSeenTime = target.GetTimeSinceSeen();
			}
			else
			{
				m_targetInfo.m_TargetEntity = null;
			}
			
			if ( m_targetInfo.m_TargetEntity )
			{
#ifdef WORKBENCH
				if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_LASTSEEN))
					m_Shape = Shape.CreateSphere(COLOR_BLUE_A, ShapeFlags.NOOUTLINE|ShapeFlags.NOZBUFFER|ShapeFlags.TRANSP, m_targetInfo.m_vLastSeenPosition, 0.1);
#endif		
				if(!m_targetOut || m_targetOut.m_TargetEntity != m_targetInfo.m_TargetEntity)
					m_targetOut = new SCR_AITargetInfo(m_targetInfo.m_TargetEntity, m_targetInfo.m_vLastSeenPosition, m_targetInfo.m_fLastSeenTime);
				SetVariableOut("EntityOut",m_targetOut.m_TargetEntity);
				SetVariableOut("EntityPos",m_targetOut.m_vLastSeenPosition);
				SetVariableOut("TargetInfo",m_targetOut);
				return ENodeResult.SUCCESS;
			}
			else
			{
				ClearVariable("EntityOut");
				ClearVariable("EntityPos");
				ClearVariable("TargetInfo");
				return ENodeResult.FAIL;
			}
		}
		else if (!confComp.m_EnablePerception)
		{
			ClearVariable("EntityOut");
			ClearVariable("EntityPos");
			ClearVariable("TargetInfo");
		}
		return ENodeResult.FAIL;
	}
	
	protected static ref TStringArray s_aVarsOut = {
		"EntityOut",
		"EntityPos",
		"TargetInfo"
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
};