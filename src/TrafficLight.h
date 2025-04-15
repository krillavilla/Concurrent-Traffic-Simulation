#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// Forward declarations to avoid circular dependencies
class Vehicle;

// Define the message queue class here since it's specific to the traffic light
template <typename T>
class MessageQueue
{
public:
    void send(T &&msg);
    T receive();

private:
    std::deque<T> _queue;
    std::condition_variable _condition;
    std::mutex _mutex;
};

// Define the traffic light phase enum
enum TrafficLightPhase
{
    red,
    green
};

class TrafficLight : public TrafficObject
{
public:
    // Constructor / Destructor
    TrafficLight();

    // Getters / setters
    TrafficLightPhase getCurrentPhase();

    // Typical behavior methods
    void waitForGreen();
    void simulate();

private:
    // Typical behavior methods
    void cycleThroughPhases();

    // Member variables
    TrafficLightPhase _currentPhase;
    MessageQueue<TrafficLightPhase> _messageQueue;
};

#endif