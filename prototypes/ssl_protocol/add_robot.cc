// See README.txt for information and build instructions.

#include <zmq.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include "discrete.pb.h"
using namespace std;

// This function fills in a Person message based on user input.
void PromptForTeam(roboime::Action* action) {
  float x,y;
  /*  Robot Position  */
  cout << " Enter the robot id: ";
  int id;
  cin >> id;
  cin.ignore(256, '\n');

  action->set_robot_id(id);

  /*  Action  */
  cout << " Enter the robot action (move, pass or kick): ";
  string action_type;
  getline(cin, action_type);
  if (action_type == "move") {
    /*  Move  */
    action->set_type(roboime::Action::MOVE);
    roboime::Action::Move* move = new roboime::Action::Move();

    /*  Target positon  */
    cout << " Enter the target x position: ";
    cin >> x;
    cin.ignore(256, '\n');

    cout << " Enter the target y position: ";
    cin >> y;
    cin.ignore(256, '\n');

    move->set_x(x);
    move->set_y(y);

    action->set_allocated_move(move);

  } else if (action_type == "pass") {
    /*  Pass  */
    action->set_type(roboime::Action::PASS);

    /*  Robot id  */
    cout << " Enter the robot id: ";
    int robot_id;
    cin >> robot_id;
    cin.ignore(256, '\n');

    roboime::Action::Pass* pass = new roboime::Action::Pass();
    pass->set_robot_id(robot_id);
    action->set_allocated_pass(pass);
  } else if (action_type == "kick") {
    /*  Kick  */
    action->set_type(roboime::Action::KICK);
    roboime::Action::Kick* kick = new roboime::Action::Kick();

    /*  Target positon  */
    cout << " Enter the target x position: ";
    cin >> x;
    cin.ignore(256, '\n');

    cout << " Enter the target y position: ";
    cin >> y;
    cin.ignore(256, '\n');

    kick->set_x(x);
    kick->set_y(y);

    action->set_allocated_kick(kick);
  } else {
    cout << " Invalid action." << endl;
  }
}

void sendCommand(string data){
  zmq::context_t context(1);
  zmq::socket_t socket (context, ZMQ_REQ);

  cout << "Connecting to server..." << endl;
  socket.connect("tcp://localhost:5555");

  zmq::message_t command_message(data.length());
  memcpy((void *) command_message.data(), data.c_str(), data.length());

  cout << "Sending data..." << endl;
  socket.send (command_message);

  cout << "Done." << endl;
}

// Main function:  Reads the entire command from a file,
//   adds one action based on user input, then writes it back out to the same
//   file.
int main(int argc, char* argv[]) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    cerr << "Usage:  " << argv[0] << " COMMAND_FILE" << endl;
    return -1;
  }

  roboime::Command command;

  {
    // Read the existing team list.
    fstream input(argv[1], ios::in | ios::binary);
    if (!input) {
      cout << argv[1] << ": File not found.  Creating a new file." << endl;
    } else if (!command.ParseFromIstream(&input)) {
      cerr << "Failed to parse command." << endl;
      return -1;
    }
  }

  // Add an action.
  PromptForTeam(command.add_action());

  {
    // Write the new command back to disk.
    fstream output(argv[1], ios::out | ios::trunc | ios::binary);
    if (!command.SerializeToOstream(&output)) {
      cerr << "Failed to write command." << endl;
      return -1;
    }

    string data;

    if(!command.SerializeToString(&data)) {
      cerr << "Failed to convert command." << endl;
    }

    sendCommand(data);
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
