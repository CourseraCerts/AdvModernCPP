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

class SensorProcessor {
 private:
  // TODO: Choose appropriate containers for different access patterns
  // Hint: You'll need containers for ID lookup, location grouping, and priority processing

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

Lab Solution Files :
// ============================================================================
// Data Processing with STL Containers - Solution Files
// Complete implementation demonstrating optimal STL container selection
// for different data access patterns in a sensor processing system
// ============================================================================

/*
 * File: sensor_processor.cpp
 * Author: Solution Implementation
 * Purpose: Complete sensor data processing system demonstrating
 *          efficient STL container usage for multiple access patterns
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

  Missing default constructor : SensorReading() {}
};

struct Alert {
  int priority;
  string message;
  string alertType;

  Alert(int p, string msg, string type) : priority(p), message(msg), alertType(type) {}
};

// Custom comparator for priority queue - ensures higher priority numbers processed first
struct AlertComparator {
  bool operator()(const Alert& a, const Alert& b) {
    // Return true if 'a' should have lower priority than 'b'
    // Since priority_queue is max-heap, we want higher priority numbers on top
    return a.priority < b.priority;
  }
};

class SensorProcessor {
 private:
  // Container for O(1) ID-based sensor lookup
  unordered_map<int, SensorReading> sensorData;

  // Container for location-based queries - maps location to sensor IDs
  unordered_map<string, vector<int>> locationIndex;

  // Priority queue for processing alerts by priority level
  priority_queue<Alert, vector<Alert>, AlertComparator> alertQueue;

 public:
  /**
   * GRADED CHALLENGE 1 - COMPLETED
   * Implements addSensorReading with dual indexing for optimal access patterns
   */
  void addSensorReading(const SensorReading& reading) {
    // Store sensor data for O(1) ID lookup
    sensorData[reading.sensorId] = reading;

    // Add sensor ID to location index for building-based queries
    locationIndex[reading.location].push_back(reading.sensorId);

    cout << "Added sensor " << reading.sensorId << " (" << reading.sensorType << ") to " << reading.location << endl;
  }

  /**
   * GRADED CHALLENGE 2 - COMPLETED
   * Implements addAlert with priority-based ordering
   */
  void addAlert(const Alert& alert) {
    // Add alert to priority queue - AlertComparator ensures correct ordering
    alertQueue.push(alert);

    cout << "Added alert: Priority " << alert.priority << " - " << alert.message << endl;
  }

  /**
   * GRADED CHALLENGE 3 - COMPLETED
   * Implements processNextAlert with proper priority handling and edge cases
   */
  void processNextAlert() {
    // Handle empty queue case
    if (alertQueue.empty()) {
      cout << "No alerts to process - queue is empty" << endl;
      return;
    }

    // Get highest priority alert (top of max-heap)
    Alert currentAlert = alertQueue.top();

    // Display alert information in specified format
    cout << "Processing Priority " << currentAlert.priority << ": " << currentAlert.message << " ("
         << currentAlert.alertType << ")" << endl;

    // Remove processed alert from queue
    alertQueue.pop();
  }

  /**
   * GRADED CHALLENGE 4 - COMPLETED
   * Implements findSensorById with O(1) average case performance
   */
  SensorReading* findSensorById(int sensorId) {
    // Use unordered_map's find() for O(1) average lookup
    auto it = sensorData.find(sensorId);

    // Return pointer to sensor data if found, nullptr otherwise
    if (it != sensorData.end()) {
      return &(it->second);
    }

    return nullptr;
  }

  /**
   * GRADED CHALLENGE 5 - COMPLETED
   * Implements getSensorsByLocation using location index for efficiency
   */
  vector<SensorReading*> getSensorsByLocation(const string& location) {
    vector<SensorReading*> result;

    // Find sensor IDs for specified location using location index
    auto locationIt = locationIndex.find(location);
    if (locationIt != locationIndex.end()) {
      // Add pointers to sensor data for each ID in this location
      for (int sensorId : locationIt->second) {
        auto sensorIt = sensorData.find(sensorId);
        if (sensorIt != sensorData.end()) {
          result.push_back(&(sensorIt->second));
        }
      }
    }

    return result;
  }

  /**
   * Function: runTests
   * Description: Comprehensive test harness validating all system functionality
   */
  void runTests() {
    cout << "=== Sensor Processing System Tests ===" << endl;

    // Add test sensor data
    cout << "\nAdding sensor readings..." << endl;
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
      cout << "✓ Found sensor " << sensor->sensorId << " in " << sensor->location << " (value: " << sensor->value << ")"
           << endl;
    } else {
      cout << "✗ Sensor 102 not found" << endl;
    }

    // Test missing sensor
    auto missingSensor = findSensorById(999);
    if (missingSensor == nullptr) {
      cout << "✓ Correctly returned nullptr for missing sensor 999" << endl;
    } else {
      cout << "✗ Should return nullptr for missing sensor" << endl;
    }

    // Test location queries
    cout << "\nTesting location-based queries..." << endl;
    auto buildingASensors = getSensorsByLocation("Building_A");
    cout << "✓ Found " << buildingASensors.size() << " sensors in Building_A:" << endl;
    for (auto sensor : buildingASensors) {
      cout << "  - Sensor " << sensor->sensorId << " (" << sensor->sensorType << ")" << endl;
    }

    auto buildingBSensors = getSensorsByLocation("Building_B");
    cout << "✓ Found " << buildingBSensors.size() << " sensors in Building_B" << endl;

    // Test empty location
    auto emptySensors = getSensorsByLocation("Building_Z");
    cout << "✓ Found " << emptySensors.size() << " sensors in Building_Z (should be 0)" << endl;

    // Test priority processing
    cout << "\nTesting priority alert processing..." << endl;
    addAlert(Alert(3, "Temperature threshold exceeded", "WARNING"));
    addAlert(Alert(9, "Critical system failure", "CRITICAL"));
    addAlert(Alert(1, "Low battery detected", "INFO"));
    addAlert(Alert(7, "Network connectivity issue", "ERROR"));

    cout << "\nProcessing alerts by priority (highest first):" << endl;
    processNextAlert();  // Should process priority 9 first
    processNextAlert();  // Should process priority 7 next
    processNextAlert();  // Should process priority 3 next
    processNextAlert();  // Should process priority 1 last

    cout << "\nTrying to process alert from empty queue:" << endl;
    processNextAlert();  // Should handle empty queue gracefully

    cout << "\n=== Testing Complete ===" << endl;
  }
};

/*
VALIDATION CHECKLIST COMPLETED:
✓ Unordered_map used for O(1) average case sensor ID lookup

✓ Location indexing implemented for efficient building-based queries

✓ Priority queue with custom comparator processes alerts by priority level

✓ Edge cases handled (missing sensors, empty locations, empty alert queue)

✓ Pointer-based data access maintains memory efficiency

✓ All container choices optimized for their specific access patterns

✓ Code follows proper C++ naming conventions and formatting standards

✓ Comprehensive testing validates all functionality including edge cases
*/

/**
 * Function: main
 * Description: Program entry point demonstrating complete sensor processing system
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