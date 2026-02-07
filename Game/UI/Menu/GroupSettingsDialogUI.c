//------------------------------------------------------------------------------------------------
class GroupSettingsDialogUI: DialogUI
{
	
	protected SCR_EditBoxComponent m_Description;
	protected SCR_ComboBoxComponent m_GroupStatus;
	protected SCR_EditBoxComponent m_GroupName;
	protected SCR_EditBoxComponent m_GroupDescription;
		
	protected SCR_PlayerControllerGroupComponent m_PlayerComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_AIGroup m_PlayerGroup;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_PlayerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerComponent)
			return;
		
		if (m_PlayerComponent.GetSelectedGroupID() == -1)
			Close();
		
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupsManager)
			return;
		
		m_PlayerGroup = m_GroupsManager.FindGroup(m_PlayerComponent.GetGroupID());
		if (!m_PlayerGroup)
			return;
		
		Widget w = GetRootWidget();
		if (!w)
			return;
		
		m_GroupStatus = SCR_ComboBoxComponent.GetComboBoxComponent("Type", w);
		if (!m_GroupStatus)
			return;
		
		m_GroupName = SCR_EditBoxComponent.GetEditBoxComponent("Name", w);
		if (!m_GroupName)
			return;
		
		m_GroupDescription = SCR_EditBoxComponent.GetEditBoxComponent("Description", w);
		if (!m_GroupDescription)
			return;
		
		m_GroupName.SetValue(m_PlayerGroup.GetCustomName());
		m_GroupDescription.SetValue(m_PlayerGroup.GetCustomDescription());
		SetupGroupStatusCombo();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return;
		
		int groupID = m_PlayerGroup.GetGroupID();
		groupController.RequestSetCustomGroupDescription(groupID, m_GroupDescription.GetValue());
		groupController.RequestSetCustomGroupName(groupID, m_GroupName.GetValue());
		
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupGroupStatusCombo()
	{
		m_GroupStatus.AddItem("#AR-Player_Groups_Public");
		m_GroupStatus.AddItem("#AR-Player_Groups_Private");
		
		m_GroupStatus.SetCurrentItem(m_PlayerGroup.IsPrivate());
		m_GroupStatus.m_OnChanged.Insert(OnStatusChanged);
		
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStatusChanged(SCR_ComboBoxComponent combo, int index)
	{
		if (!m_PlayerComponent)
			return;
		
		m_PlayerComponent.RequestPrivateGroupChange(m_PlayerComponent.GetPlayerID() , index);
	}
};
