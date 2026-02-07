/** @ingroup Editor_UI Editor_UI_Components
The Filter window of the Content Browser
*/
class SCR_ContentBrowserFiltersEditorUIComponent : SCR_BaseEditorUIComponent 
{
	const string FILTERGROUP_VLAYOUT_NAME = "FilterOptions";
	
	[Attribute(desc: "Name of widget in which labels will be spawned.", defvalue: "FilterList")]
	private string m_sLayoutName;
	
	[Attribute(desc: "Name of scroll widget for scroll manipulation.", defvalue: "Scroll")]
	private string m_sScrollLayoutName;
	
	[Attribute(desc: "Name of widget where label groups will be added under", defvalue: "FilterOptions")]
	private string m_sLabelGroupLayoutName;
	
	[Attribute(desc: "Name of text widget with label group title TextWidget", defvalue: "GroupTitle")]
	private string m_sLabelGroupTitleTextName;
	
	[Attribute(desc: "Name of imagewidget of individual filter layout", defvalue: "Image")]
	private string m_sLabelImageWidgetName;
	
	[Attribute("{B556F6D3FE4B4307}UI/layouts/Editor/ContentBrowser/ContentBrowser_FilterButton.layout")]
	private ResourceName m_LabelPrefab;
	
	[Attribute("{D87E75E2BF96D734}UI/layouts/Editor/ContentBrowser/ContentBrowser_LabelGroup.layout")]
	private ResourceName m_LabelGroupPrefab;
	
	[Attribute("0.5")]
	private float m_fLabelClickDelay;
	
	private Widget m_Layout;
	
	private ref map<EEditableEntityLabel, SCR_ListBoxElementComponent> m_ButtonByLabel = new map<EEditableEntityLabel, SCR_ListBoxElementComponent>;
	
	private ref map<EEditableEntityLabel, ref set<Widget>> m_GroupWidgetByConditionalLabel = new map<EEditableEntityLabel, ref set<Widget>>;
	private ref map<EEditableEntityLabelGroup, Widget> m_GroupWidgetByGroupLabel = new map<EEditableEntityLabelGroup, Widget>;
	
	private SCR_ContentBrowserEditorComponent m_ContentBrowserEditorComponent;
	
	private ScriptInvoker m_ShowLoadingCallback;
	private ScriptInvoker m_LabelApplyCallback;
	protected ref ScriptInvoker Event_OnFiltersReset = new ScriptInvoker;
	protected ref ScriptInvoker Event_OnActiveFilterClicked = new ScriptInvoker;
	
	protected Widget m_wFirstButton;
	
	protected bool m_bIsDoubleClick;
	protected bool m_bUsingGamePad;
	protected bool m_bFocusingLabels;
	protected Widget m_wLastFocusedLabel;
	protected float m_fLabelSelectedTimer;
	
	protected EEditableEntityLabel m_HoveredLabel;
	
	protected ScrollLayoutWidget m_wScrollview;
	protected float m_fFiltersScrollPosY;
	
	protected void SetFilterToggled(EEditableEntityLabel entityLabel, bool state, SCR_ListBoxElementComponent handler = null)
	{
		if (!handler)
		{
			handler = m_ButtonByLabel.Get(entityLabel);
		}
		if (handler)
		{
			handler.SetToggled(state, false);
		}
		CheckConditionalGroup(entityLabel, state);
		m_ContentBrowserEditorComponent.SetLabel(entityLabel, state);
	}
	
	/*!
	On Active filter pressed
	\param entityLabel EEditableEntityLabel to set disabled
	*/
	void DisableFilterFromActiveFilters(EEditableEntityLabel entityLabel)
	{
		SetFilterToggled(entityLabel, false);
		ApplyFiltersDelayed();
	}
	
	bool GetLabelFromHandler(SCR_ListBoxElementComponent filterHandler, out EEditableEntityLabel label, out bool isToggled, out EEditableEntityLabelGroup groupLabel)
	{
		if (!filterHandler) return false;
		
		SCR_EditableEntityCoreLabelSetting labelSettings = SCR_EditableEntityCoreLabelSetting.Cast(filterHandler.GetData());
		if (!labelSettings) return false;
		
		label = labelSettings.GetLabelType();
		groupLabel = labelSettings.GetLabelGroupType();
		isToggled = m_ContentBrowserEditorComponent.IsLabelActive(label);
		return true;
	}
	
