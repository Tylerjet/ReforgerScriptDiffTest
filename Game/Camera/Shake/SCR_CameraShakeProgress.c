/*!
	This object allows advancing and simulating individual camera shake(s).
*/
class SCR_CameraShakeProgress
{
	bool IsRunning()
	{
		return false;
	}

	sealed bool IsFinished()
	{
		return !IsRunning();
	}

	protected vector m_vTranslation;
	protected vector m_vRotation;
	protected float m_fFovScale = 1.0;

	/*!
		Returns current translation of this progress.
	*/
	vector GetTranslation()
	{
		return m_vTranslation;
	}

	/*!
		Returns current rotation of this progress.
	*/
	vector GetRotation()
	{
		return m_vRotation;
	}

	/*!
		Returns current field of view scale of this progress.
	*/
	float GetFovScale()
	{
		return m_fFovScale;
	}

	void Update(IEntity owner, float timeSlice)
	{
	}

	void Apply(inout vector transformMatrix[4], inout float fieldOfView)
	{
		vector shakeMatrix[4];
		Math3D.AnglesToMatrix(m_vRotation, shakeMatrix);
		shakeMatrix[3] = m_vTranslation;

		vector modifiedMatrix[4];
		Math3D.MatrixMultiply4(transformMatrix, shakeMatrix, modifiedMatrix);
		transformMatrix[0] = modifiedMatrix[0]; // Math3D.MatrixCopy does not seem to work with inout params?
		transformMatrix[1] = modifiedMatrix[1];
		transformMatrix[2] = modifiedMatrix[2];
		transformMatrix[3] = modifiedMatrix[3];

		fieldOfView = m_fFovScale * fieldOfView;
	}

	void Start(float linearMagnitude, float angularMagnitude, float inTime, float sustainTime, float outTime)
	{
	}

	void Clear()
	{
		m_vTranslation = vector.Zero;
		m_vRotation = vector.Zero;
		m_fFovScale = 0.0;
	}
};