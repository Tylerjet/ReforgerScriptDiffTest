/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Backend Request/Response Callback class for script
class BackendCallback: Managed
{
	/*!
	Request finished with successful result.
	\param code				Code to identify what request was successful
	*/
	event void OnSuccess(int code);
	/*!
	Request finished with error result.
	\param code				Error code of EBackendError type
	\param restCode	Generic HTTP result code
	\param apiCode		Error code provided by Backend API of EApiCode type
	*/
	event void OnError(int code, int restCode, int apiCode);
	//! Request did not finish due to timeout.
	event void OnTimeout();
}

/*!
\}
*/
