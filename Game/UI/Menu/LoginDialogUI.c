class LoginCallback : BackendCallback
{
	protected static int F2A_CODE = 422;
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
		if (restCode == F2A_CODE)
			dialog.OnTwoFactorAuthentication();
		else
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
};

//------------------------------------------------------------------------------------------------
class LoginDialogUI: DialogUI
{
	ref LoginCallback m_LoginCallback = new LoginCallback();
	SCR_EditBoxComponent m_UserName;
	SCR_EditBoxComponent m_Password;
	SCR_EditBoxComponent m_2FACode;
	SCR_NavigationButtonComponent m_CreateAccount;
	Widget m_PasswordSection;
	Widget m_AuthCodeSection;
	
	Widget m_WarningText;
	Widget m_CodeWarningText;
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
		Widget w = GetRootWidget();
		m_wOverlay = w.FindAnyWidget("Content");
		string name = GetGame().GetBackendApi().GetCredentialsItem(EBackendCredentials.EBCRED_NAME);
		m_UserName = SCR_EditBoxComponent.GetEditBoxComponent("UserName", w);
		m_WarningText = w.FindAnyWidget("DialogWarning");
		if (m_WarningText)
			m_WarningText.SetOpacity(0);
		if (m_UserName)
		{
			m_UserName.SetValue(name);
					m_UserName.m_OnChanged.Insert(CheckFilledEditboxes);
		}

		m_Password = SCR_EditBoxComponent.GetEditBoxComponent("Password", w);
		if (m_Password)
					m_Password.m_OnChanged.Insert(CheckFilledEditboxes);

		m_CreateAccount = SCR_NavigationButtonComponent.GetNavigationButtonComponent("CreateAccount", w);
		if (m_CreateAccount)
			m_CreateAccount.m_OnActivated.Insert(OnCreateAccount);

		m_CodeWarningText = w.FindAnyWidget("CodeWarning");
		if (m_CodeWarningText)
			m_CodeWarningText.SetOpacity(0);
		
		m_2FACode = SCR_EditBoxComponent.GetEditBoxComponent("Code", w);
		if (m_2FACode)
		{
			m_2FACode.m_OnChanged.Insert(CheckFilledEditboxes);
		}
		
		m_PasswordSection = w.FindAnyWidget("LoginAndPassword");
		m_AuthCodeSection = w.FindAnyWidget("F2A");
		if (m_PasswordSection)
			m_PasswordSection.SetVisible(true);
		if (m_AuthCodeSection)
			m_AuthCodeSection.SetVisible(false);

		CheckFilledEditboxes();
	}

	//------------------------------------------------------------------------------------------------
	private void OnCreateAccount()
	{
		GetGame().GetPlatformService().OpenBrowser(ACCOUNT_CREATION_URL);
	}

	//------------------------------------------------------------------------------------------------
	void OnTwoFactorAuthentication()
	{
		if (m_AuthCodeSection.IsVisible())
			m_2FACode.OnInvalidInput();
		else
			m_2FACode.SetValue(string.Empty);
		
		m_AuthCodeSection.SetVisible(true);
		m_PasswordSection.SetVisible(false);
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
			m_WarningText.SetOpacity(0);

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

		// If there is a two factor authentication code, send it along other credentials
		if (m_2FACode && !m_2FACode.GetValue().IsEmpty())
			GetGame().GetBackendApi().SetCredentialsItem(EBackendCredentials.EBCRED_2FA_TOKEN, m_2FACode.GetValue());

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
		m_CodeWarningText.SetVisible(true);
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
		m_Warning.SetOpacity(1);
	}
};
