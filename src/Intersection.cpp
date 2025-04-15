#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <random>

#include "Street.h"
#include "Intersection.h"
#include "Vehicle.h"
#include "TrafficLight.h"

/* Implementation of class "WaitingVehicles" */

int WaitingVehicles::getSize()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _vehicles.size();
}

void WaitingVehicles::pushBack(std::shared_ptr<Vehicle> vehicle, std::promise<void> &&promise)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _vehicles.push_back(vehicle);
    _promises.push_back(std::move(promise));
}

void WaitingVehicles::permitEntryToFirstInQueue()
{
    std::lock_guard<std::mutex> lock(_mutex);

    // Get the first vehicle and promise from the queue
    auto firstVehicle = _vehicles.begin();
    auto firstPromise = _promises.begin();

    // Fulfill promise and remove from the queue
    firstPromise->set_value();
    _vehicles.erase(firstVehicle);
    _promises.erase(firstPromise);
}

/* Implementation of class "Intersection" */

Intersection::Intersection()
{
    _type = ObjectType::objectIntersection;
    _isBlocked = false;
}

void Intersection::setIsBlocked(bool isBlocked)
{
    _isBlocked = isBlocked;
    std::cout << "Intersection #" << _id << " isBlocked=" << isBlocked << std::endl;
}

// The thread function for queue processing
void Intersection::processVehicleQueue()
{
    // Print id of the current thread
    std::cout << "Intersection #" << _id << "::processVehicleQueue: thread id = " << std::this_thread::get_id() << std::endl;

    // Continuously process the vehicle queue
    while (true)
    {
        // Sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Only proceed when at least one vehicle is waiting in the queue
        if (_waitingVehicles.getSize() > 0 && !_isBlocked)
        {
            // Set intersection to "blocked"
            this->setIsBlocked(true);

            // Permit entry to first vehicle in the queue
            _waitingVehicles.permitEntryToFirstInQueue();
        }
    }
}

void Intersection::addStreet(std::shared_ptr<Street> street)
{
    _streets.push_back(street);
}

std::vector<std::shared_ptr<Street>> Intersection::queryStreets(std::shared_ptr<Street> incoming)
{
    // Store all outgoing streets in a vector
    std::vector<std::shared_ptr<Street>> outgoings;
    for (auto it : _streets)
    {
        if (incoming->getID() != it->getID()) // Except the incoming street
        {
            outgoings.push_back(it);
        }
    }

    return outgoings;
}

// Add a new vehicle to the queue and return once the vehicle is allowed to enter
void Intersection::addVehicleToQueue(std::shared_ptr<Vehicle> vehicle)
{
    std::unique_lock<std::mutex> lck(_mtx);
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: thread id = " << std::this_thread::get_id() << std::endl;
    std::cout << "Intersection #" << _id << "::addVehicleToQueue: vehicle #" << vehicle->getID() << " is heading to intersection " << std::endl;

    // FP.6b : Use the methods TrafficLight::getCurrentPhase and TrafficLight::waitForGreen
    // to block the execution until the traffic light turns green
    if (_trafficLight.getCurrentPhase() == red)
    {
        std::cout << "Intersection #" << _id << ": Traffic light is red, waiting for green..." << std::endl;
        _trafficLight.waitForGreen();
    }

    std::cout << "Intersection #" << _id << ": Traffic light is green, proceeding..." << std::endl;

    // Add vehicle to the waiting line and create a promise
    std::promise<void> prmsVehicleAllowedToEnter;
    std::future<void> ftrVehicleAllowedToEnter = prmsVehicleAllowedToEnter.get_future();
    _waitingVehicles.pushBack(vehicle, std::move(prmsVehicleAllowedToEnter));

    // Wait until the vehicle is allowed to enter
    ftrVehicleAllowedToEnter.wait();
    lck.unlock();

    // FP.6b : use the methods TrafficLight::getCurrentPhase and TrafficLight::waitForGreen to implement the
    // traffic light functionality in Intersection::addVehicleToQueue

    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " has entered the intersection" << std::endl;
}

void Intersection::vehicleHasLeft(std::shared_ptr<Vehicle> vehicle)
{
    // Unblock the intersection after a vehicle has left
    this->setIsBlocked(false);
    std::cout << "Intersection #" << _id << ": Vehicle #" << vehicle->getID() << " has left the intersection" << std::endl;
}

void Intersection::simulate() // using threads + promises/futures + exceptions
{
    // FP.6a : Start the simulation of the traffic light
    _trafficLight.simulate();

    // Launch vehicle queue processing in a thread
    threads.emplace_back(std::thread(&Intersection::processVehicleQueue, this));
}

bool Intersection::trafficLightIsGreen()
{
   return _trafficLight.getCurrentPhase() == green;
}