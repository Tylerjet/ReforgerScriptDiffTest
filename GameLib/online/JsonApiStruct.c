
/** @file */


// -------------------------------------------------------------------------
// error codes for handle processing
// defined in C++
enum EJsonApiError
{
	ETJSON_UNKNOWN,			// invalid code

	ETJSON_OK,				// all fine
	ETJSON_COMMSEND,		// error during send
	ETJSON_COMMRECV,		// error during receive
	ETJSON_PARSERERROR,		// error during parsing
	ETJSON_PACKNOSTART,		// error - cannot start packing (invalid state)
	ETJSON_TIMEOUT,			// failed to send/ store handle due to timeout
	ETJSON_NOBUFFERS,		// not enough buffers available
	ETJSON_FAILFILELOAD,	// failed to load file
	ETJSON_FAILFILESAVE,	// failed to save file
	ETJSON_NOTARRAY,		// object is not array (ie. attempt to provide different or none object as array)
}


// -------------------------------------------------------------------------
// object which allow to parse upon generic JSON structure and format it back
//
//
class JsonApiStruct : Managed
{

	void JsonApiStruct()
	{
	}

	void ~JsonApiStruct()
	{
	}

	/**
	\brief Event when expand (unpack) process starts
	*/
	void OnExpand()
	{
	}

	/**
	\brief Event when pack starts - you will pack your stuff here
	*/
	void OnPack()
	{
	}

	/**
	\brief Event called when pending store operation is finished - callback from JsonApiHandle before handle release
	*/
	void OnSuccess( int errorCode )
	{
		// errorCode is EJsonApiError
	}

	/**
	\brief Event called when pending store operation is finished - callback from JsonApiHandle before handle release
	*/
	void OnError( int errorCode )
	{
		// errorCode is EJsonApiError
	}
	
	/**
	\brief Called when parsing object
	*/
	void OnObject( string name )
	{
	}

	/**
	\brief Called when parsing array
	*/
	void OnStartArray( string name )
	{
	}

	/**
	\brief Called when array end, returns count of items
	*/
	void OnEndArray( int itemCount )
	{
	}

	/**
	\brief Called when parsing object
	*/
	void OnItemObject( int index, string name )
	{
	}

	/**
	\brief Register script variable for auto-feature
	*/
	proto native void RegV( string name );
	/**
	\brief Unregister script variable for auto-feature
	*/
	proto native void UnregV( string name );
	/**
	\brief Register all variable present on object for auto-feature (it is not recursive!)
	*/
	proto native void RegAll();


	/**
	\brief Push object to parse (only during parse operation)
	*/
	proto native void Push( JsonApiStruct obj );	
	
	/**
	\brief Start object at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto native void StartObject( string name );

	/**
	\brief End object at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto native void EndObject();

	/**
	\brief Add scripted object to hierarchy (calls through hierarchy)
	*/
	proto native void StoreObject( string name, JsonApiStruct obj );
	
	/**
	\brief Add float value to hierarchy
	*/
	proto native void StoreFloat( string name, float value );
	
	/**
	\brief Add integer value to hierarchy
	*/
	proto native void StoreInteger( string name, int value );
	
	/**
	\brief Add boolean value to hierarchy
	*/
	proto native void StoreBoolean( string name, bool value );

	/**
	\brief Add string value to hierarchy
	*/
	proto native void StoreString( string name, string value );
	
	/**
	\brief Add vector value to hierarchy
	*/
	proto native void StoreVector( string name, vector value );
	
	/**
	\brief Start array at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto native void StartArray( string name );

	/**
	\brief End array at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto native void EndArray();
	
	/**
	\brief Add scripted unnamed/ array object
	*/
	proto native void ItemObject( JsonApiStruct obj );
	
	/**
	\brief Add unnamed/ array float value
	*/
	proto native void ItemFloat( float value );
	
	/**
	\brief Add unnamed/ array integer value
	*/
	proto native void ItemInteger( int value );
	
	/**
	\brief Add unnamed/ array boolean value
	*/
	proto native void ItemBoolean( bool value );

	/**
	\brief Add unnamed/ array string value
	*/
	proto native void ItemString( string value );

	/**
	\brief Add unnamed/ array vector value
	*/
	proto native void ItemVector( vector value );
	
	/**
	\brief Start an array inside an array
	*/
	proto native void ItemArray();

	/**
	\brief Call this when you've done packing or unpacking (interrupt operation)
	*/
	proto native void SetDone();

	/**
	\brief Call this when you've done packing or unpacking + want to generate error - prevent to send invalid data etc.
	*/
	proto native void SetFail();
	
	/**
	\brief Start object packing now - for use at main thread only!
	*/
	proto native void Pack();

	/**
	\brief Start object unpacking from RAW string data
	*/
	proto native void ExpandFromRAW( string data );

	/**
	\brief Get packed JSON as string (!only if you called Pack() first, it may return null)
	*/
	proto native string AsString();

	/**
	\brief Return true if there are present JSON data which can be expanded on script object (typically you check this after load of file)
	*/
	proto native bool HasData();
	
	/**
	\brief Pack() and save JSON to file
	*/
	proto native bool PackToFile( string FileName );
	/**
	\brief Save JSON to file (only If something was loaded or recieved previously!)
	*/
	proto native bool SaveToFile( string FileName );
	/**
	\brief Load JSON from file and Expand
	*/
	proto native bool LoadFromFile( string FileName );

}

