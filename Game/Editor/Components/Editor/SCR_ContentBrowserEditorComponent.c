[ComponentEditorProps(category: "GameScripted/Editor", description: "Management of placeable entities. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ContentBrowserEditorComponentClass : SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Management of placeable entities.
*/
class SCR_ContentBrowserEditorComponent : SCR_BaseEditorComponent
{
	protected const int ASYNC_TICK_LENGTH = 16; //--- How many ticks are allowed per async loading cycle
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Labels that are active by default", "", ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_DefaultActiveLabels;
	
	[Attribute(desc: "Content browser menu preset.", defvalue: "-1", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	private ChimeraMenuPreset m_MenuPreset;
	
	private ref array<EEditableEntityLabel> m_ActiveLabels = {};
	
	protected ref array<ref SCR_EditableEntityCoreLabelGroupSetting> m_LabelGroups = {};
	
	protected ref array<ref SCR_EditableEntityUIInfo> m_aInfos;
	protected ref array<ResourceName> m_aAsyncPrefabs;
	protected int m_iAsyncIndex;
	
	protected int m_iPageIndex;
	protected string m_sLastSearchText;
	
	protected SCR_EditableEntityComponent m_ExtendedEntity;
	protected SCR_EditableEntityCore m_EntityCore;
	
	protected ref ScriptInvoker Event_OnLabelChanged = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnActiveLabelsReset = new ScriptInvoker;
	
	bool IsMatchingToggledLabels(notnull array<EEditableEntityLabel> entityLabels)
	{
		if (!IsAnyLabelActive())
		{
			return true;
		}
		if (entityLabels.IsEmpty())
		{
			return false;
		}
		
		array<SCR_EditableEntityCoreLabelSetting> groupLabels = {};
		foreach (SCR_EditableEntityCoreLabelGroupSetting labelGroup : m_LabelGroups)
		{
			EEditableEntityLabelGroup labelGroupType = labelGroup.GetLabelGroupType();
			
			m_EntityCore.GetLabelsOfGroup(labelGroupType, groupLabels);
			
			int activeLabels = 0;
			int matchesActive = 0;
			
			bool needsAllActive = labelGroup.GetRequiresAllLabelsMatch();
			
			foreach (SCR_EditableEntityCoreLabelSetting entityLabelSetting : groupLabels)
			{
				EEditableEntityLabel entityLabel = entityLabelSetting.GetLabelType();
				if (!IsLabelActive(entityLabel))
				{
					continue;
				}
				activeLabels++;
				if (entityLabels.Find(entityLabel) != -1)
				{
					matchesActive++;
				}
			}
			
			if (activeLabels > 0)
			{
				if (needsAllActive && matchesActive != activeLabels)
				{
					return false;
				}
				else if (!needsAllActive && matchesActive == 0)
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
	bool IsLabelActive(EEditableEntityLabel entityLabel)
	{
		return m_ActiveLabels.Find(entityLabel) != -1;
	}
	
	bool IsAnyLabelActive()
	{
		return !m_ActiveLabels.IsEmpty();
	}
	
	void SetLabel(EEditableEntityLabel entityLabel, bool active)
	{
		bool labelActive = IsLabelActive(entityLabel);
		if (active && !labelActive)
		{
			m_ActiveLabels.Insert(entityLabel);
		}
		else if (!active && labelActive)
		{
			m_ActiveLabels.RemoveItem(entityLabel);
		}
		
		Event_OnLabelChanged.Invoke(entityLabel, active);
	}
	
	void ResetAllLabels()
	{
		m_ActiveLabels.Clear();
		Event_OnActiveLabelsReset.Invoke();
	}
	
	void ResetLabelsOfGroup(EEditableEntityLabelGroup groupType)
	{
		
	}
	
	void GetActiveLabels(out notnull array<EEditableEntityLabel> activeLabels)
	{
		activeLabels.InsertAll(m_ActiveLabels);
	}
	
	int GetActiveLabelCount()
	{
		return m_ActiveLabels.Count();
	}
	
	void GetLabelGroups(out notnull array<ref SCR_EditableEntityCoreLabelGroupSetting> labelGroups)
	{
		labelGroups = m_LabelGroups;
	}
	
	bool GetLabelGroupType(EEditableEntityLabel entityLabel, out EEditableEntityLabelGroup groupLabel)
	{
		return m_EntityCore.GetLabelGroupType(entityLabel, groupLabel);
	}
	
	bool GetLabelsOfGroup(EEditableEntityLabelGroup labelGroupType, out notnull array<SCR_EditableEntityCoreLabelSetting> labels)
	{
		return m_EntityCore.GetLabelsOfGroup(labelGroupType, labels);
	}
	
	bool GetLabelUIInfo(EEditableEntityLabel entityLabel, out SCR_UIInfo uiInfo)
	{
		return m_EntityCore.GetLabelUIInfo(entityLabel, uiInfo);
	}
	
	int GetLabelOrder(EEditableEntityLabel entityLabel)
	{
		// return m_EntityCore.GetLabelOrder(entityLabel);
	}
	
	string GetLabelName(EEditableEntityLabel entityLabel)
	{
		SCR_UIInfo uiInfo;
		if (GetLabelUIInfo(entityLabel, uiInfo))
		{
			return uiInfo.GetName();
		}
		return string.Empty;
	}
	
	void SetPageIndex(int pageIndex)
	{
		m_iPageIndex = pageIndex;
	}
	
	int GetPageIndex()
	{
		return m_iPageIndex;
	}
	
	void SetLastSearch(string searchText)
	{
		m_sLastSearchText = searchText;
	}
	
	string GetLastSearch()
	{
		return m_sLastSearchText;
	}
	
	/*!
	Get on label changed event
	\param Event_OnLabelChanged Script invoker
	*/
	ScriptInvoker GetOnLabelChanged()
	{
		return Event_OnLabelChanged;
	}
	
	/*!
	Get on labels reset event
	\param Event_OnActiveLabelsReset Script invoker
	*/
	ScriptInvoker GetOnActiveLabelsReset()
	{
		return Event_OnActiveLabelsReset;
	}
	
	/*!
	Set extended entity.
	Upon next opening, the asset browser will show only prefabs which extend the entity.
	\param extendedEntity Extended entity
	*/
	void SetExtendedEntity(SCR_EditableEntityComponent extendedEntity)
	{
		m_ExtendedEntity = extendedEntity;
	}
	
	/*!
	Get extended entity. The value gets erased during retrieval.
	\return Extended entity
	*/
	SCR_EditableEntityComponent GetExtendedEntity()
	{
		return m_ExtendedEntity;
	}
	
	/*!
	Get info for placeaable prefab with given index.
	\param Prefab index
	\return UI info
	*/
	int GetInfoCount()
	{
		return m_aInfos.Count();
	}
	
	/*!
	Get info for placeaable prefab with given index.
	Infos are store dint he same order as prefabs from SCR_PlacingEditorComponentClass.
	\param Prefab index
	\return UI info
	*/
	SCR_EditableEntityUIInfo GetInfo(int index)
	{
		if (m_aInfos)
		{
			return m_aInfos[index];	
		}
		else
		{
			return null;
		}
	}
	
	/*!
	Find index of given editable entity (info), used for find in content browser
	\param info EditableEntityInfo of the entity
	\return index of editable entity in content browser
	*/
	int FindIndexOfInfo(SCR_EditableEntityUIInfo info)
	{
		if (m_aInfos)
		{
			int i = 0;
			foreach (SCR_EditableEntityUIInfo browserInfo : m_aInfos)
			{
				if (browserInfo.GetName() == info.GetName())
				{
					return i;
				}
				i++;
			}
		}
		return -1;
	}
	
	/*!
	Open content browser.
	\param Active labels
	\return True if opened
	*/
	static bool OpenBrowserInstance(array<EEditableEntityLabel> labels = null)
	{
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (contentBrowserManager)
			return contentBrowserManager.OpenBrowser(0, labels);
		else
			return false;
	}
	
	/*!
	Start extending given entity.
	Content browser will be opened, showing only entities which fir the extended entity.
	Selecting one will instantly place it.
	\param extendedEntity Extended entity
	\return True if opened
	*/
	bool OpenBrowser(SCR_EditableEntityComponent extendedEntity)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true));
		if (!placingManager || placingManager.IsPlacing())
			return false;
		
		SetExtendedEntity(extendedEntity);
		if (OpenBrowser())
		{
			EEditorTransformVertical verticalMode = EEditorTransformVertical.SEA;
			SCR_PreviewEntityEditorComponent previewManager = SCR_PreviewEntityEditorComponent.Cast(SCR_PreviewEntityEditorComponent.GetInstance(SCR_PreviewEntityEditorComponent, false));
			if (previewManager)
				verticalMode = previewManager.GetVerticalMode();
			
			vector transform[4];
			extendedEntity.GetTransform(transform);
			SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(transform, verticalMode: verticalMode);
			params.SetTarget(extendedEntity);
			params.m_TargetInteraction = EEditableEntityInteraction.SLOT;
			placingManager.SetInstantPlacing(params);
			return true;
		}
		return false;
	}
	
	/*!
	Open content browser with preset labels
	\param Active labels
	\return True if opened
	*/
	bool OpenBrowser(int pageIndex = -1, array<EEditableEntityLabel> labels = null)
	{
		SCR_MenuEditorComponent editorMenu = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent, true));
		if (!editorMenu)
			return false;
		
		if (labels)
		{
			ResetAllLabels();
			foreach (EEditableEntityLabel label: labels)
			{
				SetLabel(label, true);
			}
			SetPageIndex(0);
		}
		if (pageIndex > -1)
		{
			SetPageIndex(pageIndex);
		}
		
		editorMenu.GetMenu().OpenDialog(m_MenuPreset);
		return true;
	}
	
	override void EOnEditorActivate()
	{
		
	}
	
	override void EOnEditorInit()
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_EntityCore.GetLabelGroups(m_LabelGroups);
		m_ActiveLabels.Copy(m_DefaultActiveLabels);
	}
	
	override bool EOnEditorActivateAsync(int attempt)
	{
		//--- Started
		if (attempt == 0)
		{
			SCR_PlacingEditorComponentClass placingPrefabData = SCR_PlacingEditorComponentClass.Cast(SCR_PlacingEditorComponentClass.GetInstance(SCR_PlacingEditorComponent, true));
			if (!placingPrefabData)
				return true;
			
			m_aInfos = {};
			m_aAsyncPrefabs = {};
			placingPrefabData.GetPrefabs(m_aAsyncPrefabs, true);
			m_iAsyncIndex = 0;
			return false;
		}
		
		if (!m_aAsyncPrefabs)
			return true;
		
		ResourceName entityPrefab;;
		Resource entityResource;
		IEntityComponentSource editableEntitySource;
		SCR_EditableEntityUIInfo info;
		
		int tickEnd = System.GetTickCount() + ASYNC_TICK_LENGTH;
		int count = m_aAsyncPrefabs.Count();
		while (System.GetTickCount() < tickEnd)
		{
			//--- Completed
			if (m_iAsyncIndex >= count)
			{
				m_aAsyncPrefabs = null;
				return true;
			}
			
			info = null;
			entityPrefab = m_aAsyncPrefabs[m_iAsyncIndex];
			if (entityPrefab)
			{
				entityResource = BaseContainerTools.LoadContainer(m_aAsyncPrefabs[m_iAsyncIndex]);
				if (entityResource && entityResource.IsValid())
				{
					editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entityResource);
					if (editableEntitySource)
					{
						info = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(editableEntitySource));
						if (info)
						{
							info.InitFromSource(editableEntitySource);
						}
						else
						{
							Print(string.Format("Prefab '%1' is missing UI info in SCR_EditableEntityComponent!", m_aAsyncPrefabs[m_iAsyncIndex]), LogLevel.ERROR);
						}
					}
					else
					{
						Print(string.Format("Prefab '%1' is missing SCR_EditableEntityComponent!", m_aAsyncPrefabs[m_iAsyncIndex]), LogLevel.ERROR);
					}
				}
				else
				{
					Print(string.Format("Prefab '%1' is missing at index '%2'!", m_aAsyncPrefabs[m_iAsyncIndex], m_iAsyncIndex), LogLevel.ERROR);
				}
			}
			
			//--- Register even when faulty, to keep indexes
	 		m_aInfos.Insert(info);
			
			m_iAsyncIndex++;
		}
		return false;
	}
	override void EOnEditorDeactivate()
	{
		m_aInfos = null;
	}
	
};