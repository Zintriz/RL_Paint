#include "RL_Paint.h"
#include "Draw.h"

#define EVENT_EventPostPhysicsStep "Function TAGame.EngineShare_TA.EventPostPhysicsStep"
#define EVENT_OnHitBall "Function TAGame.Car_TA.OnHitBall"
#define EVENT_EventPerformedFlipReset "Function TAGame.Car_TA.EventPerformedFlipReset"
#define EVENT_Destroyed "Function TAGame.GameEvent_Soccar_TA.Destroyed"
#define EVENT_BeginState "Function GameEvent_Soccar_TA.Countdown.BeginState"



BAKKESMOD_PLUGIN(RL_Paint, "RL_Paint", "0.0.6", PLUGINTYPE_FREEPLAY)
LinearColor colors(255, 255, 0, 255);

void RL_Paint::onLoad()
{
    this->Log("RL_Paint Loaded");
    cvarManager->registerCvar("paint_enabled", "1", "Enable/Disable RL_Paint");
    cvarManager->getCvar("paint_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        enabled = cvar.getBoolValue();
        if (enabled) {
            LoadHooks();
            //LoadCvars();
        }
        else {
            onUnload();
            cvarManager->getCvar("sv_soccar_gravity").setValue(-650);
        }
        });
    LoadHooks();
    LoadCvars();
}
void RL_Paint::LoadCvars() {

    cvarManager->registerCvar("paint_ballhits", "0", "Enable/Disable Ballhit Markers").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            show_ballhits = cvar.getBoolValue();
            points_ballhit.clear();
        });
    cvarManager->registerCvar("paint_flipreset", "0", "Enable/Disable flipreset Markers").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            show_flipreset = cvar.getBoolValue();
            points_flipreset.clear();
        });

    cvarManager->registerCvar("paint_stickDirection", "0", "Enable/Disable Stick direction").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            show_stickDirection = cvar.getBoolValue();
        });

    cvarManager->registerCvar("paint_points", "120", "Painted points before reset", true, true, 20, true, 9999).addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            points_max = cvar.getIntValue();
        });

    cvarManager->registerCvar("paint_mode", "0", "0 = trailning, 1 = on_reset, 2 = on_pointcap.").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            mode = cvar.getIntValue();
            points.clear();
        });


    cvarManager->registerCvar("paint_relative", "1", "If true, dots line follows the car instead of worldspace").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            relative = cvar.getBoolValue();
            ClearPoints();
        });

    cvarManager->registerCvar("paint_start_point", "80", "The start point of the trail", true, true, -300, true, 300).addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            start_point = cvar.getIntValue();
        });

    cvarManager->registerCvar("paint_visualize_start_point", "0", "Vizualize the start point").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            visualize_start_point = cvar.getBoolValue();
        });

    cvarManager->registerCvar("paint_startpoint_mode", "0", "Type of startpoint-display").addOnValueChanged(
        [this](std::string oldValue, CVarWrapper cvar) {
            startpoint_mode = cvar.getIntValue();
        });
    cvarManager->registerNotifier("paint_freeze", [this](std::vector<std::string> args) {
        Freeze(Vector(0, 0, 300));
        }, "", PERMISSION_ALL);
    //cvarManager->executeCommand("cl_settings_refreshplugins", false);
}
void RL_Paint::onUnload()
{
    this->Log("RL_Paint Unloaded");
    gameWrapper->UnhookEvent(EVENT_BeginState);
    gameWrapper->UnhookEvent(EVENT_Destroyed);
    gameWrapper->UnhookEvent(EVENT_EventPerformedFlipReset);
    gameWrapper->UnhookEvent(EVENT_EventPostPhysicsStep);
    gameWrapper->UnhookEventPost(EVENT_OnHitBall);
    gameWrapper->UnregisterDrawables();

    //cvarManager->removeCvar("paint_enabled");
    //cvarManager->removeCvar("paint_ballhits");
    //cvarManager->removeCvar("paint_flipreset");
    //cvarManager->removeCvar("paint_stickDirection");
    //cvarManager->removeCvar("paint_points");
    //cvarManager->removeCvar("paint_mode");
    //cvarManager->removeCvar("paint_relative");
    //cvarManager->removeCvar("paint_start_point");
    //cvarManager->removeCvar("paint_visualize_start_point");
    //cvarManager->removeCvar("paint_startpoint_mode");

    //cvarManager->removeNotifier("paint_freeze");
    cvarManager->executeCommand("writeconfig", false);
    //cvarManager->executeCommand("cl_settings_refreshplugins", false);
}

