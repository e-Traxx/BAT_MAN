from PyQt5.QtWidgets import QApplication, QMainWindow, QTabWidget, QWidget, QVBoxLayout, QPushButton, QLineEdit
from PyQt5.QtWebEngineWidgets import QWebEngineView
from PyQt5.QtCore import QUrl

app = QApplication([])

# Main window with tab widget
tabs = QTabWidget()

# Tab 1: embed Dash app in a web view
dash_view = QWebEngineView()
dash_view.load(QUrl("http://127.0.0.1:8050"))  # Dash app running locally [oai_citation_attribution:1‡stackoverflow.com](https://stackoverflow.com/questions/57063046/embedding-a-plotly-dash-plot-in-a-pyqt5-gui-handshake-failed-ssl-error#:~:text=import%20sys%20from%20PyQt5,QtWidgets%20import%20QApplication)
tabs.addTab(dash_view, "Dashboard")

# Tab 2: settings form
settings_page = QWidget()
form_layout = QVBoxLayout()
text_input = QLineEdit()        # example text field
slider = QLineEdit()            # (could use QSlider for a real slider)
save_btn = QPushButton("Save")
form_layout.addWidget(text_input)
form_layout.addWidget(slider)
form_layout.addWidget(save_btn)
settings_page.setLayout(form_layout)
tabs.addTab(settings_page, "Settings")

# Handle Save button click: send HTTP to ESP32
def send_settings():
    import requests
    value1 = text_input.text()
    value2 = slider.text()
    # Example HTTP POST to ESP32 (replace URL with actual ESP32 endpoint)
    requests.post("http://<ESP32_IP>/config", json={"param1": value1, "param2": value2})
    print("Settings sent to ESP32")
save_btn.clicked.connect(send_settings)

# Show window
window = QMainWindow()
window.setCentralWidget(tabs)
window.show()
app.exec_()
