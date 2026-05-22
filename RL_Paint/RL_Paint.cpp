#include "RL_Paint.h"


BAKKESMOD_PLUGIN(RL_Paint, "RL_Paint", "0.0.1.1", PLUGINTYPE_FREEPLAY)

void RL_Paint::onLoad()
{
    this->Log("RL_Paint Loaded");

    enabled = std::make_shared<bool>(true);
    cvarManager->registerCvar("paint_enabled", "1", "Enable/Disable RL_Paint")
        .bindTo(enabled);

    points_max = std::make_shared<int>(120);
    cvarManager->registerCvar("paint_points", "120", "Painted points before reset",true,true,20,true,9999)
        .bindTo(points_max);

    trailing = std::make_shared<bool>(true); // will be replaced by mode
    cvarManager->registerCvar("paint_trailing", "1", "Trailing tail.")
        .bindTo(trailing);
    mode = std::make_shared<int>(0); 
    cvarManager->registerCvar("paint_mode", "0", "0 = trailning, 1 = on_reset, 2 = on_pointcap.")
        .bindTo(mode);
    cvarManager->getCvar("paint_mode").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
        ClearPoints();
    });

    start_point = std::make_shared<int>(74);
    cvarManager->registerCvar("paint_start_point", "74", "The start point of the trail", true, true, -300, true, 300)
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
    gameWrapper->HookEvent("Function TAGame.EngineShare_TA.EventPostPhysicsStep",
        [this](std::string eventName) {
            if (enabled && start_point && mode && !*enabled) return;
            this->GetPointInFront(*start_point, *mode);
        });
    gameWrapper->HookEvent("Function TAGame.Car_TA.EventPerformedFlipReset",
        [this](std::string eventName) {
            if (!*enabled) return;
            this->FlipReset();
        });
    gameWrapper->HookEvent("Function TAGame.Car_TA.OnHitBall",
        [this](std::string eventName) {
            if (!*enabled) return;
            this->BallHit();
        });
    gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed",
        [this](std::string eventName) {
            if (!*enabled) return;
            this->ClearPoints();
        });
    gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Countdown.BeginState",
        [this](std::string eventName) {
            if (!*enabled) return;
            this->ClearPoints();
        });
    gameWrapper->RegisterDrawable(
        [this](CanvasWrapper canvas) {
            if (!*enabled) return;
            Render(canvas);
        });
}

//void RL_Paint::RenderSettings() {
//    ImGui::TextUnformatted("A plugin to help give training metrics when learning how to do a speedflip in Musty's training pack: A503-264C-A7EB-D282");
//
//    CVarWrapper enableCvar = cvarManager->getCvar("sf_enabled");
//    if (!enableCvar) return;
//
//    bool enabled = enableCvar.getBoolValue();
//
//    if (ImGui::Checkbox("Enable plugin", &enabled))
//        enableCvar.setValue(enabled);
//    if (ImGui::IsItemHovered())
//        ImGui::SetTooltip("Enable/Disable Speeflip trainer plugin");
//
//    // ------------------------ ANGLE ----------------------------------
//    ImGui::Separator();
//}
//
//void RL_Paint::RenderWindow() {
//}

