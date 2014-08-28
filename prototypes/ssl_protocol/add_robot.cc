// See README.txt for information and build instructions.

#include <iostream>
#include <fstream>
#include <string>
#include "team.pb.h"
using namespace std;

// This function fills in a Person message based on user input.
void PromptForTeam(ssl::Robot* robot) {
  /*  Robot Position  */
  cout << " Enter the robot x position: ";
  float x, y;
  cin >> x;
  cin.ignore(256, '\n');

  cout << " Enter the robot y position: ";
  cin >> y;
  cin.ignore(256, '\n');
  
  ssl::Robot::Pose* pose = new ssl::Robot::Pose();
  ssl::Robot::Position* position = new ssl::Robot::Position();

  position->set_x(x);
  position->set_y(y);

  /*  Robot angle  */
  cout << " Enter the robot angle: ";
  float angle;
  cin >> angle;
  cin.ignore(256, '\n');

  /*  Robot pose  */
  pose->set_allocated_position(position);
  pose->set_theta(angle);
  robot->set_allocated_pose(pose);

  /*  Action  */
  cout << " Enter the robot action (move, pass or kick): ";
  string action;
  getline(cin, action);
  if (action == "move") {
    /*  Move  */
    robot->set_action(ssl::Robot::MOVE);
    ssl::Robot::Move* move = new ssl::Robot::Move();

    /*  Target positon  */
    cout << " Enter the target x position: ";
    cin >> x;
    cin.ignore(256, '\n');

    cout << " Enter the target y position: ";
    cin >> y;
    cin.ignore(256, '\n');

    position = new ssl::Robot::Position();
    position->set_x(x);
    position->set_y(y);

    move->set_allocated_target(position);
    robot->set_allocated_move(move);

  } else if (action == "pass") {
    /*  Pass  */
    robot->set_action(ssl::Robot::PASS);

    /*  Robot id  */
    cout << " Enter the robot id: ";
    int robot_id;
    cin >> robot_id;
    cin.ignore(256, '\n');

    ssl::Robot::Pass* pass = new ssl::Robot::Pass();
    pass->set_robot_id(robot_id);
    robot->set_allocated_pass(pass);
  } else if (action == "kick") {
    /*  Kick  */
    robot->set_action(ssl::Robot::KICK);
    ssl::Robot::Kick* kick = new ssl::Robot::Kick();

    /*  Target positon  */
    cout << " Enter the target x position: ";
    cin >> x;
    cin.ignore(256, '\n');

    cout << " Enter the target y position: ";
    cin >> y;
    cin.ignore(256, '\n');

    position = new ssl::Robot::Position();
    position->set_x(x);
    position->set_y(y);

    kick->set_allocated_target(position);
    robot->set_allocated_kick(kick);
  } else {
    cout << " Unknown robot action.  Using default." << endl;
  }
}

// Main function:  Reads the entire address book from a file,
//   adds one person based on user input, then writes it back out to the same
//   file.
int main(int argc, char* argv[]) {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    cerr << "Usage:  " << argv[0] << " TEAM_FILE" << endl;
    return -1;
  }

  ssl::Team team;

  {
    // Read the existing team list.
    fstream input(argv[1], ios::in | ios::binary);
    if (!input) {
      cout << argv[1] << ": File not found.  Creating a new file." << endl;

      /*  Team color  */
      cout << " Enter the team color(blue or yellow: ";
      string color;
      getline(cin, color);
      if (color == "blue") {
        team.set_color(ssl::Team::BLUE);
      } else if(color == "yellow") {
        team.set_color(ssl::Team::YELLOW);
      } else {
        cout << " Unknown robot color.  Using default." << endl;
      }

    } else if (!team.ParseFromIstream(&input)) {
      cerr << "Failed to parse address book." << endl;
      return -1;
    }
  }

  // Add an address.
  PromptForTeam(team.add_robot());

  {
    // Write the new address book back to disk.
    fstream output(argv[1], ios::out | ios::trunc | ios::binary);
    if (!team.SerializeToOstream(&output)) {
      cerr << "Failed to write address book." << endl;
      return -1;
    }
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
