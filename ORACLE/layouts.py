from dash import html

def create_fault_table():
    """Helper function to create the fault and system check table"""
    return html.Table(
        id='faults-system-table',
        style={
            'width': '100%',
            'margin': '0 auto',
            'color': 'white',
            'border': '1px solid white',
            'textAlign': 'center'
        }
    )