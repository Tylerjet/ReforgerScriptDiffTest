class SCR_DeployMenuTileSelection : ScriptedWidgetComponent
{
	[Attribute()]
	protected ResourceName m_sTilePrefab;

	[Attribute("Tiles")]
	protected string m_sTileContainer;

	protected Widget m_wTileContainer;

	protected SCR_DeployMenuTile m_FocusedTile;
	protected ref array<SCR_DeployMenuTile> m_aTiles = {};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wTileContainer = w.FindAnyWidget(m_sTileContainer);
	}

	//------------------------------------------------------------------------------------------------
	void AddTile(SCR_DeployMenuTile tile)
	{
		m_aTiles.Insert(tile);
	}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		int count = m_aTiles.Count();
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		if (m_aTiles.IsEmpty())
			return;

		Widget child = m_wTileContainer.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			child.RemoveFromHierarchy();
			child = sibling;
		}

		m_aTiles.Clear();
	}

	//------------------------------------------------------------------------------------------------
	SCR_DeployMenuTile GetFocusedTile()
	{
		return m_FocusedTile;
	}

	//------------------------------------------------------------------------------------------------
	void FocusTile(SCR_DeployMenuTile tile)
	{
		m_FocusedTile = tile;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetTileResource()
	{
		return m_sTilePrefab;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetTileContainer()
	{
		return m_wTileContainer;
	}

	//------------------------------------------------------------------------------------------------
	void SetTilesEnabled(bool enabled)
	{
		foreach (SCR_DeployMenuTile tile : m_aTiles)
		{
			tile.SetEnabled(enabled);
		}
	}
};

//------------------------------------------------------------------------------------------------
class SCR_DeployMenuTileBase : SCR_ButtonImageComponent
{
};

//------------------------------------------------------------------------------------------------
class SCR_DeployMenuTile : SCR_DeployMenuTileBase
{
	protected TextWidget m_wText;
	protected SCR_DeployMenuTileSelection m_Parent;
	protected bool m_bClickEnabled = false;

	//------------------------------------------------------------------------------------------------
	protected void EnableButton()
	{
		m_bClickEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!m_bClickEnabled)
			return false;

		super.OnClick(w, x, y, button);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget("Text"));
		GetGame().GetCallqueue().CallLater(EnableButton, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	void ShowTile(bool show)
	{
		GetRootWidget().SetVisible(show);
		GetRootWidget().SetEnabled(show);
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		if (m_wText)
			m_wText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (m_Parent)
			m_Parent.FocusTile(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		if (m_Parent)
			m_Parent.FocusTile(null);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetParent(SCR_DeployMenuTileSelection parent)
	{
		m_Parent = parent;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_FactionMenuTile : SCR_DeployMenuTile
{
	protected Faction m_Faction;
	protected Widget m_wPlayerList;
	protected Widget m_wPlayerListScroll;
	protected TextWidget m_wPlayerCount;
	protected Widget m_wNoSpawnPoints;
	protected ImageWidget m_wFactionBackground;
	protected ref SCR_GamepadScrollComponent m_GamepadScroll;

	//------------------------------------------------------------------------------------------------
	static SCR_FactionMenuTile InitializeTile(SCR_DeployMenuTileSelection parent, SCR_Faction faction)
	{
		Widget tile = GetGame().GetWorkspace().CreateWidgets(parent.GetTileResource());
		SCR_FactionMenuTile handler = SCR_FactionMenuTile.Cast(tile.FindHandler(SCR_FactionMenuTile));
		SCR_GalleryComponent gallery_handler = SCR_GalleryComponent.Cast(parent.GetTileContainer().GetHandler(0));
		if (!handler)
			return null;

		TextWidget playerCount = TextWidget.Cast(tile.FindAnyWidget("PlayerCount"));
		ResourceName flag = faction.GetFactionFlag();

		handler.SetParent(parent);
		handler.SetImage(flag);
		handler.SetText(faction.GetFactionName());
		handler.SetSpawnPointsAvailable(faction.GetFactionKey());
		handler.SetFactionBackgroundColor(faction.GetFactionColor());
		gallery_handler.AddItem(tile);

		return handler;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wPlayerCount = TextWidget.Cast(w.FindAnyWidget("PlayerCount"));
		m_wPlayerList = w.FindAnyWidget("PlayerList");
		m_wPlayerListScroll = w.FindAnyWidget("PlayerListScroll");
		m_wNoSpawnPoints = w.FindAnyWidget("NoSpawnPoints");
		m_wFactionBackground = ImageWidget.Cast(m_wRoot.FindAnyWidget("FactionBckg"));
		if (m_wPlayerListScroll)
			m_GamepadScroll = SCR_GamepadScrollComponent.Cast(m_wPlayerListScroll.FindHandler(SCR_GamepadScrollComponent));

		if (m_wPlayerListScroll)
		{
			SCR_EventHandlerComponent eventHandler = SCR_EventHandlerComponent.Cast(m_wPlayerListScroll.FindHandler(SCR_EventHandlerComponent));
			if (eventHandler)
			{
				eventHandler.GetOnMouseEnter().Insert(FocusTile);
				eventHandler.GetOnMouseLeave().Insert(UnfocusTile);
			}
		}

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(UpdateSpawnPointsCount);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Insert(UpdateSpawnPointsCount);
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusTile(Widget w)
	{
		OnMouseEnter(m_wRoot, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnfocusTile(Widget w)
	{
		OnMouseLeave(m_wRoot, null, 0, 0);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(UpdateSpawnPointsCount);
		SCR_SpawnPoint.Event_SpawnPointRemoved.Remove(UpdateSpawnPointsCount);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && m_GamepadScroll)
			m_GamepadScroll.SetEnabled(true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && m_GamepadScroll)
			m_GamepadScroll.SetEnabled(false);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetPlayerList()
	{
		return m_wPlayerList;
	}

	//------------------------------------------------------------------------------------------------
	void SetPlayerCount(int num)
	{
		m_wPlayerCount.SetText(num.ToString());
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnPointsAvailable(string factionKey)
	{
		if (m_wText.GetText() == factionKey)
		{
			bool available;
			array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(factionKey);
			available = !spawnPoints.IsEmpty();
			m_wNoSpawnPoints.SetVisible(!available);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateSpawnPointsCount(SCR_SpawnPoint sp)
	{
		if (sp)
			SetSpawnPointsAvailable(sp.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionBackgroundColor(Color color)
	{
		m_wFactionBackground.SetColor(color);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_LoadoutMenuTile : SCR_DeployMenuTile
{
	[Attribute("LoadoutMessage", desc: "Widget name of simple message component")]
	protected string m_sSimpleMessageName;
	
	//~ Disabled when loadout page is loaded
	protected bool m_bForceDisabled;
	
	protected ImageWidget m_wIcon;
	protected ImageWidget m_wFactionBackground;
	protected SCR_LoadoutPreviewComponent m_Preview;;

	//------------------------------------------------------------------------------------------------
	static SCR_LoadoutMenuTile InitializeTile(SCR_DeployMenuTileSelection parent, SCR_BasePlayerLoadout loadout)
	{
		Widget tile = GetGame().GetWorkspace().CreateWidgets(parent.GetTileResource());
		SCR_GalleryComponent gallery_handler = SCR_GalleryComponent.Cast(parent.GetTileContainer().GetHandler(0));
		
		SCR_LoadoutMenuTile handler = SCR_LoadoutMenuTile.Cast(tile.FindHandler(SCR_LoadoutMenuTile));
		if (!handler)
			return null;

		handler.SetParent(parent);
		handler.SetPreviewedLoadout(loadout);
		handler.SetText(loadout.GetLoadoutName());
		gallery_handler.AddItem(tile);

		Resource res = Resource.Load(loadout.GetLoadoutResource());
		IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
		BaseContainer container = source.GetObject("m_UIInfo");
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		ResourceName path;
		// get the loadout img
		container.Get("m_Image", path);
		handler.SetImage(path);

		// get the loadout icon
		handler.SetIcon(info);

		return handler;
	}
	
	/*!
	Disable the layout button and show a message over it
	\param messageID ID in simple message component to set message to
	*/
	void DisableAndShowMessage(string messageID)
	{
		m_bForceDisabled = true;
		SetEnabled(false);
		ShowSimpleMessage(true, messageID);
	}
	
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		//~ Only sets the tile as selected if it was not forced disabled. Else it will set itself as null
		if (m_Parent && !m_bForceDisabled)
			m_Parent.FocusTile(this);
		else if (m_bForceDisabled)
			m_Parent.FocusTile(null);
		
		return false;
	}
	
	/*!
	Show (or hide) message using the message ID
	\param show if false the message will be hidden and if true it will be shown
	\param messageId the id of the message that should be shown
	*/
	void ShowSimpleMessage(bool show, string messageId = string.Empty)
	{
		Widget simpleMessageWidget = GetRootWidget().FindAnyWidget(m_sSimpleMessageName);
		
		if (!simpleMessageWidget)
			return;
		
		if (!show || messageId.IsEmpty())
		{
			simpleMessageWidget.SetVisible(false);
			return;
		}
		
		SCR_SimpleMessageComponent simpleMessage = SCR_SimpleMessageComponent.Cast(simpleMessageWidget.FindHandler(SCR_SimpleMessageComponent));
		if (!simpleMessage)
			return;
		
		simpleMessage.SetContentFromPreset(messageId);
		simpleMessageWidget.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wFactionBackground = ImageWidget.Cast(w.FindAnyWidget("FactionBckg"));
		Widget widget = w.FindAnyWidget("LoadoutPreview");
		m_Preview = SCR_LoadoutPreviewComponent.Cast(widget.FindHandler(SCR_LoadoutPreviewComponent));
	}

	//------------------------------------------------------------------------------------------------
	void SetPreviewedLoadout(SCR_BasePlayerLoadout loadout)
	{
		if (m_Preview)
		{
			IEntity ent = m_Preview.SetPreviewedLoadout(loadout);
			if (!ent)
				return;

			FactionAffiliationComponent affiliation = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));
			if (affiliation)
			{
				Faction faction = affiliation.GetAffiliatedFaction();
				if (faction)
					m_wFactionBackground.SetColor(faction.GetFactionColor());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetIcon(SCR_EditableEntityUIInfo info)
	{
		if (!info)
			return;
		info.SetIconTo(m_wIcon);
		m_wIcon.SetVisible(true);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_BriefingMenuTile : SCR_DeployMenuTileBase
{
	protected ImageWidget m_wHintBackground;
	protected TextWidget m_wName;
	protected TextWidget m_wDescription;

	//------------------------------------------------------------------------------------------------
	static SCR_BriefingMenuTile InitializeTile(notnull SCR_DeployMenuTileSelection parent)
	{
		Widget tile = GetGame().GetWorkspace().CreateWidgets(parent.GetTileResource());
		SCR_GalleryComponent gallery = SCR_GalleryComponent.Cast(parent.GetTileContainer().GetHandler(0));

		SCR_BriefingMenuTile handler = SCR_BriefingMenuTile.Cast(tile.FindHandler(SCR_BriefingMenuTile));
		if (!handler)
			return null;

		gallery.AddItem(tile);

		return handler;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wName = TextWidget.Cast(w.FindAnyWidget("Name"));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget("Description"));
		m_wHintBackground = ImageWidget.Cast(w.FindAnyWidget("Background"));
	}

	//------------------------------------------------------------------------------------------------
	void SetImageAndDescription(SCR_UIInfo info, int index)
	{
		index++;
		m_wName.SetText("" + index + ". " + info.GetName());
		m_wDescription.SetText(info.GetDescription());
		m_wHintBackground.LoadImageTexture(0, info.GetIconPath());
	}
};
