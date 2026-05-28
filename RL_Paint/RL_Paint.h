#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Objects/Line.h"
#include "Objects/Sphere.h"
#include "Objects/Frustum.h"

enum modes
{
	TRAILING,
	NEVER,
	RESET,
	WALL
};
enum startpoint_modes
{
	PIN,
	LINESPHERE,
	DOT
};

struct CarHitBallParams {
	uintptr_t ball;
	Vector HitLocation;
	Vector HitNormal;
};
class RL_Paint : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	
	virtual void onLoad();
	virtual void onUnload();
	void LoadHooks();
	void Render(CanvasWrapper canvas);

	void CalculatePairs();
	void ClearPoints();	
	void NewBallHitPos(Vector hitLocation);
	void DeleteTrailing(int max);
	void NewFlipResetPos();
	void GetParams(void* p, int n);
	//Vector RotatePointWithCar2(Vector offset, Vector carLocation, Rotator carRotation);

	bool IsValidGameState();
	bool HasResetIntervalElapsed();
	void AddDrawPoint(int startPoint, int mode);
	void Freeze(Vector v);

	std::vector<std::pair<Vector, Vector>> pairs;
	std::vector<Vector> points;
	std::vector<Vector> points_flipreset;
	std::vector<Vector> points_ballhit;

	std::vector<Rotator> points_rotation;


	std::shared_ptr<bool> enabled;

	std::shared_ptr<int> points_max; //134 points is a circle
	std::shared_ptr<int> mode;
	std::shared_ptr<bool> relative;
	std::shared_ptr<bool> show_ballhits;
	std::shared_ptr<bool> show_flipreset;


	std::shared_ptr<int> start_point;
	std::shared_ptr<bool> visualize_start_point;
	std::shared_ptr<int> startpoint_mode;


private:
	void Log(std::string msg);
	float timestamp = 0;
	float resetTime = 2;
};