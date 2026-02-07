/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Visual
\{
*/

enum AnimFlags
{
	/*!
	Animation won't be looping. It will freeze at
	last frame, until is cleared out.
	*/
	ONCE,
	/*!
	Animation won't be playing. Animation frame must be
	set directly by SetFrame method.
	*/
	USER,
	/*!
	Nevertheless there is already running the same animation,
	it will be restarted. If not set, call to SetAnimSlot is
	ignored when you're trying to set the same anim.
	*/
	RESET,
	/*!
	When there is -fps in anim script (anim.def), you must
	force overrride it if you want to change framerate.
	*/
	FORCEFPS,
	/*!
	Works together with AF_ONCE. When it's set, animation
	won't freeze at the last frame, but is cleared out automaticly
	and it fades out by set fadeout-time.
	*/
	BLENDOUT,
	/*!
	Animation hooks won't be called, when set.
	*/
	NOANIMHOOKS,
	/*!
	EV_ANIMEND won't be called, when set.
	*/
	NOANIMEND,
}

/*!
\}
*/
