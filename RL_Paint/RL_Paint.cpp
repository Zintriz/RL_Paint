#include "RL_Paint.h"
#include "Draw.h"

BAKKESMOD_PLUGIN(RL_Paint, "RL_Paint", "0.0.5", PLUGINTYPE_FREEPLAY)
LinearColor colors(255, 255, 0, 255);

void RL_Paint::onLoad()
{
    this->Log("RL_Paint Loaded");

    enabled = std::make_shared<bool>(true);
    cvarManager->registerCvar("paint_enabled", "1", "Enable/Disable RL_Paint")
        .bindTo(enabled);
    cvarManager->getCvar("paint_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        bool disabled = !cvar.getBoolValue();
        if (disabled) {
            cvarManager->getCvar("sv_soccar_gravity").setValue(-650);
        }
        });

    show_ballhits = std::make_shared<bool>(false);
    cvarManager->registerCvar("paint_ballhits", "0", "Enable/Disable Ballhit Markers")
        .bindTo(show_ballhits);
    cvarManager->getCvar("paint_ballhits").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        bool disabled = !cvar.getBoolValue();
        if (disabled) {
            points_ballhit.clear();
        }
        });
    show_flipreset = std::make_shared<bool>(false);
    cvarManager->registerCvar("paint_flipreset", "0", "Enable/Disable flipreset Markers")
        .bindTo(show_flipreset);
    cvarManager->getCvar("paint_flipreset").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        bool disabled = !cvar.getBoolValue();
        if (disabled) {
            points_flipreset.clear();
        }
        });
    show_stickDirection = std::make_shared<bool>(false);
    cvarManager->registerCvar("paint_stickDirection", "0", "Enable/Disable Stick direction")
        .bindTo(show_stickDirection);

    points_max = std::make_shared<int>(120);
    cvarManager->registerCvar("paint_points", "120", "Painted points before reset",true,true,20,true,9999)
        .bindTo(points_max);

    mode = std::make_shared<int>(0); 
    cvarManager->registerCvar("paint_mode", "0", "0 = trailning, 1 = on_reset, 2 = on_pointcap.")
        .bindTo(mode);
    cvarManager->getCvar("paint_mode").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        ClearPoints();
    });

    relative = std::make_shared<bool>(true);
    cvarManager->registerCvar("paint_relative", "1", "If true, dots line follows the car instead of worldspace")
        .bindTo(relative);
    cvarManager->getCvar("paint_relative").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        ClearPoints();
        });

    start_point = std::make_shared<int>(80);
    cvarManager->registerCvar("paint_start_point", "80", "The start point of the trail", true, true, -300, true, 300)
        .bindTo(start_point);

    visualize_start_point = std::make_shared<bool>(false);
    cvarManager->registerCvar("paint_visualize_start_point", "0", "Vizualize the start point")
        .bindTo(visualize_start_point);

    cvarManager->registerNotifier("paint_freeze", [this](std::vector<std::string> args) {
        Freeze(Vector(0, 0, 300));
        }, "", PERMISSION_ALL);

    startpoint_mode = std::make_shared<int>(0);
    cvarManager->registerCvar("paint_startpoint_mode", "0", "Type of startpoint-display")
        .bindTo(startpoint_mode);
    cvarManager->getCvar("paint_startpoint_mode").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        ClearPoints();
        });

    //cvarManager->executeCommand("cl_settings_refreshplugins", false);

    this->LoadHooks();
}
void RL_Paint::onUnload()
{
    this->Log("RL_Paint Unloaded");
}

void RL_Paint::LoadHooks()
{
    gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.OnHitBall",
        [this](CarWrapper caller, void* params, std::string eventname) {
            if (!enabled || !*enabled) return;
            if (!show_ballhits || !*show_ballhits) return;
            // This cast is only safe if you're 100% sure the params are correct
            CarHitBallParams* hitParams = (CarHitBallParams*)params;
            BallWrapper ballHit = BallWrapper(hitParams->ball);
            Vector v = Vector(hitParams->HitLocation);
            this->NewBallHitPos(v);
        });

    gameWrapper->HookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep",
        [this](std::string eventName) {
            if (!enabled || !*enabled) return;
            if (!start_point || !mode) return;
            this->AddDrawPoint(*start_point, *mode);
        });
    gameWrapper->HookEvent("Function TAGame.Car_TA.EventPerformedFlipReset",
        [this](std::string eventName) {
            if (!enabled || !*enabled) return;
            this->NewFlipResetPos();
        });

    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed",
        [this](std::string eventName) {
            if (!enabled || !*enabled) return;
            this->ClearPoints();
        });
    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Countdown.BeginState",
        [this](std::string eventName) {
            if (!enabled || !*enabled) return;
            this->ClearPoints();
        });
    gameWrapper->RegisterDrawable(
        [this](CanvasWrapper canvas) {
            if (!enabled || !*enabled) return;
            Render(canvas);
        });


}

