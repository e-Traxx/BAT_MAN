import sys
import threading
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QTabWidget, 
                           QVBoxLayout, QFormLayout, QLineEdit, QSpinBox, 
                           QDoubleSpinBox, QPushButton, QLabel, QGroupBox)
from PyQt6.QtWebEngineWidgets import QWebEngineView
from PyQt6.QtCore import QUrl
import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import requests
import threading
import webbrowser
from flask import Flask
import time
import json

# Initialize Dash app
dash_app = dash.Dash(__name__)
dash_app.title = "BAT-MAN"

# Constants for ESP32 server
ESP32_IP = "192.168.4.1"  # Default ESP32 AP IP
ESP32_PORT = 80

def fetch_diagnostic_data():
    try:
        response = requests.get(f"http://{ESP32_IP}:{ESP32_PORT}/api/diagnostic")
        if response.status_code == 200:
            return response.json()
    except Exception as e:
        print(f"Error fetching diagnostic data: {e}")
    return None

def fetch_cell_data():
    try:
        response = requests.get(f"http://{ESP32_IP}:{ESP32_PORT}/api/cell_data")
        if response.status_code == 200:
            return response.json()
    except Exception as e:
        print(f"Error fetching cell data: {e}")
    return None

# App layout from main.py
dash_app.layout = html.Div(
    style={'backgroundColor': '#000000', 'padding': '20px', 'color': 'white'},
    children=[
        html.H1(
            "BAT-MAN",
            style={
                'textAlign': 'center',
                'color': 'white',
                'fontFamily': 'Arial',
                'marginBottom': '30px'
            }
        ),
        html.Div(
            children=[
                dcc.Graph(id='live-graph1'),
                dcc.Graph(id='live-graph2'),
            ],
            style={'display': 'grid', 'gridTemplateColumns': '1fr', 'gap': '20px'}
        ),
        html.Div(
            children=[
                dcc.Graph(id='current-output', style={'marginBottom': '20px'}),
                dcc.Graph(id='overall-voltage', style={'marginBottom': '20px'}),
            ],
            style={'display': 'grid', 'gridTemplateColumns': '1fr 1fr', 'gap': '20px'}
        ),
        html.Div(
            children=[
                html.H3("Faults and System Check", style={'textAlign': 'center', 'color': 'white'}),
                html.Table(
                    id='faults-system-table',
                    style={
                        'width': '100%',
                        'margin': '0 auto',
                        'color': 'white',
                        'border': '1px solid white',
                        'textAlign': 'center'
                    }
                ),
            ],
            style={'marginTop': '20px'}
        ),
        dcc.Interval(
            id='interval-component',
            interval=2000,  # 2 seconds
            n_intervals=0
        ),
    ]
)

