//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Cancel
class SCR_AISendGoalMessage_Cancel : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Cancel msg = new SCR_AIMessage_Cancel();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Attack
class SCR_AISendGoalMessage_Attack : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Attack msg = new SCR_AIMessage_Attack();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackDone
class SCR_AISendGoalMessage_AttackDone : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackDone msg = new SCR_AIMessage_AttackDone();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackStatic
class SCR_AISendGoalMessage_AttackStatic : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackStatic msg = new SCR_AIMessage_AttackStatic();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_AttackStaticDone
class SCR_AISendGoalMessage_AttackStaticDone : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_AttackStaticDone msg = new SCR_AIMessage_AttackStaticDone();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_CoverAdvance
class SCR_AISendGoalMessage_CoverAdvance : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_CoverAdvance msg = new SCR_AIMessage_CoverAdvance();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_KeepFormation
class SCR_AISendGoalMessage_KeepFormation : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_KeepFormation msg = new SCR_AIMessage_KeepFormation();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Move
class SCR_AISendGoalMessage_Move : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Move msg = new SCR_AIMessage_Move();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Follow
class SCR_AISendGoalMessage_Follow : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles",
		"Distance"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Follow msg = new SCR_AIMessage_Follow();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		GetVariableIn("Distance", msg.m_fDistance);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Investigate
class SCR_AISendGoalMessage_Investigate : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	float m_fRadius;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsDangerous;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAIUnitType))]
	EAIUnitType m_eTargetUnitType;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"ObjectEntity",
		"MovePosition",
		"Radius",
		"IsDangerous"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Investigate msg = new SCR_AIMessage_Investigate();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("ObjectEntity", msg.m_ObjectEntity);
		
		GetVariableIn("MovePosition", msg.m_vMovePosition);
		
		if(!GetVariableIn("Radius", msg.m_fRadius))
			msg.m_fRadius = m_fRadius;
		
		if(!GetVariableIn("IsDangerous", msg.m_bIsDangerous))
			msg.m_bIsDangerous = m_bIsDangerous;
		
		msg.m_eTargetUnitType = m_eTargetUnitType;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_fRadius: %1\n", m_fRadius);
		s = s + string.Format("m_bIsDangerous: %1\n", m_bIsDangerous);
		s = s + string.Format("m_eTargetUnitType: %1\n", typename.EnumToString(EAIUnitType, m_eTargetUnitType));
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_SeekAndDestroy
class SCR_AISendGoalMessage_SeekAndDestroy : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_SeekAndDestroy msg = new SCR_AIMessage_SeekAndDestroy();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Heal
class SCR_AISendGoalMessage_Heal : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bUseVehicles;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"FollowEntity",
		"MovePosition",
		"UseVehicles"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Heal msg = new SCR_AIMessage_Heal();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("FollowEntity", msg.m_FollowEntity);
		
		GetVariableIn("MovePosition", msg.m_MovePosition);
		
		if(!GetVariableIn("UseVehicles", msg.m_bUseVehicles))
			msg.m_bUseVehicles = m_bUseVehicles;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_bUseVehicles: %1\n", m_bUseVehicles);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_ProvideAmmo
class SCR_AISendGoalMessage_ProvideAmmo : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"AmmoConsumerEntity",
		"MagazineWellType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_ProvideAmmo msg = new SCR_AIMessage_ProvideAmmo();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("AmmoConsumerEntity", msg.m_AmmoConsumer);
		
		GetVariableIn("MagazineWellType", msg.m_MagazineWell);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_PickupInventoryItems
class SCR_AISendGoalMessage_PickupInventoryItems : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"PickupPosition",
		"MagazineWellType"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_PickupInventoryItems msg = new SCR_AIMessage_PickupInventoryItems();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("PickupPosition", msg.m_vPickupPosition);
		
		GetVariableIn("MagazineWellType", msg.m_MagazineWellType);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Defend
class SCR_AISendGoalMessage_Defend : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	float m_fDefendAngularRange;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"Waypoint",
		"DefendLocation",
		"DefendAngularRange"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Defend msg = new SCR_AIMessage_Defend();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("Waypoint", msg.m_RelatedWaypoint);
		
		GetVariableIn("DefendLocation", msg.m_vDefendLocation);
		
		if(!GetVariableIn("DefendAngularRange", msg.m_fDefendAngularRange))
			msg.m_fDefendAngularRange = m_fDefendAngularRange;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_fDefendAngularRange: %1\n", m_fDefendAngularRange);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Retreat
