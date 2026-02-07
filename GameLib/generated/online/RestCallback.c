/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! -------------------------------------------------------------------------
//! object to be used from script for result binding
//!
//!	[Example:]
//!
//!		RestCallback cbx1 = new RestCallback;
//!		RestContext ctx = g_Game.GetRestApi().GetContext("http://somethingsomewhere.com/path/");
//!		ctx.GET(cbx1,"RequestPath?Argument=Something");
//!
//!		Event are then called upon RestCallback()
//!
class RestCallback: Managed
{
	/*!
	\brief	Called when data arrived and/ or response processed successfully
			Override this with your implementation
	\code
	Print(" !!! OnSuccess() size=" + dataSize );
	if( dataSize > 0 )
		Print(data); // !!! NOTE: Print() will not output string longer than 1024b, check your dataSize !!!
	\endcode
	*/
	event void OnSuccess(string data, int dataSize);
	/*!
	\brief	Called in case request failed (ERestResult) - Note! May be called multiple times in case of (RetryCount > 1)
			Override this with your implementation
	\code
	Print(" !!! OnError() ");
	\endcode
	*/
	event void OnError(int errorCode);
	/*!
	\brief	Called in case request timed out or handled improperly (no error, no success, no data)
			Override this with your implementation
	\code
	Print(" !!! OnTimeout() ");
	\endcode
	*/
	event void OnTimeout();
	/*!
	\brief	OnFileCreated is OBSOLETE - Will be removed.
	*/
	event void OnFileCreated(string fileName, int dataSize);
}

/*!
\}
*/
