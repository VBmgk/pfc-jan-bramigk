// See README.txt for information and build instructions.

#include <zmq.hpp>
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

  roboime::Command command;

  zmq::context_t context (1);
  zmq::socket_t socket(context, ZMQ_REP);

  socket.bind("tcp://*:5555");

  while (true) {
    zmq::message_t resultset(1000);
    socket.recv (&resultset);

    std::cout << "received request" << std::endl;

    command.ParseFromArray(resultset.data(), resultset.size());
    ListActions(command);

    //TODO: build command packet to send

    //  Send reply back to client
    zmq::message_t reply (5);
    memcpy ((void *) reply.data (), "World", 5);
    socket.send (reply);
  }

  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
