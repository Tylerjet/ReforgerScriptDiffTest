
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
};


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

};


// -------------------------------------------------------------------------
// context for request Api
class RestContext
{
	/**
	\brief Processes GET request and returns result (ERestResult) and/ or data (timeout, error) when finished
	*/
	proto native int GET( RestCallback cb, string request );

	/**
	\brief Processes GET request and returns data immediately (thread blocking operation!)
	*/
	proto native string GET_now( string request );

	/**
	\brief Processes GET request and returns result (ERestResult) and/ or stores data int specified file (timeout, error) when finished
	*/
	proto native int FILE( RestCallback cb, string request, string filename );

	/**
	\brief Processes GET request and returns result (ERestResult) and/ stores data int specified file immediately (thread blocking operation!)
	*/
	proto native int FILE_now( string request, string filename );

	/**
	\brief Pushes POST request and returns result (ERestResult) and/ or data (timeout, error) when finished
	*/
	proto native int POST( RestCallback cb, string request, string data );

	/**
	\brief Processes POST request and returns data immediately (thread blocking operation!)
	*/
	proto native string POST_now( string request, string data );

	/**
	\brief Clear all pending requests and buffers
	*/
	proto native void reset();

};


// -------------------------------------------------------------------------
// Rest Api for context create/ access + debug features
class RestApi
{
	/**
	\brief Get new or existing context for http comm GetContext("www.server915.com/interface/")
	*/
	proto native RestContext GetContext( string serverURL );
	
	/**
	\brief Get count of registered contexes
	*/
	proto native int GetContextCount();

	/**
	\brief List of all currently active contexes and processed (pending) requests
	*/
	proto native void DebugList();

};


