#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "Objects/Line.h"
#include "Objects/Sphere.h"
#include "Objects/Frustum.h"
//#include "bakkesmod/wrappers/spectatorhudwrapper.h"
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
	DOT,
	BRUSH
};

struct CarHitBallParams {
	uintptr_t ball;
	Vector HitLocation;
	Vector HitNormal;
};
class RL_Paint : public BakkesMod::Plugin::BakkesModPlugin
{
public:
	bool enabled = true;
	void onLoad() override;
	void onUnload() override;
	void LoadHooks();
	void LoadCvars();
	void Render(CanvasWrapper canvas);

	void ClearPoints();	
	void NewBallHitPos(Vector hitLocation);
	void DeleteTrailing(int max);
	void NewFlipResetPos();
	void GetParams(void* p, int n);
	//Vector RotatePointWithCar2(Vector offset, Vector carLocation, Rotator carRotation);

	bool IsValidGameState();
	bool HasResetIntervalElapsed();
	void AddDrawPoint(int startPoint, int mode);
	CarWrapper GetCar();

	void Freeze(Vector v);

	std::vector<Vector> points;
	std::vector<Vector> points_flipreset;
	std::vector<Vector> points_ballhit;

	std::vector<Rotator> points_rotation;
	std::string replaycar_name;

	int points_max = 120; //134 points is a circle
	int mode = TRAILING;
	bool relative = true;
	bool show_ballhits = false;
	bool show_flipreset = false;
	bool show_stickDirection = false;

	int start_point = 80;
	bool visualize_start_point = false;
	int startpoint_mode = PIN;


private:
	void Log(std::string msg);
	float timestamp = 0;
	float resetTime = 2;
};