// See README.txt for information and build instructions.

#include <iostream>
#include <fstream>
#include <string>
#include "team.pb.h"
using namespace std;

// Iterates though all people in the AddressBook and prints info about them.
void ListRobots(const ssl::Team& team) {
  for (int i = 0; i < team.robot_size(); i++) {
    const ssl::Robot& robot = team.robot(i);

    cout << "Robot id: " << i << endl;

    /*  TODO: check pose  */
    cout << "Robot Position: " << robot.position().x() << ", " << \
      robot.position().y() << endl;

    /*  TODO: check action  */
    const ssl::Robot::Action& action = robot.action();
    cout << " Action: ";

    switch (action) {
      case ssl::Robot::MOVE:
        cout << "\tMove -> target " << \
        /*  TODO: check move  */
          robot.move().target().x() << ", " << \
          robot.move().target().y() << endl;

        break;
      case ssl::Robot::PASS:
        cout << "\tPass -> target robot id:  " << \
        /*  TODO: check pass  */
          robot.pass().robot_id() << endl;

        break;
      case ssl::Robot::KICK:
        cout << "\tKick -> target: " << \
        /*  TODO: check kick  */
          robot.kick().target().x() << ", " << \
          robot.kick().target().y() << endl;

        break;
    }
  }
}

// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
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
    if (!team.ParseFromIstream(&input)) {
      cerr << "Failed to parse team list." << endl;
      return -1;
    }
  }

  ListRobots(team);

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
