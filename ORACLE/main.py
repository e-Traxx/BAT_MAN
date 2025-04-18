import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import requests

# URL to fetch data from
API_URL = "http://127.0.0.1:5000/data"  # Updated to local server

# Initialize Dash app
app = dash.Dash(__name__)
app.title = "BAT-MAN"

# App layout
app.layout = html.Div(
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

# Function to fetch data from the API
def fetch_data():
    try:
        response = requests.get(API_URL)
        response.raise_for_status()
        data = response.json()
        return data
    except Exception as e:
        print(f"Error fetching data: {e}")
        return {}

# Update the first bar graph
@app.callback(
    Output('live-graph1', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph1(n):
    data = fetch_data()
    x = list(range(140))
    y1 = data.get('graph1', [0] * 140)  # Fallback to zeros if no data
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

# Update the second bar graph
@app.callback(
    Output('live-graph2', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph2(n):
    data = fetch_data()
    x = list(range(140))
    y2 = data.get('graph2', [0] * 140)  # Fallback to zeros if no data
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

# Update current output
@app.callback(
    Output('current-output', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_current_output(n):
    data = fetch_data()
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

# Update overall voltage
@app.callback(
    Output('overall-voltage', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_overall_voltage(n):
    data = fetch_data()
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

# Update faults and system check table
@app.callback(
    Output('faults-system-table', 'children'),
    Input('interval-component', 'n_intervals')
)
def update_faults_system_table(n):
    data = fetch_data()
    faults = data.get('faults', {})
    system_health = data.get('system_health', False)

    table_header = [
        html.Tr([
            html.Th("Fault Type", style={'border': '1px solid white'}),
            html.Th("Status", style={'border': '1px solid white'})
        ])
    ]

    table_body = [
        html.Tr([
            html.Td("Sensor Loss", style={'border': '1px solid white'}),
            html.Td("Detected" if faults.get('sensor_loss') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Over Voltage", style={'border': '1px solid white'}),
            html.Td("Detected" if faults.get('over_voltage') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Under Voltage", style={'border': '1px solid white'}),
            html.Td("Detected" if faults.get('under_voltage') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Over Current", style={'border': '1px solid white'}),
            html.Td("Detected" if faults.get('over_current') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("Over Temperature", style={'border': '1px solid white'}),
            html.Td("Detected" if faults.get('over_temperature') else "None", style={'border': '1px solid white'})
        ]),
        html.Tr([
            html.Td("System Health", style={'border': '1px solid white'}),
            html.Td("Healthy" if system_health else "Faulty", style={'border': '1px solid white'})
        ]),
    ]

    return table_header + table_body

if __name__ == '__main__':
    app.run(debug=True)
