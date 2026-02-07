//------------------------------------------------------------------------------------------------
//! Used for storing client data to be reapplied for reconnecting clients
class SCR_CampaignClientData
{
	private int m_iID;
	private int m_iXP;
	private vector m_vPos;
	private bool m_bApplied = true;
	private SCR_CampaignFaction m_Faction;
	private ref array<string> m_aInventory = new array<string>();
	
	//------------------------------------------------------------------------------------------------
	//! Setter for client ID
	void SetID(int ID)
	{
		m_iID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for client ID
	int GetID()
	{
		return m_iID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was the data already applied? (setter)
	void SetApplied(bool applied)
	{
		m_bApplied = applied;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Was the data already applied? (getter)
	bool GetApplied()
	{
		return m_bApplied;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setter for XP
	void SetXP(int XP)
	{
		m_iXP = XP;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for XP
	int GetXP()
	{
		return m_iXP;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setter for player's starting position
	void SetStartingPosition(vector pos)
	{
		m_vPos = pos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for player's starting position
	vector GetStartingPosition()
	{
		return m_vPos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setter for player's faction
	void SetFaction(SCR_CampaignFaction faction)
	{
		m_Faction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for player's faction
	SCR_CampaignFaction GetFaction()
	{
		return m_Faction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setter for player's inventory
	void SetInventory(int PlayerID)
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for player's inventory
	void GetInventory(out notnull array<string> inventory)
	{
		inventory = m_aInventory;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignClientData()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignClientData()
	{
		m_aInventory = null;
	}
};