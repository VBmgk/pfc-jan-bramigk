// See README.txt for information and build instructions.

#include <iostream>
#include <fstream>
#include <string>
#include "discrete.pb.h"
using namespace std;

// Iterates though all people in the AddressBook and prints info about them.
void ListActions(const roboime::Command& command) {
  for (int i = 0; i < command.action_size(); i++) {
    const roboime::Action& action = command.action(i);

    cout << "Robot id: " << action.robot_id() << endl;

    /*  TODO: check action  */
    cout << " Action type: ";

    switch (action.type()) {
      case roboime::Action::MOVE:
        cout << "\tMove -> target " << \
        /*  TODO: check move  */
          action.move().x() << ", " << \
          action.move().y() << endl;

        break;
        case roboime::Action::PASS:
        cout << "\tPass -> target robot id:  " << \
        /*  TODO: check pass  */
          action.pass().robot_id() << endl;

        break;
        case roboime::Action::KICK:
        cout << "\tKick -> target: " << \
        /*  TODO: check kick  */
          action.kick().x() << ", " << \
          action.kick().y() << endl;

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

  roboime::Command command;

  {
    // Read the existing team list.
    fstream input(argv[1], ios::in | ios::binary);
    if (!command.ParseFromIstream(&input)) {
      cerr << "Failed to parse team list." << endl;
      return -1;
    }
  }

  ListActions(command);

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
