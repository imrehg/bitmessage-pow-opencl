#!/usr/bin/env python2
import os.path
import ctypes


import hashlib
from struct import unpack, pack
from time import time
import sys

def _doSafePoW(target, initialHash):
	nonce = 0
	# nonce = 223121278
	trialValue = float('inf')
	while trialValue > target:
		nonce += 1
		if nonce % 1000000 == 0:
			print trialValue, nonce
		trialValue, = unpack('>Q',hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).digest()[0:8])
	return [trialValue, nonce]

def _set_idle():
    if 'linux' in sys.platform:
        import os
        os.nice(20)  # @UndefinedVariable
    else:
        try:
            sys.getwindowsversion()
            import win32api,win32process,win32con  # @UnresolvedImport
            pid = win32api.GetCurrentProcessId()
            handle = win32api.OpenProcess(win32con.PROCESS_ALL_ACCESS, True, pid)
            win32process.SetPriorityClass(handle, win32process.IDLE_PRIORITY_CLASS)
        except:
            #Windows 64-bit
            pass

def _pool_worker(nonce, initialHash, target, pool_size):
    _set_idle()
    trialValue = float('inf')
    while trialValue > target:
        nonce += pool_size
        trialValue, = unpack('>Q',hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).digest()[0:8])
    return [trialValue, nonce]

def _doFastPoW(target, initialHash):
    import time
    from multiprocessing import Pool, cpu_count
    try:
        pool_size = cpu_count()
    except:
        pool_size = 4
    try:
        maxCores = config.getint('bitmessagesettings', 'maxcores')
    except:
        maxCores = 99999
    if pool_size > maxCores:
        pool_size = maxCores
    pool = Pool(processes=pool_size)
    result = []
    for i in range(pool_size):
        result.append(pool.apply_async(_pool_worker, args = (i, initialHash, target, pool_size)))
    while True:
        for i in range(pool_size):
            if result[i].ready():
                result = result[i].get()
                pool.terminate()
                pool.join() #Wait for the workers to exit...
                return result[0], result[1]
        time.sleep(0.2)


if __name__ == "__main__":
	# # bmpow = ctypes.CDLL("./bmpow.so")
	# me = os.path.abspath(os.path.dirname(__file__))
	# lib = ctypes.cdll.LoadLibrary(os.path.join(me, "bmpow.so"))
	# # lib.main()
	# # lib.pow()

	# func = lib.pow
	# func.restype = ctypes.c_int
	# func.argtypes = [ctypes.c_int]
	# data = func(0)

	# print type(data), data

	##### Test 2

	# target = 542272121830L
	# initialHash = "3758f55b5a8d902fd3597e4ce6a2d3f23daff735f65d9698c270987f4e67ad590b93f3ffeba0ef2fd08a8dc2f87b68ae5a0dc819ab57f22ad2c4c9c8618a43b3".decode("hex")
	# # Target:  54227212183L
	# # Solution: nonce =	(50345506936, 224121278)
	# # Target:  542272121830L
	# # Solution: 6328179  (~15s on _doFastPoW)
	# start = time()
	# nonce = _doFastPoW(target, initialHash)
	# totalTime = time() - start
	# nonce = _doSafePoW(target, initialHash)
	# print "Nonce: {}; time: {:2f}; speed: {:.2f} hash/s".format(nonce[1],
	# 															totalTime,
	# 															nonce[1]/totalTime)



	initialHash = "3758f55b5a8d902fd3597e4ce6a2d3f23daff735f65d9698c270987f4e67ad590b93f3ffeba0ef2fd08a8dc2f87b68ae5a0dc819ab57f22ad2c4c9c8618a43b3".decode("hex")
	nonce = 1
	# print pack('>Q',nonce) + initialHash
	target, = unpack('>Q',hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).digest()[0:8])
	print "Nonce: ", nonce
	print "Bitmessage-POW-out: ", target

	# target = hashlib.sha512(initialHash).hexdigest()
	# print "No nonce: ", target

	# target = hashlib.sha512(pack('>Q',nonce)).hexdigest()
	# print "Only Nonce: ", target

	target = hashlib.sha512(pack('>Q',nonce) + initialHash).hexdigest()
	print "With Nonce-1: ", target

	target = hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).hexdigest()
	print "With Nonce-2: ", target


	me = os.path.abspath(os.path.dirname(__file__))
	lib = ctypes.cdll.LoadLibrary(os.path.join(me, "bmpow.so"))
	# # lib.main()
	# # lib.pow()

	# text = ctypes.c_char_p("hello world")
	# print "initialHash: {}".format(initialHash)
	text = initialHash
	func = lib.proofOfWork
	func.restype = ctypes.c_ulonglong
	func.argtypes = [ctypes.c_ulonglong, ctypes.c_char_p]
	# target = 54227212183000L   #    270337
	# target = 5422721218300L    #   1980666
	target = 542272121830L     #   6328179 , 5.6s
	# target = 54227212183L      # 224121278
	start = time()
	nonce = func(target, text)
	totalTimeCPP = time() - start
	print "Nonce---:", nonce
	print "C\n==========="
	print "Nonce: {}; time: {:2f}; speed: {:.2f} hash/s".format(nonce,
																totalTimeCPP,
																nonce/totalTimeCPP)
	result, = unpack('>Q',hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).digest()[0:8])
	assert result <= target, "nonce gives not good enough trial value!"
	print result, target

	# for nonce in range(0,4):
	# 	target, = unpack('>Q',hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).digest()[0:8])
	# 	print "Output: {}, Nonce: {}".format(nonce, target)


	# start = time()
	# nonce = _doFastPoW(target, initialHash)
	# totalTime = time() - start
	# # nonce = _doFastPoW(target, initialHash)
	# print nonce
	# print "Python Multiprocessing\n==========="
	# print "Nonce: {}; time: {:2f}; speed: {:.2f} hash/s".format(nonce[1],
	# 															totalTime,
	# 															nonce[1]/totalTime)
