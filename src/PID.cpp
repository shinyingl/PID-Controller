#include "PID.h"

PID::PID() {}

PID::~PID() {}

void PID::Init(double Kp, double Ki, double Kd, double max_control, double min_control) {
  /* Initialize PID coefficients */
  this->Kp = Kp; //proportional
  this->Ki = Ki; //integral
  this->Kd = Kd; //derivate
  this->max_control = max_control;
  this->min_control = min_control;

  p_error = 0;
  i_error = 0;
  d_error = 0;

}

void PID::UpdateError(double cte) {
  
  //Update PID errors based on cte
  
  d_error = cte - p_error;
  p_error = cte;
  i_error += cte;


  if(i_error > max_control) {
      i_error = max_control;
    }
  if(i_error < -min_control) {
      i_error = min_control;
    }


}

double PID::TotalError() {

  //Calculate and return the total error
   
  return -Kp * p_error - Ki * i_error - Kd * d_error ;  
}

