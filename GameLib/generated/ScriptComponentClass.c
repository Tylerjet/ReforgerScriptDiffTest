/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScriptComponentClass: GenericComponentClass
{
	/*!
	\brief Implement this if you need to enforce certain creation order of components
	\param className - name of other ComponentClass class
	@code
	static override bool DependsOn(string className)
	{
		if (className == "MyOtherComponentClass")
			return true;

		return false;
	}
	@endcode
	*/
	static event bool DependsOn(string className);
	/*!
	\brief Implement this if you need to enforce some other component type
	\return array of component types
	@code
	static override array<typename> Requires(IEntityComponentSource src)
	{
		return {MyOtherComponent};
	}
	@endcode
	*/
	static event array<typename> Requires(IEntityComponentSource src);
	/*!
	\brief Implement this if you need to disallow to concurrent usage with the other type
	\return array of component types
	@code
	static override array<typename> CannotCombine(IEntityComponentSource src)
	{
		return {MyOtherComponent};
	}
	@endcode
	*/
	static event array<typename> CannotCombine(IEntityComponentSource src);
}
