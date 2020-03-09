import time
import sys
import glob
import serial
import struct
from cobs import cobs

# from: https://stackoverflow.com/a/14224477
def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result


def read(ser):
    msg = ser.read_until(terminator=b'\x00')
    #print(cobs.decode(msg[:-1]), ' | ',msg)
    return cobs.decode(msg[:-1])


def write(ser, msg):
    msg = bytearray(cobs.encode(bytearray(msg)))
    msg.append(0x00)
    ser.write(msg)


if __name__ == '__main__':

    serialports = serial_ports()
    print(serialports)
    ser = serial.Serial(serialports[0], timeout=0.3)

    time.sleep(2) # allow some time for the Arduino to completely reset
    

    write(ser, b'e')


    write(ser, b'h')

    homed = False
    while not homed:
        msg = read(ser)
        if len(msg) > 1:
            if msg[0] == ord('r'):
                print(msg,"->", "homing finished")
                homed = True

    write(ser, b'f')

    count = 0
    while count < 200:
        reply = b''
        msg = b''

        msg = read(ser)
        if len(msg) > 1:
            #print(chr(msg[0]))
            if msg[0] == ord('r'):                
                #print('msg',chr(msg[0]),chr(msg[1])," size ",(msg[2]))
                axis = msg[1]
                size = msg[2]

                #print("r",msg[1],"size",size)
                reply += b'b'
                reply += bytes([axis])
                reply += b's'
                reply += bytes([size])                  
                dt = 1000
                for m in range(size):
                    reply += bytes(struct.pack("l", dt))

                #print('msg',reply)
                write(ser, reply)

                reply = b'l'
                reply += bytes([axis])
                write(ser, reply)

            elif msg[0] == ord("l"): 
                #print("len ",len(msg),"  ",msg, " ", ':'.join(hex(x)[2:] for x in msg))
                #print(msg[2:4].encode('hex'))
                print("length ",chr(msg[1])," ",struct.unpack('<l',msg[2:6])[0],chr(msg[6])," ",struct.unpack('<l',msg[7:11])[0])



        time.sleep(0.1)
        count += 1
        #print(count, ' ', link.status, " | rxBuff ",link.rxBuff[0:5])
        #if count % 10 == 0:
        #    print(count)

    time.sleep(1)

    write(ser, b's')

    write(ser, b'd')

    ser.close()
    