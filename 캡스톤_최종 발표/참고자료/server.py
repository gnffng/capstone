import serial
import time
import os

arrByte = b'\x00'

option = 1
growLevel = 1
growSpeed = 1

humidity = 999
onPump = 0

cds = 0
cdsLevel = 0

needHumidity = [65,80]
needCds = [730, 840, 890, 910, 920, 930, 940]

ser = serial.Serial(
    port='/dev/ttyUSB0',
    baudrate=9600
)

while True:
    #debug
    #os.system('clear')
    #print("humidity  cds  option  level  speed\n"+str(humidity)+"       "+str(c                                                                                        ds)+"      "+str(option)+"      "+str(growLevel)+"      "+str(growSpeed))
    #############################

    #Sencing
    if ser.readable():
        res = ser.read()
        #print("read : "+res.hex())
        try:
            #humidity Change
            if res == b'\xb1' :
                arrByte = ser.read()
                arrByte += ser.read()
                humidity = int.from_bytes(arrByte, byteorder="little", signed=Fa                                                                                        lse)
                print("humidity change : " + str(humidity)+"\n")
            #cds Change
            if res == b'\xb2' :
                arrByte = ser.read()
                arrByte += ser.read()
                cds = int.from_bytes(arrByte, byteorder="little", signed=False)
                #print("cds change : " + str(cds))



            #keyPad inturrupt
            elif res == b'\xb3' :
                res = ser.read()
                print("read : "+res.hex())

                arrByte = b'\xc3' + option.to_bytes(1, byteorder="big")
                #print("write : " + b'\xc3'.hex())
                #print("write : "+hex(option))

                if res == b'\x30' :
                    option = option % 2 + 1
                    print("option change : "+str(option))
                    arrByte += b'\x03'
                elif res == b'\x2a' :
                    arrByte += b'\x04'
                    #print("write : 0x04")
                    if option == 1 :
                        growLevel -= 1
                        if growLevel < 1 :
                            growLevel = 1
                    elif option == 2 :
                        growSpeed -= 1
                        if growSpeed < 1 :
                            growSpeed = 1
                elif res == b'\x23' :
                    arrByte += b'\x05'
                    #print("write : 0x05")
                    if option == 1 :
                        growLevel += 1
                        if growLevel > 3 :
                            growLevel = 3
                    elif option == 2 :
                        growSpeed += 1
                        if growSpeed > 2 :
                            growSpeed = 2

                ser.write(arrByte)
                print("level/speed : "+str(growLevel)+"/"+str(growSpeed))

        except Exception as e:
            print(e)
    #Control
    #Pump Control
    if humidity < needHumidity[0] :
        onPump = 1
    if humidity > needHumidity[1] :
        onPump = 0

    if onPump == 1 :
        ser.write(b'\xc1\x01')
        print("write : 0xC1 0X01")
        time.sleep(0.9)

    #LED Control
    if cds < needCds[(growLevel-1)*2+growSpeed-1] :
        if cdsLevel < 6 :
            ser.write(b'\xc2\x08')
            cdsLevel+=1
            #print("write : 0xC2 0x08")
            #print("cdsLevel : "+str(cdsLevel))
            time.sleep(0.01)
    elif cds > needCds[(growLevel-1)*2+growSpeed] :
        if cdsLevel == 1 :
            ser.write(b'\xc2\x06')
            cdsLevel-=1
            #print("write : 0xC2 0x06")
            #print("cdsLevel : "+str(cdsLevel))
            time.sleep(0.01)
        elif cdsLevel == 0:
            continue
        else :
            ser.write(b'\xc2\x07')
            cdsLevel-=1
            #print("write : 0xC2 0x07")
            #print("cdsLevel : "+str(cdsLevel))
            time.sleep(0.001)