	protected void CheckConditionalGroup(EEditableEntityLabel filterLabel, bool value)
	{
		if (!m_GroupWidgetByConditionalLabel.Contains(filterLabel))
		{
			return;
		}
		
		float x;
		m_wScrollview.GetSliderPosPixels(x, m_fFiltersScrollPosY);
		
		// Get filter groups toggled by this filter
		set<Widget> conditionalGroupWidgets = m_GroupWidgetByConditionalLabel.Get(filterLabel);		
		foreach (Widget conditionalGroupWidget : conditionalGroupWidgets)
		{
			if (!conditionalGroupWidget) continue;
			conditionalGroupWidget.SetVisible(value);
			
			if (!value)
			{
				// Deactivate all filters in this group if filter is deactivated and conditional group is hidden
				Widget filterVerticalLayout = conditionalGroupWidget.FindAnyWidget(FILTERGROUP_VLAYOUT_NAME);
				if (!filterVerticalLayout) continue;
				
				Widget filterLayout = filterVerticalLayout.GetChildren();
				while (filterLayout)
				{
					SCR_ListBoxElementComponent filterHandler = SCR_ListBoxElementComponent.Cast(filterLayout.FindHandler(SCR_ListBoxElementComponent));
					if (!filterHandler) continue;
					
					SCR_EditableEntityCoreLabelSetting labelSettings = SCR_EditableEntityCoreLabelSetting.Cast(filterHandler.GetData());
					if (!labelSettings) continue;
					
					SetFilterToggled(labelSettings.GetLabelType(), false, filterHandler);
					
					filterLayout = filterLayout.GetSibling();
				}
			}
		}
		m_wScrollview.SetSliderPosPixels(0, m_fFiltersScrollPosY);
	}
	
	protected void OnFilterToggled(SCR_ModularButtonComponent handler, bool newToggle)
	{
		EEditableEntityLabel toggledLabel;
		EEditableEntityLabelGroup groupLabel;
		bool isToggled;
		if (GetLabelFromHandler(SCR_ListBoxElementComponent.Cast(handler), toggledLabel, isToggled, groupLabel))
		{
			CheckConditionalGroup(toggledLabel, newToggle);
			m_ContentBrowserEditorComponent.SetLabel(toggledLabel, newToggle);
		}
	}
	
	void OnMouseEnterFilterWidget(SCR_ModularButtonComponent handler)
	{
		OnFilterLabelFocus(handler.GetRootWidget(), handler);
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		// Triggered when mouse enters content browser UI / leaves filter hover
		OnFilterLabelLostFocus();
		return false;
	}
	
	void OnFilterLabelFocus(Widget label, SCR_ModularButtonComponent handler)
	{
		if (m_bUsingGamePad && label == m_wFirstButton)
			m_wScrollview.SetSliderPos(0,0);
		
		if (label)
			m_wLastFocusedLabel = label;
		
		// Cache hovered entity label, used for click/double click input handling
		SCR_ListBoxElementComponent hoveredLabelHandler = SCR_ListBoxElementComponent.Cast(handler);
		if (!hoveredLabelHandler) return;
		
		SCR_EditableEntityCoreLabelSetting labelSettings = SCR_EditableEntityCoreLabelSetting.Cast(hoveredLabelHandler.GetData());
		if (!labelSettings) return;
		
		m_HoveredLabel = labelSettings.GetLabelType();
	}
	
	void OnFilterLabelLostFocus()
	{
		m_HoveredLabel = -1;
	}
	
	Widget GetLastFocusedLabel()
	{
		if (!m_wLastFocusedLabel)
			return m_wFirstButton;
		else
			return m_wLastFocusedLabel;
	}
	
	void OnSelectClicked()
	{
		
	}
	
	void OnResetClicked()
	{
		array<EEditableEntityLabel> activeLabels = {};
		m_ContentBrowserEditorComponent.GetActiveLabels(activeLabels);
		
		foreach	(EEditableEntityLabel toggledLabel : activeLabels)
		{
			SetFilterToggled(toggledLabel, false);
		}
		
		m_wScrollview.SetSliderPos(0, 0);
		
		m_ContentBrowserEditorComponent.ResetAllLabels();
		ApplyLabelChanges();
		Event_OnFiltersReset.Invoke();
	}
	
	ScriptInvoker GetOnFiltersReset()
	{
		return Event_OnFiltersReset;
	}
	
	void ApplyLabelChanges()
	{
		if (m_LabelApplyCallback == null)
			return;
		
		//Save focussed widget.
		//~Note currently The pagination sets the focused widget and forces it to the first element if non where selected in the Grid
		//  It would be difficult to start changing the logic shared by many UI so instead I force the focus back.
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		
		//Send event to apply filter changes
		m_LabelApplyCallback.Invoke(true);
		
		//Set focus back if the widget still extist
		if (focusedWidget)
			GetGame().GetWorkspace().SetFocusedWidget(focusedWidget);
	}
	
