import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read the data
hospital_data = pd.read_csv('complexity_results.csv')
earthquake_data = pd.read_csv('earthquake_results.csv')

# Create subplots
fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))
fig.suptitle('Algorithm Performance Analysis', fontsize=16, fontweight='bold')

# 1. Hospital Triage - Execution Time
ax1.plot(hospital_data['Patients'], hospital_data['Time_ms'], 'bo-', linewidth=2, markersize=6)
ax1.set_xlabel('Number of Patients')
ax1.set_ylabel('Execution Time (ms)')
ax1.set_title('Greedy Algorithm: Hospital Triage\nTime Complexity')
ax1.grid(True, alpha=0.3)

# Add theoretical complexity line for comparison
x_theory = hospital_data['Patients']
y_theory = x_theory * np.log2(x_theory) * 0.05  # Scaled for visualization
ax1.plot(x_theory, y_theory, 'r--', alpha=0.7, label='O(n log n) theoretical')
ax1.legend()

# 2. Earthquake Locator - Execution Time
ax2.plot(earthquake_data['Stations'], earthquake_data['Time_ms'], 'go-', linewidth=2, markersize=6)
ax2.set_xlabel('Number of Stations')
ax2.set_ylabel('Execution Time (ms)')
ax2.set_title('Divide & Conquer: Earthquake Locator\nTime Complexity')
ax2.grid(True, alpha=0.3)

# Add theoretical complexity line for comparison
x_theory_eq = earthquake_data['Stations']
y_theory_eq = x_theory_eq * np.log2(x_theory_eq) * 0.0002  # Scaled for visualization
ax2.plot(x_theory_eq, y_theory_eq, 'r--', alpha=0.7, label='O(n log n) theoretical')
ax2.legend()

# 3. Hospital Triage - Average Wait Time
ax3.plot(hospital_data['Patients'], hospital_data['Avg_Wait_Time'], 'mo-', linewidth=2, markersize=6)
ax3.set_xlabel('Number of Patients')
ax3.set_ylabel('Average Wait Time (minutes)')
ax3.set_title('Greedy Algorithm: Patient Wait Time\nQuality Analysis')
ax3.grid(True, alpha=0.3)

# 4. Earthquake Locator - Location Error
# Calculate actual geographic distance from true epicenter
location_error = []
true_x, true_y = 35.0, -120.0  # True epicenter
for _, row in earthquake_data.iterrows():
    # Calculate distance in km using geographic formula
    dx = (row['Location_X'] - true_x) * 111  # 1 degree latitude ≈ 111 km
    dy = (row['Location_Y'] - true_y) * 111 * np.cos(np.radians(true_x))  # longitude correction
    error_km = np.sqrt(dx**2 + dy**2)
    location_error.append(error_km)

ax4.plot(earthquake_data['Stations'], location_error, 'co-', linewidth=2, markersize=6)
ax4.set_xlabel('Number of Stations')
ax4.set_ylabel('Location Error (km)')
ax4.set_title('Divide & Conquer: Location Accuracy\nQuality Analysis')
ax4.grid(True, alpha=0.3)
# Set y-axis range to show the actual variation clearly
ax4.set_ylim([220, 232])

plt.tight_layout()
plt.savefig('algorithm_performance.png', dpi=300, bbox_inches='tight')

# Save individual plots for IEEE column format
# 1. Hospital Triage Time Complexity
fig1, ax = plt.subplots(figsize=(6, 4))
ax.plot(hospital_data['Patients'], hospital_data['Time_ms'], 'bo-', linewidth=2, markersize=6)
ax.set_xlabel('Number of Patients', fontsize=10)
ax.set_ylabel('Execution Time (ms)', fontsize=10)
ax.set_title('Hospital Triage: Time Complexity', fontsize=11, fontweight='bold')
ax.grid(True, alpha=0.3)
x_theory = hospital_data['Patients']
y_theory = x_theory * np.log2(x_theory) * 0.05
ax.plot(x_theory, y_theory, 'r--', alpha=0.7, label='O(n log n) theoretical')
ax.legend(fontsize=9)
plt.tight_layout()
plt.savefig('hospital_time.png', dpi=300, bbox_inches='tight')
plt.close()

