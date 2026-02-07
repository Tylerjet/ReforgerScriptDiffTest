class SCR_PlayerList : ScriptedWidgetComponent
{
	[Attribute("Flag")]
	protected string m_sFlag;
	protected ImageWidget m_wFlag;
	
	[Attribute("Name")]
	protected string m_sName;
	protected TextWidget m_wName;	
	
	[Attribute("Count")]
	protected string m_sPlayerCount;
	protected TextWidget m_wPlayerCount;
	
	[Attribute("Players")]
	protected string m_sPlayerList;
	protected Widget m_wPlayerList;	
	
	[Attribute("{E49B28A2FAEBA264}UI/layouts/Menus/DeployMenu/PlayerName.layout")]
	protected ResourceName m_sPlayerName;
	
	protected ref array<SCR_PlayerName> m_aPlayerNames = {};
	protected Widget m_wRoot;

	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_wFlag = ImageWidget.Cast(w.FindAnyWidget(m_sFlag));
		m_wName = TextWidget.Cast(w.FindAnyWidget(m_sName));
		m_wPlayerCount = TextWidget.Cast(w.FindAnyWidget(m_sPlayerCount));
		m_wPlayerList = w.FindAnyWidget(m_sPlayerList);
	}
	
	void UpdatePlayerList()
	{
	}
	
	void ShowPlayerList(bool show)
	{
		m_wRoot.SetVisible(show);
	}
	
	protected void CreatePlayerName(int pid)
	{
		Widget player = GetGame().GetWorkspace().CreateWidgets(m_sPlayerName, m_wPlayerList);
		if (player)
		{
			SCR_PlayerName playerName = SCR_PlayerName.Cast(player.FindHandler(SCR_PlayerName));
			playerName.SetPlayer(pid);
			// playerName.SetIcon(ResourceName.Empty); // todo@lk: set some icon from somewhere
			m_aPlayerNames.Insert(playerName);
		}	
	}	
};

class SCR_FactionPlayerList : SCR_PlayerList
{
	protected SCR_Faction m_Faction;

	void SetFaction(Faction faction)
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(faction);
		if (!scrFaction)
			return;

		m_Faction = scrFaction;

		if (m_wFlag && !scrFaction.GetFactionFlag().IsEmpty())
			m_wFlag.LoadImageTexture(0, scrFaction.GetFactionFlag());

		if (m_wName)
			m_wName.SetText(scrFaction.GetFactionName());

		UpdatePlayerList();
	}

	override void UpdatePlayerList()
	{
		if (!m_Faction || !m_wPlayerList)
			return;

		m_aPlayerNames.Clear();

		Widget child = m_wPlayerList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);

		foreach (int pid : players)
		{
			if (SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(pid)) == m_Faction)
			{
				CreatePlayerName(pid);
			}
		}

		if (m_wPlayerCount)
			m_wPlayerCount.SetText(m_aPlayerNames.Count().ToString());
	}
};

class SCR_GroupPlayerList : SCR_PlayerList
{
	protected SCR_AIGroup m_Group;
	
	void SetGroup(SCR_AIGroup group)
	{
		if (!group)
			return;
		
		m_Group = group;

		if (m_wFlag && !group.GetGroupFlag().IsEmpty())
			m_wFlag.LoadImageTexture(0, group.GetGroupFlag());

		if (m_wName)
		{
			string name = m_Group.GetCustomName();
			if (name.IsEmpty())
			{
				string company, platoon, squad, character, format;
				m_Group.GetCallsigns(company, platoon, squad, character, format);
				name = string.Format("%1 %2 %3 %4 %5", company, platoon, squad, character, format);
			}

			m_wName.SetText(name);
		}

		UpdatePlayerList();
	}
	
	override void UpdatePlayerList()
	{
		if (!m_Group || !m_wPlayerList)
			return;

		m_aPlayerNames.Clear();

		Widget child = m_wPlayerList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		array<int> players = m_Group.GetPlayerIDs();
		foreach (int pid : players)
		{
			CreatePlayerName(pid);
		}

		if (m_wPlayerCount)
			m_wPlayerCount.SetText(m_aPlayerNames.Count().ToString());		
	}
};

class SCR_PlayerName : ScriptedWidgetComponent
{
	[Attribute("Icon")]
	protected string m_sIcon;
	protected ImageWidget m_wIcon;

	[Attribute("Name")]
	protected string m_sName;
	protected TextWidget m_wName;

	protected ResourceName m_sIcons = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset";
	protected int m_iPlayerId;

	override void HandlerAttached(Widget w)
	{
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
		m_wName = TextWidget.Cast(w.FindAnyWidget(m_sName));
	}

	void SetIcon(ResourceName icon)
	{
		if (!m_wIcon)
			return;

		if (m_wIcon)
			m_wIcon.SetVisible(m_wIcon.LoadImageTexture(0, icon));
	}

	void SetPlayer(int pid)
	{
		m_iPlayerId = pid;
		if (m_wName)
			m_wName.SetText(GetGame().GetPlayerManager().GetPlayerName(pid));
		SetPlatform();
	}

	int GetPlayerId()
	{
		return m_iPlayerId;
	}

	protected void SetPlatform()
	{
		// todo@lk: local profile pic
		PlatformKind platform = GetGame().GetPlayerManager().GetPlatformKind(m_iPlayerId);
		string icon;
		if (platform == PlatformKind.NONE)
			icon = "platform-windows";
		else if (platform == PlatformKind.XBOX)
			icon = "platform-xbox";
		else if (platform == PlatformKind.PSN)
			icon = "platform-playstation";
		else if (platform == PlatformKind.STEAM)
			icon = "platform-windows";

		if (m_wIcon)
			m_wIcon.LoadImageFromSet(0, m_sIcons, icon);
	}
};