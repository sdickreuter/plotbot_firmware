import time
import sys
import glob
import serial
import struct
from cobs import cobs
import numpy as np
from scipy import signal


from PySide2 import QtWidgets, QtGui
import plotbot_utils as pu

class Form(QtWidgets.QDialog):

    def __init__(self, parent=None):
        super(Form, self).__init__(parent)
        
        rightlayout = QtWidgets.QVBoxLayout()

        self.connect_button = QtWidgets.QPushButton("Connect")
        rightlayout.addWidget(self.connect_button)


        self.step_edit = QtWidgets.QLineEdit("1000")
        onlyInt = QtGui.QIntValidator()
        self.step_edit.setValidator(onlyInt)
        rightlayout.addWidget(self.step_edit)   


        self.home_button = QtWidgets.QPushButton("Home")
        self.home_button.setEnabled(False)
        rightlayout.addWidget(self.home_button)

        self.demo_button = QtWidgets.QPushButton("Demo")
        self.demo_button.setEnabled(False)
        rightlayout.addWidget(self.demo_button)


        leftlayout = QtWidgets.QVBoxLayout()

        self.textbox = QtWidgets.QPlainTextEdit(self)
        #self.textbox.setEnabled(False)
        self.textbox.setReadOnly(True)
        self.textbox.resize(280,40)
        leftlayout.addWidget(self.textbox)


        layout = QtWidgets.QHBoxLayout()
        layout.addLayout(leftlayout)
        layout.addLayout(rightlayout)
        self.setLayout(layout)
        

        self.connect_button.clicked.connect(self.connect)
        self.home_button.clicked.connect(self.home)


        self.bot = None

    # Greets the user
    def connect(self):
        self.bot = None
        try:
            self.bot = pu.PlotBot()
            self.bot.clear()
            self.bot.enable()
            self.home_button.setEnabled(True)
            self.demo_button.setEnabled(True)
            self.textbox.setPlainText("Connected to plotbot at "+self.bot.serial.port)
        except:
            self.textbox.setPlainText("Could not connect to plotbot")


    def home(self):
        self.plotbot.home()


    def jog(axis):
        if axis == "a":
            timings = np.repeat(0.0002, 500)
            self.bot.write_buffer(timings, np.repeat(1,len(timings)), b'x')
        elif axis == "b":
            timings = np.repeat(0.0002, 500)
            self.bot.write_buffer(timings, np.repeat(1,len(timings)), b'y')



if __name__ == '__main__':
    # Create the Qt Application
    app = QtWidgets.QApplication(sys.argv)
    # Create and show the form
    form = Form()
    form.show()
    # Run the main Qt loop
    sys.exit(app.exec_())