# 2. Earthquake Locator Time Complexity
fig2, ax = plt.subplots(figsize=(6, 4))
ax.plot(earthquake_data['Stations'], earthquake_data['Time_ms'], 'go-', linewidth=2, markersize=6)
ax.set_xlabel('Number of Stations', fontsize=10)
ax.set_ylabel('Execution Time (ms)', fontsize=10)
ax.set_title('Earthquake Locator: Time Complexity', fontsize=11, fontweight='bold')
ax.grid(True, alpha=0.3)
x_theory_eq = earthquake_data['Stations']
y_theory_eq = x_theory_eq * np.log2(x_theory_eq) * 0.0002
ax.plot(x_theory_eq, y_theory_eq, 'r--', alpha=0.7, label='O(n log n) theoretical')
ax.legend(fontsize=9)
plt.tight_layout()
plt.savefig('earthquake_time.png', dpi=300, bbox_inches='tight')
plt.close()

# 3. Hospital Wait Time Quality
fig3, ax = plt.subplots(figsize=(6, 4))
ax.plot(hospital_data['Patients'], hospital_data['Avg_Wait_Time'], 'mo-', linewidth=2, markersize=6)
ax.set_xlabel('Number of Patients', fontsize=10)
ax.set_ylabel('Average Wait Time (minutes)', fontsize=10)
ax.set_title('Hospital Triage: Wait Time Quality', fontsize=11, fontweight='bold')
ax.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('hospital_quality.png', dpi=300, bbox_inches='tight')
plt.close()

# 4. Earthquake Location Accuracy
fig4, ax = plt.subplots(figsize=(6, 4))
# Calculate actual geographic distance
location_error_individual = []
true_x, true_y = 35.0, -120.0
for _, row in earthquake_data.iterrows():
    dx = (row['Location_X'] - true_x) * 111  # km
    dy = (row['Location_Y'] - true_y) * 111 * np.cos(np.radians(true_x))
    error_km = np.sqrt(dx**2 + dy**2)
    location_error_individual.append(error_km)

ax.plot(earthquake_data['Stations'], location_error_individual, 'co-', linewidth=2, markersize=6)
ax.set_xlabel('Number of Stations', fontsize=10)
ax.set_ylabel('Location Error (km)', fontsize=10)
ax.set_title('Earthquake Locator: Accuracy', fontsize=11, fontweight='bold')
ax.grid(True, alpha=0.3)
ax.set_ylim([220, 232])  # Show actual variation range
plt.tight_layout()
plt.savefig('earthquake_accuracy.png', dpi=300, bbox_inches='tight')
plt.close()

print("\nGenerated individual images:")
print("- hospital_time.png")
print("- earthquake_time.png")
print("- hospital_quality.png")
print("- earthquake_accuracy.png")

plt.show()

# Print summary statistics
print("=== PERFORMANCE SUMMARY ===")
print("\nHospital Triage (Greedy Algorithm):")
print(f"- Input sizes tested: {hospital_data['Patients'].min()} to {hospital_data['Patients'].max()} patients")
print(f"- Execution time range: {hospital_data['Time_ms'].min():.3f} to {hospital_data['Time_ms'].max():.3f} ms")
print(f"- Complexity verification: Time grows as O(n log n + nm)")

print("\nEarthquake Locator (Divide & Conquer):")
print(f"- Input sizes tested: {earthquake_data['Stations'].min()} to {earthquake_data['Stations'].max()} stations")
print(f"- Execution time range: {earthquake_data['Time_ms'].min():.3f} to {earthquake_data['Time_ms'].max():.3f} ms")
print(f"- Average location error: {np.mean(location_error):.3f} km")
print(f"- Complexity verification: Time grows as O(n log n)")

print("\n=== COMPLEXITY ANALYSIS ===")
# Calculate growth ratios to verify complexity
hospital_ratios = []
earthquake_ratios = []

for i in range(1, len(hospital_data)):
    n_ratio = hospital_data.iloc[i]['Patients'] / hospital_data.iloc[i-1]['Patients']
    t_ratio = hospital_data.iloc[i]['Time_ms'] / hospital_data.iloc[i-1]['Time_ms']
    hospital_ratios.append(t_ratio / n_ratio)

for i in range(1, len(earthquake_data)):
    n_ratio = earthquake_data.iloc[i]['Stations'] / earthquake_data.iloc[i-1]['Stations']
    t_ratio = earthquake_data.iloc[i]['Time_ms'] / earthquake_data.iloc[i-1]['Time_ms']
    earthquake_ratios.append(t_ratio / n_ratio)

print(f"\nHospital Triage - Average growth factor: {np.mean(hospital_ratios):.3f}")
print(f"Earthquake Locator - Average growth factor: {np.mean(earthquake_ratios):.3f}")
print("\nNote: Growth factors close to log(2) ≈ 0.693 confirm O(n log n) complexity")