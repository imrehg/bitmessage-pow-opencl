#!/usr/bin/env python2
import os.path
import ctypes

if __name__ == "__main__":
	# bmpow = ctypes.CDLL("./bmpow.so")
	me = os.path.abspath(os.path.dirname(__file__))
	lib = ctypes.cdll.LoadLibrary(os.path.join(me, "bmpow.so"))
	# lib.main()
	# lib.pow()

	func = lib.pow
	func.restype = ctypes.c_int
	func.argtypes = [ctypes.c_int]
	data = func(0)

	print type(data), data
