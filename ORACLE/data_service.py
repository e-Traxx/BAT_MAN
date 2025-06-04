import requests

# Constants for ESP32 server
ESP32_IP = "192.168.4.1"  # Default ESP32 AP IP
ESP32_PORT = 80

def fetch_diagnostic_data():
    """Fetch diagnostic data from the ESP32"""
    try:
        response = requests.get(f"http://{ESP32_IP}:{ESP32_PORT}/api/diagnostic")
        if response.status_code == 200:
            return response.json()
    except Exception as e:
        print(f"Error fetching diagnostic data: {e}")
    return None

def fetch_cell_data():
    """Fetch cell data from the ESP32"""
    try:
        response = requests.get(f"http://{ESP32_IP}:{ESP32_PORT}/api/cell_data")
        if response.status_code == 200:
            return response.json()
    except Exception as e:
        print(f"Error fetching cell data: {e}")
    return None