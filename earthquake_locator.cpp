#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace chrono;

// Seismic Station representing a monitoring station
struct SeismicStation {
    double latitude;
    double longitude;
    double detection_time;
    int id;
    
    SeismicStation(int _id, double lat, double lon, double time) 
        : id(_id), latitude(lat), longitude(lon), detection_time(time) {}
};

// Point representing coordinates
struct Point {
    double x, y;
    
    Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}
    
    double distance(const Point& other) const {
        return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
    }
};

// Geographic bounds for spatial partitioning
struct GeoBounds {
    double min_lat, max_lat, min_lon, max_lon;
    
    GeoBounds(double minLat, double maxLat, double minLon, double maxLon) 
        : min_lat(minLat), max_lat(maxLat), min_lon(minLon), max_lon(maxLon) {}
    
    Point center() const {
        return Point((min_lat + max_lat) / 2, (min_lon + max_lon) / 2);
    }
    
    bool contains(const SeismicStation& station) const {
        return station.latitude >= min_lat && station.latitude <= max_lat &&
               station.longitude >= min_lon && station.longitude <= max_lon;
    }
};

// Quadrant for spatial division
struct Quadrant {
    GeoBounds bounds;
    vector<SeismicStation> stations;
    Point estimate;
    double confidence;
    
    Quadrant(const GeoBounds& b) : bounds(b), confidence(0.0) {}
};

// Result of epicenter calculation
struct EpicenterResult {
    Point location;
    double confidence;
    double error;
    
    EpicenterResult(Point loc, double conf, double err) 
        : location(loc), confidence(conf), error(err) {}
};

// Earthquake Epicenter Locator using Divide & Conquer
class EarthquakeEpicenterLocator {
private:
    static const int BASE_CASE_SIZE = 8;  // Threshold for base case
    static const double WAVE_VELOCITY;   // km/s (P-wave velocity)
    
public:
    // Main divide and conquer algorithm
    EpicenterResult locateEpicenter(vector<SeismicStation>& stations, 
                                   const GeoBounds& bounds, 
                                   int depth = 0) {
        
        // Base case: use simple triangulation
        if (stations.size() <= BASE_CASE_SIZE) {
            return simpleTriangulation(stations);
        }
        
        // Divide: Split geographic region into quadrants
        double mid_lat = (bounds.min_lat + bounds.max_lat) / 2;
        double mid_lon = (bounds.min_lon + bounds.max_lon) / 2;
        
        vector<Quadrant> quadrants = {
            Quadrant(GeoBounds(bounds.min_lat, mid_lat, bounds.min_lon, mid_lon)), // SW
            Quadrant(GeoBounds(bounds.min_lat, mid_lat, mid_lon, bounds.max_lon)),  // SE
            Quadrant(GeoBounds(mid_lat, bounds.max_lat, bounds.min_lon, mid_lon)),  // NW
            Quadrant(GeoBounds(mid_lat, bounds.max_lat, mid_lon, bounds.max_lon))   // NE
        };
        
        // Partition stations into quadrants
        for (auto& station : stations) {
            for (auto& quad : quadrants) {
                if (quad.bounds.contains(station)) {
                    quad.stations.push_back(station);
                    break;
                }
            }
        }
        
        // Recursively solve for each quadrant with stations
        vector<EpicenterResult> results;
        for (auto& quad : quadrants) {
            if (!quad.stations.empty()) {
                EpicenterResult result = locateEpicenter(quad.stations, quad.bounds, depth + 1);
                quad.estimate = result.location;
                quad.confidence = result.confidence;
                results.push_back(result);
            }
        }
        
        // Combine: Weighted average of regional estimates
        return weightedCombination(results);
    }
    
private:
    // Simple triangulation for base case
    EpicenterResult simpleTriangulation(const vector<SeismicStation>& stations) {
        if (stations.empty()) {
            return EpicenterResult(Point(0, 0), 0, 1e9);
        }
        
        if (stations.size() == 1) {
            return EpicenterResult(Point(stations[0].latitude, stations[0].longitude), 1.0, 0);
        }
        
        // Use geometric approach: intersection of circles
        // For simplicity, use least squares method
        double sum_x = 0, sum_y = 0;
        double min_time = stations[0].detection_time;
        
        // Find minimum detection time (closest to epicenter)
        for (const auto& station : stations) {
            min_time = min(min_time, station.detection_time);
        }
        
        // Calculate weighted center based on time differences
        double total_weight = 0;
        for (const auto& station : stations) {
            double time_diff = station.detection_time - min_time;
            double weight = 1.0 / (1.0 + time_diff * time_diff); // Inverse time weighting
            
            sum_x += station.latitude * weight;
            sum_y += station.longitude * weight;
            total_weight += weight;
        }
        
        Point estimated_center(sum_x / total_weight, sum_y / total_weight);
        
        // Calculate error (sum of squared residuals)
        double error = 0;
        for (const auto& station : stations) {
            double distance = estimated_center.distance(Point(station.latitude, station.longitude));
            double theoretical_time = distance / WAVE_VELOCITY;
            double actual_time = station.detection_time - min_time;
            error += (theoretical_time - actual_time) * (theoretical_time - actual_time);
        }
        
        double confidence = 1.0 / (1.0 + error / stations.size());
        return EpicenterResult(estimated_center, confidence, error);
    }
    
