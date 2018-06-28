#ifndef H_EVENT_H
#define H_EVENT_H

#pragma once

#include <inttypes.h>

typedef void (*event_function)(void*);
#define EVENT_NAME(name) event_##name
#define EVENT_FN(name) void EVENT_NAME(name)(void* data)

namespace event{
	void initialize_handler(void);
	void shutdown_handler(void);

	void set_event_handler(int event_id, event_function fn);
	void call_event_handler(int event_id, void* event_data);

	void poll_events(void);
	void wait_events(void);
	void wait_events_timeout(float timeout);
}

#endif