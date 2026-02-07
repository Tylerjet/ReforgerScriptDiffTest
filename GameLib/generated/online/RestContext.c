/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup online
* @{
*/

//! Script accessible REST context
class RestContext
{
	proto external int GET(RestCallback cb, string request);
	proto external string GET_now(string request);
	proto external int FILE(RestCallback cb, string request, string filename);
	proto external string FILE_now(string request, string filename);
	proto external string POST(RestCallback cb, string request, string data);
	proto external string POST_now(string request, string data);
	proto external void reset();
};

/** @}*/