void RL_Paint::Log(std::string msg)
{
    cvarManager->log(msg);
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
        //this->Log("Not in valid game state 1");
        return false;
    }
    ServerWrapper game = (ingame == 1) ? gameWrapper->GetGameEventAsServer() : gameWrapper->GetGameEventAsReplay();
    if (game.IsNull()) {
        //this->Log("Not in valid game state 2");
        return false;
    }

    return true;
}
void RL_Paint::Render(CanvasWrapper canvas) {
    if (!IsValidGameState())
        return;

    LinearColor colors;
    colors.R = 255;
    colors.G = 255;
    colors.B = 0;
    colors.A = 255;
    canvas.SetColor(colors);
    auto camera = gameWrapper->GetCamera();
    if (camera.IsNull()) return;
    RT::Frustum frust{ canvas, camera };
    if (pairs.empty()) return;
    for (auto p : pairs) {
        RT::Line(p.first, p.second).DrawWithinFrustum(canvas, frust);
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



    //this->Log("1");
    //this->Log(std::format("new vector = {} {} {}", rotatedVector.X, rotatedVector.Y, rotatedVector.Z));

    
    //this->Log(std::format("X: {}",RotatorToQuat(car.GetRotation()).));
    
    //RT::Sphere(cl, RotatorToQuat(r) * Quat(0.71,0,0.71,0), 50).Draw(canvas, frust, camera.GetLocation(), 30);



}
void RL_Paint::DrawBallHit(Vector p, CanvasWrapper canvas, RT::Frustum frust) {
    Vector up = Vector(p.X, p.Y, p.Z + 5);
    Vector down = Vector(p.X, p.Y, p.Z - 5);
    RT::Line(up, down).DrawWithinFrustum(canvas, frust);
}
void RL_Paint::DrawFlipReset(Vector p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation) {
    RT::Sphere(p, 50).Draw(canvas, frust, cameraLocation, 10);
}

void RL_Paint::DrawStartPoint(int p, CanvasWrapper canvas, RT::Frustum frust, Vector cameraLocation) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    Vector v = RotatePointWithCar(Vector((float) p, 0, 0), car.GetLocation(), car.GetRotation());
    RT::Sphere(v, 5).Draw(canvas, frust, cameraLocation, 10);
    Vector up = v + Vector(0, 0, 15);
    Vector down = v + Vector(0, 0, -15);
    RT::Line(up, down).DrawWithinFrustum(canvas, frust);
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
void RL_Paint::AddPoints(Vector p) {
    if (points.empty()) {
        points.push_back(p);
        return;
    }
    points.push_back(p);
}

void RL_Paint::GetPointInFront(int startPoint, int mode) {
    //if (!IsValidGameState()) return;
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    Vector v = car.GetLocation();
    Rotator r = car.GetRotation();
    Vector rp = this->RotatePointWithCar(Vector(startPoint, 0, 0), v, r);
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
    // Delete Points if needed

    // Add new points
    this->AddPoints(rp);
    this->CalculatePairs();
}


void RL_Paint::BallHit() {
    //this->Log("Ballhit Start");
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    points_ballhit.push_back(car.GetLocation());

    //this->Log("Ballhit End");
}
void RL_Paint::FlipReset() {
    ServerWrapper game = gameWrapper->GetGameEventAsServer();
    if (!game) return;
    ArrayWrapper<BallWrapper> balls = game.GetGameBalls();
    Vector x;
    for (BallWrapper ball : balls) {
        x = ball.GetTrajectoryStartLocation();
    }
    points_flipreset.push_back(x);
    CarWrapper car = gameWrapper->GetLocalCar();
    //car.SetFrozen(true);
}
void RL_Paint::Freeze(Vector v) {
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car) return;
    cvarManager->getCvar("sv_soccar_gravity").setValue(-1.0f);

    car.SetLocation(v);
    car.SetVelocity(0);
    car.SetRotation(0);
}

Vector RL_Paint::RotatePointWithCar(Vector offset,Vector carLocation, Rotator carRotation) // direct copy of hitbox plugin
{
    // offset is from middle of the car on the car so Vector(80,0,0) would be forwards from the car
    double dPitch = (double)carRotation.Pitch / 32768.0 * 3.14159;
    double dYaw = (double)carRotation.Yaw / 32768.0 * 3.14159;
    double dRoll = (double)carRotation.Roll / 32768.0 * 3.14159;

    float sx = sin(dRoll);
    float cx = cos(dRoll);
    float sy = sin(-dYaw);
    float cy = cos(-dYaw);
    float sz = sin(dPitch);
    float cz = cos(dPitch);
    offset = Vector(offset.X, offset.Y * cx - offset.Z * sx, offset.Y * sx + offset.Z * cx);
    offset = Vector(offset.X * cz - offset.Y * sz, offset.X * sz + offset.Y * cz, offset.Z);
    offset = Vector(offset.X * cy + offset.Z * sy, offset.Y, -offset.X * sy + offset.Z * cy);
    float tmp = offset.Z;
    offset.Z = offset.Y;
    offset.Y = tmp;
    return offset + carLocation;
    //Vector rotatedVector = this->Rotate(offset, dRoll, -dYaw, dPitch) + carLocation;//

}

