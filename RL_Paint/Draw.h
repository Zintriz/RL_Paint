#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Objects/Line.h"
#include "Objects/Sphere.h"
#include "Objects/Frustum.h"

class Draw
{
private:
	float inputSteer;
	float inputPitch;
	Vector carLocation;
	Rotator carRotation;
	Quat carRotationQuat;
	Vector cameraLocation;
	CanvasWrapper* canvas;
	CameraWrapper* camera;
	RT::Frustum frust;
public:
	Draw(CarWrapper car, CanvasWrapper* canvas, CameraWrapper* camera) {
		this->inputSteer = car.GetInput().Steer;
		this->inputPitch = car.GetInput().Pitch;
		this->carLocation = car.GetLocation();
		this->carRotation = car.GetRotation();
		this->carRotationQuat = RotatorToQuat(car.GetRotation());
		this->canvas = canvas;
		this->camera = camera;
		this->cameraLocation = camera->GetLocation();
		this->frust = RT::Frustum(*canvas, *camera);
	};
	Vector RotatePointWithCar(Vector offset);

	void Line(Vector v1, Vector v2, bool relativeToCar);
	void BallHit(Vector p);
	void FlipReset(Vector p);
	void FlipDirection();
	void StartPoint1(int p);
	void StartPoint2(int p);
	void StartPoint3(int p);
	void StartPoint4(int p);

	static Vector RotatePointWithCar(Vector offset, Vector carLocation, Rotator carRotation);

};