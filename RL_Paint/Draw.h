#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Objects/Line.h"
#include "Objects/Sphere.h"
#include "Objects/Frustum.h"

class Draw
{
public:
	static void Line(Vector v1, Vector v2, CarWrapper car, bool relativeToCar, CanvasWrapper canvas, RT::Frustum frust);
	static void BallHit(Vector p, CanvasWrapper canvas, RT::Frustum frust);
	static void FlipReset(Vector p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation);
	static void StartPoint1(int p, CarWrapper car, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation);
	static void StartPoint2(int p, CarWrapper car, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation);
	static void StartPoint3(int p, CarWrapper car, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation);

	static Vector RotatePointWithCar(Vector offset, Vector carLocation, Rotator carRotation);

private: 

};