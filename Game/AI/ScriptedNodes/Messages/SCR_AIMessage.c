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
	ACTION_FAILED, // Reaction is not implemented
	HEAL_FAILED
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
	INVESTIGATE,
	DEFEND,
	RETREAT,
	PERFORM_ACTION,
	CANCEL,
	SIT_REP,
	THROW_GRENADE_TO,
	PROVIDE_AMMO,
	PICKUP_INVENTORY_ITEMS
};

//----------------- EXPAND MESSAGE TYPES
class SCR_AIMessageBase : AIMessage
{
	int m_MessageType;	// here is kept enum for message type, which enum it is depends on IsInherited()
	
	void SetMessageParameters(SCR_AISendMessageBase node) // this method fills an existing message with content
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

//---------------------------------------------------------------------------------------------------
class SCR_AIMessageGoal : SCR_AIMessageBase // MESSAGE_CLASS()
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event generating the message", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	EMessageType_Goal m_eUIType;
	
	ref SCR_AIActivityBase m_RelatedGroupActivity;
	float m_fPriorityLevel;		// VARIABLE(NodePort, PriorityLevel, NodeProperty, m_fPriorityLevel)
	bool m_bIsWaypointRelated;	// VARIABLE(NodePort, IsWaypointRelated, NodeProperty, m_bIsWaypointRelated)
	
	void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node);
		m_RelatedGroupActivity = relatedActivity;
		if (!node.GetVariableIn(node.PORT_PRIORITY_LEVEL, m_fPriorityLevel))
			m_fPriorityLevel = node.m_fPriorityLevel;
		m_bIsWaypointRelated = node.m_bIsWaypointRelated;
	}
	
	override string GetDebugText()
	{
		return super.GetDebugText() + string.Format("\tType: %1\n", typename.EnumToString(EMessageType_Goal, m_eUIType));
	}
};

//---------------------------------------------------------------------------------------------------
class SCR_AIMessageInfo : SCR_AIMessageBase // MESSAGE_CLASS()
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event generating the message", "", ParamEnumArray.FromEnum(EMessageType_Info) )]
	EMessageType_Info m_eUIType;
	
	override string GetDebugText()
	{
		return super.GetDebugText() + string.Format("\tType: %1\n", typename.EnumToString(EMessageType_Info, m_eUIType));
	}
};

//----------------- EXPAND MESSAGE SUBTYPES - info type for relaying information

class SCR_AIMessage_Target : SCR_AIMessageInfo // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_Target)
{
	ref SCR_AITargetInfo m_TargetInfo; // VARIABLE(NodePort, TargetInfo)
	
	override void SetMessageParameters(SCR_AISendMessageBase node)
	{
		super.SetMessageParameters(node);
		
		SCR_AISendMessageGeneric genericNode = SCR_AISendMessageGeneric.Cast(node);
		
		IEntity targetEntity;
		vector pos;
		float time;
		
		genericNode.GetVariableIn(genericNode.PORT_ENTITY, targetEntity);
		if (!genericNode.GetVariableIn(genericNode.PORT_VECTOR, pos))
			pos = genericNode.m_vector;
		genericNode.GetVariableIn(genericNode.PORT_FLOAT, time);
		m_TargetInfo = new SCR_AITargetInfo(targetEntity, pos, time);
	}
};

class SCR_AIMessage_Contact : SCR_AIMessage_Target // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_Contact)
{
	void SCR_AIMessage_Contact() 
	{
		m_MessageType = EMessageType_Info.CONTACT;
	}
};

class SCR_AIMessage_TargetLost : SCR_AIMessage_Target // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_TargetLost)
{
	void SCR_AIMessage_TargetLost() 
	{
		m_MessageType = EMessageType_Info.TARGET_LOST;
	}
};

class SCR_AIMessage_TargetEliminated : SCR_AIMessage_Target // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_TargetEliminated)
{
	void SCR_AIMessage_TargetEliminated() 
	{
		m_MessageType = EMessageType_Info.TARGET_ELIMINATED;
	}
};

class SCR_AIMessage_NoAmmo : SCR_AIMessageInfo // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_NoAmmo)
{
	IEntity m_entityToSupply; // VARIABLE(NodePort, EntityToSupply)
	typename m_MagazineWell; // VARIABLE(NodePort, MagazineWellType)
	
	void SCR_AIMessage_NoAmmo() 
	{
		m_MessageType = EMessageType_Info.NO_AMMO;
	}
	
	override void SetMessageParameters(SCR_AISendMessageBase node)
	{
		super.SetMessageParameters(node);
		
		SCR_AISendMessageGeneric genericNode = SCR_AISendMessageGeneric.Cast(node);
		
		genericNode.GetVariableIn(genericNode.PORT_ENTITY, m_entityToSupply);
		genericNode.GetVariableIn(genericNode.PORT_TYPENAME, m_MagazineWell);
	}
	
