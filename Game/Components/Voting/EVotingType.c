enum EVotingType
{
	KICK = 0, ///< Kick a player, value is player ID
	ADMIN, ///< Give a player admin rights, value is player ID
	EDITOR_IN, ///< Give a player Game Master rights, value is player ID
	EDITOR_OUT, ///< Give a player Game Master rights, value is player ID
	RESTART, ///< Restart the world, no value
	WORLD, ///< Choose the next world, value TBD
};