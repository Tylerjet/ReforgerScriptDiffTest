[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FuelTooltipDetail: SCR_EntityTooltipDetail
{
	protected TextWidget m_Value;
	protected SCR_WLibProgressBarComponent m_Bar;
	protected FuelManagerComponent m_FuelManager;
	
	[Attribute("#AR-ValueUnit_Percentage")]
	private string m_sPercentageText;
	
	override bool NeedUpdate()
	{
		return m_FuelManager != null;
	}
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		if (!m_FuelManager) return;
		
		float fuel = m_FuelManager.GetTotalFuel() / m_FuelManager.GetTotalMaxFuel();
		
		if (m_Bar)
		{
			m_Bar.SetValue(fuel);
		}
		if (m_Value)
		{
			m_Value.SetTextFormat(m_sPercentageText, Math.Round(fuel * 100));
		}
	}
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Value = TextWidget.Cast(m_Widget.FindAnyWidget("Value"));
		
		Widget barWidget = m_Widget.FindAnyWidget("ProgressBar");
		if (barWidget)
			m_Bar = SCR_WLibProgressBarComponent.Cast(barWidget.FindHandler(SCR_WLibProgressBarComponent));
		
		m_FuelManager = FuelManagerComponent.Cast(entity.GetOwner().FindComponent(FuelManagerComponent));
		return m_FuelManager && (m_Value || m_Bar);
	}
};