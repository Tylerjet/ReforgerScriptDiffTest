class SCR_ArrayHelper
{
	//------------------------------------------------------------------------------------------------
	/**
		\brief Given array of weights, get a random index based on the weight values
		i.e in an array [80,15,5], 80% of the time, we get index 0, 15% index 1 and 5% index 2, weight sum does not have to add up to any particular value, ie [100,100,100,100] gives equal 25% distribution(provided random distribution) for all 4 indices
		\param weights \p array with weights
		\param value \p a value between [0..1], can be random, perlin, or any other
		\return \p int index selected based on weight
		@code
			array<float> weights = new array<float>;
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

			>> 'index =   0'
			>> 'index =   1'
			>> 'index =   0'
		@endcode
	*/
	static int GetWeightedIndex(array<float> weights, float value)
	{
		float probabilitySum;

		for (int i, cnt = weights.Count(); i < cnt; i++)
		{
			probabilitySum += weights[i];
		}

		float numberAdjusted = value * probabilitySum;

		float add = 0;

		for (int i, cnt = weights.Count(); i < cnt; i++)
		{
			add += weights[i];

			if (add > numberAdjusted)
			{
				return i;
			}
		}
		return 0;
	}
};