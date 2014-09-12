#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <vector>
#include <zmq.hpp>
#include "discrete.pb.h"

struct Robot {
  int i;
  float x, y, a;
  float vx, vy, va;
};

struct Field {
  float field_length,
        field_width,
        goal_width,
        defense_radius,
        defense_stretch;
};

struct RobotConfig {
  float radius,
        speed,
        kick_speed;
};

struct GameState {
  Field *field;
  RobotConfig *robot_config;
  std::vector<Robot> min_team,
                     max_team;
};

struct RobotAction {
  enum ActionType {
    Move, Pass, Kick
  } type;
  float x, y;
  int robot_id, i;
};

struct TeamCommand {
  std::vector<RobotAction> actions;
  bool min; // otherwise max
};

struct Timer {
  Timer() { reset(); }

  double elapsed() {
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
  }

  void reset() { start = std::chrono::system_clock::now(); }

  std::chrono::time_point<std::chrono::system_clock> start, end;
};

void add_kick(roboime::Action* action, float x, float y){
  /*  Kick  */
  action->set_type(roboime::Action::KICK);
  roboime::Action::Kick* kick = new roboime::Action::Kick();

  /*  Target positon  */
  kick->set_x(x);
  kick->set_y(y);

  action->set_allocated_kick(kick);
}

void add_move(roboime::Action* action, float x, float y){
  /*  Move  */
  action->set_type(roboime::Action::MOVE);
  roboime::Action::Move* move = new roboime::Action::Move();

  /*  Target positon  */
  move->set_x(x);
  move->set_y(y);

  action->set_allocated_move(move);
}

void add_pass(roboime::Action* action, int receiver_id){
  /*  Pass  */
  action->set_type(roboime::Action::PASS);

  roboime::Action::Pass* pass = new roboime::Action::Pass();
  pass->set_robot_id(receiver_id);
  action->set_allocated_pass(pass);
}

void sendCommand(std::string data, zmq::socket_t &socket){
  zmq::message_t command_message(data.length());
  memcpy((void *) command_message.data(), data.c_str(), data.length());

  std::cout << "Sending data...";
  socket.send(command_message);
}


int main() {
  using namespace std;

  Timer tmr;
  string file_path = "commands.txt";
  string line, word;
  string data;

  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  zmq::context_t context (1);
  zmq::socket_t socket(context, ZMQ_REP);

  socket.bind("tcp://*:5555");
  // get request from pyroboime
  zmq::message_t resultset(1000);


  // Read commands file
  fstream commands(file_path.c_str(), ios::in | ios::binary);
  roboime::Command* command;

  if (commands.is_open()) {
    while ( getline (commands,line) )
    {
      command = new roboime::Command();

      stringstream str_stream;
      str_stream << line;
      str_stream >> word;

      // Parser
      if( word == "#") continue;
      else if( word == "command"){
        cout << "comando ";
        int robot_id;

        while(str_stream >> robot_id){
          roboime::Action* action = command->add_action();

          cout << " robot_id: " << robot_id << " ";
          action->set_robot_id(robot_id);
          str_stream >> word;

          if(word == "pass"){
            int receiver_id;

            str_stream >> receiver_id;

            cout << word << " to " << receiver_id << endl;
            add_pass(action, receiver_id);
          } else if(word == "move"){
            float x,y;

            str_stream >> x;
            str_stream >> y;

            cout << word << " " << x << ", " << y << endl;

            add_move(action, x, y);
          } else if(word == "kick"){
            float x,y;

            str_stream >> x;
            str_stream >> y;

            cout << word << " " << x << ", " << y << endl;
            add_kick(action, x, y);
          } else{
            cout << "Unknown command" << endl;
            return -1;
          }
        }
      } else if( word == "sleep"){
        // sleep time
        double time;
        str_stream >> time;

        cout << "dormir " << time << " seg ... " << endl;
        cout.flush();

        // starting count
        tmr.reset();
        for(;;) {
          // sending last command
          socket.recv (&resultset);
          cout << "received request" << endl;
          sendCommand(data, socket);
          if(tmr.elapsed() > time)
            break;
        }

      } else cout << "Command unknown" << endl;

      // send response
      if(!command->SerializeToString(&data)) {
        cerr << "Failed to convert command." << endl;
      }

      // sending command
      socket.recv (&resultset);
      std::cout << "received request" << std::endl;
      sendCommand(data, socket);
      cout << "done!" << endl;
    }

    commands.close();
  } else cout << "Unable to open file" << endl;

  return 0;
}
