/**
 * This file copies in some required type definitions to handle events from SDL without needing to supply
 * the SDL headers themselves.
 **/
#ifndef H_EVENT_SDL_TYPE_H
#define H_EVENT_SDL_TYPE_H

#pragma once

#include <inttypes.h>
#include "event_SDL_enum.h"

typedef uint32_t Uint32;
typedef uint16_t Uint16;
typedef uint8_t  Uint8;
typedef int32_t Sint32;
typedef int16_t Sint16;


/**
 *  \brief The SDL keysym structure, used in key events.
 *
 *  \note  If you are looking for translated character input, see the ::SDL_TEXTINPUT event.
 */
typedef struct SDL_Keysym
{
    SDL_Scancode scancode;      /**< SDL physical key code - see ::SDL_Scancode for details */
    SDL_Keycode sym;            /**< SDL virtual key code - see ::SDL_Keycode for details */
    Uint16 mod;                 /**< current key modifiers */
    Uint32 unused;
} SDL_Keysym;


/**
 *  \brief Fields shared by every event
 */
typedef struct SDL_CommonEvent
{
    Uint32 type;
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
} SDL_CommonEvent;

/**
 *  \brief Window state change event data (event.window.*)
 */
typedef struct SDL_WindowEvent
{
    Uint32 type;        /**< ::SDL_WINDOWEVENT */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The associated window */
    Uint8 event;        /**< ::SDL_WindowEventID */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint32 data1;       /**< event dependent data */
    Sint32 data2;       /**< event dependent data */
} SDL_WindowEvent;

/**
 *  \brief Keyboard button event structure (event.key.*)
 */
typedef struct SDL_KeyboardEvent
{
    Uint32 type;        /**< ::SDL_KEYDOWN or ::SDL_KEYUP */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The window with keyboard focus, if any */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 repeat;       /**< Non-zero if this is a key repeat */
    Uint8 padding2;
    Uint8 padding3;
    SDL_Keysym keysym;  /**< The key that was pressed or released */
} SDL_KeyboardEvent;

#define SDL_TEXTEDITINGEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text editing event structure (event.edit.*)
 */
typedef struct SDL_TextEditingEvent
{
    Uint32 type;                                /**< ::SDL_TEXTEDITING */
    Uint32 timestamp;                           /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;                            /**< The window with keyboard focus, if any */
    char text[SDL_TEXTEDITINGEVENT_TEXT_SIZE];  /**< The editing text */
    Sint32 start;                               /**< The start cursor of selected editing text */
    Sint32 length;                              /**< The length of selected editing text */
} SDL_TextEditingEvent;


#define SDL_TEXTINPUTEVENT_TEXT_SIZE (32)
/**
 *  \brief Keyboard text input event structure (event.text.*)
 */
typedef struct SDL_TextInputEvent
{
    Uint32 type;                              /**< ::SDL_TEXTINPUT */
    Uint32 timestamp;                         /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;                          /**< The window with keyboard focus, if any */
    char text[SDL_TEXTINPUTEVENT_TEXT_SIZE];  /**< The input text */
} SDL_TextInputEvent;

/**
 *  \brief Mouse motion event structure (event.motion.*)
 */
typedef struct SDL_MouseMotionEvent
{
    Uint32 type;        /**< ::SDL_MOUSEMOTION */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Uint32 state;       /**< The current button state */
    Sint32 x;           /**< X coordinate, relative to window */
    Sint32 y;           /**< Y coordinate, relative to window */
    Sint32 xrel;        /**< The relative motion in the X direction */
    Sint32 yrel;        /**< The relative motion in the Y direction */
} SDL_MouseMotionEvent;

/**
 *  \brief Mouse button event structure (event.button.*)
 */
typedef struct SDL_MouseButtonEvent
{
    Uint32 type;        /**< ::SDL_MOUSEBUTTONDOWN or ::SDL_MOUSEBUTTONUP */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Uint8 button;       /**< The mouse button index */
    Uint8 state;        /**< ::SDL_PRESSED or ::SDL_RELEASED */
    Uint8 clicks;       /**< 1 for single-click, 2 for double-click, etc. */
    Uint8 padding1;
    Sint32 x;           /**< X coordinate, relative to window */
    Sint32 y;           /**< Y coordinate, relative to window */
} SDL_MouseButtonEvent;

/**
 *  \brief Mouse wheel event structure (event.wheel.*)
 */
typedef struct SDL_MouseWheelEvent
{
    Uint32 type;        /**< ::SDL_MOUSEWHEEL */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The window with mouse focus, if any */
    Uint32 which;       /**< The mouse instance id, or SDL_TOUCH_MOUSEID */
    Sint32 x;           /**< The amount scrolled horizontally, positive to the right and negative to the left */
    Sint32 y;           /**< The amount scrolled vertically, positive away from the user and negative toward the user */
    Uint32 direction;   /**< Set to one of the SDL_MOUSEWHEEL_* defines. When FLIPPED the values in X and Y will be opposite. Multiply by -1 to change them back */
} SDL_MouseWheelEvent;

// Not supporting Joysticks
// Not supporting Controllers