    // Weighted combination of multiple estimates
    EpicenterResult weightedCombination(const vector<EpicenterResult>& results) {
        if (results.empty()) {
            return EpicenterResult(Point(0, 0), 0, 1e9);
        }
        
        if (results.size() == 1) {
            return results[0];
        }
        
        double total_weight = 0;
        double weighted_x = 0, weighted_y = 0;
        double combined_error = 0;
        
        for (const auto& result : results) {
            double weight = result.confidence;
            weighted_x += result.location.x * weight;
            weighted_y += result.location.y * weight;
            total_weight += weight;
            combined_error += result.error * weight;
        }
        
        Point combined_location(weighted_x / total_weight, weighted_y / total_weight);
        double combined_confidence = total_weight / results.size();
        
        return EpicenterResult(combined_location, combined_confidence, combined_error / total_weight);
    }
    
public:
    // Generate synthetic earthquake data
    static vector<SeismicStation> generateEarthquakeData(int num_stations, 
                                                        Point true_epicenter,
                                                        const GeoBounds& region) {
        vector<SeismicStation> stations;
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> lat_dist(region.min_lat, region.max_lat);
        uniform_real_distribution<> lon_dist(region.min_lon, region.max_lon);
        uniform_real_distribution<> noise_dist(-0.5, 0.5); // Time noise
        
        for (int i = 0; i < num_stations; i++) {
            double lat = lat_dist(gen);
            double lon = lon_dist(gen);
            
            // Calculate theoretical arrival time
            double distance = true_epicenter.distance(Point(lat, lon));
            double travel_time = distance / WAVE_VELOCITY;
            double detected_time = travel_time + noise_dist(gen); // Add noise
            
            stations.emplace_back(i, lat, lon, detected_time);
        }
        
        return stations;
    }
    
    // Performance testing
    static double measureExecutionTime(EarthquakeEpicenterLocator& locator,
                                      vector<SeismicStation>& stations,
                                      const GeoBounds& bounds) {
        auto start = high_resolution_clock::now();
        locator.locateEpicenter(stations, bounds);
        auto end = high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(end - start);
        return duration.count() / 1000.0; // Convert to milliseconds
    }
    
