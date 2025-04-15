#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : Implement the send method
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : Implement the receive method
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });

    // Get the latest message from the queue
    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : Implement the waitForGreen method
    while (true)
    {
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Start a simulation thread using the thread queue from the base class
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// Virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop to measure the time between two loop cycles
    // and to toggle the current phase of the traffic light between red and green

    // Create random device and distribution for cycle duration (4-6 seconds)
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> dist(4000, 6000); // distribution in milliseconds

    // Get cycle duration
    long cycleDuration = dist(eng);

    // Init stopwatch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();

    // Infinite loop
    while (true)
    {
        // Sleep for 1ms between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Compute time difference
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - lastUpdate).count();

        // Toggle traffic light phase if cycle duration has elapsed
        if (timeSinceLastUpdate >= cycleDuration)
        {
            // Toggle phase
            _currentPhase = (_currentPhase == red) ? green : red;

            // Send update to message queue using move semantics
            _messageQueue.send(std::move(_currentPhase));

            // Reset cycle duration to a new random value
            cycleDuration = dist(eng);

            // Reset stopwatch
            lastUpdate = std::chrono::system_clock::now();
        }
    }
}