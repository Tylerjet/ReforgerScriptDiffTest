class PlatformService
{
	// Native C++ methods

	/*!
	Requests privilege from platform. Implemented as queue. When the result for
	one type of privilege is received, all requests of that type receive the result.
	\param[privilege] Requested privilege
	\param[callback] Callback object used to signalize result
	\return Returns if the request was successfuly queued
	*/
	proto native bool GetPrivilegeAsync(UserPrivilege privilege, PlatformRequestCallback callback);

	/*!
	Checks for given user privilege without invoking any UI.
	\param[privilege] Requested privilege
	\return Returns true if the privilege is granted
	*/
	proto native bool GetPrivilege(UserPrivilege privilege);

	/*!
	Open native web browser on specified url.
	*/
	proto native void OpenBrowser(string url);

	/*!
	Returns \see PlatformKind for this particular executable.
	*/
	proto native PlatformKind GetLocalPlatformKind();
}
