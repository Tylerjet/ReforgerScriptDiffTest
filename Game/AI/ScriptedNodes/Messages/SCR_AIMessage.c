enum EMessageType_Info
{
	NONE,
	CONTACT,
	TARGET_LOST,
	NO_AMMO,
	WOUNDED,
	FOUND_CORPSE,
	UNDER_FIRE,
	REQ_RETREAT,
	REQ_AMMO,
	TARGET_ELIMINATED,
	SIT_REP,	
};

enum EMessageType_Goal
{
	NONE,
	ATTACK,
	ATTACK_DONE,
	MOVE_IN_FORMATION,
	GROUP_ATTACK,
	GROUP_ATTACK_DONE,
	ATTACK_STATIC,
	ATTACK_STATIC_DONE,	
	SEEK_DESTROY,
	FLEE,
	GET_IN_VEHICLE,
	GET_OUT_VEHICLE,
	COVER_ADVANCE,
	MOVE,
	FOLLOW,
	UNGROUP,
	HEAL,
	HEAL_WAIT,
	RESUPPLY,
	INVESTIGATE,
	DEFEND,
	RETREAT,
	PERFORM_ACTION,
	CANCEL,
	SIT_REP
};

//----------------- EXPAND MESSAGE TYPES
class SCR_AIMessageBase : AIMessage
{
	int m_MessageType;	// here is kept enum for message type, which enum it is depends on IsInherited()
	
	void SetMessageParameters(SCR_AISendMessage node) // this method fills an existing message with content
	{
		string debugText;
		if (!node.GetVariableIn(node.PORT_STRING,debugText))
			debugText = node.m_string;
		SetText(debugText);			
		SetReceiver(node.m_Receiver);
		
	}	
	
	string GetDebugText()
	{
		return string.Format("\t%1", this);
	}
};

class SCR_AIMessageGoal : SCR_AIMessageBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event generating the message", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	EMessageType_Goal m_eUIType;
	
	ref SCR_AIActivityBase m_RelatedGroupActivity;
	bool m_bIsPriority;	
	bool m_bIsWaypointRelated;
	
	void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node);
		m_RelatedGroupActivity = relatedActivity;
		if (!node.GetVariableIn(node.PORT_PRIORITIZE,m_bIsPriority))
			m_bIsPriority = node.m_bPrioritize;
		m_bIsWaypointRelated = node.m_bIsWaypointRelated;
	}
	
	override string GetDebugText()
	{
		return super.GetDebugText() + string.Format("\tType: %1\n", typename.EnumToString(EMessageType_Goal, m_eUIType));
	}
};

class SCR_AIMessageInfo : SCR_AIMessageBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event generating the message", "", ParamEnumArray.FromEnum(EMessageType_Info) )]
	EMessageType_Info m_eUIType;
	
	override string GetDebugText()
	{
		return super.GetDebugText() + string.Format("\tType: %1\n", typename.EnumToString(EMessageType_Info, m_eUIType));
	}
};

//----------------- EXPAND MESSAGE SUBTYPES - info type for relaying information

class SCR_AIMessage_Target : SCR_AIMessageInfo
{
	IEntity m_Target;
	vector m_LastSeenPosition;	
	
	
	override void SetMessageParameters(SCR_AISendMessage node)
	{
		super.SetMessageParameters(node);
		
		node.GetVariableIn(node.PORT_ENTITY, m_Target);
		if (!node.GetVariableIn(node.PORT_VECTOR, m_LastSeenPosition))
			m_LastSeenPosition = node.m_vector;
	}
};

class SCR_AIMessage_Contact : SCR_AIMessage_Target
{
	void SCR_AIMessage_Contact() 
	{
		m_MessageType = EMessageType_Info.CONTACT;
	}
};

class SCR_AIMessage_TargetLost : SCR_AIMessage_Target
{
	void SCR_AIMessage_TargetLost() 
	{
		m_MessageType = EMessageType_Info.TARGET_LOST;
	}
};

