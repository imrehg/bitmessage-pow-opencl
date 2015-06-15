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
	me = os.path.abspath(os.path.dirname(__file__))
	lib = ctypes.cdll.LoadLibrary(os.path.join(me, "bmpow.so"))

	func = lib.proofOfWork
	func.restype = ctypes.c_ulonglong
	func.argtypes = [ctypes.c_ulonglong, ctypes.c_char_p]

	# Known results
	initialHash = "3758f55b5a8d902fd3597e4ce6a2d3f23daff735f65d9698c270987f4e67ad590b93f3ffeba0ef2fd08a8dc2f87b68ae5a0dc819ab57f22ad2c4c9c8618a43b3".decode("hex")
	target = 54227212183000L   # nonce:    270337
	# target = 5422721218300L    # nonce:   1980666
	# target = 542272121830L     # nonce:   6328179
	# target = 54227212183L      # nonce: 224121278

	start = time()
	nonce = func(target, initialHash)
	totalTimeCPP = time() - start
	print "C\n==========="
	print "Nonce: {}; time: {:2f}; speed: {:.2f} hash/s".format(nonce,
																totalTimeCPP,
																nonce/totalTimeCPP)
	result, = unpack('>Q',hashlib.sha512(hashlib.sha512(pack('>Q',nonce) + initialHash).digest()).digest()[0:8])
	assert result <= target, "nonce gives not good enough trial value!"
	print result, target

	# # Compare to speed to Python result
	# start = time()
	# nonce = _doFastPoW(target, initialHash)
	# totalTime = time() - start
	# print nonce
	# print "Python Multiprocessing\n==========="
	# print "Nonce: {}; time: {:2f}; speed: {:.2f} hash/s".format(nonce[1],
	# 															totalTime,
	# 															nonce[1]/totalTime)
