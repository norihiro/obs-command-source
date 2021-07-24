#! /usr/bin/python3

import asyncio
import json

loop = asyncio.get_event_loop()

async def connect_and_send(rr):
	# NOTE: User has to edit host, port, and password.
	try:
		import simpleobsws
	except ModuleNotFoundError:
		import sys
		sys.stderr.write('Error: This script requires a module simpleobsws (https://github.com/IRLToolkit/simpleobsws).\n')
		sys.exit(1)
	obsws_port = 4444
	try:
		import os
		obsws_port = int(os.environ['OBSWS_PORT'])
	except:
		pass
	ws = simpleobsws.obsws(host='127.0.0.1', port=obsws_port, password=None, loop=loop)
	await ws.connect()
	for req, data in rr:
		print('req="%s" data=%s'%(str(req), str(data)))
		if req=='sleep':
			await asyncio.sleep(data)
		else:
			res = await ws.call(req, data)
			print(json.dumps(res, indent="\t"))
	await ws.disconnect()

help_str='''NAME
	request-websocket.py - a helper command to send requests to OBS Studio

SYNOPSIS
	request-websocket.py [request_type data] ...
	request-websocket.py --help

DESCRIPTION
	This script will take pairs of request_type and data and send each pair to OBS Studio.
	The last pair can skip data if you don't need to send any data.

	request_type
		This is a request-type supported by obs-websocket protocol or 'sleep'.
		If request_type is 'sleep', data has to be a non-negative number to sleep in seconds.
		Otherwise, the request-type will be directly send to obs-websocket.
	data
		The data is a JSON text. The data can be an empty string if you don't need to send any data.

EXAMPLES
	request-websocket.py GetVersion
	request-websocket.py SetPreviewScene '{"scene-name": "Scene"}'
	request-websocket.py SetCurrentScene '{"scene-name": "Scene1"}' sleep 10 SetCurrentScene '{"scene-name": "Scene2"}'
'''

def main():
	import sys
	req = None
	rr = list()
	st = 0
	for a in sys.argv[1:]:
		if a=='--help':
			print(help_str)
			return 0
		if st==0:
			req = a
			st = 1
		elif st==1:
			if len(a):
				data = json.loads(a)
			else:
				data = None
			rr.append((req, data))
			st = 0
	if st==1:
		rr.append((req, None))
	loop.run_until_complete(connect_and_send(rr))

if __name__ == '__main__':
	main()
