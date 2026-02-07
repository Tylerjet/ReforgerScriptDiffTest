/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class AnimationPlayerComponentClass: GenericComponentClass
{
}

class AnimationPlayerComponent: GenericComponent
{
	//! Check whether the animation is being played at the moment.
	proto external bool IsPlaying();
	//! Prepare this component for playing, provide an animation, start time, playing speed and loop flag.
	proto bool Prepare(ResourceName animation, float startTime, float speed, bool loop);
	//! Start playing the animation. Call 'Prepare' first if you need to change the setup of a component!
	proto external void Play(IEntity owner);
	//! Stop the animation.
	proto external void Stop(IEntity owner);
}

/*!
\}
*/
