/*
 * File: sensor_processor.cpp
 * Purpose: This program implements a sensor data processing system
 *          demonstrating STL container selection for different data
 *          access patterns including fast lookups and priority processing.
 */

#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct SensorReading {
  int sensorId;
  string location;
  string sensorType;
  double value;

  SensorReading(int id, string loc, string type, double val)
      : sensorId(id), location(loc), sensorType(type), value(val) {}
  // Missing default constructor:
  SensorReading() {}
};

struct Alert {
  int priority;
  string message;
  string alertType;

  Alert(int p, string msg, string type) : priority(p), message(msg), alertType(type) {}
};
// Comparator: higher priority numbers should be processed first. Use const refs for safety.
auto alertCmp = [](const Alert& a, const Alert& b) { return a.priority < b.priority; };

class SensorProcessor {
 private:
  // TODO: Choose appropriate containers for different access patterns
  // Hint: You'll need containers for ID lookup, location grouping, and priority processing
  unordered_map<int, SensorReading> m_readings;
  unordered_map<string, vector<SensorReading>> m_loc_readings;
  // Use brace-init to initialize the priority_queue with our lambda comparator
  priority_queue<Alert, vector<Alert>, decltype(alertCmp)> m_alerts{alertCmp};

 public:
  /**
   * GRADED CHALLENGE 1
   * TASK: Implement addSensorReading to store sensor data with fast ID lookup
   * and location-based grouping capabilities.
   */
  void addSensorReading(const SensorReading& reading) {
    // YOUR CODE HERE
    // Consider: What containers provide O(1) ID lookup?
    // How can you group sensors by location efficiently?
  }

  /**
   * GRADED CHALLENGE 2
   * TASK: Implement addAlert to store alerts in priority order.
   * Higher priority numbers should be processed first.
   */
  void addAlert(const Alert& alert) {
    // YOUR CODE HERE
    // Consider: Which STL container automatically maintains priority order?
    // Do you need a custom comparator?
  }

  /**
   * GRADED CHALLENGE 3
   * TASK: Implement processNextAlert to handle the highest priority alert.
   * Display the alert information and remove it from the queue.
   */
  void processNextAlert() {
    // YOUR CODE HERE
    // Process and display the highest priority alert
    // Format: "Processing Priority [X]: [message] ([type])"
    // Handle empty queue case
  }

  /**
   * GRADED CHALLENGE 4
   * TASK: Implement findSensorById for fast sensor lookup by ID.
   * Return pointer to sensor data or nullptr if not found.
   */
  SensorReading* findSensorById(int sensorId) {
    // YOUR CODE HERE
    // Achieve O(1) average case lookup performance
    // Return nullptr if sensor not found
  }

  /**
   * GRADED CHALLENGE 5
   * TASK: Implement getSensorsByLocation to return all sensors
   * in a specified building location.
   */
  vector<SensorReading*> getSensorsByLocation(const string& location) {
    // YOUR CODE HERE
    // Return all sensors at the specified location
    // Return empty vector if location not found
  }

  /**
   * Function: runTests
   * Description: Test harness to verify basic functionality
   */
  void runTests() {
    cout << "=== Sensor Processing System Tests ===" << endl;

    // Add test sensor data
    cout << "Adding sensor readings..." << endl;
    addSensorReading(SensorReading(101, "Building_A", "temperature", 72.5));
    addSensorReading(SensorReading(102, "Building_A", "humidity", 45.2));
    addSensorReading(SensorReading(201, "Building_B", "temperature", 68.1));
    addSensorReading(SensorReading(202, "Building_B", "humidity", 52.7));
    addSensorReading(SensorReading(301, "Building_C", "temperature", 74.3));
    cout << "✓ Sensor data added" << endl;

    // Test ID lookup
    cout << "\nTesting ID-based lookup..." << endl;
    auto sensor = findSensorById(102);
    if (sensor != nullptr) {
      cout << "✓ Found sensor " << sensor->sensorId << " in " << sensor->location << endl;
    }

    // Test location queries
    cout << "\nTesting location-based queries..." << endl;
    auto buildingASensors = getSensorsByLocation("Building_A");
    cout << "✓ Found " << buildingASensors.size() << " sensors in Building_A" << endl;

    // Test priority processing
    cout << "\nTesting priority alert processing..." << endl;
    addAlert(Alert(3, "Temperature threshold exceeded", "WARNING"));
    addAlert(Alert(9, "Critical system failure", "CRITICAL"));
    addAlert(Alert(1, "Low battery detected", "INFO"));
    addAlert(Alert(7, "Network connectivity issue", "ERROR"));

    cout << "Processing alerts by priority:" << endl;
    processNextAlert();
    processNextAlert();
    processNextAlert();
    processNextAlert();

    cout << "\n=== Testing Complete ===" << endl;
  }
};

/**
 * Function: main
 * Description: Program entry point demonstrating the sensor processing system
 */
int main() {
  cout << "=== Sensor Data Processing System ===" << endl;
  cout << "Demonstrating STL container selection for efficient data processing\n" << endl;

  SensorProcessor processor;
  processor.runTests();

  return 0;
}

/*
EXPECTED COMPLETE OUTPUT:
=== Sensor Data Processing System ===
Demonstrating STL container selection for efficient data processing

=== Sensor Processing System Tests ===

Adding sensor readings...
Added sensor 101 (temperature) to Building_A
Added sensor 102 (humidity) to Building_A
Added sensor 201 (temperature) to Building_B
Added sensor 202 (humidity) to Building_B
Added sensor 301 (temperature) to Building_C
✓ Sensor data added

Testing ID-based lookup...
✓ Found sensor 102 in Building_A (value: 45.2)
✓ Correctly returned nullptr for missing sensor 999

Testing location-based queries...
✓ Found 2 sensors in Building_A:
  - Sensor 101 (temperature)
  - Sensor 102 (humidity)
✓ Found 2 sensors in Building_B
✓ Found 0 sensors in Building_Z (should be 0)

Testing priority alert processing...
Added alert: Priority 3 - Temperature threshold exceeded
Added alert: Priority 9 - Critical system failure
Added alert: Priority 1 - Low battery detected
Added alert: Priority 7 - Network connectivity issue

Processing alerts by priority (highest first):
Processing Priority 9: Critical system failure (CRITICAL)
Processing Priority 7: Network connectivity issue (ERROR)
Processing Priority 3: Temperature threshold exceeded (WARNING)
Processing Priority 1: Low battery detected (INFO)

Trying to process alert from empty queue:
No alerts to process - queue is empty

=== Testing Complete ===

PERFORMANCE ANALYSIS:
- ID Lookup: O(1) average case using unordered_map
- Location Queries: O(1) location lookup + O(k) sensor retrieval where k = sensors per location
- Priority Processing: O(log n) insertion, O(1) top access, O(log n) removal
- Memory Usage: Efficient with pointer-based access, minimal data duplication
- Scalability: System maintains performance characteristics as data size increases
*/