void RL_Paint::LoadHooks()
{
    gameWrapper->HookEventWithCallerPost<CarWrapper>(EVENT_OnHitBall,
        [this](CarWrapper caller, void* params, std::string eventname) {
            if (!show_ballhits) return;
            // This cast is only safe if you're 100% sure the params are correct
            CarHitBallParams* hitParams = (CarHitBallParams*)params;
            BallWrapper ballHit = BallWrapper(hitParams->ball);
            Vector v = Vector(hitParams->HitLocation);
            this->NewBallHitPos(v);
        });

    gameWrapper->HookEvent(EVENT_EventPostPhysicsStep,
        [this](std::string eventName) {
            this->AddDrawPoint(start_point, mode);
        });
    gameWrapper->HookEvent(EVENT_EventPerformedFlipReset,
        [this](std::string eventName) {
            if (!show_flipreset) return;
            this->NewFlipResetPos();
        });

    gameWrapper->HookEvent(EVENT_Destroyed,
        [this](std::string eventName) {
            this->ClearPoints();
        });
    gameWrapper->HookEvent(EVENT_BeginState,
        [this](std::string eventName) {
            this->ClearPoints();
        });
    gameWrapper->RegisterDrawable(
        [this](CanvasWrapper canvas) {
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
    canvas.SetColor(colors);
    canvas.DrawString("RL_paint enabled");
    if (!IsValidGameState())
        return;
    CameraWrapper camera = gameWrapper->GetCamera();
    if (camera.IsNull()) return;
    Vector cameraLocation = camera.GetLocation();
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car || !&canvas ||  !&camera)
        return; // check pointers

    Draw frame = Draw(car, &canvas, &camera);
    if (points.size() > 1) {
        for (size_t i = 0; i < points.size()-1; i++) {
            frame.Line(points[i], points[i+1], relative);
        }
    }
    for (Vector p : points_ballhit) {
        frame.BallHit(p);
    }
    for (Vector fr : points_flipreset) {
        frame.FlipReset(fr);
    }
    if (show_stickDirection) {
        frame.FlipDirection();
    }
    if (visualize_start_point) {
        switch (startpoint_mode) {
            case PIN:
                frame.StartPoint1(start_point);
                break;
            case LINESPHERE:
                frame.StartPoint2(start_point);
                break;
            case DOT:
                frame.StartPoint3(start_point);
                break;
            case BRUSH:
                frame.StartPoint4(start_point);
                break;
            default:
                frame.StartPoint1(start_point);
                break;
        }
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
    if (!car) return;
    Vector v = relative ? 0 : car.GetLocation();
    Rotator r = car.GetRotation();
    Vector rp = Draw::RotatePointWithCar(Vector((float)startPoint, 0, 0), v, r);
    switch (mode) {
        case TRAILING:
            DeleteTrailing(points_max);
            points.push_back(rp);
            break;
        case NEVER:
                DeleteTrailing(9999);
                points.push_back(rp);
            break;
        case RESET:
            if ((int)points.size() > points_max) {
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
            DeleteTrailing(points_max);
            points.push_back(rp);
    };
}

void RL_Paint::NewBallHitPos(Vector hitLocation) {
    points_ballhit.push_back(hitLocation);
}
void RL_Paint::NewFlipResetPos() {
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
    car.SetAngularVelocity(Vector(0,0,0),false);
    car.SetRotation(0);
}