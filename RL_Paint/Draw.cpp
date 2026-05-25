#include "Draw.h"


void Draw::Line(Vector v1, Vector v2, CarWrapper car, bool relativeToCar, CanvasWrapper canvas, RT::Frustum frust)
{
    if (relativeToCar) {
        if (!car) return;
        RT::Line(v1 + car.GetLocation(), v2 + car.GetLocation()).DrawWithinFrustum(canvas, frust);
    }
    else {
        RT::Line(v1, v2).DrawWithinFrustum(canvas, frust);
    }

}


void Draw::BallHit(Vector p, CanvasWrapper canvas, RT::Frustum frust) {
    Vector up = Vector(p.X, p.Y, p.Z + 5);
    Vector down = Vector(p.X, p.Y, p.Z - 5);
    RT::Line(up, down).DrawWithinFrustum(canvas, frust);
}
void Draw::FlipReset(Vector p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation) {
    RT::Sphere(p, 50).Draw(canvas, frust, cameraLocation, 10);
}

void Draw::StartPoint(int p, CarWrapper car, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation) {
    Vector v1 = RotatePointWithCar(Vector((float)p, 0, 0), car.GetLocation(), car.GetRotation());
    Vector v2 = RotatePointWithCar(Vector((float)p, 0, -10), car.GetLocation(), car.GetRotation());
    RT::Sphere(v1, 2).Draw(canvas, frust, cameraLocation, 10);
    RT::Line(v1, v2).DrawWithinFrustum(canvas, frust);
}

Vector Draw::RotatePointWithCar(Vector offset, Vector carLocation, Rotator carRotation) // should probably not be in this file
{
    Quat q = RotatorToQuat(carRotation);
    return RotateVectorWithQuat(offset, q) + carLocation;
}