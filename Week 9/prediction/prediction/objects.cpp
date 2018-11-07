#include <cassert>

#include "objects.h"

void Tank::add_message(const TankMessage& msg) {
	messages_.push_back(msg);
}

void Tank::predict_position(float time) {
	const int msize = messages_.size();
	assert(msize >= 3);
	const TankMessage& msg0 = messages_[msize - 1];
	const TankMessage& msg1 = messages_[msize - 2];
	const TankMessage& msg2 = messages_[msize - 3];

	// Linear
	/*float timePassed = msg0.time - msg1.time;

	float x_speed = (msg0.x - msg1.x) / timePassed;
	float y_speed = (msg0.y - msg1.y) / timePassed;

	timePassed = time - msg0.time;

	x_ = msg0.x + (x_speed * timePassed);
	y_ = msg0.y + (y_speed * timePassed);*/

	// Quadratic

	// Acceleration
	// a = (v - u)/t

	float x_acc, y_acc;
	float x_v = 0.f, y_v = 0.f;
	float x_u = 0.f, y_u = 0.f;

	float timePassed = msg0.time - msg1.time;

	x_v = (msg0.x - msg1.x) / timePassed;
	y_v = (msg0.y - msg1.y) / timePassed;

	timePassed = msg1.time - msg2.time;

	x_u = (msg1.x - msg2.x) / timePassed;
	y_u = (msg1.y - msg2.y) / timePassed;

	x_acc = x_v - x_u;
	y_acc = y_v - y_u;

	x_acc /= time - msg2.time;
	y_acc /= time - msg2.time;

	// Displacement
	// s = ut + 0.5 * at ^2

	float x_disp = 0.f;
	timePassed = time - msg0.time;

	x_disp += x_u * timePassed;
	x_disp += 0.5 * pow(x_acc * timePassed, 2);

	x_ = msg0.x + x_disp;

	float y_disp = 0.f;
	timePassed = time - msg0.time;

	y_disp += y_u * timePassed;
	y_disp += 0.5 * pow(y_acc * timePassed, 2);

	y_ = msg0.y + y_disp;

	// FIXME: Implement prediction here!
	// You have:
	// - the history of position messages received, in "messages_"
	//   (msg0 is the most recent, msg1 the 2nd most recent, msg2 the 3rd most recent)
	// - the current time, in "time"
	// You need to update:
	// - the predicted position at the current time, in "x_" and "y_"
}