	static SCR_AIMessage_NoAmmo Create(IEntity entityToResupply, typename magazineWell)
	{
		SCR_AIMessage_NoAmmo msg = new SCR_AIMessage_NoAmmo();
		
		msg.m_entityToSupply = entityToResupply;
		msg.m_MagazineWell = magazineWell;
		
		return msg;
	}
};

class SCR_AIMessage_UnderFire : SCR_AIMessageInfo // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_UnderFire)
{
	vector m_vPosition;	// VARIABLE(NodePort, Position)
	
	void SCR_AIMessage_UnderFire() 
	{
		m_MessageType = EMessageType_Info.UNDER_FIRE;
	}
	
	override void SetMessageParameters(SCR_AISendMessageBase node)
	{
		super.SetMessageParameters(node);
		
		SCR_AISendMessageGeneric genericNode = SCR_AISendMessageGeneric.Cast(node);
		
		if (!genericNode.GetVariableIn(genericNode.PORT_VECTOR, m_vPosition))
			m_vPosition = genericNode.m_vector;
	}
};

class SCR_AIMessage_FoundCorpse : SCR_AIMessageInfo // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_FoundCorpse)
{
	vector m_vPosition; // VARIABLE(NodePort, Position)
	
	void SCR_AIMessage_FoundCorpse() 
	{
		m_MessageType = EMessageType_Info.FOUND_CORPSE;
	}
	
	override void SetMessageParameters(SCR_AISendMessageBase node)
	{
		super.SetMessageParameters(node);
		
		SCR_AISendMessageGeneric genericNode = SCR_AISendMessageGeneric.Cast(node);
		
		if (!genericNode.GetVariableIn(genericNode.PORT_VECTOR, m_vPosition))
			m_vPosition = genericNode.m_vector;
	}
};

class SCR_AIMessage_Wounded : SCR_AIMessage_FoundCorpse // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_Wounded)
{
	IEntity m_WoundedEntity; // VARIABLE(NodePort, WoundedEntity)
	
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
	
	override void SetMessageParameters(SCR_AISendMessageBase node)
	{
		super.SetMessageParameters(node);
		
		SCR_AISendMessageGeneric genericNode = SCR_AISendMessageGeneric.Cast(node);
		
		genericNode.GetVariableIn(genericNode.PORT_ENTITY, m_WoundedEntity);
		if (!genericNode.GetVariableIn(genericNode.PORT_VECTOR, m_vPosition))
			m_vPosition = genericNode.m_vector;
	}
};

class SCR_AIMessage_SitRep : SCR_AIMessageInfo // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_SitRep)
{
	int m_MagazinesCount,m_BandagesCount;
	
	void SCR_AIMessage_SitRep() 
	{
		m_MessageType = EMessageType_Info.SIT_REP;
	}
};

class SCR_AIMessage_ActionFailed : SCR_AIMessageInfo
{
	// The action which we have failed
	typename m_ActionTypename; 
	
	void SCR_AIMessage_ActionFailed()
	{
		m_MessageType = EMessageType_Info.ACTION_FAILED;
	}
};

class SCR_AIMessage_HealFailed : SCR_AIMessageInfo // MESSAGE_CLASS(GenerateSendInfoMessage, SCR_AISendInfoMessage_HealFailed)
{
	IEntity m_TargetEntity; // VARIABLE(NodePort, TargetEntity)
	
	void SCR_AIMessage_HealFailed()
	{
		m_MessageType = EMessageType_Info.HEAL_FAILED;
	}
};

//----------------- EXPAND MESSAGE SUBTYPES - goal type for issuing commands

class SCR_AIMessage_Cancel : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Cancel)
{
	void SCR_AIMessage_Cancel() 
	{
		m_MessageType = EMessageType_Goal.CANCEL;
	}	
};

class SCR_AIMessage_Attack : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Attack)
{
	ref SCR_AITargetInfo m_TargetInfo; // VARIABLE(NodePort, TargetInfo)
	
	void SCR_AIMessage_Attack() 
	{
		m_MessageType = EMessageType_Goal.ATTACK;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{	
		super.SetMessageParameters(node,relatedActivity);
		
		IEntity targetEntity;
		vector pos;
		float time;
		
		node.GetVariableIn(node.PORT_ENTITY, targetEntity);
		if (!node.GetVariableIn(node.PORT_VECTOR, pos))
			pos = node.m_vector;
		node.GetVariableIn(node.PORT_FLOAT, time);
		m_TargetInfo = new SCR_AITargetInfo(targetEntity, pos, time);		
	}
};

class SCR_AIMessage_AttackDone : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_AttackDone)
{
	void SCR_AIMessage_AttackDone() 
	{
		m_MessageType = EMessageType_Goal.ATTACK_DONE;
	}
};

