#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <queue>

#include "messages.h"

const int NUM_PLAYERS = 2;

/** The simulated game engine. */
class GameSimulator {
public:
	/** Constructor. */
	GameSimulator();

	/** Return the number of this player's tank. */
	int this_player() const { return 0; }

	/** Receive the next message from the network. Non-blocking.
	    Returns true if a message was received, false if there are no more messages to process. */
	bool receive_message(TankMessage &result);

	/** Return the current value of the game's clock. */
	float time() const { return time_; }

	/** Wait until the next display frame begins. Blocking.
	    (In a real game, this would be updating the display,
		synchronising with the vertical blank, etc.) */
	void wait_next_frame();

private:
	/** The current simulated time. */
	float time_ = 9.9f;
	
	/** Internal state for the message simulator. */
	float sent_time_ = 7.0f;
	float sent_x_ = 120.0f, sent_y_ = 160.0f;
	float sent_vx_ = 20.0f, sent_vy_ = 30.0f;
	std::queue<TankMessage> message_queue_;
};

#endif