class SCR_AIMessage_TargetEliminated : SCR_AIMessage_Target
{
	void SCR_AIMessage_TargetEliminated() 
	{
		m_MessageType = EMessageType_Info.TARGET_ELIMINATED;
	}
};

class SCR_AIMessage_NoAmmo : SCR_AIMessageInfo
{
	IEntity m_entityToSupply;
	typename m_MagazineWell;
	
	void SCR_AIMessage_NoAmmo() 
	{
		m_MessageType = EMessageType_Info.NO_AMMO;
	}
	
	override void SetMessageParameters(SCR_AISendMessage node)
	{
		super.SetMessageParameters(node);
		
		node.GetVariableIn(node.PORT_ENTITY, m_entityToSupply);
		node.GetVariableIn(node.PORT_TYPENAME, m_MagazineWell);
	}
};

class SCR_AIMessage_UnderFire : SCR_AIMessageInfo
{
	vector m_vPosition;	
	
	void SCR_AIMessage_UnderFire() 
	{
		m_MessageType = EMessageType_Info.UNDER_FIRE;
	}
	
	override void SetMessageParameters(SCR_AISendMessage node)
	{
		super.SetMessageParameters(node);
		
		if (!node.GetVariableIn(node.PORT_VECTOR, m_vPosition))
			m_vPosition = node.m_vector;
	}
};

class SCR_AIMessage_FoundCorpse : SCR_AIMessageInfo
{
	vector m_vPosition;
	
	void SCR_AIMessage_FoundCorpse() 
	{
		m_MessageType = EMessageType_Info.FOUND_CORPSE;
	}
	
	override void SetMessageParameters(SCR_AISendMessage node)
	{
		super.SetMessageParameters(node);
		
		if (!node.GetVariableIn(node.PORT_VECTOR, m_vPosition))
			m_vPosition = node.m_vector;
	}		
};

class SCR_AIMessage_Wounded : SCR_AIMessage_FoundCorpse
{
	IEntity m_WoundedEntity;
	
	void SCR_AIMessage_Wounded() 
	{
		m_MessageType = EMessageType_Info.WOUNDED;
	}
	
	static SCR_AIMessage_Wounded Create(IEntity woundedEntity)
	{
		SCR_AIMessage_Wounded m = new SCR_AIMessage_Wounded();
		m.m_WoundedEntity = woundedEntity;
		return m;
	}
	
	override void SetMessageParameters(SCR_AISendMessage node)
	{
		super.SetMessageParameters(node);
		
		node.GetVariableIn(node.PORT_ENTITY, m_WoundedEntity);
		if (!node.GetVariableIn(node.PORT_VECTOR, m_vPosition))
			m_vPosition = node.m_vector;
	}		
};

class SCR_AIMessage_SitRep : SCR_AIMessageInfo
{
	int m_MagazinesCount,m_BandagesCount;
	
	void SCR_AIMessage_SitRep() 
	{
		m_MessageType = EMessageType_Info.SIT_REP;
	}		
};

//----------------- EXPAND MESSAGE SUBTYPES - goal type for issuing commands

class SCR_AIMessage_Cancel : SCR_AIMessageGoal 
{
	void SCR_AIMessage_Cancel() 
	{
		m_MessageType = EMessageType_Goal.CANCEL;
	}	
};

class SCR_AIMessage_Attack : SCR_AIMessageGoal
{
	IEntity m_Target;
	vector m_LastSeenPosition;	
	
