//------------------------------------------------------------------------------------------------
//! Used for storing client data to be reapplied for reconnecting clients
class SCR_CampaignClientData
{
	private string m_sID;
	private int m_iXP;
	private vector m_vPos;
	private bool m_bApplied = true;
	private int m_iFaction = -1;
	private ref array<string> m_aInventory = new array<string>();
	
	//------------------------------------------------------------------------------------------------
	//! Setter for client ID
	void SetID(string ID)
	{
		m_sID = ID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for client ID
	string GetID()
	{
		return m_sID;
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
	void SetFactionIndex(int faction)
	{
		m_iFaction = faction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter for player's faction
	int GetFactionIndex()
	{
		return m_iFaction;
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