//
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_EntityConditionTooltipDetail: SCR_EntityTooltipDetail
{
	protected SCR_MultiTextTooltipUIComponent m_MultiLineTextWidget;
	
	[Attribute()]
	protected ref array<ref SCR_DamageOverTimeUIVisuals> m_aDamageOverTimeConditions;
	
	[Attribute("#AR-Editor_TooltipDetail_Conditions_Destroyed")]
	protected LocalizedString m_sDestroyedConditionName;
	
	[Attribute("#AR-Editor_TooltipDetail_Conditions_None")]
	protected LocalizedString m_sNoConditionsName;
	
	protected DamageManagerComponent m_DamageManager;
	
	override bool NeedUpdate()
	{
		return m_DamageManager && m_MultiLineTextWidget;
	}
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{				
		m_MultiLineTextWidget.ClearAllText();
		
		if (m_DamageManager.GetState() == EDamageState.DESTROYED)
		{
			m_MultiLineTextWidget.AddText(m_sDestroyedConditionName);
			return;
		}
		
		bool HasAnyValidCondition = false;
		foreach (SCR_DamageOverTimeUIVisuals dpsVisual: m_aDamageOverTimeConditions)
		{
			if (m_DamageManager.IsDamagedOverTime(dpsVisual.m_iDamageOvertimeType))
			{
				m_MultiLineTextWidget.AddText(dpsVisual.m_UiInfo.GetName());
				HasAnyValidCondition = true;
			}	
		}
		
		if (!HasAnyValidCondition)
		{
			m_MultiLineTextWidget.AddText(m_sNoConditionsName);
		}
	}
	
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_MultiLineTextWidget = SCR_MultiTextTooltipUIComponent.Cast(widget.FindHandler(SCR_MultiTextTooltipUIComponent));
		if (!m_MultiLineTextWidget)
			return false;
		
		m_DamageManager = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		
		return m_DamageManager != null && (m_DamageManager.GetState() == EDamageState.DESTROYED || HasAnyValidCondition());
	}
	
	protected bool HasAnyValidCondition()
	{
		foreach (SCR_DamageOverTimeUIVisuals dpsVisual: m_aDamageOverTimeConditions)
		{
			if (m_DamageManager.IsDamagedOverTime(dpsVisual.m_iDamageOvertimeType))
				return true;
		}
		
		return false;
	}
};

//

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EDamageType, "m_iDamageOvertimeType")]
class SCR_DamageOverTimeUIVisuals
{
	[Attribute("0", UIWidgets.ComboBox, "Damage overtime Type", "", ParamEnumArray.FromEnum(EDamageType) )]
	EDamageType m_iDamageOvertimeType;
	
	[Attribute()]
	ref SCR_UIInfo m_UiInfo;
};