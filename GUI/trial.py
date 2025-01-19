import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go
import numpy as np
import time

# Initialize Dash app
app = dash.Dash(__name__)
app.title = "Live Bar Graphs"

# App layout
app.layout = html.Div(children=[
    html.H1("Live Bar Graphs", style={'textAlign': 'center'}),
    
    dcc.Graph(id='live-graph1'),
    dcc.Graph(id='live-graph2'),
    
    # Interval component for updating graphs
    dcc.Interval(
        id='interval-component',
        interval=1000,  # in milliseconds (1 second)
        n_intervals=0
    )
])

# Update the first bar graph
@app.callback(
    Output('live-graph1', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph1(n):
    x = np.arange(140)
    y = np.random.randint(1, 100, 140)
    figure = {
        'data': [go.Bar(x=x, y=y, marker_color='blue')],
        'layout': go.Layout(
            title='Live Bar Graph 1',
            xaxis={'title': 'Index'},
            yaxis={'title': 'Value'},
            yaxis_range=[0, 100]
        )
    }
    return figure

# Update the second bar graph
@app.callback(
    Output('live-graph2', 'figure'),
    Input('interval-component', 'n_intervals')
)
def update_graph2(n):
    x = np.arange(140)
    y = np.random.randint(1, 100, 140)
    figure = {
        'data': [go.Bar(x=x, y=y, marker_color='green')],
        'layout': go.Layout(
            title='Live Bar Graph 2',
            xaxis={'title': 'Index'},
            yaxis={'title': 'Value'},
            yaxis_range=[0, 100]
        )
    }
    return figure

if __name__ == '__main__':
    app.run_server(debug=True)
