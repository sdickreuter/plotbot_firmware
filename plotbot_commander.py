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
        
        self.bot = None

        rightlayout = QtWidgets.QVBoxLayout()

        self.connect_button = QtWidgets.QPushButton("Connect")
        rightlayout.addWidget(self.connect_button)


        self.step_edit = QtWidgets.QLineEdit("500")
        onlyInt = QtGui.QIntValidator()
        self.step_edit.setValidator(onlyInt)
        rightlayout.addWidget(self.step_edit)   


        self.home_button = QtWidgets.QPushButton("Home")
        self.home_button.setEnabled(False)
        rightlayout.addWidget(self.home_button)

        self.demo_button = QtWidgets.QPushButton("Demo")
        self.demo_button.setEnabled(False)
        rightlayout.addWidget(self.demo_button)


        jog_a_layout = QtWidgets.QHBoxLayout()
        jog_a_layout.addWidget(QtWidgets.QLabel("a "))
        self.jog_a_left_button = QtWidgets.QPushButton("<-")
        self.jog_a_left_button.setEnabled(False)
        jog_a_layout.addWidget(self.jog_a_left_button)
        self.jog_a_right_button = QtWidgets.QPushButton("->")
        self.jog_a_right_button.setEnabled(False)
        jog_a_layout.addWidget(self.jog_a_right_button)
        rightlayout.addLayout(jog_a_layout)

        jog_b_layout = QtWidgets.QHBoxLayout()
        jog_b_layout.addWidget(QtWidgets.QLabel("b "))
        self.jog_b_left_button = QtWidgets.QPushButton("<-")
        self.jog_b_left_button.setEnabled(False)
        jog_b_layout.addWidget(self.jog_b_left_button)
        self.jog_b_right_button = QtWidgets.QPushButton("->")
        self.jog_b_right_button.setEnabled(False)
        jog_b_layout.addWidget(self.jog_b_right_button)
        rightlayout.addLayout(jog_b_layout)


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

        self.jog_a_right_button.clicked.connect(self.jog_a_right)
        self.jog_a_left_button.clicked.connect(self.jog_a_left)
        self.jog_b_right_button.clicked.connect(self.jog_b_right)
        self.jog_b_left_button.clicked.connect(self.jog_b_left)


    # Greets the user
    def connect(self):
        self.bot = None
        try:
            self.bot = pu.PlotBot()
            self.bot.clear()
            self.bot.enable()
            self.home_button.setEnabled(True)
            self.demo_button.setEnabled(True)
            self.jog_a_left_button.setEnabled(True)
            self.jog_a_right_button.setEnabled(True)
            self.jog_b_left_button.setEnabled(True)
            self.jog_b_right_button.setEnabled(True)

            self.textbox.setPlainText("Connected to plotbot at "+self.bot.serial.port)
        except:
            self.textbox.setPlainText("Could not connect to plotbot")


    def home(self):
        self.bot.clear()
        self.bot.home()

    def jog_a_right(self):
        self.jog("a", 1)

    def jog_a_left(self):
        self.jog("a", -1)

    def jog_b_right(self):
        self.jog("b", 1)

    def jog_b_left(self):
        self.jog("b", -1)


    def jog(self, axis, dir):
        self.bot.clear()
        #lena, lenb = self.bot.read_bufferlength()
        #if ((axis == "a" and lena < 2000) or (axis == "b" and lenb < 2000)) :
        timings = np.repeat(dir*0.0002, int(self.step_edit.text()))
        self.bot.write_buffer(timings, np.repeat(1,len(timings)), bytes(axis.encode()))
        self.textbox.setPlainText("buffer lengths "+ str(self.bot.read_bufferlength()))
        self.bot.start_moving()
            #time.sleep(0.1) # VERY IMPORTANT! without buffer on the teensy will overflow
        #else:
        #    self.textbox.setPlainText("buffer is full")

    def __del__(self):
        if self.bot is not None:
            self.bot.disable()


if __name__ == '__main__':
    # Create the Qt Application
    app = QtWidgets.QApplication(sys.argv)
    # Create and show the form
    form = Form()
    form.show()
    # Run the main Qt loop
    sys.exit(app.exec_())
