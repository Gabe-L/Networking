/** Implementation of GameSimulator. */

#include <iostream>

#include "messages.h"
#include "simulator.h"

using std::cout;

GameSimulator::GameSimulator() {
	wait_next_frame();
}

bool GameSimulator::receive_message(TankMessage &result) {
	if (message_queue_.empty()) {
		return false;
	} else {
		result = message_queue_.front();
		message_queue_.pop();
		return true;
	}
}

void GameSimulator::wait_next_frame() {
	time_ += 0.1f;

	// Simulate messages being sent from a remote host every "period",
	// delivered to this host after "latency".
	const float period = 0.5f;
	const float latency = 1.3f;
	while (sent_time_ + latency < time_) {
		message_queue_.push({ 1, sent_x_, sent_y_, sent_time_ });

		sent_time_ += period;

		// Change the path of the tank after a while.
		if (sent_time_ >= 12.7f) {
			sent_vx_ = 40.0f;
			sent_vy_ = -10.0f;
		}

		sent_x_ += (sent_vx_ * period);
		sent_y_ += (sent_vy_ * period);
	}

	cout << "--- New frame: time=" << time_ << "s ---\n";
}
