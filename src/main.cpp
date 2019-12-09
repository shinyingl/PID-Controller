#include <math.h>
#include <uWS/uWS.h>
#include <iostream>
#include <string>
#include "json.hpp"
#include "PID.h"

// for convenience
using nlohmann::json;
using std::string;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

#define KONST_p 0.2 // 0.1 marginal turn, 0.2 ok
#define KONST_i 0.001 // 0.01,0.001 slow; 0.1, 0.005 vibrate
#define KONST_d 6.0 //5.0; 0.0 vibrate // damping term. too much will include noise
#define MAX_ANGLE (pi()*.25) //45 degree
#define MAX_SPEED 100.0
#define MIN_SPEED 0.0  
#define MAX_STEER (pi()*.25) //45 degree

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
string hasData(string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != string::npos) {
    return "";
  }
  else if (b1 != string::npos && b2 != string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}

int main() {
  uWS::Hub h;

  PID steer_pid;
  PID speed_pid;
  
  //Initialize the pid variable
  steer_pid.Init(KONST_p, KONST_i, KONST_d, MAX_ANGLE, -MAX_ANGLE); 
  speed_pid.Init(KONST_p, KONST_i, KONST_d, MAX_SPEED,  MIN_SPEED);

  h.onMessage([&steer_pid, &speed_pid](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, 
                     uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2') {
      auto s = hasData(string(data).substr(0, length));

      if (s != "") {
        auto j = json::parse(s);

        string event = j[0].get<string>();

        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<string>());
          double speed = std::stod(j[1]["speed"].get<string>());
          double angle = std::stod(j[1]["steering_angle"].get<string>());
          double steer_value;
          double throttle;
          double cte_speed;
          double speed_target;
          
          // steering angle PID
          steer_pid.UpdateError(cte);
          steer_value = steer_pid.TotalError();  
          //limit steer angel between -pi/4 to pi/4
          if(steer_value > MAX_STEER) {
            steer_value = MAX_STEER;
          }
          if(steer_value < -MAX_STEER) {
            steer_value = -MAX_STEER;
          }

          // throttle PID
          speed_target = 90. * (1. - 0.1*abs(steer_value)) + 10.;
          cte_speed = speed - speed_target;
          speed_pid.UpdateError(cte_speed);
          throttle = speed_pid.TotalError();

          
          // DEBUG
          std::cout << speed_target <<","<< speed <<","<< cte_speed <<","<< steer_value <<","<< cte <<std::endl;
          // std::cout << "speed: " << speed << std::endl;
          // std::cout << "speed_target: " << speed_target << std::endl;
          // std::cout << "CTE: " << cte << " Steering Value: " << steer_value 
          //           << std::endl;

          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          // std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }  // end "telemetry" if
      } else {
        // Manual driving
        string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }  // end websocket message if
  }); // end h.onMessage

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, 
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  
  h.run();
}