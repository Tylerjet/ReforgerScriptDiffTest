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
//! states, (result + error) codes
//! defined in C++
enum ERestResult
{
	EREST_EMPTY,
	//!> not initialized
	EREST_PENDING,
	//!> awaiting processing
	EREST_FEEDING,
	//!> awaiting incoming data
	EREST_SUCCESS,
	//!> result and/ or data are ready (success), awaiting data processing to be finished (no longer blocking queue processing)
	EREST_PROCESSED,
	//!> finished and ready for removal (manually released by owner)
	EREST_DEFAULT,
	//!> keep the original state
	EREST_ERROR,
	//!> (state >= EREST_ERROR) == error happened
	EREST_ERROR_CLIENTERROR,
	EREST_ERROR_SERVERERROR,
	EREST_ERROR_APPERROR,
	EREST_ERROR_TIMEOUT,
	EREST_ERROR_NOTIMPLEMENTED,
	EREST_ERROR_PACKINGREQUEST,
	//!> packing request to JSON failed (too much data?)
	EREST_ERROR_CREATE,
	//!> failed to even create REST request (send failed)
	EREST_ERROR_RECV,
	//!> failed to receive response
	EREST_ERROR_SEND,
	//!> failed to send request
	EREST_ERROR_STORE,
	//!> unable to store received data (not enough space?)
	EREST_ERROR_UNKNOWN,
}

/*!
\}
*/
