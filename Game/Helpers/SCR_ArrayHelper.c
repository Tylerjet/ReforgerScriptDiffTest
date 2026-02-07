class SCR_ArrayHelper
{
	//------------------------------------------------------------------------------------------------
	/**
		\brief Given array of weights, get a random index based on the weight values
		i.e in an array [80,15,5], 80% of the time, we get index 0, 15% index 1 and 5% index 2, weight sum does not have to add up to any particular value,
		ie [100,100,100,100] gives equal 25% distribution(provided random distribution) for all 4 indices
		\param weights \p array with weights. If empty, the method returns -1
		\param value \p a value between [0..1], can be random, perlin, or any other. if outside these boundaries, will be clamped
		\return \p int index selected based on weight
		@code
			array<float> weights = {};
			weights.Insert(80);
			weights.Insert(15);
			weights.Insert(5);

			float rand01;
			int index;

			rand01 = Math.RandomFloat01();
			index = SCR_ArrayHelper.GetWeightedIndex(weights, rand01);
			Print(index);
			rand01 = Math.RandomFloat01();
			index = SCR_ArrayHelper.GetWeightedIndex(weights, rand01);
			Print(index);
			rand01 = Math.RandomFloat01();
			index = SCR_ArrayHelper.GetWeightedIndex(weights, rand01);
			Print(index);

			>> 'index = 0'
			>> 'index = 1'
			>> 'index = 0'
		@endcode
	*/
	static int GetWeightedIndex(notnull array<float> weights, float value)
	{
		if (weights.IsEmpty())
			return -1;

		float probabilitySum;
		for (int i, cnt = weights.Count(); i < cnt; i++)
		{
			probabilitySum += weights[i];
		}

		if (value < 0)
			value = 0;
		else if (value > 1)
			value = 1;

		float add = 0;
		probabilitySum *= value;

		for (int i, cnt = weights.Count(); i < cnt; i++)
		{
			add += weights[i];

			if (add > probabilitySum)
				return i;
		}

		return 0;
	}
};

//! T being the array ITEM class, not the full array<item>
class SCR_ArrayHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! Gets all items that are common to the two arrays
	static void Intersect(notnull array<T> array1, notnull array<T> array2, notnull out array<T> result/*, bool unique = true */)
	{
		result.Clear();
		array<T> smallArray;
		array<T> bigArray;
		if (array1.Count() > array2.Count())
		{
			smallArray = array2;
			bigArray = array1;
		}
		else
		{
			bigArray = array1;
			smallArray = array2;
		}

		for (int i = smallArray.Count() - 1; i >= 0; i--)
		{
			if (bigArray.Contains(smallArray[i]))
				result.Insert(smallArray[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reverse item orders
	//! OBSOLETE when array.Reverse appears
	//! @code
	//! array<int> values = { 2, 42, 33, 0 };
	//! SCR_ArrayHelperT<int>.Reverse(values);
	//! Print(values); // outputs { 0, 33, 42, 2 }
	//! @endcode
	static void Reverse(notnull inout array<T> items)
	{
		int itemsCount = items.Count();
		if (itemsCount < 2)
			return;

		int flooredMiddle = itemsCount * 0.5;
		itemsCount--;

		// 4 indices
		// flooredMiddle = 2
		// 0 <-> 3
		// 1 <-> 2
		// { 0 1 2 3 }
		// { 3 2 1 0 }

		// 5 indices
		// flooredMiddle = 2
		// 0 <-> 4
		// 1 <-> 3
		// 2 is central
		// { 0 1 2 3 4 }
		// { 4 3 2 1 0 }

		for (int i; i < flooredMiddle; i++)
		{
			items.SwapItems(i, itemsCount - i);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void ArrayToSet(notnull array<T> toConvert, notnull set<T> result)
	{
		result.Clear();
		foreach (T item : arrayToConvert)
		{
			result.Set(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void SetToArray(notnull set<T> toConvert, notnull array<T> result)
	{
		result.Clear();
		foreach (T item : toConvert)
		{
			result.Insert(item);
		}
	}
};
