
/** @file */


// -------------------------------------------------------------------------
// states, (result + error) codes
// defined in C++
enum ERestResult
{
	EREST_EMPTY,				// not initialized
	EREST_PENDING,				// awaiting processing
	EREST_FEEDING,				// awaiting incoming data
	EREST_SUCCESS,				// result and/ or data are ready (success), awaiting data processing to be finished (no longer blocking queue processing)
	EREST_PROCESSED,			// finished (either successfully or with failure) and eill be removed ASAP

	EREST_ERROR,				// (state >= EREST_ERROR) == error happened
	EREST_ERROR_CLIENTERROR,	//  (EREST_ERROR == EREST_ERROR_CLIENTERROR)
	EREST_ERROR_SERVERERROR,
	EREST_ERROR_APPERROR,
	EREST_ERROR_TIMEOUT,
	EREST_ERROR_NOTIMPLEMENTED,
	EREST_ERROR_UNKNOWN,
}


// -------------------------------------------------------------------------
// object to be used from script for result binding
//
//	[Example:]
//
//		RestCallback cbx1 = new RestCallback;
//		RestContext ctx = g_Game.GetRestApi().GetContext("http://somethingsomewhere.com/path/");
//		ctx.GET(cbx1,"RequestPath?Argument=Something");
//
//		Event are then called upon RestCallback()
//
class RestCallback : Managed
{
	/**
	\brief Called in case request failed (ERestResult) - Note! May be called multiple times in case of (RetryCount > 1)
	*/
	void OnError( int errorCode )
	{
		// override this with your implementation
		//Print(" !!! OnError() ");
	};

	/**
	\brief Called in case request timed out or handled improperly (no error, no success, no data)
	*/
	void OnTimeout()
	{
		// override this with your implementation
		//Print(" !!! OnTimeout() ");
	};

	/**
	\brief Called when data arrived and/ or response processed successfully
	*/
	void OnSuccess( string data, int dataSize )
	{
		// override this with your implementation
		//Print(" !!! OnSuccess() size=" + dataSize );
		//if( dataSize > 0 )
		//	Print(data); // !!! NOTE: Print() will not output string longer than 1024b, check your dataSize !!!
	};

	/**
	\brief Called when data arrived and/ or file created successfully
	*/
	void OnFileCreated( string fileName, int dataSize )
	{
		// override this with your implementation
		//Print(" !!! OnFileCreated() file=" + fileName + " size=" + dataSize );
	};

}


