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


        aposlayout = QtWidgets.QHBoxLayout()
        aposlayout.addWidget(QtWidgets.QLabel("a pos"))
        self.a_spin = QtWidgets.QSpinBox()
        self.a_spin.setRange(0, 45000)
        self.a_spin.setSingleStep(100)
        self.a_spin.setValue(1000)
        aposlayout.addWidget(self.a_spin)
        rightlayout.addLayout(aposlayout)

        bposlayout = QtWidgets.QHBoxLayout()
        bposlayout.addWidget(QtWidgets.QLabel("b pos"))
        self.b_spin = QtWidgets.QSpinBox()
        self.b_spin.setRange(0, 45000)
        self.b_spin.setSingleStep(100)
        self.b_spin.setValue(1000)
        bposlayout.addWidget(self.b_spin)
        rightlayout.addLayout(bposlayout)


        self.moveto_button = QtWidgets.QPushButton("Move to")
        self.moveto_button.setEnabled(False)
        rightlayout.addWidget(self.moveto_button)


        self.manualpos_button = QtWidgets.QPushButton("Get Manual Pos")
        self.manualpos_button.setEnabled(False)
        rightlayout.addWidget(self.manualpos_button)


        servo_layout = QtWidgets.QHBoxLayout()
        self.servo_button = QtWidgets.QPushButton("Set Servo")
        self.servo_button.setEnabled(False)
        servo_layout.addWidget(self.servo_button)
        self.servo_spin = QtWidgets.QSpinBox()
        self.servo_spin.setRange(0, 180)
        self.servo_spin.setSingleStep(10)
        self.servo_spin.setValue(90)
        servo_layout.addWidget(self.servo_spin)
        rightlayout.addLayout(servo_layout)


        pen_layout = QtWidgets.QHBoxLayout()
        pen_layout.addWidget(QtWidgets.QLabel("Pen "))
        self.up_button = QtWidgets.QPushButton("Up")
        self.up_button.setEnabled(False)
        pen_layout.addWidget(self.up_button)
        self.down_button = QtWidgets.QPushButton("Down")
        self.down_button.setEnabled(False)
        pen_layout.addWidget(self.down_button)
        rightlayout.addLayout(pen_layout)


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

        self.moveto_button.clicked.connect(self.moveto)

        self.manualpos_button.clicked.connect(self.get_manual_position)
        
        self.servo_button.clicked.connect(self.set_servo)



    # Greets the user
    def connect(self):
        self.bot = None
        try:
            self.bot = pu.PlotBot()
            self.bot.clear()

            self.up_button.clicked.connect(self.bot.pen_up)
            self.down_button.clicked.connect(self.bot.pen_down)

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
            self.moveto_button.setEnabled(True)
            self.servo_button.setEnabled(True)
            self.up_button.setEnabled(True)
            self.down_button.setEnabled(True)

            self.textbox.appendPlainText("Connected to plotbot at "+self.bot.serial.port)
        except:
            self.textbox.appendPlainText("Could not connect to plotbot")


    def togglemotors(self):
        if self.motors_enabled:
            self.bot.disable()
            self.motors_enabled = False
            self.togglemotors_button.setText("Enable")
        else:
            self.bot.enable()
            self.motors_enabled = True
            self.togglemotors_button.setText("Disable")


    def set_servo(self):
        self.bot.set_servo(self.servo_spin.value())

    def get_manual_position(self):
        self.bot.clear()
        #self.bot.home()
        #self.bot.zero()
        self.bot.disable()        

        msgBox = QtWidgets.QMessageBox()
        msgBox.setText('Manually position pen and press ok when finished')
        msgBox.exec();

        self.bot.home()

        pos = self.bot.read_positions()
        print("manual pos: ",pos)
        self.textbox.appendPlainText("manual pos: " + str(pos))
        
        self.bot.zero()
        #return pos


    def home(self):
        self.bot.clear()
        self.bot.home()
        self.bot.zero()
        self.motors_enabled = True
        self.togglemotors_button.setText("Disable")
        self.manualpos_button.setEnabled(True)


    def rhome(self):
        self.bot.clear()
        self.motors_enabled = True
        self.togglemotors_button.setText("Disable")
        self.bot.home_reverse()


    def readpos(self):
        pos = self.bot.read_positions()
        self.textbox.appendPlainText("pos "+ str(pos))


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


    def moveto(self):
        a = self.a_spin.value()
        b = self.b_spin.value()
        self.textbox.appendPlainText("moveto "+str(a)+" "+str(b))

        self.bot.clear()
        self.move()

        apos, bpos = self.bot.read_positions()
        asteps = a - apos
        bsteps = b - bpos

        adir = np.sign(asteps)
        bdir = np.sign(bsteps)

        asteps = np.abs(asteps)
        bsteps = np.abs(bsteps)

        if asteps > bsteps:
            dta = np.repeat(0.0005, asteps)    
            dtb = np.repeat( asteps*0.0005/ bsteps, bsteps)
        else:
            dta = np.repeat( bsteps*0.0005/ asteps, asteps)
            dtb = np.repeat(0.0005, bsteps)

        dta[dta < 0.0005] += 0.0005
        dtb[dtb < 0.0005] += 0.0005

        dta *= adir
        dtb *= bdir

        if len(dta) > 0:
            a_finished = False
        else:
            a_finished = True
        if len(dtb) > 0:
            b_finished = False
        else:
            b_finished = True

        size = 500

        a_ind = 0
        b_ind = 0
        while not (a_finished and b_finished):
            
            alen, blen = self.bot.read_bufferlength()
            #print(not (a_finished and b_finished), alen, blen)

            if alen is not None:
                if (((alen <= blen) and (not a_finished)) or b_finished):
                    if alen < (3000-size):
                        send = dta[a_ind:a_ind+size]
                        if len(send) > 0:
                            self.bot.write_buffer(send, np.repeat(1,len(send)), b'a')
                            a_ind += size
                        else:
                            a_finished = True  

                elif not b_finished:
                    if blen < (3000-size):
                        send = dtb[b_ind:b_ind+size]
                        if len(send) > 0:
                            self.bot.write_buffer(send, np.repeat(1,len(send)), b'b')
                            b_ind += size
                        else:
                            b_finished = True  


            time.sleep(0.01)


    def demo(self):
        #xpos, ypos = self.bot.read_positions()

        size = 500

        count = 0
        ta = 0
        tb = 0
        while count < 49:

            #print(self.bot.read_positions())
            alen, blen = self.bot.read_bufferlength()
            #print(count,"buffer length",xlen,ylen)

            if alen is not None:
                if alen <= blen:
                    if alen < (3000-size):
                        timings = pu.generate_sine_movement(np.arange(start=ta,stop=ta+size))
                        #timings = pu.generate_sine_movement(np.arange(start=tx,stop=tx+size))
                        ta += size
                        self.bot.write_buffer(timings, np.repeat(1,len(timings)), b'a')
                        
                else:
                    if blen < (3000-size):
                        timings = pu.generate_sine_movement(np.arange(start=tb,stop=tb+size),phase=np.pi)
                        #timings = pu.generate_sine_movement(np.arange(start=ty,stop=ty+size),phase=np.pi)
                        tb += size
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