    // Complexity analysis
    static void runComplexityAnalysis() {
        cout << "\n=== DIVIDE & CONQUER COMPLEXITY ANALYSIS ===\n";
        cout << "Testing earthquake epicenter location with varying station counts...\n";
        cout << "Format: Stations, Time(ms), Error, Estimated_Location\n\n";
        
        ofstream file("earthquake_results.csv");
        file << "Stations,Time_ms,Error,Location_X,Location_Y\n";
        
        vector<int> test_sizes = {25, 50, 100, 200, 500, 1000, 1500, 2000};
        Point true_epicenter(35.0, -120.0); // California coordinates
        GeoBounds california(32.0, 42.0, -125.0, -114.0); // California region
        
        for (int n : test_sizes) {
            double total_time = 0.0;
            double total_error = 0.0;
            Point avg_location(0, 0);
            int trials = 5;
            
            for (int trial = 0; trial < trials; trial++) {
                EarthquakeEpicenterLocator locator;
                vector<SeismicStation> stations = generateEarthquakeData(n, true_epicenter, california);
                
                double exec_time = measureExecutionTime(locator, stations, california);
                EpicenterResult result = locator.locateEpicenter(stations, california);
                
                total_time += exec_time;
                total_error += result.error;
                avg_location.x += result.location.x;
                avg_location.y += result.location.y;
            }
            
            double avg_time = total_time / trials;
            double avg_error = total_error / trials;
            avg_location.x /= trials;
            avg_location.y /= trials;
            
            double location_error = true_epicenter.distance(avg_location);
            
            cout << setw(8) << n 
                 << setw(12) << fixed << setprecision(3) << avg_time
                 << setw(12) << fixed << setprecision(6) << avg_error
                 << setw(12) << fixed << setprecision(3) << location_error << " km"
                 << " (" << fixed << setprecision(3) << avg_location.x 
                 << ", " << avg_location.y << ")" << endl;
            
            file << n << "," << avg_time << "," << avg_error << "," 
                 << avg_location.x << "," << avg_location.y << "\n";
        }
        
        file.close();
        cout << "\nResults saved to earthquake_results.csv\n";
        cout << "True epicenter: (" << true_epicenter.x << ", " << true_epicenter.y << ")\n";
    }
};

// Static member definition
const double EarthquakeEpicenterLocator::WAVE_VELOCITY = 6.0; // km/s

int main() {
    cout << "Earthquake Epicenter Location - Divide & Conquer Algorithm Implementation\n";
    cout << "======================================================================\n\n";
    
    // Demo with small dataset
    cout << "DEMO: Earthquake location with synthetic data\n";
    cout << "--------------------------------------------\n";
    
    Point true_epicenter(35.5, -119.5); // Central California
    GeoBounds demo_region(34.0, 37.0, -121.0, -118.0);
    
    EarthquakeEpicenterLocator locator;
    vector<SeismicStation> demo_stations = 
        EarthquakeEpicenterLocator::generateEarthquakeData(12, true_epicenter, demo_region);
    
    cout << "True epicenter: (" << true_epicenter.x << ", " << true_epicenter.y << ")\n";
    cout << "Seismic stations:\n";
    for (const auto& station : demo_stations) {
        cout << "Station " << station.id << ": (" 
             << fixed << setprecision(2) << station.latitude << ", " 
             << station.longitude << ") Time: " << station.detection_time << "s\n";
    }
    
    auto start_time = high_resolution_clock::now();
    EpicenterResult result = locator.locateEpicenter(demo_stations, demo_region);
    auto end_time = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(end_time - start_time);
    
    cout << "\nCalculated epicenter: (" << fixed << setprecision(3) 
         << result.location.x << ", " << result.location.y << ")\n";
    cout << "Confidence: " << fixed << setprecision(3) << result.confidence << "\n";
    cout << "Error: " << fixed << setprecision(6) << result.error << "\n";
    cout << "Location accuracy: " << fixed << setprecision(3) 
         << true_epicenter.distance(result.location) << " km\n";
    cout << "Execution time: " << duration.count() / 1000.0 << " ms\n";
    
    // Performance analysis
    EarthquakeEpicenterLocator::runComplexityAnalysis();
    
    cout << "\nExperimental validation complete!\n";
    cout << "Theoretical complexity: O(n log n)\n";
    cout << "Where n = number of seismic stations\n";
    
    return 0;
}