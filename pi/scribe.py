#!/usr/bin/python
#
# The Keg Scribe - Translating Beer into Data
#
# Dependencies:
#	requests (sudo pip install requests)
#
import RPi.GPIO as GPIO
import httplib
import threading
import csv
import requests
import time
from datetime import datetime
from flowmeter import *
from thermal_onewire import *

# constants
MS_IN_A_SECOND = 1000
MIN_POUR = 0.01

testUri = "http://demo.pndlm.com/"
uploadUri = "http://hakase.pndlm.com/import/simpleCSV"

fileWriteRate = 10 # append to CSV this often (s)
fileUploadRate = 10 # upload CSV this often (s)

csvFilename = "pour_values.csv"
csvColumns = ["ImportCode", "UtcTimestamp", "Value"]

importTapOneCode = "KegScribeTap1"
importTapTwoCode = "KegScribeTap2"
importTempOfficeCode = "KegScribeAmbientTemperature"
importTempKegCode = "KegScribeFridgeTemperature"

Tap1Pin = 17
Tap2Pin = 18

lastPinState = False
lastPinChange = 0
lastPour = 0
lastUpload = 0

# application state
tap1 = FlowMeter("metric", "beer")
tap2 = FlowMeter("metric", "beer")

def log(str):
	print "[{}] - {}".format(datetime.now().time(), str)

def testConnection():
	try:
		res = requests.get(testUri)
		return True
	except Exception, ex:
		log("Error: {}".format(ex))
	return False	

def hasChangesForUpload():
	return sum(1 for line in open(csvFilename)) > 1 # account for the header that is always present

def putCsvFile():
	try:
		uploadFile = {"file": ("ksup.csv", open(csvFilename, "r"), "test/csv")}
		res = requests.post(uploadUri, auth=requests.auth.HTTPBasicAuth("KEGSCRIBE", "KEGSCRIBE"), files=uploadFile)
		log(res.text)
		if res.status_code == requests.codes.ok:
			lastUpload = int(time.time())
			return True
		else:
			log("Upload failed with response: {}".format(res.text))
			return False
	except Exception, ex:
		log("Upload failed with exception: {}".format(ex))
	return False

def getSample(code, pourLiters):
	return {"ImportCode": code, "UtcTimestamp": int(time.time()), "Value": pourLiters}

def resetFile():
	# nuke and reset the file in place
	with open(csvFilename, "w") as csvfile:
		writer = csv.DictWriter(csvfile, csvColumns)
		writer.writeheader()

def setupPins():
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(Tap1Pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
	GPIO.setup(Tap2Pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)

def writePour(tapNumber, value):
	importCode = " "
	if tapNumber == 1:
		importCode = importTapOneCode
	elif tapNumber == 2:
		importCode = importTapTwoCode
	if tapNumber < 0 or tapNumber > 2: # TODO: Support tap2
		log("Invalid tap number")
		return
	with open(csvFilename, "a") as csvfile:
		writer = csv.DictWriter(csvfile, csvColumns) # DickWriter, huehuehue
		writer.writerow(getSample(importCode, value))
	if putCsvFile():
		resetFile()
		log("Upload successful!")
	else:
		log("Upload failed - retaining file")

def writeTemps(ambient, keg):
	with open(csvFilename, "a") as csvfile:
		writer = csv.DictWriter(csvfile, csvColumns)
		writer.writerow(getSample(importTempOfficeCode, ambient))
		writer.writerow(getSample(importTempKegCode, keg))

def hookIOEvents():
	GPIO.add_event_detect(Tap1Pin, GPIO.RISING, callback=updateTap1, bouncetime=20)
	GPIO.add_event_detect(Tap2Pin, GPIO.RISING, callback=updateTap2, bouncetime=20)

def updateTap1(channel):
	currentTime = int(time.time() * MS_IN_A_SECOND)
	if tap1.enabled == True:
		tap1.update(currentTime)

def updateTap2(channel):
	currentTime = int(time.time() * MS_IN_A_SECOND)
	if tap2.enabled == True:
		tap2.update(currentTime)

def enterWorker():
	log("Virtual alcohol moderator is serving, lol")
	lastTap1Pour = 0
	lastTap2Pour = 0
	lastThermalProbe = 0
	pourThreshold = 5000
	tempThreshold = 60000
	while True:
		currentTime = int(time.time()*MS_IN_A_SECOND)
		if tap1.thisPour > MIN_POUR and (currentTime - lastTap1Pour) > 5000:
			log("Tap 1 poured {} of beer".format(tap1.getFormattedThisPour()))
			writePour(1, tap1.thisPour)			
			lastTap1Pour = currentTime
			tap1.thisPour = 0.0
		if tap2.thisPour > MIN_POUR and (currentTime - lastTap2Pour) > 5000:
			log("Tap 2 poured {} of beer".format(tap2.getFormattedThisPour()))
			writePour(2, tap2.thisPour)
			lastTap2Pour = currentTime
			tap2.thisPour = 0.0
		if (currentTime - lastThermalProbe) > tempThreshold:
			ambientTmpF = get_external_temp()
			kegTmpF = get_kegerator_temp()
			log("Temperature probes: (Amb: {}, Keg: {})".format(str(ambientTmpF), str(kegTmpF)))
			writeTemps(ambientTmpF, kegTmpF)
			lastThermalProbe = currentTime

def main():
	print "KegScribe booting..."
	if not testConnection():
		log("OFFLINE -- I require network, FEED ME SEYMOUR!")
	else:
		log("ONLINE -- Turning the ignition key")
		resetFile() # establish file
		setupPins() # hook input pins
		hookIOEvents() # on edge events...
		enterWorker() # block the parent thread so the process doesn't exit

# entry point of script
if __name__ == "__main__":
	main()
