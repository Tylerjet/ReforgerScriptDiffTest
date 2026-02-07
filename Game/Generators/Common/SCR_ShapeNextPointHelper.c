//! This helper provides the next point in shape's relative position
//! - anchor points = points that define the shape (the ones draggable in Workbench)
//! - tesselated points = all points of the shape (equals to anchor points for PolylineShapeEntity)
//! - section = anchor to anchor - 1 tesselated points; [anchorA, tessPoint, tessPoint], [anchorB, tessPoint, tessPoint], etc
//! - segment = straight line between two tesselated points
//! - section point index = pointJ <= position < pointK = pointJ

//	TODO:
//		- in shape's length (straightLine parameter)
//		- done length
//		- define section lengths properly
//			- sectionLengths[0] = point0 → point1
//			- sectionLengths[1] = point1 → point2

class SCR_ShapeNextPointHelper
{
	protected ref array<vector> m_aAnchorPoints;
	protected ref array<ref array<vector>> m_aSegments;
	protected ref array<ref array<vector>> m_aSections;
	protected ref array<ref array<float>> m_aSectionLengths;
	protected int m_iSegmentsCount;
	protected int m_iAnchorsCount;

	protected int m_iCurrentSectionIndex;
	protected int m_iCurrentSectionPointIndex;
	protected int m_iCurrentSegmentIndex;
	protected vector m_vCurrentPosition;
	protected float m_fTotalLength;

	//------------------------------------------------------------------------------------------------
	float GetShapeLength()
	{
		return m_fTotalLength;
	}

	//------------------------------------------------------------------------------------------------
	// ...later
//	float GetDoneShapeLength()
//	{
//		return m_fDoneLength;
//	}

	//------------------------------------------------------------------------------------------------
	// ...later
//	float GetRemainingShapeLength()
//	{
//		return m_fTotalLength - m_fDoneLength;
//	}

	//------------------------------------------------------------------------------------------------
	//! Get the next point from the current position, but without going over the provided anchor index
	//! \param[in] distance
	//! \param[in] result
	//! \param[in] anchorLimit 0-based anchor index before which the point must be found; -1 for no limit
	//! \param[in] straightLine [NOT IMPLEMENTED, straight line only] if true, find in straight line distance; if false, finds in shape length (following the shape)
	//! \param[in] doNotMove if true, get the next point without internally moving (next Get will start from the previous position)
	//! \return true on success, false on failure
	bool GetNextPoint(float distance, out vector result, int anchorLimit = -1, bool straightLine = true, bool doNotMove = false)
	{
		if (distance <= 0)
		{
			Print("SCR_ShapeNextPointHelper.GetNextPointBeforeAnchor's distance parameter cannot be zero or negative!", LogLevel.ERROR);
			return false; // wrong distance
		}

		if (anchorLimit == -1)
			anchorLimit = m_iAnchorsCount - 1;

		if (anchorLimit < 1 || anchorLimit >= m_iAnchorsCount)
		{
			Print("SCR_ShapeNextPointHelper.GetNextPointBeforeAnchor's anchorLimit parameter is invalid (must be in range 1.." + m_iAnchorsCount + ", was " + anchorLimit + ")", LogLevel.ERROR);
			return false;
		}

		if (anchorLimit <= m_iCurrentSectionIndex)
		{
			Print("SCR_ShapeNextPointHelper.GetNextPointBeforeAnchor's anchorLimit parameter is already passed (" + anchorLimit + " requested, " + m_iCurrentSectionIndex + " current)", LogLevel.WARNING);
			return false;
		}

		float distanceSq = distance * distance;

		int currentSectionIndex = m_iCurrentSectionIndex;
		int currentSectionPoint = m_iCurrentSectionPointIndex + 1;
//		float addedLength;

		if (!m_aSections[currentSectionIndex].IsIndexValid(currentSectionPoint))
		{
			currentSectionIndex++;
			currentSectionPoint = 1; // because end point of previous section = first point of next section
		}

		vector prevPoint = m_aSections[m_iCurrentSectionIndex][m_iCurrentSectionPointIndex];

		array<vector> sectionPoints;
//		array<float> sectionSegmentsLength;
		for (; currentSectionIndex < anchorLimit; currentSectionIndex++)
		{
			sectionPoints = m_aSections[currentSectionIndex];
//			sectionSegmentsLength = m_aSectionLengths[currentSectionIndex];

			for (int pointIndex = currentSectionPoint, sectionPointsCount = sectionPoints.Count(); pointIndex < sectionPointsCount; pointIndex++)
			{
				vector nextPoint = sectionPoints[pointIndex];
//				if (straightLine)
//				{
					if (vector.DistanceSq(m_vCurrentPosition, nextPoint) > distanceSq)
					{
						vector endToStartDirNormalised = vector.Direction(nextPoint, prevPoint).Normalized();
						result = nextPoint + endToStartDirNormalised * Math3D.IntersectionRaySphere(nextPoint, endToStartDirNormalised, m_vCurrentPosition, distance);

						if (!doNotMove)
						{
							m_vCurrentPosition = result;
							m_iCurrentSectionIndex = currentSectionIndex;
							m_iCurrentSectionPointIndex = pointIndex - 1;
//							m_fDoneLength += addedLength + vector.Distance(prevPoint, result);
						}

						return true;
					}
//				}
//				else
//				{
//					if (addedLength + sectionSegmentsLength[pointIndex] > distance)
//					{
//						float remainder = distance - addedLength;
//						vector prevToNextDirNormalised = vector.Direction(prevPoint, nextPoint).Normalized();
//						result = prevPoint + prevToNextDirNormalised * remainder;
//
//						if (!doNotMove)
//						{
//							m_vCurrentPosition = result;
//							m_iCurrentSectionIndex = currentSectionIndex;
//							m_iCurrentSectionPointIndex = pointIndex - 1;
//							m_fDoneLength += distance;
//						}
//
//						return true;
//					}
//				}

//				addedLength += sectionSegmentsLength[pointIndex];
				prevPoint = nextPoint;
			}
		}

		return false; // shape end
	}

//	//------------------------------------------------------------------------------------------------
//	//! \return shape distance (following curve) from the current point to the next anchor
//	protected float GetShapeDistanceToNextAnchor()
//	{
//		float result = vector.Distance(m_vCurrentPosition, m_aSegments[m_iCurrentSegmentIndex][1]);
//		if (result == 0)
//			return 0;
//
//		array<vector> segment;
//		for (int i = m_iCurrentSegmentIndex + 1; i < m_iSegmentsCount; i++)
//		{
//			segment = m_aSegments[i];
//			result += vector.Distance(segment[0], segment[1]);
//			if (m_aAnchorPoints.Contains(segment[1]))
//				return result;
//		}
//
//		return result;
//	}

//	//------------------------------------------------------------------------------------------------
//	//! \return straight distance (not following curve) from the current point to the next anchor, -1 on error
//	float GetStraightDistanceToNextAnchor()
//	{
//		Print("Not Implemented " + __FILE__ + "@" + __LINE__, LogLevel.WARNING);
//		return -1;
//	}