	void SCR_AIMessage_Attack() 
	{
		m_MessageType = EMessageType_Goal.ATTACK;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{	
		super.SetMessageParameters(node,relatedActivity);
		
		node.GetVariableIn(node.PORT_ENTITY, m_Target);
		if (!node.GetVariableIn(node.PORT_VECTOR, m_LastSeenPosition))
			m_LastSeenPosition = node.m_vector;
	}		
};

class SCR_AIMessage_AttackDone : SCR_AIMessageGoal 
{
	void SCR_AIMessage_AttackDone() 
	{
		m_MessageType = EMessageType_Goal.ATTACK_DONE;
	}
};

class SCR_AIMessage_AttackStatic : SCR_AIMessage_Attack
{
	void SCR_AIMessage_AttackStatic() 
	{
		m_MessageType = EMessageType_Goal.ATTACK_STATIC;
	}
};

class SCR_AIMessage_AttackStaticDone : SCR_AIMessageGoal
{
	void SCR_AIMessage_AttackStaticDone() 
	{
		m_MessageType = EMessageType_Goal.ATTACK_STATIC_DONE;
	}
};

class SCR_AIMessage_CoverAdvance : SCR_AIMessage_Attack 
{
	void SCR_AIMessage_CoverAdvance() 
	{
		m_MessageType = EMessageType_Goal.COVER_ADVANCE;
	}
};

class SCR_AIMessage_KeepFormation : SCR_AIMessageGoal 
{
	void SCR_AIMessage_KeepFormation() 
	{
		m_MessageType = EMessageType_Goal.MOVE_IN_FORMATION;
	}
};

class SCR_AIMessage_Move : SCR_AIMessageGoal
{
	IEntity m_FollowEntity;
	vector m_MovePosition;		
	bool m_UseVehicles;
	
	void SCR_AIMessage_Move() 
	{
		m_MessageType = EMessageType_Goal.MOVE;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_FollowEntity);
		if (!node.GetVariableIn(node.PORT_VECTOR, m_MovePosition))
			m_MovePosition = node.m_vector;
		if (!node.GetVariableIn(node.PORT_BOOL, m_UseVehicles))
			m_UseVehicles = node.m_bool;
		if (!node.GetVariableIn(node.PORT_PRIORITIZE,m_bIsPriority))
			m_bIsPriority = SCR_AISendGoalMessage.Cast(node).m_bPrioritize;
	}		
};

class SCR_AIMessage_Follow : SCR_AIMessage_Move
{
	float m_fDistance;
	
	void SCR_AIMessage_Follow() 
	{
		m_MessageType = EMessageType_Goal.FOLLOW;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_FollowEntity);
		node.GetVariableIn(node.PORT_FLOAT, m_fDistance);
		if (!node.GetVariableIn(node.PORT_BOOL, m_UseVehicles))
			m_UseVehicles = node.m_bool;
		if (!node.GetVariableIn(node.PORT_PRIORITIZE,m_bIsPriority))
			m_bIsPriority = SCR_AISendGoalMessage.Cast(node).m_bPrioritize;
	}
};

class SCR_AIMessage_Investigate : SCR_AIMessageGoal
{
	IEntity m_ObjectEntity;
	vector m_vMovePosition;
	float m_fRadius;
	bool m_bIsDangerous;
	
	void SCR_AIMessage_Investigate() 
	{
		m_MessageType = EMessageType_Goal.INVESTIGATE;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_ObjectEntity);
		node.GetVariableIn(node.PORT_VECTOR, m_vMovePosition);
		if (!node.GetVariableIn(node.PORT_FLOAT, m_fRadius))
			m_fRadius = 10.0;		
		if (!node.GetVariableIn(node.PORT_BOOL, m_bIsDangerous))
			m_bIsDangerous = false;
		if (!node.GetVariableIn(node.PORT_PRIORITIZE,m_bIsPriority))
			m_bIsPriority = SCR_AISendGoalMessage.Cast(node).m_bPrioritize;
	}
};

class SCR_AIMessage_SeekAndDestroy : SCR_AIMessage_Move
{
	void SCR_AIMessage_SeekAndDestroy() 
	{
		m_MessageType = EMessageType_Goal.SEEK_DESTROY;
	}
};

class SCR_AIMessage_Heal : SCR_AIMessage_Move
{
	void SCR_AIMessage_Heal() 
	{
		m_MessageType = EMessageType_Goal.HEAL;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_FollowEntity);
		if (!node.GetVariableIn(node.PORT_BOOL, m_UseVehicles))
			m_UseVehicles = node.m_bool;
	}
};

