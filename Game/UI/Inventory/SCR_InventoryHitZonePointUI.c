class SCR_InventoryHitZonePointContainerUI : ScriptedWidgetComponent
{
	[Attribute("Bleeding")]
	protected string m_sBleedingHitZoneName;

	[Attribute("Damage")]
	protected string m_sDamageHitZoneName;	

	protected SCR_InventoryHitZonePointUI m_pBleedingHandler;
	protected SCR_InventoryHitZonePointUI m_pDamageHandler;

	protected ref array<HitZone> m_aGroupHitZones = {};
	protected SCR_CharacterBloodHitZone m_pBloodHitZone;
	protected ECharacterHitZoneGroup m_eHitZoneGroup;
	protected IEntity m_Player;
	SCR_CharacterDamageManagerComponent m_pCharDmgManager;
	protected TNodeId m_iBoneIndex;

	protected Widget m_wRoot;
	protected SCR_InventoryMenuUI m_pInventoryMenu;
	protected BaseInventoryStorageComponent m_pStorage;
	protected ref SCR_InventoryHitZoneUI m_pStorageUI;

	protected bool m_bSelected;
	protected bool m_bApplicableItemsShown;
	static SCR_InventoryHitZonePointContainerUI s_pSelectedPoint;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		Widget bleedingW = w.FindAnyWidget(m_sBleedingHitZoneName);
		m_pBleedingHandler = SCR_InventoryHitZonePointUI.Cast(bleedingW.FindHandler(SCR_InventoryHitZonePointUI));
		m_pBleedingHandler.SetParentContainer(this);

		Widget damageW = w.FindAnyWidget(m_sDamageHitZoneName);
		m_pDamageHandler = SCR_InventoryHitZonePointUI.Cast(damageW.FindHandler(SCR_InventoryHitZonePointUI));
		m_pDamageHandler.SetParentContainer(this);
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == 0)
		{
			if (!m_bApplicableItemsShown)
				ShowApplicableItems();
			else
				HideApplicableItems();
		}

		if (button == 1)
		{
			SCR_TourniquetStorageComponent tqStorageComp = SCR_TourniquetStorageComponent.Cast(m_Player.FindComponent(SCR_TourniquetStorageComponent));
			if (!tqStorageComp)
				return false;
			
			tqStorageComp.RemoveTourniquetFromSlot(m_eHitZoneGroup);	
		}

		return false;
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_InventorySpinBoxComponent spinbox = m_pInventoryMenu.GetAttachmentSpinBoxComponent();
		if (spinbox)
		{
			int id = spinbox.FindItem(GetHitZoneName());
			if (id > -1)
				spinbox.SetCurrentItem(id);
		}

		return false;
	}

	protected void Highlight(bool highlight)
	{
		m_pBleedingHandler.Highlight(highlight);
		m_pDamageHandler.Highlight(highlight);
	}

	void Select(bool select = true)
	{
		if (s_pSelectedPoint)
		{
			s_pSelectedPoint.m_bSelected = false;
			s_pSelectedPoint.Highlight(false);
		}

		m_bSelected = select;
		Highlight(select);
		if (select)
		{
			s_pSelectedPoint = this;
		}
		else
		{
			s_pSelectedPoint = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowApplicableItems()
	{
		m_bApplicableItemsShown = true;
		Select(true);
		SCR_ApplicableMedicalItemPredicate predicate = new SCR_ApplicableMedicalItemPredicate();
		predicate.characterEntity = m_Player;
		predicate.hitZoneGroup = m_eHitZoneGroup;
		m_pInventoryMenu.ShowAttachmentStorage(predicate);
	}

	void HideApplicableItems()
	{
		m_bApplicableItemsShown = false;
		if (m_pInventoryMenu.GetAttachmentStorageUI())
		{
			m_pInventoryMenu.RemoveAttachmentStorage(m_pInventoryMenu.GetAttachmentStorageUI());
			m_pStorageUI.GetRootWidget().SetVisible(false);
			Select(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	string GetHitZoneName()
	{
		switch (m_eHitZoneGroup)
		{
			case ECharacterHitZoneGroup.HEAD:
			{
				return "#AR-Inventory_Head";
			} break;

			case ECharacterHitZoneGroup.UPPERTORSO:
			{
				return "#AR-Inventory_Chest";
			} break;

			case ECharacterHitZoneGroup.LOWERTORSO:
			{
				return "#AR-Inventory_Abdomen";
			} break;

			case ECharacterHitZoneGroup.LEFTARM:
			{
				return "#AR-Inventory_LeftArm";
			} break;

			case ECharacterHitZoneGroup.RIGHTARM:
			{
				return "#AR-Inventory_RightArm";
			} break;

			case ECharacterHitZoneGroup.LEFTLEG:
			{
				return "#AR-Inventory_LeftLeg";
			} break;

			case ECharacterHitZoneGroup.RIGHTLEG:
			{
				return "#AR-Inventory_RightLeg";
			} break;
		}

		return string.Empty;
	}

	void InitializeHitZoneUI(BaseInventoryStorageComponent storage, SCR_InventoryMenuUI menuManager, int hitZoneId, IEntity player)
	{
		m_pStorage = storage;
		m_pInventoryMenu = menuManager;
		m_Player = player;
		m_eHitZoneGroup = hitZoneId;

		m_pCharDmgManager = SCR_CharacterDamageManagerComponent.Cast(player.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_pCharDmgManager)
			return;

		m_pBloodHitZone = SCR_CharacterBloodHitZone.Cast(m_pCharDmgManager.GetBloodHitZone());
		m_pBloodHitZone.GetOnDamageStateChanged().Insert(UpdateHitZoneState);
		m_aGroupHitZones.Insert(m_pBloodHitZone);

		UpdateHitZoneState(m_pBloodHitZone);
		m_pCharDmgManager.GetOnDamageOverTimeAdded().Insert(UpdateBloodHitZone);
		m_pCharDmgManager.GetOnDamageOverTimeRemoved().Insert(UpdateBloodHitZoneRemoved);
		m_pCharDmgManager.GetGroupHitZones(m_eHitZoneGroup, m_aGroupHitZones);
		string boneName = m_pCharDmgManager.GetBoneName(m_eHitZoneGroup);
		m_iBoneIndex = m_Player.GetBoneIndex(boneName);

		foreach (HitZone hz : m_aGroupHitZones)
		{
			SCR_CharacterHitZone scrHZ = SCR_CharacterHitZone.Cast(hz);
			if (scrHZ)
			{
				scrHZ.GetOnDamageStateChanged().Insert(UpdateHitZoneState);
				UpdateHitZoneState(scrHZ);
			}
		}

		Widget parent = m_wRoot.FindAnyWidget("Storage");
		Widget newStorage = GetGame().GetWorkspace().CreateWidgets(m_pInventoryMenu.BACKPACK_STORAGE_LAYOUT, parent);
		m_pStorageUI = new SCR_InventoryHitZoneUI(storage, null, menuManager, 0, null, this);
		newStorage.AddHandler(m_pStorageUI);
		m_pStorageUI.GetRootWidget().SetVisible(false);
	}

	protected void UpdateBloodHitZone(EDamageType dType, float dps, HitZone hz = null)
	{
		if (dType != EDamageType.BLEEDING)
			return;

		UpdateHitZoneState(ScriptedHitZone.Cast(hz));
	}

	protected void UpdateBloodHitZoneRemoved(EDamageType dType, HitZone hz = null)
	{
		if (dType != EDamageType.BLEEDING)
			return;

		UpdateHitZoneState(ScriptedHitZone.Cast(hz));
	}	
	
	protected void UpdateHitZoneState(ScriptedHitZone hz)
	{
		if (!hz || !m_aGroupHitZones.Contains(hz))
			return;
		
		float bleeding = m_pCharDmgManager.GetGroupDamageOverTime(m_eHitZoneGroup, EDamageType.BLEEDING);
		bool tourniquetted = m_pCharDmgManager.GetGroupTourniquetted(m_eHitZoneGroup);
		m_pBleedingHandler.UpdateBleedingHitZoneState(bleeding, tourniquetted);

		float health = m_pCharDmgManager.GetGroupHealthScaled(m_eHitZoneGroup);
		m_pDamageHandler.UpdateDamageHitZoneState(health);

		m_pBleedingHandler.GetRootWidget().SetVisible(bleeding > 0 || tourniquetted);
		m_pDamageHandler.GetRootWidget().SetVisible(health < 1 && bleeding == 0);
		
		if (m_bSelected)
			ShowApplicableItems();

		if (!m_pInventoryMenu)
			return;

		if (bleeding > 0 || health < 1 || tourniquetted)
			m_pInventoryMenu.AddItemToAttachmentSelection(GetHitZoneName(), this);
		else
			m_pInventoryMenu.RemoveItemFromAttachmentSelection(GetHitZoneName());
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	TNodeId GetBoneIndex()
	{
		return m_iBoneIndex;
	}

	SCR_InventoryMenuUI GetInventoryHandler()
	{
		return m_pInventoryMenu;
	}

	SCR_InventoryHitZoneUI GetStorage()
	{
		return m_pStorageUI;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_InventoryHitZonePointUI : ScriptedWidgetComponent
{
	[Attribute("Background")]
	protected string m_sBackground;
	protected ImageWidget m_wBackground;

	[Attribute("Outline")]
	protected string m_sOutline;
	protected ImageWidget m_wOutline;

	[Attribute("Icon")]
	protected string m_sIcon;
	protected ImageWidget m_wIcon;

	[Attribute("Icon")]
	protected string m_sIconAlt;
	protected Widget m_wIconAlt;

	[Attribute("Arrows")]
	protected string m_sArrows;
	protected VerticalLayoutWidget m_wArrows;
	protected ref array<Widget> m_aArrows = {};

	[Attribute("Regen")]
	protected string m_sRegen;
	protected Widget m_wRegen;

	[Attribute("RegenBckg")]
	protected string m_sRegenBckg;
	protected Widget m_wRegenBckg;

	protected const Color m_aRegenColors[4] = {
		Color.Green,
		Color.Yellow,
		Color.Orange,
		Color.Red
	};

	[Attribute("1")]
	protected bool m_bAllowHoverOver;

	[Attribute("0")]
	protected bool m_bBleedingHitZone;

	[Attribute()]
	protected ref array<int> m_aDOTLevels;

	protected Widget m_wRoot;
	protected ButtonWidget m_wButton;

	protected static SCR_InventoryHitZonePointUI s_pSelectedPoint;
	protected bool m_bSelected;

	protected const float OPACITY_DEFAULT = 0.4;
	protected const float OPACITY_SOLID = 1;

	//------------------------------------------------------------------------------------------------
	protected ECharacterHitZoneGroup m_eHitZoneGroup;
	protected ref array<HitZone> m_aGroupHitZones = {};
	
	protected string m_sHigh = "#AR-Settings_QualityPreset_High";
	protected string m_sMedium = "#AR-Settings_QualityPreset_Medium";
	protected string m_sLow = "#AR-Settings_QualityPreset_Low";
	protected string m_sBleeding = "#AR-Inventory_Bleeding";
	protected string m_sDamage = "#AR-FieldManual_Category_Damage_Title";
	
	[Attribute("0.66", UIWidgets.EditBox, "Arbitrary 'medium' damage amount for visualization", category: "General")]
	protected float m_fMediumThreshold;	

	[Attribute("0.33", UIWidgets.EditBox, "Arbitrary 'low' damage amount for visualization", category: "General")]
	protected float m_fLowThreshold;

	protected IEntity m_Player;
	protected ref SCR_InventoryHitZoneUI m_pStorageHandler;
	protected SCR_InventoryHitZonePointContainerUI m_pParentContainer;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		float opacity = OPACITY_DEFAULT;
		if (!m_bAllowHoverOver)
			opacity = OPACITY_SOLID;

		m_wButton = ButtonWidget.Cast(w.FindAnyWidget("Btn"));
		m_wBackground = ImageWidget.Cast(w.FindAnyWidget(m_sBackground));

		m_wOutline = ImageWidget.Cast(w.FindAnyWidget(m_sOutline));
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sIcon));
		m_wIconAlt = w.FindAnyWidget(m_sIconAlt);
		m_wArrows = VerticalLayoutWidget.Cast(w.FindAnyWidget(m_sArrows));

		m_wRegen = m_wRoot.FindAnyWidget(m_sRegen);
		m_wRegenBckg = m_wRoot.FindAnyWidget(m_sRegenBckg);

		Highlight(!m_bAllowHoverOver);

		if (m_wArrows)
		{
			Widget arrow = m_wArrows.GetChildren();
			while (arrow)
			{
				m_aArrows.Insert(arrow);
				arrow = arrow.GetSibling();
			}
		}
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		Highlight(true);
		ShowHitZoneInfo();

		return false;
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (!m_bSelected)
			Highlight(false);
		ShowHitZoneInfo(false);

		return false;
	}

	void Select(bool select)
	{
		if (s_pSelectedPoint)
		{
			s_pSelectedPoint.m_bSelected = false;
			s_pSelectedPoint.Highlight(false);
		}

		m_bSelected = select;
		Highlight(select);
		if (select)
			s_pSelectedPoint = this;
		else
			s_pSelectedPoint = null;
	}

	void Highlight(bool highlight)
	{
		if (!m_bAllowHoverOver)
			return;

		float opacity = OPACITY_SOLID;
		if (!highlight)
			opacity = OPACITY_DEFAULT;
		if (m_wArrows)
			m_wArrows.SetOpacity(opacity);
		if (m_wOutline)
			m_wOutline.SetOpacity(opacity);
		if (m_wIcon)
			m_wIcon.SetOpacity(opacity);
		if (m_wIconAlt)
			m_wIconAlt.SetOpacity(opacity);
	}

	protected void ShowHitZoneInfo(bool show = true)
	{
		if (!m_pParentContainer)
			return;

		if (!show)
		{
			m_pParentContainer.GetInventoryHandler().HideItemInfo();
			return;
		}

		ECharacterHitZoneGroup group = m_pParentContainer.GetHitZoneGroup();
		
		int groupDamageIntensity;
		float bleedingrate;
		string sDescr;
		string intensity;
		string bleeding;
		
		GetHitZoneInfo(group, groupDamageIntensity, bleedingrate);
		
		if (groupDamageIntensity == 3)
			intensity = m_sHigh;
		else if (groupDamageIntensity == 2)
			intensity = m_sMedium;
		else if (groupDamageIntensity == 1)
			intensity = m_sLow;
		
		if (bleedingrate)
			bleeding = m_sBleeding;
		else
			bleeding = "";
	
		if (groupDamageIntensity != 0)
			sDescr = string.Format("%1: %2 \n%3", m_sDamage, intensity, bleeding);
		else if (bleedingrate != 0)
			sDescr = string.Format("%1", bleeding);

		m_pParentContainer.GetInventoryHandler().ShowItemInfo(m_pParentContainer.GetHitZoneName(), sDescr);
	}
	
	protected void GetHitZoneInfo(EHitZoneGroup group, out int groupDamageIntensity, out float bleedingRate)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast( character.GetDamageManager() );
		if (!damageMan)
			return;		
		
		float groupHealth = damageMan.GetGroupHealthScaled(group);

		if (groupHealth == 1)
			groupDamageIntensity = 0;
		else if (groupHealth < m_fLowThreshold)
			groupDamageIntensity = 3;
		else if (groupHealth >= m_fLowThreshold && groupHealth < m_fMediumThreshold)
			groupDamageIntensity = 2;
		else if (groupHealth >= m_fMediumThreshold)
			groupDamageIntensity = 1;

		bleedingRate = damageMan.GetGroupDamageOverTime(group, EDamageType.BLEEDING);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowArrows(int count = -1)
	{
		if (!m_bBleedingHitZone)
			return;

		m_wRoot.SetVisible(count > 0);
		m_wArrows.SetVisible(m_bAllowHoverOver);

		for (int i = 0; i < m_aArrows.Count(); ++i)
		{
			m_aArrows[i].SetVisible(i < count);
		}
	}

	void UpdateRegen(int state = -1)
	{
		if (!m_wRegen || !m_wRegenBckg)
			return;

		if (state > 3)
			state = 3;

		m_wRegen.SetVisible(state > -1);
		if (state > -1)
			m_wRegenBckg.SetColor(m_aRegenColors[state]);
	}

	void UpdateBleedingHitZoneState(float bleeding, bool tourniquetted)
	{
		if (!m_bBleedingHitZone)
			return;

		int arrowCount = -1;
		if (bleeding > 0)
		{
			for (int i = 0; i < m_aDOTLevels.Count(); ++i)
			{
				if (bleeding > m_aDOTLevels[i])
				{
					arrowCount = i;
					continue;
				}
			}
		}

		m_wIcon.SetVisible(!tourniquetted);
		m_wIconAlt.SetVisible(tourniquetted);
		if (tourniquetted)
			arrowCount = 0;

		ShowArrows(arrowCount);
	}

	void UpdateDamageHitZoneState(float health)
	{
		if (m_bBleedingHitZone)
			return;

		int arrowCount = -1;

		if (health < 1)
		{
			arrowCount = health / (1 - health);
		}

		ShowArrows(arrowCount);
	}
	
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	void SetParentContainer(SCR_InventoryHitZonePointContainerUI container)
	{
		m_pParentContainer = container;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_HitZoneRegenState : Managed
{
	[Attribute()]
	ref Color m_Color;
};