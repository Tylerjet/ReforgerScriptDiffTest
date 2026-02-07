class SCR_ServerConfigAdvancedComponent : SCR_ConfigListComponent
{
	protected const string BIND_IP = "bindAddress";
	protected const string BIND_PORT = "bindPort";
	protected const string IP = "publicAddress";
	protected const string PORT = "publicPort";
	
	protected SCR_WidgetListEntryEditBox m_BindPortEdit;
	protected SCR_WidgetListEntryEditBox m_PortEdit;
	
	protected bool m_bWasPortEdited = false;
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Find widges 
		m_BindPortEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(BIND_PORT));
		if (m_BindPortEdit)
			m_BindPortEdit.GetEditBoxComponent().m_OnChanged.Insert(OnBindPortChanged);
		
		m_PortEdit = SCR_WidgetListEntryEditBox.Cast(FindEntry(PORT));
		if (m_PortEdit)
			m_PortEdit.GetEditBoxComponent().m_OnChanged.Insert(OnPortChanged);
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnBindPortChanged(SCR_EditBoxComponent edit, string text)
	{
		// Edit port by bind port
		if (!m_bWasPortEdited && m_PortEdit)
		{
			m_PortEdit.SetValue(text);
			m_PortEdit.ClearInvalidInput();
		}
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnPortChanged(SCR_EditBoxComponent edit, string text)
	{
		// Registed port edited
		if (!m_bWasPortEdited && !text.IsEmpty())
		{
			m_bWasPortEdited = true;
		}
		else if (m_bWasPortEdited && text.IsEmpty())
		{
			m_bWasPortEdited = false;
		}
	}
	
	//-------------------------------------------------------------------------------------------
	//! Fill all entries with values from given DS config 
	void FillFromDSConfig(notnull SCR_DSConfig config)
	{
		FindEntry(BIND_IP).SetValue(config.bindAddress);
		FindEntry(BIND_PORT).SetValue(config.bindPort.ToString());
		FindEntry(IP).SetValue(config.publicAddress);
		FindEntry(PORT).SetValue(config.publicPort.ToString());
	}
	
	//-------------------------------------------------------------------------------------------
	void SetIPPort(DSConfig config)
	{
		FindEntry(BIND_IP).SetValue(config.bindAddress);
		FindEntry(BIND_PORT).SetValue(config.bindPort.ToString());
		FindEntry(IP).SetValue(config.publicAddress);
		FindEntry(PORT).SetValue(config.publicPort.ToString());
	}
}