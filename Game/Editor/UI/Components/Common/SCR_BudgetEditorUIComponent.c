// Script File
class SCR_BudgetEditorUIComponent : SCR_BaseEditorUIComponent
{	
	[Attribute("0.1")]
	protected float m_fDisabledOpacity;
	
	const string WIDGET_BUDGET_TEXT = "PercentageText";
	const string WIDGET_BUDGET_ICON = "Icon";
	const string WIDGET_BUDGET_PROGRESSBAR = "RadialProgressBar";
	const string WIDGET_BUDGETPREVIEW_TEXT = "Preview";
	const string WIDGET_ICON_AREA = "IconArea";
	const string WIDGET_LOCK_VISUALS = "LockVisuals";
	
	protected SCR_BudgetEditorComponent m_BudgetComponent;
	protected ResourceName m_BudgetEntryPrefab;
	
	protected Widget m_Layout;
	
	protected bool m_bIsEnabled = true;
	
	private ref map<EEditableEntityBudget, Widget> m_BudgetWidgets = new map<EEditableEntityBudget, Widget>;
	
	protected void OnBudgetUpdate(EEditableEntityBudget budgetType, float updatedBudget, float maxBudget)
	{
		TextWidget budgetWidget;
		if (m_BudgetWidgets.Find(budgetType, budgetWidget))
		{
			SetWidgetData(budgetWidget, updatedBudget, maxBudget);
		}
	}
	
	protected void OnBudgetPreviewUpdate(EEditableEntityBudget budgetType, float previewBudgetValue, float budgetChange)
	{
		Widget budgetWidget = m_BudgetWidgets.Get(budgetType);
		if (budgetWidget)
		{
			SetWidgetPreviewData(budgetWidget, previewBudgetValue, budgetChange);
		}
	}
	
