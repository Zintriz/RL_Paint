#include "Draw.h"

void Draw::Line(Vector v1, Vector v2, bool relativeToCar)
{
    if (relativeToCar) {
        RT::Line(v1 + carLocation, v2 + carLocation).DrawWithinFrustum(*canvas, frust);
    }
    else {
        RT::Line(v1, v2).DrawWithinFrustum(*canvas, frust);
    }
}

void Draw::BallHit(Vector p) {
    Vector up = Vector(p.X, p.Y, p.Z + 5);
    Vector down = Vector(p.X, p.Y, p.Z - 5);
    RT::Line(up, down).DrawWithinFrustum(*canvas, frust);
}
void Draw::FlipReset(Vector p) {
    RT::Sphere(p, 50).Draw(*canvas, frust, cameraLocation, 10);
}

void Draw::StartPoint1(int p) {
    Vector v1 = RotatePointWithCar(Vector((float)p, 0, 0), carLocation, carRotation);
    Vector v2 = RotatePointWithCar(Vector((float)p, 0, -10), carLocation, carRotation);
    RT::Sphere(v1, 2).Draw(*canvas, frust, cameraLocation, 10);
    RT::Line(v1, v2).DrawWithinFrustum(*canvas, frust);
}
void Draw::StartPoint2(int p) {
    Vector v = RotatePointWithCar(Vector((float)p, 0, 0), carLocation, carRotation);
    RT::Sphere(v, 5).Draw(*canvas, frust, cameraLocation, 10);
    Vector up = v + Vector(0, 0, 15);
    Vector down = v + Vector(0, 0, -15);
    RT::Line(up, down).DrawWithinFrustum(*canvas, frust);
}
void Draw::StartPoint3(int p)
{
    Vector v = RotatePointWithCar(Vector((float)p, 0, 0), carLocation, carRotation);
    RT::Sphere(v, 2).Draw(*canvas, frust, cameraLocation, 8);
}
void Draw::StartPoint4(int p)
{
    Vector v = RotatePointWithCar(Vector((float)p, 0, 0), carLocation, carRotation);
    RT::Line(carLocation, v).DrawWithinFrustum(*canvas, frust);
}

void Draw::FlipDirection() {
//  Vector v = RotatePointWithCar(Vector(80, 0, 0), carLocation, Rotator(0,carRotation.Yaw,0));

    Vector c = Vector(-inputPitch * 80, inputSteer * 80, 0);
    c = RotatePointWithCar(c);

    RT::Line(carLocation, c).DrawWithinFrustum(*canvas, frust);
}

Vector Draw::RotatePointWithCar(Vector offset, Vector carLocation, Rotator carRotation) // should probably not be in this file
{
    Quat q = RotatorToQuat(carRotation);
    return RotateVectorWithQuat(offset, q) + carLocation;
}
Vector Draw::RotatePointWithCar(Vector offset) // should probably not be in this file
{
    return RotateVectorWithQuat(offset, carRotationQuat) + carLocation;
}