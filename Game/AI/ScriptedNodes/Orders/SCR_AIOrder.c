enum EOrderType_Character
{
	NONE,
	STANCE,
	WEAPON_RAISED,
	MOVEMENT_TYPE,
	BACK_TO_DEFAULT,
	WEAPON_TYPE,
	AI_STATE,
	UNIT_STATE,
	COMBAT_TYPE,
};

class SCR_AIOrderBase : AIOrder
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event generating the order", "", ParamEnumArray.FromEnum(EOrderType_Character) )]
	EOrderType_Character m_eUIType;
	
	EOrderType_Character m_eOrderType;
	void GetOrderParameters(SCR_AIProcessOrder node);
	void SetOrderParameters(SCR_AISendOrder node)
	{
		string orderDebugText;
		if (!node.GetVariableIn(node.ORDER_DEBUG_TEXT, orderDebugText))
			orderDebugText = node.m_string;
		SetText(orderDebugText);
		SetReceiver(node.m_Receiver);
	}
};

class SCR_AIOrder_Stance : SCR_AIOrderBase
{
	ECharacterStance m_eStance;
	
	void SCR_AIOrder_Stance()
	{
		m_eOrderType = EOrderType_Character.STANCE;
	}
	
	static SCR_AIOrder_Stance Create(ECharacterStance stance)
	{
		SCR_AIOrder_Stance m = new SCR_AIOrder_Stance();
		m.m_eStance = stance;
		return m;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.STANCE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eStance);
		if (node.m_bDebugMe) 
			node.m_sDebugString = "Stance "+ typename.EnumToString(ECharacterStance, m_eStance) + " by " + node.m_Order.GetText();
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_eStance))
			m_eStance = node.m_integer;
	}
};

class SCR_AIOrder_WeaponRaised : SCR_AIOrderBase
{
	int m_bWeaponRaised;
	
	void SCR_AIOrder_WeaponRaised()
	{
		m_eOrderType = EOrderType_Character.WEAPON_RAISED;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.WEAPON_RAISED);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_bWeaponRaised);
		if (node.m_bDebugMe) 
			node.m_sDebugString = "Weapon raised "+ m_bWeaponRaised.ToString() + " by " + node.m_Order.GetText();
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_bWeaponRaised))
			m_bWeaponRaised = node.m_integer;
	}
};

class SCR_AIOrder_MovementType : SCR_AIOrderBase
{
	EMovementType m_eMovementType;
	
	void SCR_AIOrder_MovementType()
	{
		m_eOrderType = EOrderType_Character.MOVEMENT_TYPE;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.MOVEMENT_TYPE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eMovementType);
		if (node.m_bDebugMe) 
			node.m_sDebugString = "MovementType "+ typename.EnumToString(EMovementType, m_eMovementType) + " by " + node.m_Order.GetText();
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_eMovementType))
			m_eMovementType = node.m_integer;
	}
};

class SCR_AIOrder_ReturnToDefault : SCR_AIOrderBase
{
	void SCR_AIOrder_ReturnToDefault()
	{
		m_eOrderType = EOrderType_Character.BACK_TO_DEFAULT;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.BACK_TO_DEFAULT);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		if (node.m_bDebugMe) 
			node.m_sDebugString = "Default set ";
	}	
};

class SCR_AIOrder_WeaponType : SCR_AIOrderBase
{
	EWeaponType m_eWeaponType;
	
	void SCR_AIOrder_WeaponType()
	{
		m_eOrderType = EOrderType_Character.WEAPON_TYPE;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.WEAPON_TYPE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eWeaponType);
		if (node.m_bDebugMe)
			node.m_sDebugString = "Weapon type "+ typename.EnumToString(EWeaponType, m_eWeaponType) + " by " + node.m_Order.GetText();
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_eWeaponType))
			m_eWeaponType = node.m_integer;
	}
};

class SCR_AIOrder_AIState : SCR_AIOrderBase
{
	EUnitAIState m_eAIState;
	
	void SCR_AIOrder_AIState()
	{
		m_eOrderType = EOrderType_Character.AI_STATE;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.AI_STATE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eAIState);
		if (node.m_bDebugMe)
			node.m_sDebugString = "AI State "+ typename.EnumToString(EUnitAIState, m_eAIState) + " by " + node.m_Order.GetText(); 						
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_eAIState))
			m_eAIState = node.m_integer;
	}
};

class SCR_AIOrder_Unit_State : SCR_AIOrderBase
{
	EUnitState m_eUnitState;
	
	void SCR_AIOrder_State()
	{
		m_eOrderType = EOrderType_Character.UNIT_STATE;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.UNIT_STATE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eUnitState);
		if (node.m_bDebugMe) 
			node.m_sDebugString = "State change "+ typename.EnumToString(EUnitState, m_eUnitState) + " by " + node.m_Order.GetText();
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_eUnitState))
			m_eUnitState = node.m_integer;
	}
};

class SCR_AIOrder_CombatType : SCR_AIOrderBase
{
	EAICombatType m_eCombatType;
	
	void SCR_AIOrder_CombatType()
	{
		m_eOrderType = EOrderType_Character.COMBAT_TYPE;
	}
	
	override void GetOrderParameters(SCR_AIProcessOrder node)
	{
		node.SetVariableOut(node.ORDER_TYPE, EOrderType_Character.COMBAT_TYPE);
		node.SetVariableOut(node.SCRIPTED_ORDER, true);
		node.SetVariableOut(node.ORDER_VALUE, m_eCombatType);
		if (node.m_bDebugMe) 
			node.m_sDebugString = "Combat type "+ typename.EnumToString(EAICombatType, m_eCombatType) + " by " + node.m_Order.GetText();
	}
	
	override void SetOrderParameters(SCR_AISendOrder node)
	{
		super.SetOrderParameters(node);
		if(!node.GetVariableIn(node.ORDER_VALUE, m_eCombatType))
			m_eCombatType = node.m_integer;
	}
};