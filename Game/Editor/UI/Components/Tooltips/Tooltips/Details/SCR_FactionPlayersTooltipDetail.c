[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_FactionPlayersTooltipDetail: SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	protected SCR_RespawnSystemComponent m_RespawnSystem;
	
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		Faction faction = entity.GetFaction();
		m_Text.SetText(m_RespawnSystem.GetFactionPlayerCount(faction).ToString());
	}
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_RespawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!m_RespawnSystem)
			return false;
		
		m_Text = TextWidget.Cast(widget);
		return m_Text != null;
	}
};