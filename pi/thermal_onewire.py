#!/usr/bin/python

import os
import glob
import time
import subprocess

os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

base_dir = '/sys/bus/w1/devices/'
kegerator_descriptor = base_dir + '28-0000065df6c3' + '/w1_slave'
external_descriptor = base_dir + '28-0000065e3f25' + '/w1_slave'

def read_temp_raw(desc):
	catdata = subprocess.Popen(['cat', desc], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	out, err = catdata.communicate()
	out_decode = out.decode('utf-8')
	lines = out_decode.split('\n')
	return lines

def read_temp(desc):
	lines = read_temp_raw(desc)
	while lines[0].strip()[-3:] != 'YES':
		time.sleep(0.2)
		lines = read_temp_raw(desc)
	equals_pos = lines[1].find('t=')
	if equals_pos != -1:
		temp_string = lines[1][equals_pos+2:]
		temp_c = float(temp_string) / 1000.0
		temp_f = temp_c * 9.0 / 5.0 + 32.0
		return temp_c, temp_f

def get_external_temp():
	return read_temp(external_descriptor)[1]

def get_kegerator_temp():
	return read_temp(kegerator_descriptor)[1]
