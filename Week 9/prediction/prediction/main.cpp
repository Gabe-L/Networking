// Prediction lab exercise.
// Adam Sampson <a.sampson@abertay.ac.uk>

#include <iostream>

#include "messages.h"
#include "objects.h"
#include "simulator.h"

using std::cin;
using std::cout;

int main(int argc, char *argv[]) {
	cout << "Game starting\n";
	GameSimulator game;

	Tank tanks[NUM_PLAYERS];

	// While the game is still running...
	// (Adjust the time here to make the simulation run longer.)
	while (game.time() < 18.0f) {

		// Receive any incoming messages, and pass them to the appropriate tank.
		TankMessage msg;
		while (game.receive_message(msg)) {
			cout << "Received message: id=" << msg.id << " pos=" << msg.x << "," << msg.y << " time=" << msg.time << "\n";
			tanks[msg.id].add_message(msg);
		}

		// Predict the positions of the other tanks.
		for (int i = 0; i < NUM_PLAYERS; ++i) {
			if (i != game.this_player()) {
				tanks[i].predict_position(game.time());
				cout << "Predicted for tank " << i << ": pos=" << tanks[1].x() << "," << tanks[1].y() << "\n";
			}
		}

		// Update the display and wait for the next frame to begin.
		game.wait_next_frame();
	}

	cout << "Game finished! Press return to exit...\n";
	cin.get();

	return 0;
}