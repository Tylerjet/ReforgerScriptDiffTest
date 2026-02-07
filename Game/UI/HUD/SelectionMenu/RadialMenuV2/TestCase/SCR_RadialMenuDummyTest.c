//------------------------------------------------------------------------------------------------
//! Testing radial menu case for multiple entries
class SCR_RadialMenuDummyTest : SCR_RadialMenuHandler
{
	protected int count = 16;
	
	[Attribute("", UIWidgets.Object, "")]
	protected ref array<ref ScriptedSelectionMenuEntry> m_aEntries;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnOpen(IEntity owner)
	{
		//UpdateEntriesData(owner, m_aEntries.Count());

		super.OnOpen(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void FillEntry(IEntity owner, BaseSelectionMenuEntry entry, int index)
	{
		// Get data
		if (index > count)
			return;
		
		// Setup entry - this will define entry type
		//SCR_DummyMenuEntry dummyEntry = new ref SCR_DummyMenuEntry();
		entry = m_aEntries[index];
		
		// Pass entry
		super.FillEntry(owner, entry, index);
	}
};