	//------------------------------------------------------------------------------------------------
	//! Sets to the provided anchor's position
	//! \param[in] anchorIndex the index of the anchor - while useless, it is possible to set it to the last point
	//! \return true on success, false on failure
	bool SetOnAnchor(int anchorIndex)
	{
		if (anchorIndex < 0 || anchorIndex >= m_iAnchorsCount)
			return false;

		m_vCurrentPosition = m_aAnchorPoints[anchorIndex];
		m_iCurrentSectionIndex = anchorIndex;
		m_iCurrentSectionPointIndex = 0;

//		m_fDoneLength = 0;
//		foreach (int i, array<float> segmentLengths : m_aSectionLengths)
//		{
//			if (i >= anchorIndex)
//				break;
//
//			foreach (float segmentLength : segmentLengths)
//			{
//				m_fDoneLength += segmentLength;
//			}
//
//			// TODO: add segment start to result
//		}

		return true;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Sets to the provided position IF on the shape
//	//! \param[in] position the wanted (relative) position
//	//! \param[in] segmentIndex the position's exact segmentIndex, if known (for performance only; otherwise any negative value will do)
//	//! \return true on success, false on failure
//	bool SetPos(vector position)
//	{
//		foreach (int segmentIndex, array<vector> segment : m_aSegments)
//		{
//			if (
//				(segmentIndex == m_iSegmentsCount - 1 || position != segment[1]) &&				// on start point, not on end
//				Math3D.PointLineSegmentDistanceSqr(position, segment[0], segment[1]) < 0.001)	// on segment
//			{
//				m_vCurrentPosition = position;
//				m_iCurrentSegmentIndex = segmentIndex;
//				return true;
//			}
//		}
//
//		return false;
//	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the provided shape is valid (more than one point and with some length), false otherwise
	bool IsValid()
	{
		return m_iAnchorsCount > 1 && m_fTotalLength > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset to the beginning
	void Reset()
	{
		if (m_iAnchorsCount > 0)
			SetOnAnchor(0);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] shapeEntity the shape to analyse (Polyline or Spline)
	void SCR_ShapeNextPointHelper(notnull ShapeEntity shapeEntity)
	{
		m_aAnchorPoints = {};
		m_aSections = {};
		m_aSectionLengths = {};
		array<vector> anchorSection;
		array<float> sectionSegmentLengths;
		array<vector> tesselatedPoints = {};
		shapeEntity.GetPointsPositions(m_aAnchorPoints);
		shapeEntity.GenerateTesselatedShape(tesselatedPoints);
		m_aSegments = {};
		int anchorIndex = 0;

		if (!m_aAnchorPoints.IsEmpty())
			m_vCurrentPosition = m_aAnchorPoints[0];

		vector prevPoint;
		foreach (int i, vector currPoint : tesselatedPoints)
		{
			float segmentLength = vector.Distance(currPoint, prevPoint);

			if (i > 0)										// normal point or anchor point
			{
				anchorSection.Insert(currPoint);
				sectionSegmentLengths.Insert(segmentLength);
				m_aSegments.Insert({ prevPoint, currPoint });
			}

			if (currPoint == m_aAnchorPoints[anchorIndex])	// anchor point (including first and last ones)
			{
				if (i > 0)
				{
					m_aSections.Insert(anchorSection);
					sectionSegmentLengths.Insert(segmentLength);
					m_aSectionLengths.Insert(sectionSegmentLengths);
				}

				anchorSection = { currPoint };
				sectionSegmentLengths = {};

				anchorIndex++; // if last point, foreach ends and m_aAnchorPoints[maxIndex + 1] never occurs
			}

			m_fTotalLength += segmentLength;

			prevPoint = currPoint;
		}

		m_iAnchorsCount = m_aAnchorPoints.Count();
		m_iSegmentsCount = m_aSegments.Count();
	}
}