/**
 *  \brief Audio device event structure (event.adevice.*)
 */
typedef struct SDL_AudioDeviceEvent
{
    Uint32 type;        /**< ::SDL_AUDIODEVICEADDED, or ::SDL_AUDIODEVICEREMOVED */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 which;       /**< The audio device index for the ADDED event (valid until next SDL_GetNumAudioDevices() call), SDL_AudioDeviceID for the REMOVED event */
    Uint8 iscapture;    /**< zero if an output device, non-zero if a capture device. */
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL_AudioDeviceEvent;

// Not supporting Touch events
// Not supporting Multigesture events
// Not supporting DollarGestureEvents

/**
 *  \brief An event used to request a file open by the system (event.drop.*)
 *         This event is enabled by default, you can disable it with SDL_EventState().
 *  \note If this event is enabled, you must free the filename in the event.
 */
typedef struct SDL_DropEvent
{
    Uint32 type;        /**< ::SDL_DROPBEGIN or ::SDL_DROPFILE or ::SDL_DROPTEXT or ::SDL_DROPCOMPLETE */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    char *file;         /**< The file name, which should be freed with SDL_free(), is NULL on begin/complete */
    Uint32 windowID;    /**< The window that was dropped on, if any */
} SDL_DropEvent;


/**
 *  \brief The "quit requested" event
 */
typedef struct SDL_QuitEvent
{
    Uint32 type;        /**< ::SDL_QUIT */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
} SDL_QuitEvent;

/**
 *  \brief OS Specific event
 */
typedef struct SDL_OSEvent
{
    Uint32 type;        /**< ::SDL_QUIT */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
} SDL_OSEvent;

/**
 *  \brief A user-defined event type (event.user.*)
 */
typedef struct SDL_UserEvent
{
    Uint32 type;        /**< ::SDL_USEREVENT through ::SDL_LASTEVENT-1 */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    Uint32 windowID;    /**< The associated window if any */
    Sint32 code;        /**< User defined event code */
    void *data1;        /**< User defined data pointer */
    void *data2;        /**< User defined data pointer */
} SDL_UserEvent;


struct SDL_SysWMmsg;
typedef struct SDL_SysWMmsg SDL_SysWMmsg;

/**
 *  \brief A video driver dependent system event (event.syswm.*)
 *         This event is disabled by default, you can enable it with SDL_EventState()
 *
 *  \note If you want to use this event, you should include SDL_syswm.h.
 */
typedef struct SDL_SysWMEvent
{
    Uint32 type;        /**< ::SDL_SYSWMEVENT */
    Uint32 timestamp;   /**< In milliseconds, populated using SDL_GetTicks() */
    SDL_SysWMmsg *msg;  /**< driver dependent data, defined in SDL_syswm.h */
} SDL_SysWMEvent;

/**
 *  \brief General event structure
 */
typedef union SDL_Event
{
    Uint32 type;                    /**< Event type, shared with all events */
    SDL_CommonEvent common;         /**< Common event data */
    SDL_WindowEvent window;         /**< Window event data */
    SDL_KeyboardEvent key;          /**< Keyboard event data */
    SDL_TextEditingEvent edit;      /**< Text editing event data */
    SDL_TextInputEvent text;        /**< Text input event data */
    SDL_MouseMotionEvent motion;    /**< Mouse motion event data */
    SDL_MouseButtonEvent button;    /**< Mouse button event data */
    SDL_MouseWheelEvent wheel;      /**< Mouse wheel event data */
    //SDL_JoyAxisEvent jaxis;         /**< Joystick axis event data */
    //SDL_JoyBallEvent jball;         /**< Joystick ball event data */
    //SDL_JoyHatEvent jhat;           /**< Joystick hat event data */
    //SDL_JoyButtonEvent jbutton;     /**< Joystick button event data */
    //SDL_JoyDeviceEvent jdevice;     /**< Joystick device change event data */
    //SDL_ControllerAxisEvent caxis;      /**< Game Controller axis event data */
    //SDL_ControllerButtonEvent cbutton;  /**< Game Controller button event data */
    //SDL_ControllerDeviceEvent cdevice;  /**< Game Controller device event data */
    SDL_AudioDeviceEvent adevice;   /**< Audio device event data */
    SDL_QuitEvent quit;             /**< Quit request event data */
    SDL_UserEvent user;             /**< Custom event data */
    SDL_SysWMEvent syswm;           /**< System dependent window event data */
    //SDL_TouchFingerEvent tfinger;   /**< Touch finger event data */
    //SDL_MultiGestureEvent mgesture; /**< Gesture event data */
    //SDL_DollarGestureEvent dgesture; /**< Gesture event data */
    SDL_DropEvent drop;             /**< Drag and drop event data */

    /* This is necessary for ABI compatibility between Visual C++ and GCC
       Visual C++ will respect the push pack pragma and use 52 bytes for
       this structure, and GCC will use the alignment of the largest datatype
       within the union, which is 8 bytes.

       So... we'll add padding to force the size to be 56 bytes for both.
    */
    Uint8 padding[56];
} SDL_Event;


#endif