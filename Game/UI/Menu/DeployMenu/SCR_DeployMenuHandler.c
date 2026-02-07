/*!
	This component serves as a config for deploy menu elements.
	Set the names of widgets where request handlers are attached to in the deploy menu layout.
	Has to be attached at the root of the deploy menu layout.
*/
class SCR_DeployMenuHandler : ScriptedWidgetComponent
{
	[Attribute("FactionOverlay")]
	protected string m_sFactionUIHandler;
	protected SCR_FactionRequestUIComponent m_FactionRequestHandler;

	[Attribute("LoadoutSelector")]
	protected string m_sLoadoutUIHandler;
	protected SCR_LoadoutRequestUIComponent m_LoadoutRequestHandler;

	[Attribute("GroupSelector")]
	protected string m_sGroupUIHandler;
	protected SCR_GroupRequestUIComponent m_GroupRequestHandler;

	[Attribute("SpawnPointSelector")]
	protected string m_sSpawnPointUIHandler;
	protected SCR_SpawnPointRequestUIComponent m_SpawnPointUIHandler;
	
	[Attribute("FactionPlayerList")]
	protected string m_sFactionPlayerList;
	protected SCR_FactionPlayerList m_FactionPlayerList;	

	[Attribute("GroupPlayerList")]
	protected string m_sGroupPlayerList;
	protected SCR_GroupPlayerList m_GroupPlayerLIst;

	private Widget m_wRoot;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}

	SCR_FactionRequestUIComponent GetFactionRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sFactionUIHandler);
		if (!tmp)
			return null;

		return SCR_FactionRequestUIComponent.Cast(tmp.FindHandler(SCR_FactionRequestUIComponent));
	}

	SCR_LoadoutRequestUIComponent GetLoadoutRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sLoadoutUIHandler);
		if (!tmp)
			return null;

		return SCR_LoadoutRequestUIComponent.Cast(tmp.FindHandler(SCR_LoadoutRequestUIComponent));
	}

	SCR_GroupRequestUIComponent GetGroupRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sGroupUIHandler);
		if (!tmp)
			return null;

		return SCR_GroupRequestUIComponent.Cast(tmp.FindHandler(SCR_GroupRequestUIComponent));
	}

	SCR_SpawnPointRequestUIComponent GetSpawnPointRequestHandler()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sSpawnPointUIHandler);
		if (!tmp)
			return null;

		return SCR_SpawnPointRequestUIComponent.Cast(tmp.FindHandler(SCR_SpawnPointRequestUIComponent));
	}
	
	SCR_FactionPlayerList GetFactionPlayerList()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sFactionPlayerList);
		if (!tmp)
			return null;
		
		return SCR_FactionPlayerList.Cast(tmp.FindHandler(SCR_FactionPlayerList));
	}
	
	SCR_GroupPlayerList GetGroupPlayerList()
	{
		Widget tmp = m_wRoot.FindAnyWidget(m_sGroupPlayerList);
		if (!tmp)
			return null;
		
		return SCR_GroupPlayerList.Cast(tmp.FindHandler(SCR_GroupPlayerList));	
	}
};