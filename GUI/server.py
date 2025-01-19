from flask import Flask, jsonify
import random

app = Flask(__name__)

@app.route('/data', methods=['GET'])
def get_data():
    # Generate random data for bar graphs
    graph1 = [random.uniform(0.0, 4.3) for _ in range(140)]  # Voltage: 0V to 4.3V
    graph2 = [random.uniform(0.0, 80.0) for _ in range(140)]  # Temperature: 0°C to 80°C

    # Generate metrics for statistics
    highest_discharge_current = random.uniform(50.0, 150.0)  # Random value for highest discharge current in Amps
    total_charge = random.uniform(50.0, 200.0)  # Random value for total charge in Ah
    total_discharge = random.uniform(50.0, 200.0)  # Random value for total discharge in Ah
    state_of_charge = random.uniform(0.0, 100.0)  # Random value for SOC in %
    state_of_health = random.uniform(80.0, 100.0)  # Random value for SOH in %
    charge_time = random.uniform(1.0, 10.0)  # Random value for charge time in hours
    max_charge_current = random.uniform(10.0, 100.0)  # Random value for max charge current in Amps

    # Max temperature recorded and corresponding cell
    max_temperature_value = random.uniform(50.0, 80.0)
    max_temperature_cell = random.randint(1, 140)

    # Generate single values
    current_output = random.uniform(0.0, 150.0)  # Current output in Amperes
    overall_voltage = sum(graph1) / len(graph1)  # Average voltage as overall voltage


    # Faults
    faults = {
        "sensor_loss": random.choice([True, False]),
        "over_voltage": random.choice([True, False]),
        "under_voltage": random.choice([True, False]),
        "over_current": random.choice([True, False]),
        "over_temperature": random.choice([True, False]),
    }

    # Simulate system health
    system_health = all([not faults["sensor_loss"], not faults["over_voltage"], not faults["under_voltage"]])

    # Return data as JSON
    return jsonify({
        "graph1": graph1,
        "graph2": graph2,
        "highest_discharge_current": highest_discharge_current,
        "total_charge": total_charge,
        "total_discharge": total_discharge,
        "state_of_charge": state_of_charge,
        "state_of_health": state_of_health,
        "charge_time": charge_time,
        "max_charge_current": max_charge_current,
        "max_temperature": {
            "value": max_temperature_value,
            "cell": max_temperature_cell
        },
        "faults": faults,
        "system_health": system_health,
        "current_output": current_output,
        "overall_voltage": overall_voltage,

        "current_output": current_output,
            "overall_voltage": overall_voltage,})

if __name__ == '__main__':
    app.run(debug=True)
