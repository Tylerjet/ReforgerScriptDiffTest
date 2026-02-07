//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted", description: "Handles the character's rank.", color: "0 0 255 255")]
class SCR_CharacterRankComponentClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CharacterRankComponent : ScriptComponent
{	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "Rank", enums: ParamEnumArray.FromEnum(ECharacterRank))]
	protected ECharacterRank m_iRank;
	
	protected IEntity m_Owner;
	static ref ScriptInvoker s_OnRankChanged = new ref ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDoSetCharacterRank(ECharacterRank newRank, ECharacterRank prevRank)
	{
		ECharacterRank oldRank = m_iRank;
		m_iRank = newRank;
		OnRankChanged(oldRank, newRank);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_CharacterRankComponent GetCharacterRankComponent(IEntity unit)
	{
		return SCR_CharacterRankComponent.Cast(unit.FindComponent(SCR_CharacterRankComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCharacterRank(ECharacterRank rank)
	{
		if (rank != m_iRank)
		{
			Rpc(RpcDoSetCharacterRank, rank, m_iRank);
			RpcDoSetCharacterRank(rank, m_iRank);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnRankChanged(ECharacterRank prevRank, ECharacterRank newRank)
	{
		s_OnRankChanged.Invoke(prevRank, newRank, m_Owner);
	}
	
	//------------------------------------------------------------------------------------------------
	// !Helper method to easily read a character's rank by providing just the character parameter
	static ECharacterRank GetCharacterRank(IEntity unit)
	{
		if (!unit)
			return ECharacterRank.INVALID;
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return ECharacterRank.INVALID;
		
		return comp.GetCharacterRank();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_MilitaryFaction GetCharacterFaction(IEntity unit)
	{
		if (!unit)
			return null;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return null;
		
		Faction faction = character.GetFaction();
		if (!faction)
			return null;

		return SCR_MilitaryFaction.Cast(faction);
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetCharacterRankName(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		ECharacterRank rank = comp.GetCharacterRank();
		SCR_MilitaryFaction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankName(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetCharacterRankNameUpperCase(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		ECharacterRank rank = comp.GetCharacterRank();
		SCR_MilitaryFaction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankNameUpperCase(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetCharacterRankNameShort(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		ECharacterRank rank = comp.GetCharacterRank();
		SCR_MilitaryFaction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankNameShort(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	static ResourceName GetCharacterRankInsignia(IEntity unit)
	{
		if (!unit)
			return "";
		
		SCR_CharacterRankComponent comp = GetCharacterRankComponent(unit);
		
		if (!comp)
			return "";
		
		ECharacterRank rank = comp.GetCharacterRank();
		SCR_MilitaryFaction faction = comp.GetCharacterFaction(unit);
		
		if (!faction)
			return "";
		
		return faction.GetRankInsignia(rank);
	}
	
	//------------------------------------------------------------------------------------------------
	protected ECharacterRank GetCharacterRank()
	{
		return m_iRank;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteIntRange(m_iRank, 0, ECharacterRank.INVALID-1);
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadIntRange(m_iRank, 0, ECharacterRank.INVALID-1);

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!ChimeraCharacter.Cast(owner))
			Print("SCR_CharacterRankComponent must be attached to ChimeraCharacter!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CharacterRankComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CharacterRankComponent()
	{
	}
};