class SCR_AIMessage_Resupply : SCR_AIMessageGoal
{
	IEntity m_ResupplyEntity;
	vector m_ResupplyPosition;		
	typename m_MagazineWell;
	
	void SCR_AIMessage_Resupply() 
	{
		m_MessageType = EMessageType_Goal.RESUPPLY;
	}
		
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_ResupplyEntity);
		node.GetVariableIn(node.PORT_VECTOR, m_ResupplyPosition);
		node.GetVariableIn(node.PORT_TYPENAME, m_MagazineWell);
	}
};

class SCR_AIMessage_Defend : SCR_AIMessageGoal
{
	IEntity m_WaypointEntity
	
	void SCR_AIMessage_Defend() 
	{
		m_MessageType = EMessageType_Goal.DEFEND;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_WaypointEntity);
	}	
};

class SCR_AIMessage_Retreat : SCR_AIMessageGoal
{
	EAIRetreatBehaviorType m_eRetreatType;
	
	void SCR_AIMessage_Retreat()
	{
		m_MessageType = EMessageType_Goal.RETREAT;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		m_eRetreatType = node.m_integer;
	}
};

class SCR_AIMessage_HealWait : SCR_AIMessageGoal
{
	IEntity m_HealProvider;
	
	void SCR_AIMessage_HealWait()
	{
		m_MessageType = EMessageType_Goal.HEAL_WAIT;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_HealProvider);
	}
};

class SCR_AIMessage_PerformAction : SCR_AIMessageGoal
{
	SCR_AISmartActionComponent m_SmartActionComponent
	IEntity m_SmartActionEntity
	string m_SmartActionTag;
	
	void SCR_AIMessage_PerformAction() 
	{
		m_MessageType = EMessageType_Goal.PERFORM_ACTION;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		if (node.IsInherited(SCR_AISendSmartAction))
		{
			node.GetVariableIn(node.PORT_SMARTACTION, m_SmartActionComponent);
			// BLOCKING SMART ACTION FOR OTHER UserSettings
			m_SmartActionComponent.ReserveAction(GetReceiver());
		}	
		else
		{		
			node.GetVariableIn(node.PORT_ENTITY, m_SmartActionEntity);
			node.GetVariableIn(node.PORT_STRING, m_SmartActionTag);
		}	
	}
};

class SCR_AIMessage_Vehicle : SCR_AIMessageGoal
{
	IEntity m_Vehicle;		
};

class SCR_AIMessage_GetIn : SCR_AIMessage_Vehicle
{
	ECompartmentType m_eRoleInVehicle;
	
	void SCR_AIMessage_GetIn() 
	{
		m_MessageType = EMessageType_Goal.GET_IN_VEHICLE;
	}	
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_Vehicle);
		if (!node.GetVariableIn(node.PORT_INTEGER,m_eRoleInVehicle))
			m_eRoleInVehicle = node.m_integer;
	}	
};

class SCR_AIMessage_GetOut : SCR_AIMessage_Vehicle
{
	void SCR_AIMessage_GetOut() 
	{
		m_MessageType = EMessageType_Goal.GET_OUT_VEHICLE;
	}
		
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_Vehicle);
	}
};

class SCR_AIMessage_Flee : SCR_AIMessageGoal
{
	void SCR_AIMessage_Flee() 
	{
		m_MessageType = EMessageType_Goal.FLEE;
	}
};

class SCR_AIMessage_GroupAttack : SCR_AIMessage_Attack
{
	void SCR_AIMessage_GroupAttack() 
	{
		m_MessageType = EMessageType_Goal.GROUP_ATTACK;
	}
};

class SCR_AIMessage_GroupAttackDone : SCR_AIMessageGoal
{
	void SCR_AIMessage_GroupAttackDone() 
	{
		m_MessageType = EMessageType_Goal.GROUP_ATTACK_DONE;
	}
};