# Callback functions from main.py
@dash_app.callback(
    Output('live-graph1', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph1(n):
    cell_data = fetch_cell_data()
    if cell_data and 'cell_voltages' in cell_data:
        # Flatten the nested array structure
        voltages = []
        for stack in cell_data['cell_voltages']:
            voltages.extend(stack)
        x = list(range(len(voltages)))
        figure = {
            'data': [go.Bar(x=x, y=voltages, marker_color='cyan')],
            'layout': go.Layout(
                title='Voltage',
                title_font_color='white',
                xaxis={'title': 'Cell', 'color': 'white'},
                yaxis={'title': 'Voltage in V', 'color': 'white'},
                yaxis_range=[0, 4.3],
                paper_bgcolor='black',
                plot_bgcolor='black',
                font_color='white'
            )
        }
        return figure
    
    # Fallback to simulated data if fetch fails
    data = {'graph1': [2 + (i % 5) * 0.1 for i in range(140)]}
    x = list(range(140))
    y1 = data.get('graph1', [0] * 140)
    figure = {
        'data': [go.Bar(x=x, y=y1, marker_color='cyan')],
        'layout': go.Layout(
            title='Voltage',
            title_font_color='white',
            xaxis={'title': 'Cell', 'color': 'white'},
            yaxis={'title': 'Voltage in V', 'color': 'white'},
            yaxis_range=[0, 4.3],
            paper_bgcolor='black',
            plot_bgcolor='black',
            font_color='white'
        )
    }
    return figure

@dash_app.callback(
    Output('live-graph2', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph2(n):
    cell_data = fetch_cell_data()
    if cell_data and 'cell_temperatures' in cell_data:
        # Flatten the nested array structure
        temperatures = []
        for stack in cell_data['cell_temperatures']:
            temperatures.extend(stack)
        x = list(range(len(temperatures)))
        figure = {
            'data': [go.Bar(x=x, y=temperatures, marker_color='lime')],
            'layout': go.Layout(
                title='Temperature',
                title_font_color='white',
                xaxis={'title': 'Cell', 'color': 'white'},
                yaxis={'title': 'Temperature in °C', 'color': 'white'},
                yaxis_range=[0, 80],
                paper_bgcolor='black',
                plot_bgcolor='black',
                font_color='white'
            )
        }
        return figure
    
    # Fallback to simulated data if fetch fails
    data = {'graph2': [25 + (i % 10) for i in range(140)]}
    x = list(range(140))
    y2 = data.get('graph2', [0] * 140)
    figure = {
        'data': [go.Bar(x=x, y=y2, marker_color='lime')],
        'layout': go.Layout(
            title='Temperature',
            title_font_color='white',
            xaxis={'title': 'Cell', 'color': 'white'},
            yaxis={'title': 'Temperature in °C', 'color': 'white'},
            yaxis_range=[0, 80],
            paper_bgcolor='black',
            plot_bgcolor='black',
            font_color='white'
        )
    }
    return figure

@dash_app.callback(
    Output('current-output', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_current_output(n):
    diag_data = fetch_diagnostic_data()
    if diag_data and 'current' in diag_data:
        current = diag_data['current']
        figure = {
            'data': [go.Indicator(
                mode="gauge+number",
                value=current,
                title={'text': "Current (A)"},
                gauge={
                    'axis': {'range': [0, 150]},
                    'bar': {'color': 'cyan'},
                }
            )],
            'layout': go.Layout(
                paper_bgcolor='black',
                font_color='white'
            )
        }
        return figure
    
    # Fallback to simulated data if fetch fails
    data = {'current_output': 75 + (n % 20)}
    current = data.get('current_output', 0)
    figure = {
        'data': [go.Indicator(
            mode="gauge+number",
            value=current,
            title={'text': "Current (A)"},
            gauge={
                'axis': {'range': [0, 150]},
                'bar': {'color': 'cyan'},
            }
        )],
        'layout': go.Layout(
            paper_bgcolor='black',
            font_color='white'
        )
    }
    return figure

@dash_app.callback(
    Output('overall-voltage', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_overall_voltage(n):
    diag_data = fetch_diagnostic_data()
    if diag_data and 'overall_voltage' in diag_data:
        voltage = diag_data['overall_voltage']
        figure = {
            'data': [go.Indicator(
                mode="gauge+number",
                value=voltage,
                title={'text': "Voltage (V)"},
                gauge={
                    'axis': {'range': [0, 4.3]},
                    'bar': {'color': 'lime'},
                }
            )],
            'layout': go.Layout(
                paper_bgcolor='black',
                font_color='white'
            )
        }
        return figure
    
    # Fallback to simulated data if fetch fails
    data = {'overall_voltage': 3.6 + (n % 5) * 0.1}
    voltage = data.get('overall_voltage', 0)
    figure = {
        'data': [go.Indicator(
            mode="gauge+number",
            value=voltage,
            title={'text': "Voltage (V)"},
            gauge={
                'axis': {'range': [0, 4.3]},
                'bar': {'color': 'lime'},
            }
        )],
        'layout': go.Layout(
            paper_bgcolor='black',
            font_color='white'
        )
    }
    return figure

@dash_app.callback(
    Output('faults-system-table', 'children'),
    Input('interval-component', 'n_intervals')
)
def update_faults_system_table(n):
    diag_data = fetch_diagnostic_data()
    if diag_data and 'flags' in diag_data:
        flags = diag_data['flags']
        table_header = [
            html.Tr([
                html.Th("Fault Type", style={'border': '1px solid white'}),
                html.Th("Status", style={'border': '1px solid white'})
            ])
        ]

        table_body = [
            html.Tr([
                html.Td("Sensor Loss", style={'border': '1px solid white'}),
                html.Td("Detected" if flags.get('lost_comm') or flags.get('voltage_sense_error') or flags.get('temp_sensor_loss') or flags.get('current_sensor_loss') else "None", style={'border': '1px solid white'})
            ]),
            html.Tr([
                html.Td("Over Voltage", style={'border': '1px solid white'}),
                html.Td("Detected" if flags.get('overall_overvoltage') else "None", style={'border': '1px solid white'})
            ]),
            html.Tr([
                html.Td("Under Voltage", style={'border': '1px solid white'}),
                html.Td("Detected" if flags.get('pack_undervoltage') else "None", style={'border': '1px solid white'})
            ]),
            html.Tr([
                html.Td("Over Current", style={'border': '1px solid white'}),
                html.Td("Detected" if flags.get('current_sensor_loss') else "None", style={'border': '1px solid white'})
            ]),
            html.Tr([
                html.Td("Over Temperature", style={'border': '1px solid white'}),
                html.Td("Detected" if flags.get('high_temp') else "None", style={'border': '1px solid white'})
            ]),
            html.Tr([
                html.Td("System Health", style={'border': '1px solid white'}),
                html.Td("Healthy" if not any(flags.values()) else "Faulty", style={'border': '1px solid white'})
            ]),
        ]

        return table_header + table_body
    
    # Fallback to simulated data if fetch fails
    data = {
        'faults': {
            'sensor_loss': False,
            'over_voltage': False,
            'under_voltage': False,
            'over_current': False,
            'over_temperature': False
        },
        'system_health': True
    }
    
    table_header = [
        html.Tr([
            html.Th("Fault Type", style={'border': '1px solid white'}),
            html.Th("Status", style={'border': '1px solid white'})
        ])
    ]

    table_body = [
        html.Tr([
            html.Td("Sensor Loss", style={'border': '1px solid white'}),
            html.Td("Detected" if data['faults'].get('sensor_loss') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Over Voltage", style={'border': '1px solid white'}),
            html.Td("Detected" if data['faults'].get('over_voltage') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Under Voltage", style={'border': '1px solid white'}),
            html.Td("Detected" if data['faults'].get('under_voltage') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Over Current", style={'border': '1px solid white'}),
            html.Td("Detected" if data['faults'].get('over_current') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Over Temperature", style={'border': '1px solid white'}),
            html.Td("Detected" if data['faults'].get('over_temperature') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("System Health", style={'border': '1px solid white'}),
            html.Td("Healthy" if data.get('system_health') else "Faulty", style={'border': '1px solid white'})
        ]),
    ]

    return table_header + table_body

class BMSApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("BAT-MAN Interface")
        self.setGeometry(100, 100, 1200, 800)
        
        # Create the tab widget
        tabs = QTabWidget()
        self.setCentralWidget(tabs)
        
        # Create the Plotly tab
        plotly_tab = QWidget()
        plotly_layout = QVBoxLayout()
        web_view = QWebEngineView()
        web_view.setUrl(QUrl("http://127.0.0.1:8051"))
        plotly_layout.addWidget(web_view)
        plotly_tab.setLayout(plotly_layout)
        
        # Create the Settings tab
        settings_tab = QWidget()
        settings_layout = QVBoxLayout()
        
        # Battery Parameters Group
        battery_group = QGroupBox("Battery Parameters")
        battery_form = QFormLayout()
        
        self.max_voltage = QDoubleSpinBox()
        self.max_voltage.setRange(0, 5.0)
        self.max_voltage.setValue(4.2)
        self.max_voltage.setSingleStep(0.1)
        battery_form.addRow("Max Cell Voltage (V):", self.max_voltage)
        
        self.min_voltage = QDoubleSpinBox()
        self.min_voltage.setRange(0, 5.0)
        self.min_voltage.setValue(2.5)
        self.min_voltage.setSingleStep(0.1)
        battery_form.addRow("Min Cell Voltage (V):", self.min_voltage)
        
        self.max_temp = QSpinBox()
        self.max_temp.setRange(0, 100)
        self.max_temp.setValue(60)
        battery_form.addRow("Max Temperature (°C):", self.max_temp)
        
        self.capacity = QSpinBox()
        self.capacity.setRange(0, 1000)
        self.capacity.setValue(100)
        battery_form.addRow("Battery Capacity (Ah):", self.capacity)
        
        battery_group.setLayout(battery_form)
        
        # Protection Parameters Group
        protection_group = QGroupBox("Protection Parameters")
        protection_form = QFormLayout()
        
        self.overcurrent_limit = QDoubleSpinBox()
        self.overcurrent_limit.setRange(0, 500)
        self.overcurrent_limit.setValue(150)
        protection_form.addRow("Overcurrent Limit (A):", self.overcurrent_limit)
        
        self.temp_hysteresis = QDoubleSpinBox()
        self.temp_hysteresis.setRange(0, 10)
        self.temp_hysteresis.setValue(2)
        self.temp_hysteresis.setSingleStep(0.5)
        protection_form.addRow("Temperature Hysteresis (°C):", self.temp_hysteresis)
        
        protection_group.setLayout(protection_form)
        
        # Communication Settings Group
        comm_group = QGroupBox("Communication Settings")
        comm_form = QFormLayout()
        
        self.can_speed = QSpinBox()
        self.can_speed.setRange(100000, 1000000)
        self.can_speed.setValue(500000)
        self.can_speed.setSingleStep(100000)
        comm_form.addRow("CAN Bus Speed (bps):", self.can_speed)
        
        self.device_id = QSpinBox()
        self.device_id.setRange(1, 63)
        self.device_id.setValue(1)
        comm_form.addRow("Device ID:", self.device_id)
        
        comm_group.setLayout(comm_form)
        
        # Add groups to settings layout
        settings_layout.addWidget(battery_group)
        settings_layout.addWidget(protection_group)
        settings_layout.addWidget(comm_group)
        
        # Add save button
        save_button = QPushButton("Save Settings")
        save_button.clicked.connect(self.save_settings)
        settings_layout.addWidget(save_button)
        
        settings_tab.setLayout(settings_layout)
        
        # Add tabs to widget
        tabs.addTab(plotly_tab, "Dashboard")
        tabs.addTab(settings_tab, "Settings")
    
    def save_settings(self):
        print("Settings saved!")
        # You can access all settings via self.max_voltage.value(), self.max_temp.value(), etc.

def run_dash():
    dash_app.run_server(debug=False, use_reloader=False, port=8051)

def main():
    # Start Dash server in a separate thread
    dash_thread = threading.Thread(target=run_dash, daemon=True)
    dash_thread.start()
    
    # Give the Dash server a moment to start
    time.sleep(2)
    
    # Start Qt application
    app = QApplication(sys.argv)
    window = BMSApp()
    window.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main() 