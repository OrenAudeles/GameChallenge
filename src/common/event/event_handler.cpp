#include "event.h"
#include <unordered_map>

namespace {
	typedef std::unordered_map<int, event_function> EventBindingMap;

	EventBindingMap *bindings = nullptr;

	EVENT_FN(null_binding){}
}


void event::initialize_handler(void){
	bindings = new EventBindingMap;
}
void event::shutdown_handler(void){
	delete bindings;
	bindings = nullptr;
}

void event::set_event_handler(int event_id, event_function fn){
	if (!fn){
		fn = EVENT_NAME(null_binding);
	}
	(*bindings)[event_id] = fn;
}
void event::call_event_handler(int event_id, void* event_data){
	if ((*bindings).count(event_id) > 0){
		(*bindings)[event_id](event_data);
	}
}