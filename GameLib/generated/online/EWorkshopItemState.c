/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Workshop item stats (values are bit flags!)
enum EWorkshopItemState
{
	//! 2
	EWSTATE_ONLINE,
	//! 4 [Obsolete("Use Revision.IsDownloaded")]
	EWSTATE_OFFLINE,
	//! 8
	EWSTATE_UPLOADING,
	//! 16
	EWSTATE_DOWNLOADING,
	//! 32
	EWSTATE_OUTDATED,
	//! 64
	EWSTATE_CORRUPTED,
	EWSTATE_QUEUED,
	EWSTATE_FAVOURITE,
	EWSTATE_PURCHASED,
	EWSTATE_RECOMMENDED,
	EWSTATE_HIGHLIGHTED,
	//! 4096
	EWSTATE_MYCREATION,
	EWSTATE_BANNED,
	EWSTATE_ABORTING_DOWNLOAD,
}

/*!
\}
*/