class SCR_AIMessage_AttackStatic : SCR_AIMessage_Attack // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_AttackStatic)
{
	void SCR_AIMessage_AttackStatic() 
	{
		m_MessageType = EMessageType_Goal.ATTACK_STATIC;
	}
};

class SCR_AIMessage_AttackStaticDone : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_AttackStaticDone)
{
	void SCR_AIMessage_AttackStaticDone() 
	{
		m_MessageType = EMessageType_Goal.ATTACK_STATIC_DONE;
	}
};

class SCR_AIMessage_CoverAdvance : SCR_AIMessage_Attack // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_CoverAdvance)
{
	void SCR_AIMessage_CoverAdvance() 
	{
		m_MessageType = EMessageType_Goal.COVER_ADVANCE;
	}
};

class SCR_AIMessage_KeepFormation : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_KeepFormation)
{
	void SCR_AIMessage_KeepFormation() 
	{
		m_MessageType = EMessageType_Goal.MOVE_IN_FORMATION;
	}
};

class SCR_AIMessage_Move : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Move)
{
	IEntity m_FollowEntity; // VARIABLE(NodePort, FollowEntity)
	vector m_MovePosition; // VARIABLE(NodePort, MovePosition)
	bool m_bUseVehicles; // VARIABLE(NodePort, UseVehicles, NodeProperty, m_bUseVehicles)	
	
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
		if (!node.GetVariableIn(node.PORT_BOOL, m_bUseVehicles))
			m_bUseVehicles = node.m_bool;
		if (!node.GetVariableIn(node.PORT_PRIORITY_LEVEL,m_fPriorityLevel))
			m_fPriorityLevel = SCR_AISendGoalMessage.Cast(node).m_fPriorityLevel;
	}		
};

class SCR_AIMessage_Follow : SCR_AIMessage_Move // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Follow)
{
	float m_fDistance; // VARIABLE(NodePort, Distance)
	
	void SCR_AIMessage_Follow() 
	{
		m_MessageType = EMessageType_Goal.FOLLOW;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_FollowEntity);
		node.GetVariableIn(node.PORT_FLOAT, m_fDistance);
		if (!node.GetVariableIn(node.PORT_BOOL, m_bUseVehicles))
			m_bUseVehicles = node.m_bool;
		if (!node.GetVariableIn(node.PORT_PRIORITY_LEVEL,m_fPriorityLevel))
			m_fPriorityLevel = SCR_AISendGoalMessage.Cast(node).m_fPriorityLevel;
	}
};

class SCR_AIMessage_Investigate : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Investigate)
{
	IEntity m_ObjectEntity; // VARIABLE(NodePort, ObjectEntity)
	vector m_vMovePosition; // VARIABLE(NodePort, MovePosition)
	float m_fRadius; // VARIABLE(NodePort, Radius, NodeProperty, m_fRadius)
	bool m_bIsDangerous; // VARIABLE(NodePort, IsDangerous, NodeProperty, m_bIsDangerous)
	EAIUnitType m_eTargetUnitType = EAIUnitType.UnitType_Infantry; // VARIABLE(NodePropertyEnum, m_eTargetUnitType)
	
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
		if (!node.GetVariableIn(node.PORT_PRIORITY_LEVEL,m_fPriorityLevel))
			m_fPriorityLevel = SCR_AISendGoalMessage.Cast(node).m_fPriorityLevel;
	}
};

class SCR_AIMessage_SeekAndDestroy : SCR_AIMessage_Move // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_SeekAndDestroy)
{
	void SCR_AIMessage_SeekAndDestroy() 
	{
		m_MessageType = EMessageType_Goal.SEEK_DESTROY;
	}
};

class SCR_AIMessage_Heal : SCR_AIMessage_Move // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Heal)
{
	void SCR_AIMessage_Heal() 
	{
		m_MessageType = EMessageType_Goal.HEAL;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_FollowEntity);
		if (!node.GetVariableIn(node.PORT_BOOL, m_bUseVehicles))
			m_bUseVehicles = node.m_bool;
	}
};

class SCR_AIMessage_ProvideAmmo : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_ProvideAmmo)
{
	IEntity m_AmmoConsumer; // VARIABLE(NodePort, AmmoConsumerEntity)
	typename m_MagazineWell; // VARIABLE(NodePort, MagazineWellType)
	
	void SCR_AIMessage_ProvideAmmo()
	{
		m_MessageType = EMessageType_Goal.PROVIDE_AMMO;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(SCR_AISendGoalMessage.PORT_ENTITY, m_AmmoConsumer);
		node.GetVariableIn(SCR_AISendGoalMessage.PORT_TYPENAME, m_MagazineWell);
	}
}

