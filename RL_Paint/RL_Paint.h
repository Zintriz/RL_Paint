#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Objects/Line.h"
#include "Objects/Sphere.h"
#include "Objects/Frustum.h"

enum modes
{
	TRAILING,
	ON_RESET,
	ON_POINTRESET,
};
class RL_Paint : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	
	virtual void onLoad();
	virtual void onUnload();
	void LoadHooks();
	void Render(CanvasWrapper canvas);
	void DrawBallHit(Vector p, CanvasWrapper canvas, RT::Frustum frust);
	void DrawFlipReset(Vector p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation);
	void DrawStartPoint(int p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation);
	void CalculatePairs();
	void ClearPoints();	
	void BallHit();
	void AddPoints(Vector p);
	void DeleteTrailing();
	void FlipReset();
	Vector RotatePointWithCar(Vector offset, Vector carLocation, Rotator carRotation);
	bool IsValidGameState();
	bool HasResetIntervalElapsed();
	void GetPointInFront(int startPoint, int mode);
	void Freeze(Vector v);


	std::vector<std::pair<Vector, Vector>> pairs;
	std::vector<Vector> points;
	std::vector<Vector> points_flipreset;
	std::vector<Vector> points_ballhit;



	std::shared_ptr<bool> enabled;
	std::shared_ptr<int> points_max; //134 points is a circle
	std::shared_ptr<int> mode;

	std::shared_ptr<int> start_point;
	std::shared_ptr<bool> visualize_start_point;


private:
	void Log(std::string msg);
	float timestamp = 0;
	float resetTime = 2;
};