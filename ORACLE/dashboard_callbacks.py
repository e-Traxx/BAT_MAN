from dash.dependencies import Input, Output
import plotly.graph_objs as go
from data_service import fetch_diagnostic_data, fetch_cell_data
from dash import html

def register_dashboard_callbacks(app):
    @app.callback(
        Output('live-graph1', 'figure'),
        Input('interval-component', 'n_intervals')
    )
    def update_graph1(n):
        cell_data = fetch_cell_data()
        if cell_data and 'cell_voltages' in cell_data:
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
        
        # Fallback to simulated data
        data = {'graph1': [2 + (i % 5) * 0.1 for i in range(140)]}
        x = list(range(140))
        y1 = data.get('graph1', [0] * 140)
        return {
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

    @app.callback(
        Output('live-graph2', 'figure'),
        Input('interval-component', 'n_intervals')
    )
    def update_graph2(n):
        cell_data = fetch_cell_data()
        if cell_data and 'cell_temperatures' in cell_data:
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
        
        # Fallback to simulated data
        data = {'graph2': [25 + (i % 10) for i in range(140)]}
        x = list(range(140))
        y2 = data.get('graph2', [0] * 140)
        return {
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

    @app.callback(
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
        
        # Fallback to simulated data
        data = {'current_output': 75 + (n % 20)}
        current = data.get('current_output', 0)
        return {
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

    @app.callback(
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
        
        # Fallback to simulated data
        data = {'overall_voltage': 3.6 + (n % 5) * 0.1}
        voltage = data.get('overall_voltage', 0)
        return {
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

    @app.callback(
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
        
        # Fallback to simulated data
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