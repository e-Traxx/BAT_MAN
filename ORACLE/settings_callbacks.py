from dash.dependencies import Input, Output, State
import requests
from data_service import ESP32_IP, ESP32_PORT

def register_settings_callbacks(app):
    @app.callback(
        Output('save-status', 'children'),
        Input('save-settings', 'n_clicks'),
        [State('max-voltage', 'value'),
         State('min-voltage', 'value'),
         State('max-temp', 'value'),
         State('capacity', 'value'),
         State('overcurrent-limit', 'value'),
         State('temp-hysteresis', 'value'),
         State('can-speed', 'value'),
         State('device-id', 'value')],
        prevent_initial_call=True
    )
    def save_settings(n_clicks, max_v, min_v, max_t, cap, oc_limit, temp_hyst, can_spd, dev_id):
        if n_clicks:
            settings = {
                'max_voltage': max_v,
                'min_voltage': min_v,
                'max_temperature': max_t,
                'capacity': cap,
                'overcurrent_limit': oc_limit,
                'temperature_hysteresis': temp_hyst,
                'can_bus_speed': can_spd,
                'device_id': dev_id
            }
            
            try:
                response = requests.post(
                    f"http://{ESP32_IP}:{ESP32_PORT}/api/settings",
                    json=settings
                )
                if response.status_code == 200:
                    return "Settings saved successfully!"
                else:
                    return f"Error saving settings: {response.status_code}"
            except Exception as e:
                return f"Error saving settings: {str(e)}"
            
        return ""