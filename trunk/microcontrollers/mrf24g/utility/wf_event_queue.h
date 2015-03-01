/*
 * MRF24WG Universal Driver Event Queue
 *
 * This module contains event queue defintions.
 */
#ifndef __WF_EVENT_QUEUE_H
#define __WF_EVENT_QUEUE_H

#define MAX_EVENTS  (10)

typedef struct eventStruct
{
    uint8_t  eventType;
    uint32_t eventData;
} t_event;

typedef struct eventQueueStruct
{
   uint8_t  writeIndex;
   uint8_t  readIndex;
   t_event  event[MAX_EVENTS + 1]; // one unused slot
} t_wfEventQueue;

void EventQInit(void);
void EventEnqueue(uint8_t eventType, uint32_t eventData);
void EventDequeue(t_event *p_event);
bool isEventQEmpty(void);
bool isEventQFull(void);

#endif /* __WF_EVENT_QUEUE_H */
