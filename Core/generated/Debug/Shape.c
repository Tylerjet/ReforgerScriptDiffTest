/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Debug
\{
*/

//! Instance of created debug visualizer.
class Shape: Managed
{
	//!don't call destructor directly!
	proto void ~Shape();
	private void Shape();

	static proto ref Shape Create(ShapeType type, int color, ShapeFlags flags, vector p1, vector p2);
	static proto ref Shape CreateLines(int color, ShapeFlags flags, vector p[], int num);
	static proto ref Shape CreateTris(int color, ShapeFlags flags, vector p[], int num);
	static proto ref Shape CreateSphere(int color, ShapeFlags flags, vector origin, float radius);
	static proto ref Shape CreateCylinder(int color, ShapeFlags flags, vector origin, float radius, float length);
	static proto ref Shape CreateArrow(vector from, vector to, float size, int color, ShapeFlags flags);
	proto external void SetMatrix(vector mat[4]);
	proto external void GetMatrix(out vector mat[4]);
	proto external void SetColor(int color);
	proto external void SetFlags(ShapeFlags flags);
}

/*!
\}
*/