	protected void OnBudgetAdd(SCR_EditableEntityCoreBudgetSetting budget, int maxBudget)
	{
		if (!budget || !m_Layout || m_BudgetEntryPrefab.IsEmpty() || m_BudgetWidgets.Contains(budget.GetBudgetType())) return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game) return;

		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace) return;
		
		EEditableEntityBudget budgetType = budget.GetBudgetType();
		
		Widget budgetWidget = workspace.CreateWidgets(m_BudgetEntryPrefab, m_Layout);
		
 		int currentBudget = m_BudgetComponent.GetCurrentBudget(budgetType);
		
		SetWidgetData(budgetWidget, currentBudget, maxBudget, budget.GetInfo());
		
		m_Layout.SetVisible(m_BudgetWidgets.Count() > 0);
		
		m_BudgetWidgets.Insert(budgetType, budgetWidget);
	}
	
	protected void InitializeBudgets()
	{
		array<ref SCR_EditableEntityCoreBudgetSetting> budgets = {};
		m_BudgetComponent.GetBudgets(budgets);
		
		foreach (SCR_EditableEntityCoreBudgetSetting budget : budgets)
		{
			int maxBudget = m_BudgetComponent.GetMaxBudgetValue(budget.GetBudgetType());
			OnBudgetAdd(budget, maxBudget);
		}	
	}
	
	private void SetWidgetData(Widget budgetWidget, float budgetValue, int maxBudgetValue, SCR_UIInfo info = null)
	{
		float budgetProgress;
		if (maxBudgetValue > 0)
		{
			budgetProgress = budgetValue / maxBudgetValue;
		}
		else
		{
			budgetProgress = 1;	
		}
		
		TextWidget budgetValueText = TextWidget.Cast(budgetWidget.FindAnyWidget(WIDGET_BUDGET_TEXT));
		if (budgetValueText)
		{
			budgetValueText.SetTextFormat("#AR-ValueUnit_Percentage", (budgetProgress * 100).ToString(-1, 0));
		}
		
		if ((maxBudgetValue <= 0 && m_bIsEnabled) || (maxBudgetValue > 0 && !m_bIsEnabled))
		{
			m_bIsEnabled = (maxBudgetValue > 0);
			
			//~ TODO: Current;y budget percentage is hidden. This should still show the server percentage
			if (budgetValueText)
				budgetValueText.SetVisible(m_bIsEnabled);
			
			Widget lockVisuals = budgetWidget.FindAnyWidget(WIDGET_LOCK_VISUALS);
			if (lockVisuals)
				lockVisuals.SetVisible(!m_bIsEnabled);
			
			Widget iconVisuals = budgetWidget.FindAnyWidget(WIDGET_ICON_AREA);
			if (iconVisuals)
			{
				if (!m_bIsEnabled)
					iconVisuals.SetOpacity(m_fDisabledOpacity);
				else 
					iconVisuals.SetOpacity(1);
			}
		}
		
		if (info)
		{
			ImageWidget budgetIcon = ImageWidget.Cast(budgetWidget.FindAnyWidget(WIDGET_BUDGET_ICON));
			info.SetIconTo(budgetIcon);
			SCR_LinkTooltipTargetEditorUIComponent.SetInfo(budgetWidget, info);
		}
		
		Widget progressBarWidget = budgetWidget.FindAnyWidget(WIDGET_BUDGET_PROGRESSBAR);
		if (!progressBarWidget)
			return;
		
		SCR_RadialProgressBarUIComponent radialProgressBar = SCR_RadialProgressBarUIComponent.Cast(progressBarWidget.FindHandler(SCR_RadialProgressBarUIComponent));
		if (!radialProgressBar)
			return;
		
		if (m_bIsEnabled)
			radialProgressBar.SetProgress(budgetProgress);
		else 
			radialProgressBar.SetProgress(0);
	}
	
	private void SetWidgetPreviewData(Widget w, float previewBudgetValue, float budgetChange)
	{		
		TextWidget previewText = TextWidget.Cast(w.FindAnyWidget(WIDGET_BUDGETPREVIEW_TEXT));
		Widget progressBarWidget = w.FindAnyWidget(WIDGET_BUDGET_PROGRESSBAR);
		SCR_RadialProgressBarUIComponent radialProgressBar;
		if (progressBarWidget)
		{
			radialProgressBar = SCR_RadialProgressBarUIComponent.Cast(progressBarWidget.FindHandler(SCR_RadialProgressBarUIComponent));
		}
		
		if (previewText && budgetChange != 0)
		{
			previewText.SetVisible(true);
			
			// If budget change is <1%, only show +, once progress bars are implemented this will simply make the bar red
			bool isLessThenOne = false;
			string amount;
			if (budgetChange > 0 && budgetChange < 1)
			{
				isLessThenOne = true;
				amount = "1";
			}
			else
			{
				amount = budgetChange.ToString(-1, 0);
			}
			
			if (!isLessThenOne)
				previewText.SetTextFormat("#AR-ValueUnit_Percentage_Add", amount);
			else 
				previewText.SetTextFormat("#AR-ValueUnit_Percentage_AddLessThen", amount);
			
			if (radialProgressBar)
				radialProgressBar.SetPreviewProgress(previewBudgetValue / 100);
		}
		else if (previewText)
		{
			previewText.SetVisible(false);
			
			if(radialProgressBar)
				radialProgressBar.SetPreviewProgress(0);
		}
	}
	
	private void ResetWidgetPreviewData()
	{
		Widget budgetWidget = m_Layout.GetChildren();
		int i = 0;
		while (budgetWidget && i++ < 100)
		{
			SetWidgetPreviewData(budgetWidget, 0, 0);
			budgetWidget = budgetWidget.GetSibling();
		}
	}
	
	override void HandlerAttachedScripted(Widget w)
	{
		m_Layout = w;
		
		m_BudgetEntryPrefab = SCR_LayoutTemplateComponent.GetLayout(m_Layout);
		if (m_BudgetEntryPrefab.IsEmpty()) return;		
		
		// Remove any existing budget widgets, used for configuring UI layouts
		Widget debugBudgetWidget = m_Layout.GetChildren();
		int i = 0;
		while(debugBudgetWidget && i++ < 100)
		{
			Widget debugFilterSibling = debugBudgetWidget.GetSibling();
			debugBudgetWidget.RemoveFromHierarchy();
			debugBudgetWidget = debugFilterSibling;
		}
		
		m_BudgetComponent = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, true));
		
		if (m_BudgetComponent)
		{
			InitializeBudgets();
			m_BudgetComponent.GetOnBudgetUpdatedEvent().Insert(OnBudgetUpdate);
			m_BudgetComponent.GetOnBudgetPreviewUpdatedEvent().Insert(OnBudgetPreviewUpdate);
			m_BudgetComponent.GetOnBudgetPreviewResetEvent().Insert(ResetWidgetPreviewData);
		}
	}
	
	override void HandlerDeattached(Widget w)
	{
		m_BudgetWidgets.Clear();
		
		if (m_BudgetComponent)
		{
			m_BudgetComponent.GetOnBudgetUpdatedEvent().Remove(OnBudgetUpdate);
			m_BudgetComponent.GetOnBudgetPreviewUpdatedEvent().Remove(OnBudgetPreviewUpdate);
			m_BudgetComponent.GetOnBudgetPreviewResetEvent().Remove(ResetWidgetPreviewData);
		}
	}
	
};