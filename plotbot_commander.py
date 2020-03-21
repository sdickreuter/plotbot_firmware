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


        self.togglemotors_button = QtWidgets.QPushButton("Enable")
        self.togglemotors_button.setEnabled(False)
        rightlayout.addWidget(self.togglemotors_button)


        self.home_button = QtWidgets.QPushButton("Home")
        self.home_button.setEnabled(False)
        rightlayout.addWidget(self.home_button)


        self.move_button = QtWidgets.QPushButton("Start Timers")
        self.move_button.setEnabled(False)
        rightlayout.addWidget(self.move_button)

        self.demo_button = QtWidgets.QPushButton("Demo")
        self.demo_button.setEnabled(False)
        rightlayout.addWidget(self.demo_button)


        rightlayout.addWidget(QtWidgets.QLabel("#Steps:"))   
        self.step_spin = QtWidgets.QSpinBox()
        self.step_spin.setRange(0, 3000)
        self.step_spin.setSingleStep(10)
        self.step_spin.setValue(500)
        rightlayout.addWidget(self.step_spin)   


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

        self.rhome_button = QtWidgets.QPushButton("Reverse Home")
        self.rhome_button.setEnabled(False)
        rightlayout.addWidget(self.rhome_button)

        self.readpos_button = QtWidgets.QPushButton("Read Positions")
        self.readpos_button.setEnabled(False)
        rightlayout.addWidget(self.readpos_button)


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

        self.demo_button.clicked.connect(self.demo)

        self.move_button.clicked.connect(self.move)
        self.togglemotors_button.clicked.connect(self.togglemotors)
        self.motors_enabled = False

        self.rhome_button.clicked.connect(self.rhome)
        self.readpos_button.clicked.connect(self.readpos)

    # Greets the user
    def connect(self):
        self.bot = None
        try:
            self.bot = pu.PlotBot()
            self.bot.clear()
            self.home_button.setEnabled(True)
            #self.demo_button.setEnabled(True)
            self.jog_a_left_button.setEnabled(True)
            self.jog_a_right_button.setEnabled(True)
            self.jog_b_left_button.setEnabled(True)
            self.jog_b_right_button.setEnabled(True)
            self.move_button.setEnabled(True)
            self.togglemotors_button.setEnabled(True)
            self.rhome_button.setEnabled(True)
            self.readpos_button.setEnabled(True)

            self.textbox.setPlainText("Connected to plotbot at "+self.bot.serial.port)
        except:
            self.textbox.setPlainText("Could not connect to plotbot")


    def togglemotors(self):
        if self.motors_enabled:
            self.bot.disable()
            self.motors_enabled = False
            self.togglemotors_button.setText("Enable")
        else:
            self.bot.enable()
            self.motors_enabled = True
            self.togglemotors_button.setText("Disable")


    def home(self):
        self.bot.clear()
        self.bot.home()
        self.motors_enabled = True
        self.togglemotors_button.setText("Disable")
        self.bot.zero()


    def rhome(self):
        self.bot.clear()
        self.motors_enabled = True
        self.togglemotors_button.setText("Disable")
        self.bot.home_reverse()


    def readpos(self):
        pos = self.bot.read_positions()
        self.textbox.setPlainText("pos "+ str(pos))


    def jog_a_right(self):
        self.jog(b"a", 1)

    def jog_a_left(self):
        self.jog(b"a", -1)

    def jog_b_right(self):
        self.jog(b"b", 1)

    def jog_b_left(self):
        self.jog(b"b", -1)


    def jog(self, axis, dir):
        self.bot.jog(axis,dir*self.step_spin.value())

    def move(self):
        self.bot.start_moving()
        self.demo_button.setEnabled(True)


    def demo(self):
        #xpos, ypos = self.bot.read_positions()

        size = 500

        count = 0
        tx = 0
        ty = 0
        while count < 59:

            #print(self.bot.read_positions())
            xlen, ylen = self.bot.read_bufferlength()
            #print(count,"buffer length",xlen,ylen)

            if xlen is not None:
                if xlen <= ylen:
                    if xlen < (3000-size):
                        timings = pu.generate_sine_movement(np.arange(start=tx,stop=tx+size))
                        #timings = pu.generate_sine_movement(np.arange(start=tx,stop=tx+size))
                        tx += size
                        self.bot.write_buffer(timings, np.repeat(1,len(timings)), b'a')
                        
                else:
                    if ylen < (3000-size):
                        timings = pu.generate_sine_movement(np.arange(start=ty,stop=ty+size),phase=np.pi)
                        #timings = pu.generate_sine_movement(np.arange(start=ty,stop=ty+size),phase=np.pi)
                        ty += size
                        self.bot.write_buffer(timings, np.repeat(1,len(timings)), b'b')
                        count += 1

            time.sleep(0.01)



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
