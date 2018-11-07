#ifndef OBJECTS_H
#define OBJECTS_H

#include <vector>

#include "messages.h"

class Tank {
public:
	/** Compute the predicted position at the given time,
		based on messages_, updating x_ and y_. */
	void predict_position(float time);

	/** Add a message to this Tank's collection. */
	void add_message(const TankMessage& msg);

	// Accessors.
	float x() const { return x_; }
	float y() const { return y_; }

private:
	/** The current predicted position of this tank. */
	float x_ = -1.0f, y_ = -1.0f;

	/** The messages that have been received about this tank. */
	std::vector<TankMessage> messages_;
};

#endif