	void ResetLabelsOfGroupLabel(EEditableEntityLabelGroup groupLabel, Widget ignoreButton = null)
	{
		Widget filterGroupLayout = m_GroupWidgetByGroupLabel.Get(groupLabel);
		
		if (!filterGroupLayout) return;
		
		Widget filterWidget = filterGroupLayout.GetChildren();
		int i = 0;
		while (filterWidget)
		{
			SCR_ListBoxElementComponent buttonComponent = SCR_ListBoxElementComponent.Cast(filterWidget.FindHandler(SCR_ListBoxElementComponent));
			if (buttonComponent && buttonComponent.GetToggled() && (!ignoreButton || ignoreButton != buttonComponent.GetRootWidget()))
			{
				buttonComponent.SetToggled(false, true);
			}
			
			filterWidget = filterWidget.GetSibling();
			i++;
			if (i > 100) break; //Failsafe
		}
	}
	
	void SetContentBrowserCallbacks(ScriptInvoker showLoadingCallback, ScriptInvoker labelApplyCallback)
	{
		m_ShowLoadingCallback = showLoadingCallback;
		m_LabelApplyCallback = labelApplyCallback;
	}
	
	void InitializeLabels()
	{
		if (!m_ContentBrowserEditorComponent)
		{
			Print("SCR_ContentBrowserEditorComponent not set on current editor mode prefab", LogLevel.ERROR);
			return;
		}
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		Widget existingLabelWidget = m_Layout.GetChildren();
		while(existingLabelWidget)
		{
			existingLabelWidget.RemoveFromHierarchy();
			existingLabelWidget = existingLabelWidget.GetSibling();
		}
		
		array<ref SCR_EditableEntityCoreLabelGroupSetting> labelGroups = {};
		
		m_ContentBrowserEditorComponent.GetLabelGroups(labelGroups);
		
		array<SCR_EditableEntityCoreLabelSetting> groupLabels = {};
		Widget labelGroupWidget, labelGroupLayout, labelWidget;
		TextWidget groupTextWidget;
		EEditableEntityLabel conditionalLabel;
		
		string entityLabelString;
		SCR_ButtonTextComponent selectableButton;
		foreach (SCR_EditableEntityCoreLabelGroupSetting labelGroup : labelGroups)
		{
			EEditableEntityLabelGroup groupLabel = labelGroup.GetLabelGroupType();
			
			m_ContentBrowserEditorComponent.GetLabelsOfGroup(groupLabel, groupLabels);
			
			labelGroupWidget = workspace.CreateWidgets(m_LabelGroupPrefab, m_Layout);
			labelGroupLayout = labelGroupWidget.FindAnyWidget(m_sLabelGroupLayoutName);
			
			SCR_UIInfo labelGroupInfo = labelGroup.GetInfo();
			if (labelGroupInfo)
			{
				groupTextWidget = TextWidget.Cast(labelGroupWidget.FindAnyWidget(m_sLabelGroupTitleTextName));
				labelGroupInfo.SetNameTo(groupTextWidget);
			}
			
			conditionalLabel = labelGroup.GetConditionalLabelType();
			if (conditionalLabel != EEditableEntityLabel.NONE)
			{
				labelGroupWidget.SetVisible(m_ContentBrowserEditorComponent.IsLabelActive(conditionalLabel));
				
				set<Widget> conditionalGroupWidgets = m_GroupWidgetByConditionalLabel.Get(conditionalLabel);
				
				if (conditionalGroupWidgets)
				{
					conditionalGroupWidgets.Insert(labelGroupWidget);
				}
				else
				{
					conditionalGroupWidgets = new set<Widget>();
					conditionalGroupWidgets.Insert(labelGroupWidget);
					m_GroupWidgetByConditionalLabel.Set(conditionalLabel, conditionalGroupWidgets);
				}
			}
			
			m_GroupWidgetByGroupLabel.Insert(groupLabel, labelGroupLayout);
			
			bool hasAtLeastOneLabelVisible = false;
			
			foreach (SCR_EditableEntityCoreLabelSetting entityLabelSettings : groupLabels)
			{		
				if (!entityLabelSettings.GetFilterEnabled()) continue;
				hasAtLeastOneLabelVisible = true;
				
				SCR_UIInfo labelUIInfo = entityLabelSettings.GetInfo();
				
				EEditableEntityLabel entityLabel = entityLabelSettings.GetLabelType();
				string labelTitle = labelUIInfo.GetName();
				Widget labelLayout = workspace.CreateWidgets(m_LabelPrefab, labelGroupLayout);
				
				SCR_ListBoxElementComponent labelOptionHandler = SCR_ListBoxElementComponent.Cast(labelLayout.FindHandler(SCR_ListBoxElementComponent));
				
				ImageWidget labelImageWidget = ImageWidget.Cast(labelOptionHandler.GetRootWidget().FindAnyWidget(m_sLabelImageWidgetName));
				if (labelImageWidget)
				{
					labelUIInfo.SetIconTo(labelImageWidget);
				}
				
				SCR_LinkTooltipTargetEditorUIComponent linkedTooltipComponent = SCR_LinkTooltipTargetEditorUIComponent.Cast(labelLayout.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
				if (linkedTooltipComponent)
					linkedTooltipComponent.SetInfo(labelUIInfo);
				
				if (!m_wFirstButton)
					m_wFirstButton = labelLayout;
				
				bool isLabelToggled = m_ContentBrowserEditorComponent.IsLabelActive(entityLabel);
				
				labelOptionHandler.SetText(labelTitle);
				labelOptionHandler.SetToggled(isLabelToggled, false);
				labelOptionHandler.SetData(entityLabelSettings);
				labelOptionHandler.m_OnMouseEnter.Insert(OnMouseEnterFilterWidget);
				labelOptionHandler.m_OnFocus.Insert(OnMouseEnterFilterWidget);
				labelOptionHandler.m_OnClicked.Insert(OnFilterClicked);
				labelOptionHandler.m_OnDoubleClicked.Insert(OnFilterDoubleClicked);
				labelOptionHandler.m_OnToggled.Insert(OnFilterToggled);
				m_ButtonByLabel.Insert(entityLabel, labelOptionHandler);
			}
			
			//Hide the label group if all the labels within are hidden
			if (!hasAtLeastOneLabelVisible)
				labelGroupWidget.SetVisible(false);
		}
	}
	
	protected void OnInputDeviceChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		m_bUsingGamePad = (newDevice == EInputDeviceType.GAMEPAD || newDevice == EInputDeviceType.JOYSTICK);
		
		if (m_bFocusingLabels)
			OnFilterLabelFocus(null, null);
		else 
			OnFilterLabelLostFocus();
	}
	
