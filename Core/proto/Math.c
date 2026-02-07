/**
 * \defgroup Math
 * \desc Math library
 * @{
 */

class FilterSmoothCD: Managed
{
	// settings
	float m_fSmoothTime;
	float m_fMaxVelocity;
	
	// values
	float m_fActVal;
	float m_fVelocity;
	
	void FilterSmoothCD(float pSmoothTime = 0.1, float pMaxVel = FLT_MAX)
	{
		m_fSmoothTime = pSmoothTime;
		m_fMaxVelocity = pMaxVel;
	}
	
	//! resets filter 
	void Reset(float pVal)
	{		
		m_fActVal = pVal;
		m_fVelocity	= 0;
	}

	//! sets the value only - retains velocity ! 
	void SetValue(float pVal)
	{
		m_fActVal = pVal;
	}
	
	float GetValue()
	{
		return m_fActVal;
	}
	
	//! 1 filter step
	float Filter(float pDt, float pVal)
	{
		m_fActVal = Math.SmoothCD(m_fActVal, pVal, m_fVelocity, m_fSmoothTime, m_fMaxVelocity, pDt);
		return m_fActVal;
	}
};

class FilterSmoothCDAng: FilterSmoothCD
{
	override float Filter(float pDt, float pVal)
	{
		m_fActVal = Math.SmoothCDPI2PI(m_fActVal, pVal, m_fVelocity, m_fSmoothTime, m_fMaxVelocity, pDt);
		return m_fActVal;
	}
};

/*!
Represents a single 2D curve, where in each vector only the x and y coordinates are used
Intended to be used as a property in following way
@code
	[Attribute("", UIWidgets.GraphDialog)]
	CurvePoints Curve;
@endcode

May be evaluated with
@code
	Math3D.Curve(ECurveType.CatmullRom, t, Curve);
@endcode
*/
class Curve : array<vector>
{
	// Empty class just to distinguish this array<vector> from the others
}

/*!
Represents 3 2D curves, where each 4 floats next to each other in the underlying array are organized as follows
(x, y0, y1, y2), each curve shares the x value and then three separate y values are present
Indended to be used as a property in following way
@code
	[Attribute("", UIWidgets.GraphDialog)]
	Curve3Points Curve;
@endcode

May be evaluated with following code, where i is index of the curve to be evaluated from 0 to 2
@code
	Math3D.Curve3(ECurveType.CatmullRom, t, Curve, i);
@endcode
*/
class Curve3 : array<float>
{
	/*!
	\return vector in format (x, y, 0)
	*/
	vector GetPoint(int pointIndex, int curveIndex)
	{
		vector result;
		result[0] = this[pointIndex * 4];
		result[1] = this[pointIndex * 4 + curveIndex + 1];
		return result;
	}

	/*!
	*/
	void SetPointValue(int pointIndex, int curveIndex, float value)
	{
		this[pointIndex * 4 + curveIndex + 1] = value;
	}
}

//@}