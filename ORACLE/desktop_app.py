import dash
from dash import dcc, html
from dashboard_callbacks import register_dashboard_callbacks
from settings_callbacks import register_settings_callbacks

# Initialize Dash app
dash_app = dash.Dash(__name__)
dash_app.title = "BAT-MAN"

# App layout
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
        dcc.Tabs(
            id='tabs',
            value='dashboard-tab',
            style={
                'color': 'white',
                'backgroundColor': 'black'
            },
            children=[
                dcc.Tab(
                    label='Dashboard',
                    value='dashboard-tab',
                    style={'backgroundColor': 'black', 'color': 'white'},
                    selected_style={'backgroundColor': '#1f1f1f', 'color': 'white'},
                    children=[
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
                    ]
                ),
                dcc.Tab(
                    label='Settings',
                    value='settings-tab',
                    style={'backgroundColor': 'black', 'color': 'white'},
                    selected_style={'backgroundColor': '#1f1f1f', 'color': 'white'},
                    children=[
                        html.Div([
                            html.H3('Battery Management System Settings', style={'textAlign': 'center', 'marginBottom': '20px'}),
                            html.Div([
                                html.Label('Maximum Voltage (V)', style={'marginRight': '10px'}),
                                dcc.Input(id='max-voltage', type='number', value=4.2, style={'marginRight': '20px'}),
                                
                                html.Label('Minimum Voltage (V)', style={'marginRight': '10px'}),
                                dcc.Input(id='min-voltage', type='number', value=2.5, style={'marginRight': '20px'}),
                                
                                html.Label('Maximum Temperature (°C)', style={'marginRight': '10px'}),
                                dcc.Input(id='max-temp', type='number', value=60, style={'marginRight': '20px'}),
                            ], style={'marginBottom': '20px'}),
                            
                            html.Div([
                                html.Label('Battery Capacity (Ah)', style={'marginRight': '10px'}),
                                dcc.Input(id='capacity', type='number', value=100, style={'marginRight': '20px'}),
                                
                                html.Label('Overcurrent Limit (A)', style={'marginRight': '10px'}),
                                dcc.Input(id='overcurrent-limit', type='number', value=150, style={'marginRight': '20px'}),
                                
                                html.Label('Temperature Hysteresis (°C)', style={'marginRight': '10px'}),
                                dcc.Input(id='temp-hysteresis', type='number', value=2, style={'marginRight': '20px'}),
                            ], style={'marginBottom': '20px'}),
                            
                            html.Div([
                                html.Label('CAN Bus Speed (kbps)', style={'marginRight': '10px'}),
                                dcc.Input(id='can-speed', type='number', value=500, style={'marginRight': '20px'}),
                                
                                html.Label('Device ID', style={'marginRight': '10px'}),
                                dcc.Input(id='device-id', type='number', value=1, style={'marginRight': '20px'}),
                            ], style={'marginBottom': '20px'}),
                            
                            html.Button('Save Settings', id='save-settings', style={
                                'backgroundColor': '#4CAF50',
                                'color': 'white',
                                'padding': '10px 20px',
                                'border': 'none',
                                'borderRadius': '4px',
                                'cursor': 'pointer'
                            }),
                            
                            html.Div(id='save-status', style={'marginTop': '20px', 'color': 'white'})
                        ])
                    ]
                )
            ]
        ),
        dcc.Interval(
            id='interval-component',
            interval=2000,  # 2 seconds
            n_intervals=0
        ),
    ]
)

# Register callbacks
register_dashboard_callbacks(dash_app)
register_settings_callbacks(dash_app)

if __name__ == "__main__":
    dash_app.run_server(debug=False, port=8052)