	protected void OnFilterClicked(SCR_ListBoxElementComponent handler)
	{
		if (m_bIsDoubleClick)
		{
			m_bIsDoubleClick = false;
			return;
		}
		
		EEditableEntityLabel entityLabel;
		EEditableEntityLabelGroup groupLabel;
		bool isToggled;
		if (GetLabelFromHandler(handler, entityLabel, isToggled, groupLabel))
		{
			SetFilterToggled(entityLabel, !isToggled, handler);
			ApplyFiltersDelayed();
		}
	}
	
	protected void OnFilterDoubleClicked(SCR_ListBoxElementComponent handler)
	{
		if (m_ContentBrowserEditorComponent.GetActiveLabelCount() <= 1)
			return;
		
		m_bIsDoubleClick = true;
		
		EEditableEntityLabel entityLabel;
		EEditableEntityLabelGroup groupLabel;
		bool isToggled;
		if (GetLabelFromHandler(handler, entityLabel, isToggled, groupLabel))
		{
			SetFilterToggled(entityLabel, true, handler);
			ResetLabelsOfGroupLabel(groupLabel, handler.GetRootWidget());
			ApplyFiltersDelayed();
		}
	}
	
	protected void ApplyFiltersDelayed()
	{
		m_fLabelSelectedTimer = m_fLabelClickDelay;
		if (m_ShowLoadingCallback)
		{
			m_ShowLoadingCallback.Invoke(true);
		}
	}
	
	protected void OnMenuUpdate(float timeSlice)
	{
		if (m_fLabelSelectedTimer > 0)
		{
			m_fLabelSelectedTimer -= timeSlice;
			if (m_fLabelSelectedTimer < 0)
			{
				ApplyLabelChanges();
			}
		}
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		if (SCR_Global.IsEditMode()) return;
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChanged);
		GetMenu().GetOnMenuUpdate().Insert(OnMenuUpdate);
		
		m_ContentBrowserEditorComponent = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		
		m_Layout = w.FindAnyWidget(m_sLayoutName);
		
		m_wScrollview = ScrollLayoutWidget.Cast(w.FindAnyWidget(m_sScrollLayoutName));
		
		// Remove any existing filter groups, used for configuring UI layouts
		Widget debugFilterGroup = m_Layout.GetChildren();
		int i = 0;
		while(debugFilterGroup && i++ < 100)
		{
			Widget debugFilterSibling = debugFilterGroup.GetSibling();
			debugFilterGroup.RemoveFromHierarchy();
			debugFilterGroup = debugFilterSibling;
		}
		
		if (!m_Layout)
		{
			Print("Can't find layout for content browser label list", LogLevel.ERROR);
			return;
		}
		
		if (m_LabelPrefab.IsEmpty())
		{
			Print("Content Browser Label List does not have a label prefab assigned", LogLevel.ERROR);
			return;
		}
		
		InitializeLabels();
	}
	
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		if (SCR_Global.IsEditMode()) return;
		GetGame().OnInputDeviceUserChangedInvoker().Remove(OnInputDeviceChanged);
		GetMenu().GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
};