#include "RL_Paint.h"


BAKKESMOD_PLUGIN(RL_Paint, "RL_Paint", "0.0.2.2", PLUGINTYPE_FREEPLAY)

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
        .bindTo(enabled);
    cvarManager->getCvar("paint_ballhits").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        bool disabled = !cvar.getBoolValue();
        if (disabled) {
            points_ballhit.clear();
        }
        });
    show_flipreset = std::make_shared<bool>(false);
    cvarManager->registerCvar("paint_flipreset", "0", "Enable/Disable flipreset Markers")
        .bindTo(enabled);
    cvarManager->getCvar("paint_flipreset").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        bool disabled = !cvar.getBoolValue();
        if (disabled) {
            points_flipreset.clear();
        }
        });

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

    
    //cvarManager->getCvar("paint_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
    //    //addOnValueChanged is a callback, where everything in this block will be called whenever the cvar value is changed. You can call functions, or just log information.
    //    if (cvar.getStringValue() == "1") {
    //        *enabled = true;
    //    };
    //    if (cvar.getStringValue() == "0") {
    //        *enabled = false;
    //    };
    //    }
    //);
    //cvarManager->getCvar("plugin_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {});

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
            // This cast is only safe if you're 100% sure the params are correct
            CarHitBallParams* hitParams = (CarHitBallParams*)params;
            BallWrapper ballHit = BallWrapper(hitParams->ball);
            Vector v = Vector(hitParams->HitLocation);
            this->BallHit(v);
            // Now you know what ball was hit!
        });

    gameWrapper->HookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep",
        [this](std::string eventName) {
            if (!enabled || !*enabled) return;
            this->GetPointInFront(*start_point, *mode);
        });
    gameWrapper->HookEvent("Function TAGame.Car_TA.EventPerformedFlipReset",
        [this](std::string eventName) {
            if (!enabled || !*enabled) return;
            this->FlipReset();
        });
    //gameWrapper->HookEvent("Function TAGame.Car_TA.OnHitBall",
    //    [this](std::string eventName) {
    //        if (!enabled || !*show_ballhits) return;
    //        this->BallHit();
    //    });
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
void RL_Paint::CalculatePairs() {
    pairs.clear();
    for (size_t i = 0; i < points.size() - 1; i++) {
        pairs.push_back({ points[i], points[i + 1] });
    }
}
void RL_Paint::ClearPoints() {
    this->Log("Clearing Points");
    points.clear();
    points_flipreset.clear();
    points_ballhit.clear();
    pairs.clear();
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

    auto camera = gameWrapper->GetCamera();
    if (camera.IsNull()) return;
    RT::Frustum frust{ canvas, camera };
    if (pairs.empty()) return;
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    for (auto p : pairs) {
        DrawLine(p.first, p.second, *relative, canvas, frust);
    }
    for (Vector p : points_ballhit) {
        DrawBallHit(p, canvas, frust);
    }
    for (Vector fr : points_flipreset) {
        DrawFlipReset(fr, canvas, frust, camera.GetLocation());
    }
    if (visualize_start_point && start_point && *visualize_start_point) {
        DrawStartPoint(*start_point, canvas, frust, camera.GetLocation());
    }
}
void RL_Paint::DrawBallHit(Vector p, CanvasWrapper canvas, RT::Frustum frust) {
    Vector up = Vector(p.X, p.Y, p.Z + 5);
    Vector down = Vector(p.X, p.Y, p.Z - 5);
    RT::Line(up, down).DrawWithinFrustum(canvas, frust);
}
void RL_Paint::DrawLine(Vector v1, Vector v2, bool relativeToCar, CanvasWrapper canvas, RT::Frustum frust)
{
    if (relativeToCar) {
        CarWrapper car = gameWrapper->GetLocalCar();
        if (!car) return;
        RT::Line(v1+car.GetLocation(), v2+car.GetLocation()).DrawWithinFrustum(canvas, frust);
    } else {
        RT::Line(v1, v2).DrawWithinFrustum(canvas, frust);
    }
    
}
void RL_Paint::DrawFlipReset(Vector p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation) {
    RT::Sphere(p, 50).Draw(canvas, frust, cameraLocation, 10);
}