class SCR_AIMessage_PickupInventoryItems : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_PickupInventoryItems)
{
	vector m_vPickupPosition; // VARIABLE(NodePort, PickupPosition)
	typename m_MagazineWellType; // VARIABLE(NodePort, MagazineWellType)
	
	void SCR_AIMessage_PickupInventoryItems()
	{
		m_MessageType = EMessageType_Goal.PICKUP_INVENTORY_ITEMS;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(SCR_AISendGoalMessage.PORT_VECTOR, m_vPickupPosition);
		node.GetVariableIn(SCR_AISendGoalMessage.PORT_TYPENAME, m_MagazineWellType);
	}
}

class SCR_AIMessage_Defend : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Defend)
{
	AIWaypoint m_RelatedWaypoint; 	// VARIABLE(NodePort, Waypoint)
	vector m_vDefendLocation; 		// VARIABLE(NodePort, DefendLocation)
	float m_fDefendAngularRange; 	// VARIABLE(NodePort, DefendAngularRange, NodeProperty, m_fDefendAngularRange)
	
	void SCR_AIMessage_Defend() 
	{
		m_MessageType = EMessageType_Goal.DEFEND;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_RelatedWaypoint);
	}	
};

class SCR_AIMessage_Retreat : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Retreat)
{
	void SCR_AIMessage_Retreat()
	{
		m_MessageType = EMessageType_Goal.RETREAT;
	}
};

class SCR_AIMessage_HealWait : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_HealWait)
{
	IEntity m_HealProvider; // VARIABLE(NodePort, HealProviderEntity)
	
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

class SCR_AIMessage_PerformAction : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_PerformAction)
{
	SCR_AISmartActionComponent m_SmartActionComponent; // VARIABLE(NodePort, SmartAction)
	IEntity m_SmartActionEntity; // VARIABLE(NodePort, SmartActionEntity)
	string m_SmartActionTag; // VARIABLE(NodePort, SmartActionTag, NodeProperty, m_sSmartActionTag)
	
	void SCR_AIMessage_PerformAction() 
	{
		m_MessageType = EMessageType_Goal.PERFORM_ACTION;
	}
	
	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node, relatedActivity);
		node.GetVariableIn(node.PORT_ENTITY, m_SmartActionEntity);
		node.GetVariableIn(node.PORT_STRING, m_SmartActionTag);
	}
};

class SCR_AIMessage_Vehicle : SCR_AIMessageGoal // MESSAGE_CLASS()
{
	IEntity m_Vehicle; // VARIABLE(NodePort, VehicleEntity)
};

class SCR_AIMessage_GetIn : SCR_AIMessage_Vehicle // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_GetIn)
{
	EAICompartmentType m_eRoleInVehicle; // VARIABLE(NodePort, RoleInVehicle, NodePropertyEnum, m_eRoleInVehicle)
	
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

class SCR_AIMessage_GetOut : SCR_AIMessage_Vehicle // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_GetOut)
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

class SCR_AIMessage_Flee : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_Flee)
{
	void SCR_AIMessage_Flee() 
	{
		m_MessageType = EMessageType_Goal.FLEE;
	}
};

class SCR_AIMessage_ThrowGrenadeTo : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_ThrowGrenadeTo)
{
	IEntity m_TargetEntity; // VARIABLE(NodePort, TargetEntity)
	vector m_vTargetPosition; // VARIABLE(NodePort, TargetPosition)
	
	void SCR_AIMessage_ThrowGrenadeTo()
	{
		m_MessageType = EMessageType_Goal.THROW_GRENADE_TO;
	}
	

	override void SetMessageParameters(SCR_AISendGoalMessage node, SCR_AIActivityBase relatedActivity)
	{
		super.SetMessageParameters(node);
		
		SCR_AISendMessageGeneric genericNode = node;
		
		genericNode.GetVariableIn(genericNode.PORT_ENTITY, m_TargetEntity);
		if (!genericNode.GetVariableIn(genericNode.PORT_VECTOR, m_vTargetPosition))
			m_vTargetPosition = genericNode.m_vector;
	}
};

class SCR_AIMessage_GroupAttack : SCR_AIMessage_Attack // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_GroupAttack)
{
	void SCR_AIMessage_GroupAttack() 
	{
		m_MessageType = EMessageType_Goal.GROUP_ATTACK;
	}
};

class SCR_AIMessage_GroupAttackDone : SCR_AIMessageGoal // MESSAGE_CLASS(GenerateSendGoalMessage, SCR_AISendGoalMessage_GroupAttackDone)
{
	void SCR_AIMessage_GroupAttackDone() 
	{
		m_MessageType = EMessageType_Goal.GROUP_ATTACK_DONE;
	}
};