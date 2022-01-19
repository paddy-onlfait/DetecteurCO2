# script python pour enregistrer les donnees du detecteur de CO2
# dans un fichier csv : log_CO2.csv
# voir tuto sur https://wikifab.org/wiki/DetecteurCO2
import serial
import time
import csv

ser = serial.Serial('/dev/ttyUSB0',9600)
ser.flushInput()
IsInterrupt = False


while True:
    try:
        ser_bytes = ser.readline()
        decoded_bytes = float(ser_bytes[0:len(ser_bytes)-2].decode("utf-8"))
        print(int(decoded_bytes))
        IsInterrupt=False
        if (int(decoded_bytes) > 350): 
			with open("log_CO2.csv","a") as f:
				writer = csv.writer(f,delimiter=",")
				writer.writerow([time.asctime(),int(decoded_bytes)])
    except:
		print("Keyboard Interrupt")
		if (IsInterrupt):
			break;
		else:
			IsInterrupt = True
        #break