void RL_Paint::DrawStartPoint(int p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    Vector v1 = RotatePointWithCar(Vector((float) p, 0, 0), car.GetLocation(), car.GetRotation());
    Vector v2 = RotatePointWithCar(Vector((float)p, 0, -10), car.GetLocation(), car.GetRotation());
    RT::Sphere(v1, 2).Draw(canvas, frust, cameraLocation, 10);
    RT::Line(v1, v2).DrawWithinFrustum(canvas, frust);
}

bool RL_Paint::HasResetIntervalElapsed() {
    float now = gameWrapper->GetEngine().GetPhysicsTime();

    bool elapsed = (now - timestamp) > resetTime;
    //this->Log(std::format("ct:{}, stamp:{}", now, timestamp));
    timestamp = now;
    return elapsed;
}
void RL_Paint::DeleteTrailing() {
    //BUG does not remove excess points over max fast enough
    if (points.size() > *points_max) {
        points.erase(points.begin());
    }
}
void RL_Paint::AddPoint(Vector p) {
    points.push_back(p);
}

void RL_Paint::GetPointInFront(int startPoint, int mode) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    Vector v = *relative ? 0 : car.GetLocation();
    Rotator r = car.GetRotation();
    Vector rp = this->RotatePointWithCar(Vector((float)startPoint, 0, 0), v, r);
    switch (mode) {
        case TRAILING:
            DeleteTrailing();
            break;
        case ON_RESET:
            break;
        case ON_POINTRESET:
            if ((int)points.size() > *points_max) {
                this->ClearPoints();
            }
            break;
        default:
            DeleteTrailing();
    };
    this->AddPoint(rp);
    this->CalculatePairs();
}


void RL_Paint::BallHit(Vector HitLocation) {
    //this->Log("Ballhit Start");
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    points_ballhit.push_back(HitLocation);

    //this->Log("Ballhit End");
}
void RL_Paint::FlipReset() {
    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (!game) return;
    ArrayWrapper<BallWrapper> balls = game.GetGameBalls();
    for (BallWrapper ball : balls) {
        points_flipreset.push_back(ball.GetTrajectoryStartLocation());
    }
}
void RL_Paint::Freeze(Vector v) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    cvarManager->getCvar("sv_soccar_gravity").setValue(-1.0f);

    car.SetLocation(v);
    car.SetVelocity(0);
    car.SetRotation(0);
}


//Vector RL_Paint::RotatePointWithCar2(Vector offset,Vector carLocation, Rotator carRotation) // direct copy of hitbox plugin
//{
//    // offset is from middle of the car on the car so Vector(80,0,0) would be forwards from the car
//    double dPitch = (double)carRotation.Pitch / 32768.0 * 3.14159;
//    double dYaw = (double)carRotation.Yaw / 32768.0 * 3.14159;
//    double dRoll = (double)carRotation.Roll / 32768.0 * 3.14159;
//
//    float sx = sin(dRoll);
//    float cx = cos(dRoll);
//    float sy = sin(-dYaw);
//    float cy = cos(-dYaw);
//    float sz = sin(dPitch);
//    float cz = cos(dPitch);
//    offset = Vector(offset.X, offset.Y * cx - offset.Z * sx, offset.Y * sx + offset.Z * cx);
//    offset = Vector(offset.X * cz - offset.Y * sz, offset.X * sz + offset.Y * cz, offset.Z);
//    offset = Vector(offset.X * cy + offset.Z * sy, offset.Y, -offset.X * sy + offset.Z * cy);
//    float tmp = offset.Z;
//    offset.Z = offset.Y;
//    offset.Y = tmp;
//    return offset + carLocation;
//    //Vector rotatedVector = this->Rotate(offset, dRoll, -dYaw, dPitch) + carLocation;//
//
//}


Vector RL_Paint::RotatePointWithCar(Vector offset, Vector carLocation, Rotator carRotation)
{
    Quat q = RotatorToQuat(carRotation);
    return RotateVectorWithQuat(offset, q)+carLocation;
}
