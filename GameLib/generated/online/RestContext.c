/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Script accessible REST context
class RestContext
{
	proto external int GET(RestCallback cb, string request);
	proto external string GET_now(string request);
	proto external int FILE(RestCallback cb, string request, string filename);
	proto external int FILE_now(string request, string filename);
	proto external int POST(RestCallback cb, string request, string data);
	proto external string POST_now(string request, string data);
	proto external int PUT(RestCallback cb, string request, string data);
	proto external string PUT_now(string request, string data);
	proto external int DELETE(RestCallback cb, string request, string data);
	proto external string DELETE_now(string request, string data);
	proto external void reset();
}

/*!
\}
*/
