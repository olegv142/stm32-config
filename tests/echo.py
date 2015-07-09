import serial
import random
import sys

letters = [chr(ord('A') + i) for i in range(30)]

def random_str():
	n, str = random.randrange(1, 1024), ''
	for i in range(n):
		str += random.choice(letters)
	return str

def test1(com):
	s = random_str()
	s += '\r'
	com.write(s)
	r = com.read(len(s))
	assert r == s

if __name__ == '__main__':
	n = 0
	port = sys.argv[1]
	com = serial.Serial(port)
	while True:
		test1(com)
		n += 1
		if not n % 100:
			print '.',

