// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include "Plane.h"

#define MAX_SINK_RATE 3
/* 
   call parachute library update
*/
void Plane::parachute_check()
{
    parachute.update();
    
    void parachute_emergency_sink();
}

/*
  parachute_release - trigger the release of the parachute
*/
void Plane::parachute_release()
{
    if (parachute.released()) {
        return;
    }
    
    // send message to gcs and dataflash
    gcs_send_text(MAV_SEVERITY_CRITICAL,"Parachute: Released");

    // release parachute
    parachute.release();
}

/*
  parachute_manual_release - trigger the release of the parachute,
  after performing some checks for pilot error checks if the vehicle
  is landed
*/
bool Plane::parachute_manual_release()
{
    // exit immediately if parachute is not enabled
    if (!parachute.enabled() || parachute.released()) {
        return false;
    }

    if (parachute.alt_min() > 0 && relative_ground_altitude(false) < parachute.alt_min() &&
            auto_state.last_flying_ms > 0) {
        // Allow manual ground tests by only checking if flying too low if we've taken off
        gcs_send_text_fmt(MAV_SEVERITY_WARNING, "Parachute: Too low");
        return false;
    }

    // if we get this far release parachute
    parachute_release();

    return true;
}

void Plane::parachute_emergency_sink()
{
     // exit immediately if parachute is not enabled
     if (!parachute.enabled() || parachute.released()) {
         return;
     }
     
     // only automatically release in AUTO mode
     if (control_mode != AUTO) {
         return;
     }
     
     // do not release if vehicle is not flying
     if (!is_flying()) {
         return;
     }
     
     // do not release if taking off or landing
     if (auto_state.takeoff_complete == false || mission.get_current_nav_cmd().id == MAV_CMD_NAV_LAND) {
         return;
    }
    
    // if the sink rate gets above the limits, release the parachute
    if (gps_vz > MAX_SINK_RATE)
    {
        gcs_send_text(MAV_SEVERITY_CRITICAL,"Parachute: Stall detected");
        parachute_release();
    }
}
