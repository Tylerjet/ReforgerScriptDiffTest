class LoginCallback : BackendCallback
{
	LoginDialogUI dialog;
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		dialog.ShowLoadinAnim(false);
		dialog.OnSuccess();
		dialog.CloseDelayed();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError( int code, int restCode, int apiCode )
	{
		dialog.ShowLoadinAnim(false);
		dialog.ShowErrorMessage(code);	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
		dialog.ShowLoadinAnim(false);
		dialog.ShowTimeoutMessage();
	}
};

//------------------------------------------------------------------------------------------------
class LogoutCallback : BackendCallback
{
	LogoutDialogUI dialog;
	
	//------------------------------------------------------------------------------------------------
	override void OnSuccess(int code)
	{
		dialog.OnSuccess();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnError( int code, int restCode, int apiCode )
	{
		
		dialog.OnError();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTimeout()
	{
	}
}

//------------------------------------------------------------------------------------------------
class LoginDialogUI: DialogUI
{
	ref LoginCallback m_LoginCallback = new LoginCallback();
	SCR_EditBoxComponent m_UserName;
	SCR_EditBoxComponent m_Password;
	SCR_NavigationButtonComponent m_CreateAccount;
	Widget m_WarningText;
	Widget m_wOverlay;
	int m_iDelayMs = 500;
	SCR_LoadingOverlay m_LoadingOverlay;
	const string ACCOUNT_CREATION_URL = "https:\/\/accounts.bistudio.com/auth/register";
	
	ref ScriptInvoker m_OnDialogClosed = new ScriptInvoker();
	ref ScriptInvoker m_OnLogin = new ScriptInvoker();
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Load credentials
		m_wOverlay = GetRootWidget().FindAnyWidget("Content");
		string name = GetGame().GetBackendApi().GetCredentialsItem(EBackendCredentials.EBCRED_NAME);
		m_UserName = SCR_EditBoxComponent.GetEditBoxComponent("UserName", GetRootWidget());
		m_WarningText = GetRootWidget().FindAnyWidget("DialogWarning");
		if (m_WarningText)
			m_WarningText.SetVisible(false);
		if (m_UserName)
		{
			m_UserName.SetValue(name);
					m_UserName.m_OnChanged.Insert(CheckFilledEditboxes);

		}
		
		string password = GetGame().GetBackendApi().GetCredentialsItem(EBackendCredentials.EBCRED_PWD);
		m_Password = SCR_EditBoxComponent.GetEditBoxComponent("Password", GetRootWidget());
		if (m_Password)
		{
			m_Password.SetValue(password);
					m_Password.m_OnChanged.Insert(CheckFilledEditboxes);

		}
		
		m_CreateAccount = SCR_NavigationButtonComponent.GetNavigationButtonComponent("CreateAccount", GetRootWidget());
		if (m_CreateAccount)
			m_CreateAccount.m_OnActivated.Insert(OnCreateAccount);
		
		CheckFilledEditboxes();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnCreateAccount()
	{
		GetGame().GetPlatformService().OpenBrowser(ACCOUNT_CREATION_URL);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		ShowLoadinAnim(false);
		m_OnDialogClosed.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		m_LoginCallback.dialog = this;
		// Store credentials
		if (m_WarningText)
			m_WarningText.SetVisible(false);
		if (m_UserName)
		{
			string name = m_UserName.GetValue();
			GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_NAME,name);
		}
		
		if (m_Password)
		{
			string password = m_Password.GetValue();
			GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_PWD,password);			
		}
		
		GetGame().GetBackendApi().VerifyCredentials(m_LoginCallback, true);
		ShowLoadinAnim(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void CheckFilledEditboxes()
	{
		bool enabled = m_UserName.GetValue() != string.Empty && m_Password.GetValue() != string.Empty;
		m_Confirm.SetEnabled(enabled);
	}

	
	//------------------------------------------------------------------------------------------------
	void CloseDelayed()
	{
		GetGame().GetCallqueue().CallLater(CloseAnimated, m_iDelayMs);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowErrorMessage(int code)
	{
		Print("[LoginDialogUI] Login error", LogLevel.WARNING);
		if (!m_WarningText)
			return;
		m_WarningText.SetVisible(true);
		m_UserName.OnInvalidInput();
		m_Password.OnInvalidInput();
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowTimeoutMessage()
	{
		Print("[LoginDialogUI] Login error - timeout", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSuccess()
	{
		Print("[LoginDialogUI] Login succeeded", LogLevel.WARNING);
		m_OnLogin.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowLoadinAnim(bool show)
	{
		if (show && m_wOverlay)
			 m_LoadingOverlay = SCR_LoadingOverlay.ShowForWidget(m_wOverlay, string.Empty, false, true);
		else if (m_LoadingOverlay)
			m_LoadingOverlay.HideAndDelete();
	}
};

//------------------------------------------------------------------------------------------------
class LogoutDialogUI: DialogUI
{
	ref LogoutCallback m_LogoutCallback = new LogoutCallback();
	Widget m_Warning;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_Warning = GetRootWidget().FindAnyWidget("Warning");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		m_LogoutCallback.dialog = this;
		GetGame().GetBackendApi().Unlink(m_LogoutCallback);
	}
	 
	
	//------------------------------------------------------------------------------------------------
	void OnSuccess()
	{
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnError()
	{
		m_Warning.SetVisible(true);
	}
};