void RL_Paint::Log(std::string msg)
{
    cvarManager->log(msg);
}
void RL_Paint::GetParams(void* p, int n) {
    for (size_t i = 0; i < n; i++) {
        uintptr_t param = *(uintptr_t*)((uint8_t*)p + i*8);
        this->Log(std::format("param{} {:x}",i, param));
    }
}
void RL_Paint::ClearPoints() {
    this->Log("Clearing Points");
    points.clear();
    points_flipreset.clear();
    points_ballhit.clear();
}
bool RL_Paint::IsValidGameState()
{
    int ingame = (gameWrapper->IsInGame()) ? 1 : (gameWrapper->IsInReplay()) ? 2 : 0;
    if (!ingame) {
        return false;
    }
    ServerWrapper game = (ingame == 1) ? gameWrapper->GetGameEventAsServer() : gameWrapper->GetGameEventAsReplay();
    if (game.IsNull()) {
        return false;
    }
    if (gameWrapper->IsPaused()) return false;
    return true;
}


void RL_Paint::Render(CanvasWrapper canvas) {
    if (!IsValidGameState())
        return;
    canvas.SetColor(colors);
    CameraWrapper camera = gameWrapper->GetCamera();
    if (camera.IsNull()) return;
    Vector cameraLocation = camera.GetLocation();
    RT::Frustum frust{ canvas, camera };
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car || !relative || !&canvas ||  !&camera)
        return; // check pointers

    Draw frame = Draw(car, &canvas, &camera);
    if (points.size() > 1) {
        for (size_t i = 0; i < points.size()-1; i++) {
            frame.Line(points[i], points[i+1], *relative);
        }
    }
    for (Vector p : points_ballhit) {
        frame.BallHit(p);
    }
    for (Vector fr : points_flipreset) {
        frame.FlipReset(fr);
    }
    if (*show_stickDirection) {
        frame.FlipDirection();
    }
    if (!visualize_start_point || !start_point) return;
    if (!*visualize_start_point) return;
    switch (*startpoint_mode) {
        case PIN:
            frame.StartPoint1(*start_point);
            break;
        case LINESPHERE:
            frame.StartPoint2(*start_point);
            break;
        case DOT:
            frame.StartPoint3(*start_point);
            break;
        case BRUSH:
            frame.StartPoint4(*start_point);
            break;
        default:
            frame.StartPoint1(*start_point);
            break;
    }
    
}

void RL_Paint::DeleteTrailing(int max) {
    if ((int)points.size() > max+1) {
        this->Log("reset");
        points.clear();
        return;
    }

    if (points.size() > max) {
        points.erase(points.begin());
    }
}

void RL_Paint::AddDrawPoint(int startPoint, int mode) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car || !relative || !points_max) return;
    Vector v = *relative ? 0 : car.GetLocation();
    Rotator r = car.GetRotation();
    Vector rp = Draw::RotatePointWithCar(Vector((float)startPoint, 0, 0), v, r);
    switch (mode) {
        case TRAILING:
            DeleteTrailing(*points_max);
            points.push_back(rp);
            break;
        case NEVER:
                DeleteTrailing(9999);
                points.push_back(rp);
            break;
        case RESET:
            if ((int)points.size() > *points_max) {
                this->ClearPoints();
            }
            points.push_back(rp);
            break;
        case WALL:
            if (car.IsOnWall()) {
                this->ClearPoints();
            }
            if (!car.IsOnGround()) {
                points.push_back(rp);
            }
            break;
        default:
            DeleteTrailing(*points_max);
            points.push_back(rp);
    };
}

void RL_Paint::NewBallHitPos(Vector hitLocation) {
    if (!show_ballhits || !*show_ballhits) return;
    points_ballhit.push_back(hitLocation);
}
void RL_Paint::NewFlipResetPos() {
    if (!show_flipreset || !*show_flipreset) return;
    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (!game) return;
    Vector ballpos = game.GetBall().GetTrajectoryStartLocation();
    points_flipreset.push_back(ballpos);

}
void RL_Paint::Freeze(Vector v) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    cvarManager->getCvar("sv_soccar_gravity").setValue(-1.0f);

    car.SetLocation(v);
    car.SetVelocity(0);
    car.SetRotation(0);
}