class SCR_AISendGoalMessage_Retreat : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Retreat msg = new SCR_AIMessage_Retreat();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_HealWait
class SCR_AISendGoalMessage_HealWait : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"HealProviderEntity"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_HealWait msg = new SCR_AIMessage_HealWait();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("HealProviderEntity", msg.m_HealProvider);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_PerformAction
class SCR_AISendGoalMessage_PerformAction : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.Auto)]
	string m_sSmartActionTag;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"SmartAction",
		"SmartActionEntity",
		"SmartActionTag"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_PerformAction msg = new SCR_AIMessage_PerformAction();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("SmartAction", msg.m_SmartActionComponent);
		
		GetVariableIn("SmartActionEntity", msg.m_SmartActionEntity);
		
		if(!GetVariableIn("SmartActionTag", msg.m_SmartActionTag))
			msg.m_SmartActionTag = m_sSmartActionTag;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_sSmartActionTag: %1\n", m_sSmartActionTag);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_GetIn
class SCR_AISendGoalMessage_GetIn : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAICompartmentType))]
	EAICompartmentType m_eRoleInVehicle;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"VehicleEntity",
		"RoleInVehicle"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_GetIn msg = new SCR_AIMessage_GetIn();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("VehicleEntity", msg.m_Vehicle);
		
		if(!GetVariableIn("RoleInVehicle", msg.m_eRoleInVehicle))
			msg.m_eRoleInVehicle = m_eRoleInVehicle;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		s = s + string.Format("m_eRoleInVehicle: %1\n", typename.EnumToString(EAICompartmentType, m_eRoleInVehicle));
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_GetOut
class SCR_AISendGoalMessage_GetOut : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"VehicleEntity"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_GetOut msg = new SCR_AIMessage_GetOut();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("VehicleEntity", msg.m_Vehicle);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_Flee
class SCR_AISendGoalMessage_Flee : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_Flee msg = new SCR_AIMessage_Flee();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_ThrowGrenadeTo
class SCR_AISendGoalMessage_ThrowGrenadeTo : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetEntity",
		"TargetPosition"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_ThrowGrenadeTo msg = new SCR_AIMessage_ThrowGrenadeTo();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetEntity", msg.m_TargetEntity);
		
		GetVariableIn("TargetPosition", msg.m_vTargetPosition);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_GroupAttack
class SCR_AISendGoalMessage_GroupAttack : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated",
		"TargetInfo"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_GroupAttack msg = new SCR_AIMessage_GroupAttack();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		GetVariableIn("TargetInfo", msg.m_TargetInfo);
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
}

//---------------------------------------------------------------------------------------
// Generated from class: SCR_AIMessage_GroupAttackDone
class SCR_AISendGoalMessage_GroupAttackDone : SCR_AISendMessageGenerated
{
	[Attribute("", UIWidgets.Auto)]
	float m_fPriorityLevel;
	
	[Attribute("", UIWidgets.Auto)]
	bool m_bIsWaypointRelated;
	
	protected static ref TStringArray _s_aVarsIn =
	{
		SCR_AISendMessageGenerated.PORT_RECEIVER,
		"PriorityLevel",
		"IsWaypointRelated"
	};
	override TStringArray GetVariablesIn() { return _s_aVarsIn; }
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIAgent receiver = GetReceiverAgent(owner);
		SCR_AIMessage_GroupAttackDone msg = new SCR_AIMessage_GroupAttackDone();
		
		msg.m_RelatedGroupActivity = GetRelatedActivity(owner);
		
		msg.SetText(m_sText);
		
		if(!GetVariableIn("PriorityLevel", msg.m_fPriorityLevel))
			msg.m_fPriorityLevel = m_fPriorityLevel;
		
		if(!GetVariableIn("IsWaypointRelated", msg.m_bIsWaypointRelated))
			msg.m_bIsWaypointRelated = m_bIsWaypointRelated;
		
		if (SendMessage(owner, receiver, msg))
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	override string GetNodeMiddleText()
	{
		string s;
		s = s + string.Format("m_fPriorityLevel: %1\n", m_fPriorityLevel);
		s = s + string.Format("m_bIsWaypointRelated: %1\n", m_bIsWaypointRelated);
		return s;
	}
	override bool VisibleInPalette() { return true; }
};
